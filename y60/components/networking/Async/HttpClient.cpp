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
#include "HttpClient.h"
#include "NetAsync.h"

#include <string>
#include <y60/jsbase/JScppUtils.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <netsrc/spidermonkey/jsapi.h>

using namespace std;
using namespace asl;
using namespace jslib;

namespace y60 {
namespace async {
namespace http {

    Client::Client(JSContext * cx, JSObject * theOpts) :
        _curlHandle(0),
        _socket(NetAsync::io_service()),
        _jsContext(cx),
        _jsOptsObject(theOpts),
        _myErrorBuffer(CURL_ERROR_SIZE, '\0'),
        _socketState(0),
        _read_in_progress(false),
        _write_in_progress(false)
    {
        _curlHandle = curl_easy_init();
        AC_DEBUG << "curl init " << curl_version();

        CURLcode myStatus;
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_ERRORBUFFER, asl::begin_ptr(_myErrorBuffer));
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_PRIVATE, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_WRITEFUNCTION, &Client::_writeFunction);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_WRITEDATA, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_OPENSOCKETFUNCTION, &Client::_openSocket);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_OPENSOCKETDATA, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);

        setCurlOption<std::string>(_jsOptsObject, "url", CURLOPT_URL);
        setCurlOption<bool>(_jsOptsObject, "verbose", CURLOPT_VERBOSE);

        get();
    }

    void
    Client::get() {
        AC_DEBUG << "starting request " << this;
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        parentPlugin->addClient(this);
    }

    Client::~Client()
    {
        AC_DEBUG << "~Client " << this;
    }
    
    void
    Client::checkCurlStatus(CURLcode theStatusCode, const string & theWhere) const {
        if (theStatusCode != CURLE_OK) {
            throw Exception(string(asl::begin_ptr(_myErrorBuffer)), theWhere);
        }
    }

    void
    Client::onSocketState(int theAction) {
        _socketState = theAction;
        handleOperations();
    };

    void
    Client::handleOperations() {
        if (_socketState == CURL_POLL_OUT && !_write_in_progress) {
            AC_DEBUG << "queuing write " << this;
            _write_in_progress = true;
            _socket.async_write_some(
                    boost::asio::null_buffers(),
                    boost::bind(&Client::handleWrite, this,
                        boost::asio::placeholders::error));

        } else if (_socketState == CURL_POLL_IN && !_read_in_progress) {
            AC_TRACE << "queuing read " << this;
            _read_in_progress = true;
            _socket.async_read_some(
                    boost::asio::null_buffers(),
                    boost::bind(&Client::handleRead, this,
                        boost::asio::placeholders::error));

        } else if (_socketState == CURL_POLL_REMOVE) {
            asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
            parentPlugin->removeClient(this);
        }
    }

    void 
    Client::handleRead(const boost::system::error_code& error) {
        AC_TRACE << "doing read " << this;
        _read_in_progress = false;
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        parentPlugin->doSocketRead(this->_socket.native_handle());
        AC_TRACE << " done read " << this;
        handleOperations();
    };

    void 
    Client::handleWrite(const boost::system::error_code& error) {
        AC_DEBUG << "doing write " << this;
        _write_in_progress = false;
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        parentPlugin->doSocketWrite(this->_socket.native_handle());
        AC_DEBUG << " done write " << this;
        handleOperations();
    };

    void
    Client::onDone() {
        AC_WARNING << "onDone. looking for success";
        if (hasCallback("success")) {
            AC_WARNING << "calling success";
            jsval argv[1], rval;
            /*JSBool ok =*/ JSA_CallFunctionName(_jsContext, _jsOptsObject, "success", 0, argv, &rval);
        }
    }

    bool
        Client::hasCallback(const char * theName) {
            jsval myValue;
            AC_WARNING << "looking for " << theName << " in " <<_jsContext << "," << _jsOptsObject; 
            if (JS_GetProperty(_jsContext, _jsOptsObject, theName, &myValue)) {
                AC_WARNING << "hasCallback " << theName << " " << 2;
                if (JS_TypeOfValue(_jsContext, myValue) == JSTYPE_FUNCTION) {
                    AC_WARNING << "hasCallback " << theName << " " << 3;
                    return true;
                }
                AC_WARNING << "Property '" << theName << "' is not a function: type=" << JS_TypeOfValue(_jsContext, myValue);
            }
            AC_WARNING << "hasCallback " << theName << " " << 5;
            return false;
        }

    size_t 
    Client::writeFunction( char *ptr, size_t size, size_t nmemb) {
        return size * nmemb;
    };

    curl_socket_t 
    Client::openSocket(curlsocktype purpose, struct curl_sockaddr *addr) {
        _socket.open(boost::asio::ip::tcp::v4());
        boost::asio::ip::tcp::socket::non_blocking_io non_blocking_io(true);
        _socket.io_control(non_blocking_io);
        
        AC_DEBUG << "open socket";
        return _socket.native_handle(); 
    };
    template<>
    bool 
    Client::setCurlOption<std::string>(JSObject* opts, std::string theProperty, CURLoption theCurlOption) {
        jsval propValue;
        std::string nativeValue;

        JS_GetProperty(_jsContext, opts, theProperty.c_str(), &propValue);
        if (JSVAL_IS_VOID(propValue)) {
            return false;
        }
        if (!jslib::convertFrom(_jsContext, propValue, nativeValue)) {
            JS_ReportError(_jsContext, "Type mismatch on %s", theProperty.c_str());
        }
        CURLcode myStatus = curl_easy_setopt(_curlHandle, theCurlOption, nativeValue.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        AC_WARNING << "set string " << nativeValue;
        return true;
    };
    /* 
    int 
    Client::closeSocket(curl_socket_t item) {
        AC_DEBUG << "close socket";
        _socket.close();
        return 0;
    };
    */
}
}
}
