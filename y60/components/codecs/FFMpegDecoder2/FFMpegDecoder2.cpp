/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below. 
//    
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#include "FFMpegDecoder2.h"
#include "FFMpegURLProtocol.h"
#include "Demux.h"

#include <y60/video/Movie.h>
#include <y60/sound/SoundManager.h>
#include <asl/base/Ptr.h>
#include <asl/base/Auto.h>
#include <asl/audio/Pump.h>
#include <asl/base/Logger.h>
#include <asl/base/file_functions.h>
#include <asl/base/string_functions.h>

// remove ffmpeg macros
#ifdef START_TIMER
#   undef START_TIMER
#endif
#ifdef STOP_TIMER
#   undef STOP_TIMER
#endif



#include <asl/base/Dashboard.h>
#include <iostream>
#include <stdlib.h>

#define DB(x) //x 

using namespace std;
using namespace asl;

extern "C"
EXPORT asl::PlugInBase * FFMpegDecoder2_instantiatePlugIn(asl::DLHandle myDLHandle) {
	return new y60::FFMpegDecoder2(myDLHandle);
}

namespace y60 {


    const double FFMpegDecoder2::AUDIO_BUFFER_SIZE = 0.5;

    asl::Block FFMpegDecoder2::_myResampledSamples(AVCODEC_MAX_AUDIO_FRAME_SIZE*10);

    FFMpegDecoder2::FFMpegDecoder2(asl::DLHandle theDLHandle) :
        AsyncDecoder(),
        PosixThread(),
        PlugInBase(theDLHandle),
        _myLastVideoFrame(),
        _myAVCodecLock(),
        _myFormatContext(0),
        _myVStreamIndex(-1),
        _myVStream(0),
        _myAStreamIndex(-1),
        _myAStream(0),
        _myMsgQueue(),
        _myFrame(0),
        _myDemux(),
        _myStartTimestamp(-1),
        _myDestinationPixelFormat(PIX_FMT_BGR24),
        _myResampleContext(0),
        _myNumFramesDecoded(0),
        _myNumIFramesDecoded(0),
        _myDecodedPacketsPerFrame(0),
        _myTimeUnitsPerSecond(-1),
        _myFrameRate(25),
        _myMaxCacheSize(8),
        _myFrameWidth(0),
        _myFrameHeight(0),
        _myBytesPerPixel(1),
        _myLastFrameTime(0),
        _hasShutDown(false)
    {}

    FFMpegDecoder2::~FFMpegDecoder2() {
        DB(AC_DEBUG << "FFMpegDecoder2::~FFMpegDecoder2()");
        shutdown();
    }
    void FFMpegDecoder2::shutdown() {
        if (!_hasShutDown) {
           DB(AC_DEBUG << "FFMpegDecoder2::shutdown()");
           closeMovie();
            if (_myResampleContext) {
                audio_resample_close(_myResampleContext);
                _myResampleContext = 0;
            }
        }
        _hasShutDown = true;
    }
    asl::Ptr<MovieDecoderBase> FFMpegDecoder2::instance() const {
        return asl::Ptr<MovieDecoderBase>(new FFMpegDecoder2(getDLHandle()));
    }

    std::string
    FFMpegDecoder2::canDecode(const std::string & theUrl, asl::Ptr<asl::ReadableStreamHandle> theStream) {
        if (asl::toLowerCase(asl::getExtension(theUrl)) == "mpg" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "m2v" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "avi" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mp4" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mkv" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mov" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mpeg") {
            AC_TRACE << "FFMpegDecoder2 can decode :" << theUrl << endl;
            return MIME_TYPE_MPG;
        } else {
            AC_TRACE << "FFMpegDecoder2 can not decode :" << theUrl <<
                    "responsible for extensions mpg and m2v" << endl;
            return "";
        }
    }

    void
    FFMpegDecoder2::load(asl::Ptr<ReadableStream> theSource, const string & theFilename) {
        string theStreamID = string("acstream://") + theFilename;
        registerStream(theStreamID, theSource);
        load(theStreamID);
    }

    void
    FFMpegDecoder2::load(const std::string & theFilename) {
        AC_INFO << "FFMpegDecoder2::load(" << theFilename << ")";
        // register all formats and codecs
        static bool avRegistered = false;
        if (!avRegistered) {
            AC_INFO << "FFMpegDecoder2::load " << LIBAVCODEC_IDENT;
            av_log_set_level(AV_LOG_ERROR);
            av_register_all();
            avRegistered = true;
        }

        if (av_open_input_file(&_myFormatContext, theFilename.c_str(), 0, FFM_PACKET_SIZE, 0)
                < 0) {
            throw FFMpegDecoder2Exception(std::string("Unable to open input file: ")
                    + theFilename, PLUS_FILE_LINE);
        }

        if (av_find_stream_info(_myFormatContext) < 0) {
            throw FFMpegDecoder2Exception(std::string("Unable to find stream info: ")
                    + theFilename, PLUS_FILE_LINE);
        }
        // for debugging you can let ffmpeg tell detail about the movie
            //char myString[200];
            //dump_format(_myFormatContext, 0, myString, 0);
        
        // find video/audio streams
        for (unsigned i = 0; i < static_cast<unsigned>(_myFormatContext->nb_streams); ++i) {
            int myCodecType =  _myFormatContext->streams[i]->codec->codec_type;
            if (_myVStreamIndex == -1 && myCodecType == CODEC_TYPE_VIDEO) {
                _myVStreamIndex = i;
                _myVStream = _myFormatContext->streams[i];
            }
            else if (_myAStreamIndex == -1 && myCodecType == CODEC_TYPE_AUDIO) {
                _myAStreamIndex = i;
                _myAStream = _myFormatContext->streams[i];
            }
        }
        
        if (_myVStream) {
            setupVideo(theFilename);
        } else {
            AC_INFO << "FFMpegDecoder2::load " << theFilename << " no video stream found";
            _myVStream = 0;
            _myVStreamIndex = -1;
        }
        if (_myAStream && getAudioFlag()) {
            setupAudio(theFilename);
        } else {
            AC_DEBUG << "FFMpegDecoder2::load " << theFilename 
                    << " no audio stream found or disabled";
            _myAudioSink = HWSampleSinkPtr();
            _myAStream = 0;
            _myAStreamIndex = -1;
        }
        _myDemux = asl::Ptr<Demux>(new Demux(_myFormatContext));
        if (_myVStreamIndex != -1) {
            _myDemux->enableStream(_myVStreamIndex);
        }
        if (_myAStreamIndex != -1) {
            _myDemux->enableStream(_myAStreamIndex);
        }
    }

    void FFMpegDecoder2::startOverAgain() {
        AC_DEBUG <<"FFMpegDecoder2::startOverAgain";
        
        if (isUnjoined()) {
            DB(AC_DEBUG << "Joining FFMpegDecoder Thread");
            join();
        }
        doSeek(-1);
        setState(RUN);
        
        if (!isActive()) {
            decodeFrame();
        }
        if (!isUnjoined()) {
            AC_DEBUG << "Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_DEBUG << "Thread already running. No forking.";
        }
    }

    void FFMpegDecoder2::startMovie(double theStartTime, bool theStartAudioFlag) {
        AC_INFO << "FFMpegDecoder2::startMovie, time: " << theStartTime;
        Movie * myMovie = getMovie();
        _myMaxCacheSize = myMovie->get<MaxCacheSizeTag>();
        _myDecodedPacketsPerFrame = 0; // reset counter 
        _myLastVideoFrame = VideoMsgPtr();

        if (theStartAudioFlag && _myAudioSink && getDecodeAudioFlag()) {            
            _myAudioSink->stop();
        } 
        
        if (!isActive()) {
            decodeFrame();
        }
        if (hasAudio() && getDecodeAudioFlag()) {
            readAudio();
        }

        setState(RUN);
        if (!isUnjoined()) {
            AC_DEBUG << "Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_DEBUG << "Thread already running. No forking.";
        }
        AsyncDecoder::startMovie(theStartTime, theStartAudioFlag);
    }

    void FFMpegDecoder2::resumeMovie(double theStartTime, bool theResumeAudioFlag) {
        AC_INFO << "FFMpegDecoder2::resumeMovie, time: " << theStartTime;
        Movie * myMovie = getMovie();
        _myMaxCacheSize = myMovie->get<MaxCacheSizeTag>();
        _myLastVideoFrame = VideoMsgPtr();
        setState(RUN);
        if (!isUnjoined()) {
            AC_DEBUG << "Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_DEBUG << "Thread already running. No forking.";
        }
        AsyncDecoder::resumeMovie(theStartTime, theResumeAudioFlag);
    }

    void FFMpegDecoder2::stopMovie(bool theStopAudioFlag) {
        AC_DEBUG << "FFMpegDecoder2::stopMovie";
        
        if (isUnjoined()) {
            DB(AC_DEBUG << "Joining FFMpegDecoder Thread");
            join();
        }
        if (getState() != STOP) {
            AC_DEBUG << "Stopping Movie";
            
            _myDecodedPacketsPerFrame = 0; // reset counter    
            _myLastVideoFrame = VideoMsgPtr();
            
            // seek to start (-1 indicates doSeek() to do so)
            // TODO: only do this on loop?
            doSeek(-1);
            Movie * myMovie = getMovie();
            myMovie->set<CurrentFrameTag>(0);
            
            _myDemux->clearPacketCache();
            _myMsgQueue.clear();
            _myMsgQueue.reset();
            dumpCache();
            AsyncDecoder::stopMovie(theStopAudioFlag);
        }
    }

    void
    FFMpegDecoder2::closeMovie() {
        AC_DEBUG << "FFMpegDecoder2::closeMovie";
        // stop thread
        stopMovie();

        AutoLocker<ThreadLock> myLocker(_myAVCodecLock);
        // codecs
        if (_myVStream) {
            avcodec_close(_myVStream->codec);
            _myVStreamIndex = -1;
            _myVStream = 0;
        }
        if (_myFrame) {
            av_free(_myFrame);
            _myFrame = 0;
        }
        if (_myAStream) {
            avcodec_close(_myAStream->codec);
            _myAStreamIndex = -1;
            _myAStream = 0;
        }
        if (_myFormatContext) {
            av_close_input_file(_myFormatContext);
            _myFormatContext = 0;
        }
        AsyncDecoder::closeMovie();
    }

    
    bool FFMpegDecoder2::readAudio() {
        AC_TRACE << "---- FFMpegDecoder2::readAudio:";
        double myDestBufferedTime = _myAudioSink->getBufferedTime()+8/_myFrameRate;
        if (myDestBufferedTime > AUDIO_BUFFER_SIZE) {
            myDestBufferedTime = AUDIO_BUFFER_SIZE;
        }
        while (double(_myAudioSink->getBufferedTime()) < myDestBufferedTime) {
            AC_DEBUG << "---- FFMpegDecoder2::readAudio: getBufferedTime="
                    << _myAudioSink->getBufferedTime();
            AVPacket * myPacket = _myDemux->getPacket(_myAStreamIndex);
            if (!myPacket) {
                _myAudioSink->stop(true);
                AC_DEBUG << "---- FFMpegDecoder::readAudio(): eof" << endl;
                return false;
            } 
            addAudioPacket(*myPacket);
            av_free_packet(myPacket);
            delete myPacket;
        }
        return true;
    }

    void FFMpegDecoder2::addAudioPacket(const AVPacket & thePacket) {
        AC_TRACE << "FFMpegDecoder2::addAudioPacket()";
        
        double myTime = thePacket.dts / _myTimeUnitsPerSecond;
        
        // we need an aligned buffer
        int16_t * myAlignedBuf;
        myAlignedBuf = (int16_t *)av_malloc( AVCODEC_MAX_AUDIO_FRAME_SIZE *10 );
        
        int myBytesDecoded = 0; // decompressed sample size in bytes
                 
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52,27,0)
        AVPacket myTempPacket;
        av_init_packet(&myTempPacket);
        myTempPacket.data = thePacket.data;
        myTempPacket.size = thePacket.size;
        while (myTempPacket.size > 0) {
            myBytesDecoded = AVCODEC_MAX_AUDIO_FRAME_SIZE *10; 
            int myLen = avcodec_decode_audio3(_myAStream->codec,
                myAlignedBuf, &myBytesDecoded, &myTempPacket);
            if (myLen < 0) {
                AC_WARNING << "av_decode_audio error";
                myTempPacket.size = 0;
                break;
            }
            myTempPacket.data += myLen;
            myTempPacket.size -= myLen;
            AC_TRACE << "data left " << myTempPacket.size << " read " << myLen;
            
#else
        const uint8_t* myData = thePacket.data;
        int myDataLen = thePacket.size;
        while (myDataLen > 0) {
#   if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(51,28,0)            
            myBytesDecoded = AVCODEC_MAX_AUDIO_FRAME_SIZE *10; 
            int myLen = avcodec_decode_audio2(_myAStream->codec,
                myAlignedBuf, &myBytesDecoded, myData, myDataLen);
#   else            
            int myLen = avcodec_decode_audio(_myAStream->codec,
                myAlignedBuf, &myBytesDecoded, myData, myDataLen);    
#   endif
            if (myLen < 0) {
                AC_WARNING << "av_decode_audio error";
                myDataLen = 0;
                break;
            }
            myData += myLen;
            myDataLen -= myLen;
            AC_TRACE << "data left " << myDataLen << " read " << myLen;
            
#endif
            if ( myBytesDecoded <= 0 ) {
                continue;
            }
            int myNumChannels = _myAStream->codec->channels;
            int numFrames = myBytesDecoded/(getBytesPerSample(SF_S16)*myNumChannels);
            AC_TRACE << "FFMpegDecoder2::decode(): Frames per buffer= " << numFrames;
            // queue audio sample
            AudioBufferPtr myBuffer;
            if (_myResampleContext) {
                numFrames = audio_resample(_myResampleContext,
                        (int16_t*)(_myResampledSamples.begin()),
                        myAlignedBuf, numFrames);
                myBuffer = Pump::get().createBuffer(numFrames);
                myBuffer->convert(_myResampledSamples.begin(), SF_S16, myNumChannels);
            } else {
                myBuffer = Pump::get().createBuffer(numFrames);
                myBuffer->convert(myAlignedBuf, SF_S16, myNumChannels);
            }
            _myAudioSink->queueSamples(myBuffer);
            AC_TRACE << "decoded audio time=" << myTime;
        } // while
        
        av_free( myAlignedBuf );
        // adjust volume
        float myVolume = getMovie()->get<VolumeTag>();
        _myAudioSink->setVolume(myVolume);
    }

    bool FFMpegDecoder2::decodeFrame() {
        AC_DEBUG << "---- FFMpegDecoder2::decodeFrame";
        AVPacket * myPacket = 0;
        START_TIMER(decodeFrame_ffmpegdecode);
        // until a frame is found or eof
        bool myEndOfFileFlag = false;
        while (!myEndOfFileFlag) {
            AC_TRACE << "---- decodeFrame: getPacket";
            myPacket = _myDemux->getPacket(_myVStreamIndex);
            if (myPacket == 0) {
                myEndOfFileFlag = true;

                // Usually, we're done at this point. mpeg1 and mpeg2, however,
                // deliver another frame...
                AC_DEBUG << "---- decodeFrame: eof";
                int myFrameCompleteFlag = 0;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52,27,0)
                AVPacket myTempPacket;
                av_init_packet(&myTempPacket);
                /*int myLen =*/ avcodec_decode_video2(_myVStream->codec, _myFrame,
                        &myFrameCompleteFlag, &myTempPacket);
#else                
                /*int myLen =*/ avcodec_decode_video(_myVStream->codec, _myFrame,
                        &myFrameCompleteFlag, 0, 0);
#endif                        
                if (myFrameCompleteFlag) {
		            AC_DEBUG << "---- decodeFrame: Last frame.";
                    // The only way to get the timestamp of this frame is to take
                    // the timestamp of the previous frame and add an appropriate
                    // amount.
                    // (_myFrame->coded_picture_number contains garbage on the second
                    // and successive loops.)
                    // Yuck.
                    addCacheFrame(_myFrame, _myLastFrameTime+1.0/_myFrameRate);
                    _myNumFramesDecoded++;
                    if (_myFrame->pict_type == FF_I_TYPE) {
                        AC_DEBUG << "***** I_FRAME *****";
                        _myNumIFramesDecoded++;
                    }
                    break;
                }
            } else {
                AC_DEBUG << "---- myPacket->dts=" << myPacket->dts;
                int myFrameCompleteFlag = 0;
                START_TIMER(decodeFrame_avcodec_decode);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52,27,0)
                AVPacket myTempPacket;
                av_init_packet(&myTempPacket);
                myTempPacket.data = myPacket->data;
                myTempPacket.size = myPacket->size;
                int myLen = avcodec_decode_video2(_myVStream->codec, _myFrame,
                        &myFrameCompleteFlag, &myTempPacket);
#else
                // try to decode the frame
                int myLen = avcodec_decode_video(_myVStream->codec, _myFrame,
                        &myFrameCompleteFlag, myPacket->data, myPacket->size);
#endif                
                STOP_TIMER(decodeFrame_avcodec_decode);
                AC_DEBUG <<"FFMpegDecoder2::decodeFrame frame doneflag :  "<< myFrameCompleteFlag<<" len: "<<myLen;
                AC_DEBUG<< "dts=" << myPacket->dts << ", position=" <<
                        myPacket->pos << ", duration=" << 
                        myPacket->duration << ", packet pts=" <<
                        myPacket->pts<<", frame pts: "<<_myFrame->pts<<", size: "<<myPacket->size;
                if (myLen < 0) {
                    AC_ERROR << "---- av_decode_video error";
                } else if (myLen < myPacket->size) {
                    AC_ERROR << "---- av_decode_video: Could not decode video in one step";
                }
                
                if(!myFrameCompleteFlag) {
                    // count dropped frames
                    _myDecodedPacketsPerFrame++;                    
                    AC_DEBUG << "### needed packets to decode frame: "<<_myDecodedPacketsPerFrame;
                }
                // start_time indicates the begin of the video
                if (myFrameCompleteFlag  && (myPacket->dts >= _myVStream->start_time)) {
                    STOP_TIMER(decodeFrame_ffmpegdecode);
                    if (_myStartTimestamp == -1) {
                        _myStartTimestamp = myPacket->dts;//_myVStream->start_time;
                        AC_DEBUG << "Set StartTimestamp to packet timestamp= " << _myStartTimestamp;
                    }
                    int64_t myNextPacketTimestamp = myPacket->dts;
                    double myFrameTime = (double)myNextPacketTimestamp/_myTimeUnitsPerSecond;
                    AC_DEBUG << "---- add frame"<< " time_base:"<<_myTimeUnitsPerSecond
		                        <<" FrameTime: "<<myFrameTime;
                    addCacheFrame(_myFrame, myFrameTime);
                    _myLastFrameTime = myFrameTime;
                    _myNumFramesDecoded++;
                    if (_myFrame->pict_type == FF_I_TYPE) {
                        AC_DEBUG << "***** I_FRAME *****";
                        _myNumIFramesDecoded++;
                    }

                    av_free_packet(myPacket);
                    delete myPacket;
                    break;
                }
                av_free_packet(myPacket);
                delete myPacket;
            }
        }
        return !myEndOfFileFlag;
    }

    void FFMpegDecoder2::convertFrame(AVFrame* theFrame, unsigned char* theBuffer) {
        if (!theFrame) {
            AC_ERROR << "FFMpegDecoder::convertFrame invalid AVFrame";
            return;
        }

        AVPicture myDestPict;
        myDestPict.data[0] = theBuffer;
        myDestPict.data[1] = theBuffer+1;
        myDestPict.data[2] = theBuffer+2;

        unsigned myLineSizeBytes = _myFrameWidth * _myBytesPerPixel;
        myDestPict.linesize[0] = myLineSizeBytes;
        myDestPict.linesize[1] = myLineSizeBytes;
        myDestPict.linesize[2] = myLineSizeBytes;

        AVCodecContext * myVCodec = _myVStream->codec;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(51,38,0) 
        START_TIMER(decodeFrame_img_convert);
        img_convert(&myDestPict, _myDestinationPixelFormat,
                    (AVPicture*)theFrame, myVCodec->pix_fmt,
                    myVCodec->width, myVCodec->height);
        STOP_TIMER(decodeFrame_img_convert);
                        
#else
    START_TIMER(decodeFrame_sws_scale);
                
        int mySWSFlags = SWS_FAST_BILINEAR;//SWS_BICUBIC;           
        SwsContext * img_convert_ctx = sws_getContext(myVCodec->width, myVCodec->height,
            myVCodec->pix_fmt,
            myVCodec->width, myVCodec->height,
            _myDestinationPixelFormat,
            mySWSFlags, NULL, NULL, NULL);
        sws_scale(img_convert_ctx, ((AVPicture*)theFrame)->data, 
            ((AVPicture*)theFrame)->linesize, 0, myVCodec->height, 
            myDestPict.data, myDestPict.linesize);
        
        sws_freeContext(img_convert_ctx);
    STOP_TIMER(decodeFrame_sws_scale);
                
#endif        
    }

    VideoMsgPtr FFMpegDecoder2::createFrame(double theTimestamp) {
        AC_DEBUG << "FFMpegDecoder2::createFrame";

        vector<unsigned> myBufferSizes;
        switch (_myDestinationPixelFormat) {
            case PIX_FMT_BGRA:
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight * 4);
                break;
            case PIX_FMT_GRAY8:
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight * 1);
                break;
            case PIX_FMT_YUV420P:
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight);
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight / 4);
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight / 4);
                break;
            default:
                myBufferSizes.push_back(_myFrameWidth * _myFrameHeight * 3);
                break;
        }
        return VideoMsgPtr(new VideoMsg(VideoMsg::MSG_FRAME, theTimestamp, myBufferSizes)); 
    }

    void
    FFMpegDecoder2::dumpCache() {
        AC_DEBUG << "VideoMsgQueue size: " << _myMsgQueue.size();
        _myDemux->dump();
    }

    double
    FFMpegDecoder2::readFrame(double theTime, unsigned /*theFrame*/, RasterVector theTargetRaster)
    {
        AC_DEBUG << "readFrame, Time wanted=" << theTime;
        ASSURE(!getEOF());
        
        if (getPlayMode() == y60::PLAY_MODE_STOP) {
            AC_DEBUG << "readFrame: not playing.";
            return theTime;
        }
        VideoMsgPtr myVideoMsg;
        double myStreamTime = theTime;
        if (_myStartTimestamp != -1) {
            myStreamTime += _myStartTimestamp/_myTimeUnitsPerSecond;
        }
        AC_DEBUG << "_myTimeUnitsPerSecond: "<< _myTimeUnitsPerSecond <<" _myStartTimestamp: "
                <<_myStartTimestamp<<" Time=" << theTime;
        AC_DEBUG <<" - Reading frame timestamp: " << myStreamTime<< " from queue.";

        try {
            bool useLastVideoFrame = false;
            if (_myLastVideoFrame) {
                if (getPlayMode() == y60::PLAY_MODE_PAUSE){
                    if(shouldSeek(_myLastVideoFrame->getTime(), myStreamTime)) {
                        seek(myStreamTime);
                        if (hasAudio() && getDecodeAudioFlag()) {
                            AC_DEBUG << "seek: set Audio";
                            _myAudioSink->setCurrentTime(theTime);
                            if (getState() == RUN) {
                                readAudio();
                                _myAudioSink->play();
                            }
                        }
                    } else if (hasAudio()&& getDecodeAudioFlag()) {
                        AC_DEBUG << "video paused no seeking and audio not running-> setcurrenttime: "<<theTime;
                        _myAudioSink->setCurrentTime(theTime);
                    }
                }
                double myFrameTime = _myLastVideoFrame->getTime();
                double myFrameDiff = (myStreamTime-myFrameTime)*_myFrameRate;
                if (myFrameDiff < 0.5 && myFrameDiff > -0.9) {
                    AC_DEBUG << "readFrame: Reusing last frame. myFrameTime=" << myFrameTime
                        << ", myStreamTime=" << myStreamTime;
                    myVideoMsg = _myLastVideoFrame;
                    useLastVideoFrame = true;
                }
            }
            bool myFrameDroppedFlag = false;

            if (!useLastVideoFrame) {
                for(;;) {
                    myVideoMsg = _myMsgQueue.pop_front();
                    bool myEOFFoundFlag = _myMsgQueue.hasEOF();
                    if (myEOFFoundFlag && !isActive()) {
                        startOverAgain();
                    }
                    
                    if (myVideoMsg->getType() == VideoMsg::MSG_EOF) {
                        setEOF(true);
                        return theTime;
                    }
                    double myTimestamp = myVideoMsg->getTime();
                    AC_DEBUG << "readFrame: FrameTime="
                        << myVideoMsg->getTime()
                        << ", Calculated frame #=" << (myTimestamp - (_myStartTimestamp/_myTimeUnitsPerSecond))*_myFrameRate
                        << ", Cache size=" << _myMsgQueue.size();
                    double myFrameDiff = (myStreamTime - myTimestamp)*_myFrameRate;
                    AC_DEBUG << "           myFrameDiff=" << myFrameDiff; 
                    if (myFrameDiff < 0.5 && myFrameDiff > -1.0) {
                        break;
                    } else if (myFrameDiff <= -1.0){ 
                        // no video frame for audio time in the queue -> wait until the times match again
                        AC_DEBUG<<"can not find frame: "<<myStreamTime<< " -> decoder must have dropped it!!";
                        myFrameDroppedFlag = true;
                        _myMsgQueue.push_front(myVideoMsg); // don't loose the current video frame
                        break;
                    }
                }
            }
            if(!myFrameDroppedFlag){
                _myLastVideoFrame = myVideoMsg;
                // Current frame is in myVideoMsg now. Convert to a format that Y60 can use.
                // we think this is for version bumping
                for (unsigned i = 0 ; i < theTargetRaster.size(); i++) {
                    theTargetRaster[i]->resize( theTargetRaster[i]->width(), theTargetRaster[i]->height());
                }
                if (myVideoMsg) {
                    AC_DEBUG << "readFrame: Frame delivered. wanted=" << theTime
                            << ", got=" << (myVideoMsg->getTime()- _myStartTimestamp/_myTimeUnitsPerSecond);
                    theTime = myVideoMsg->getTime() - _myStartTimestamp/_myTimeUnitsPerSecond;
                    for (unsigned i = 0 ; i < theTargetRaster.size(); i++) {
                        memcpy(theTargetRaster[i]->pixels().begin(), myVideoMsg->getBuffer(i),
                                theTargetRaster[i]->pixels().size());
                    }
                } else {
                    // TODO: Figure out if/why this happens. Delete?
                    AC_WARNING << "readFrame, empty frame.";
                    for (unsigned i = 0 ; i < theTargetRaster.size(); i++) {
                        memset(theTargetRaster[i]->pixels().begin(), 0, theTargetRaster[i]->pixels().size());
                    }
                }
            }
            getMovie()->set<CacheSizeTag>(_myMsgQueue.size());
        } catch (asl::ThreadSemaphore::ClosedException &) {
            AC_WARNING << "Semaphore Destroyed while in readframe";
        }
        return theTime;
    }

    void FFMpegDecoder2::run() {
        AC_DEBUG << "---- run starting";
        if (_myVStream == 0 && _myAStream == 0) {
			AC_WARNING << "---- Neither audio nor video stream in FFMpegDecoder2::run";
            return;
        }
        
        // decoder loop
        bool isDone = false;
        while (!shouldTerminate() && !isDone) {
			AC_TRACE << "---- FFMpegDecoder2::loop";
            if (_myMsgQueue.size() >= _myMaxCacheSize) {
                // decode the audio...but let the video thread sleep
                if (hasAudio() && getDecodeAudioFlag()) {
                    AC_DEBUG<<"---sleeping ---still decode audio";
                    readAudio();
                }
                asl::msleep(10);
                yield();
                continue;
            }
            bool myAudioEofFlag = false;
            if (hasAudio()&& getDecodeAudioFlag())
            {
                myAudioEofFlag = !readAudio();
                AC_DEBUG<<"decode audio eof: "<<myAudioEofFlag;
            }
            AC_TRACE << "---- Updating cache";
            try {
                if (!decodeFrame() || myAudioEofFlag) {
                    std::vector<unsigned> myFrameSize;
                    myFrameSize.push_back(0);
                    _myMsgQueue.push_back(VideoMsgPtr(new VideoMsg(VideoMsg::MSG_EOF, 0, myFrameSize)));
                    isDone = true;
					AC_DEBUG << "---- EOF Yielding Thread.";
                    continue;
                }
            } catch (asl::ThreadSemaphore::ClosedException &) {
                // This should never happen.
                AC_WARNING << "---- Semaphore destroyed while in run. Terminating Thread.";
                return;
            }

			AC_TRACE << "---- end of loop";
        }
        AC_DEBUG << "---- FFMpegDecoder2::run terminating";
    }

    void FFMpegDecoder2::setupVideo(const std::string & theFilename) {
        AC_DEBUG << "FFMpegDecoder2::setupVideo";
        AVCodecContext * myVCodec = _myVStream->codec;
        // open codec
        AVCodec * myCodec = avcodec_find_decoder(myVCodec->codec_id);
        if (!myCodec) {
            throw FFMpegDecoder2Exception(std::string("Unable to find video codec: ")
                    + theFilename, PLUS_FILE_LINE);
        }
        
        {
            AutoLocker<ThreadLock> myLocker(_myAVCodecLock);
            if (avcodec_open(myVCodec, myCodec) < 0 ) {
                throw FFMpegDecoder2Exception(std::string("Unable to open video codec: ")
                                              + theFilename, PLUS_FILE_LINE);
            }
        }
 
        Movie * myMovie = getMovie();
        AC_TRACE << "PF=" << myMovie->get<RasterPixelFormatTag>();

        PixelEncoding myRasterEncoding = PixelEncoding(getEnumFromString(myMovie->get<RasterPixelFormatTag>(), PixelEncodingString));

        // TargetPixelFormatTag is the format the incoming movieframe will be converted in
        if (myMovie->get<TargetPixelFormatTag>() != "") {
            TextureInternalFormat myTargetPixelFormat = TextureInternalFormat(getEnumFromString(myMovie->get<TargetPixelFormatTag>(), TextureInternalFormatStrings));
            switch(myTargetPixelFormat) {
                case TEXTURE_IFMT_YUV420:
                    myRasterEncoding = YUV420;
                    break;
                case TEXTURE_IFMT_RGBA8:
                    myRasterEncoding = RGBA;
                    break;
                case TEXTURE_IFMT_ALPHA:
                    myRasterEncoding = ALPHA;
                    break;

                case TEXTURE_IFMT_LUMINANCE:
                case TEXTURE_IFMT_LUMINANCE8:
                case TEXTURE_IFMT_LUMINANCE16:
                case TEXTURE_IFMT_INTENSITY:
                    myRasterEncoding = GRAY;
                    break;
                case TEXTURE_IFMT_RGB:
                    myRasterEncoding = RGB;
                    break;
                default:
                	AC_FATAL << "Unsupported pixel format " << myMovie->get<TargetPixelFormatTag>() << " in FFMpegDecoder2";
                	break;
            }
        }

        // Setup size and image matrix
        _myFrameWidth = myVCodec->width;
        _myFrameHeight = myVCodec->height;
        
        
        switch (myRasterEncoding) {            
            case RGBA:
				{AC_TRACE << "Using TEXTURE_IFMT_RGBA8 pixels";}
                _myDestinationPixelFormat = PIX_FMT_BGRA;
                _myBytesPerPixel = 4;
                myMovie->createRaster(_myFrameWidth, _myFrameHeight, 1, y60::BGRA);
                break;
            case ALPHA:
				{AC_TRACE << "Using Alpha pixels";}
	            _myDestinationPixelFormat = PIX_FMT_GRAY8;
	            _myBytesPerPixel = 1;
	            myMovie->createRaster(_myFrameWidth, _myFrameHeight, 1, y60::ALPHA);
	            break;
            case GRAY:
				{AC_TRACE << "Using GRAY pixels";}
                _myDestinationPixelFormat = PIX_FMT_GRAY8;
                _myBytesPerPixel = 1;
                myMovie->createRaster(_myFrameWidth, _myFrameHeight, 1, y60::GRAY);
                break;
            case YUV420:
				{AC_TRACE << "Using YUV420 pixels";}
				_myDestinationPixelFormat = PIX_FMT_YUV420P;
                myMovie->createRaster(_myFrameWidth, _myFrameHeight, 1, y60::GRAY);
                myMovie->addRasterValue(createRasterValue( y60::GRAY, _myFrameWidth/2, _myFrameHeight/2), y60::GRAY, 1);                
                myMovie->addRasterValue(createRasterValue( y60::GRAY, _myFrameWidth/2, _myFrameHeight/2), y60::GRAY, 1);    
                break;                            
            case RGB:
            default:
				{AC_TRACE << "Using BGR pixels";}
                _myDestinationPixelFormat = PIX_FMT_BGR24;
                _myBytesPerPixel = 3;                
                myMovie->createRaster(_myFrameWidth, _myFrameHeight, 1, y60::BGR);
                break;
        }
        unsigned myRasterCount = myMovie->getNode().childNodesLength();
        for (unsigned i = 0; i < myRasterCount; i++) {
            myMovie->getRasterPtr(i)->clear();
        }

        _myFrameRate = av_q2d(_myVStream->r_frame_rate);
        
        myMovie->set<FrameRateTag>(_myFrameRate);
        if (myVCodec->codec_id == CODEC_ID_MPEG1VIDEO || myVCodec->codec_id == CODEC_ID_MPEG2VIDEO )
        {
            // For some codecs, the duration value is not set. For MPEG1 and MPEG2,
            // ffmpeg gives often a wrong value.
            _myTimeUnitsPerSecond = 1/ av_q2d(_myVStream->time_base);
            //unsigned myFrameCount = unsigned(_myVStream->duration*_myFrameRate/_myTimeUnitsPerSecond);
            
        } else if (myVCodec->codec_id == CODEC_ID_WMV1 || myVCodec->codec_id == CODEC_ID_WMV2 || 
                   myVCodec->codec_id == CODEC_ID_WMV3)
        {
            myMovie->set<FrameCountTag>(int(_myVStream->duration * _myFrameRate / 1000));
            _myTimeUnitsPerSecond = 1/ av_q2d(_myVStream->time_base);
        } else {
	        double myDuration = 0.0;
            if(_myFormatContext->start_time == AV_NOPTS_VALUE) {
                myDuration = (_myFormatContext->duration )*_myFrameRate/(double)AV_TIME_BASE;
            } else {
                myDuration = (_myFormatContext->duration - _myFormatContext->start_time )*_myFrameRate/(double)AV_TIME_BASE;
            }
            myMovie->set<FrameCountTag>(int(myDuration));
	        _myTimeUnitsPerSecond = 1/ av_q2d(_myVStream->time_base);//_myFrameRate;
	    }
        AC_INFO << "FFMpegDecoder2::setupVideo() " << theFilename << " fps="
                << _myFrameRate << " framecount=" << getFrameCount()<< " time_base: "
                <<_myTimeUnitsPerSecond;
        AC_INFO << "r_framerate den: " <<_myVStream->r_frame_rate.den<< " r_framerate num: "<< _myVStream->r_frame_rate.num;
        AC_INFO << "stream time_base: " << _myVStream->time_base.den << ","<<_myVStream->time_base.num;
        AC_INFO << "codec time_base: " << _myVStream->codec->time_base.den << ","<<_myVStream->codec->time_base.num;
        AC_INFO << "formatcontex start_time: " << _myFormatContext->start_time<<" stream start_time: "<<_myVStream->start_time;
        
        // allocate frame for YUV data
        _myFrame = avcodec_alloc_frame();
        _myStartTimestamp = -1;
            
    }
    void
    FFMpegDecoder2::setupAudio(const std::string & theFilename) {
        AVCodecContext * myACodec = _myAStream->codec;

        // open codec
        AVCodec * myCodec = avcodec_find_decoder(myACodec->codec_id);
        if (!myCodec) {
            throw FFMpegDecoder2Exception(std::string("Unable to find audio decoder: ")
                    + theFilename, PLUS_FILE_LINE);
        }
        
        {
            AutoLocker<ThreadLock> myLocker(_myAVCodecLock);
            if (avcodec_open(myACodec, myCodec) < 0 ) {
                throw FFMpegDecoder2Exception(std::string("Unable to open audio codec: ")
                                              + theFilename, PLUS_FILE_LINE);
            }
        }

        _myAudioSink = Pump::get().createSampleSink(theFilename);

        if (myACodec->sample_rate != static_cast<int>(Pump::get().getNativeSampleRate()))
        {
            //XXX: find a way to handle ffmpeg::SampleFormat and asl::SampleFormat            
/*#if  LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(51,28,0)            
            _myResampleContext = av_audio_resample_init(
                    myACodec->channels, myACodec->channels,
                    Pump::get().getNativeSampleRate(), myACodec->sample_rate,
                    Pump::get().getNativeSampleFormat(), myACodec->sample_fmt,
                    16, 10, 0, 0.8);
#else*/            
            _myResampleContext = audio_resample_init(myACodec->channels,
                    myACodec->channels, Pump::get().getNativeSampleRate(),
                    myACodec->sample_rate);
/*#endif*/            
        }
        AC_TRACE << "FFMpegDecoder2::setupAudio() done. resampling "
            << (_myResampleContext != 0);
    }
    
    void FFMpegDecoder2::addCacheFrame(AVFrame* theFrame, double theTime) {
		AC_DEBUG << "---- try to add frame at " << theTime;
        VideoMsgPtr myVideoFrame = createFrame(theTime);
        AVCodecContext * myVCodec = _myVStream->codec;
        if (myVCodec->pix_fmt==PIX_FMT_PAL8) {
            copyPlaneToRaster(myVideoFrame->getBuffer(0), theFrame->data[0], theFrame->linesize[0], _myFrameWidth, _myFrameHeight);
        } else {
            if (_myDestinationPixelFormat == PIX_FMT_YUV420P ) {
                copyPlaneToRaster(myVideoFrame->getBuffer(0), theFrame->data[0], theFrame->linesize[0], _myFrameWidth, _myFrameHeight);
                copyPlaneToRaster(myVideoFrame->getBuffer(1), theFrame->data[1], theFrame->linesize[1], _myFrameWidth/2, _myFrameHeight/2);
                copyPlaneToRaster(myVideoFrame->getBuffer(2), theFrame->data[2], theFrame->linesize[2], _myFrameWidth/2, _myFrameHeight/2);
            } else {
                convertFrame(theFrame, myVideoFrame->getBuffer());            
            }       
        }
        _myMsgQueue.push_back(myVideoFrame);
        
        AC_DEBUG << "---- Added Frame to cache, Frame # : "
            << double(theTime - _myStartTimestamp/_myTimeUnitsPerSecond)*_myFrameRate
            << " cache size=" << _myMsgQueue.size();
    }

    bool FFMpegDecoder2::shouldSeek(double theCurrentTime, double theDestTime) {
        AC_DEBUG<<"FFMpegDecoder2::shouldSeek: "<<"Dest=" << theDestTime << ", Curr=" << theCurrentTime;
        double myDistance = (theDestTime-theCurrentTime)*_myFrameRate;
        AC_DEBUG<<"myDistance: "<<myDistance;
        bool myShouldSeek;
        if (_myNumIFramesDecoded < 2) {
            myShouldSeek = myDistance>100 || myDistance < 0;
        } else {
            myShouldSeek =  ((myDistance < 0) || 
                             (
                              (myDistance > 10) && 
                              (myDistance > unsigned(3/2*(_myNumFramesDecoded/_myNumIFramesDecoded)))
                             )
                            );
        }
        if (myShouldSeek) {
            AC_DEBUG << "Dest=" << theDestTime << ", Curr=" << theCurrentTime
                << ", myDistance=" << myDistance;
            AC_DEBUG << "_myNumFramesDecoded=" << _myNumFramesDecoded << ", _myNumIFramesDecoded="
                << _myNumIFramesDecoded;
        }
        return  myShouldSeek;
    }

    void FFMpegDecoder2::seek(double theDestTime) {
        AC_DEBUG << "FFMpegDecoder2::seek() desttime: "<<theDestTime;
        if (isUnjoined()) {
            DB(AC_DEBUG << "Joining FFMpegDecoder Thread");
            join();
        }
        if (_myAudioSink && getDecodeAudioFlag()) {
            _myAudioSink->stop();
        }
        _myDemux->clearPacketCache();
        _myMsgQueue.clear();
        _myMsgQueue.reset();

        doSeek(theDestTime);

        if (!isActive()) {
            decodeFrame();
        }
        

        if (!isUnjoined()) {
            AC_DEBUG << "seek: Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_DEBUG << "Thread already running. No forking.";
        }
    }
    // Uses a heuristic based on the number of I-Frames in the video
    // to find the next IFrame
    // Calls ffmpeg seek, flushes buffers etc.
    void FFMpegDecoder2::doSeek(double theDestTime) {
        int64_t mySeekTimeInTimeBaseUnits;
        if(theDestTime == -1){
            //seek to begin of file
            AC_DEBUG<<"seeking to start"<<" starttime: "<< _myStartTimestamp;
            /*int myResult =*/ av_seek_frame(_myFormatContext, _myVStreamIndex, 0, AVSEEK_FLAG_ANY);
        } else {
            unsigned myLastIFrameOffset;
            if(_myNumIFramesDecoded == 0){
                myLastIFrameOffset = 1;
            } else {
                myLastIFrameOffset = unsigned(2*(_myNumFramesDecoded/_myNumIFramesDecoded));
            }
            AC_DEBUG<<"seekOffset: "<<myLastIFrameOffset<<" numIFrames: "<<_myNumIFramesDecoded;
            int64_t myDestFrame = int64_t(theDestTime*_myFrameRate);
            //int64_t myCalculatedSeekTime = int64_t(theDestTime*_myTimeUnitsPerSecond - myLastIFrameOffset*_myTimeUnitsPerSecond/_myFrameRate);
            double myCalculatedSeekTime = theDestTime - myLastIFrameOffset/_myFrameRate;
            myCalculatedSeekTime = ( myCalculatedSeekTime < 0) ? 0 : myCalculatedSeekTime;
            mySeekTimeInTimeBaseUnits = int64_t(myCalculatedSeekTime*_myTimeUnitsPerSecond);
            AC_DEBUG<<"FFMpegDecoder2::doSeek real destTime: "<<theDestTime<<" destFrame: "<<myDestFrame
                    <<"calculated SeekTime: "<< myCalculatedSeekTime<<" calculated SeekFrame: "<<myCalculatedSeekTime*_myFrameRate;
            /*int myResult =*/ av_seek_frame(_myFormatContext, _myVStreamIndex,
                mySeekTimeInTimeBaseUnits, AVSEEK_FLAG_BACKWARD);
        }
        _myDecodedPacketsPerFrame = 0;
        avcodec_flush_buffers(_myVStream->codec);
        if (_myAStream) {
            avcodec_flush_buffers(_myAStream->codec);
        }
    }
}
