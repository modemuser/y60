#!/bin/bash
USAGE="usage: createTestMovies.sh MOVIE_WIDTH MOVIE_HEIGHT FRAMERATE\n"
USAGE=${USAGE}"    Movie length is exactly one frame longer than the number of seconds given."
ARGS=3        # Number of arguments expected.
E_BADARGS=65  # Exit value if incorrect number of args passed.

test $# -ne $ARGS && echo -e $USAGE && exit $E_BADARGS

MOVIE_WIDTH=$1
MOVIE_HEIGHT=$2
FRAMERATE=$3

function createImages {
    y60 CreateTestMovie.js $MOVIE_WIDTH $MOVIE_HEIGHT $FRAMERATE 60
}

function createMovie {
    # Note that MOVIE_LENGTH_IN_SECONDS must be an integer!
    MOVIE_LENGTH_IN_SECONDS=$1
    USE_AUDIO=$2
 
    echo --- Generating ${MOVIE_FILENAME}

    MOVIE_FILENAME="testmovies/${MOVIE_WIDTH}x${MOVIE_HEIGHT}_${FRAMERATE}_${MOVIE_LENGTH_IN_SECONDS}"
    if [ "$USE_AUDIO" = "true" ]
    then
        if [ ${MOVIE_LENGTH_IN_SECONDS} -ge 2 ]
        then  
            sox testmovies/OneSecondClick.wav testmovies/tmp/tmp.wav repeat ${MOVIE_LENGTH_IN_SECONDS}
        else
            cp testmovies/OneSecondClick.wav testmovies/tmp/tmp.wav
        fi
        AUDIO_PARAMS="-i testmovies/tmp/tmp.wav"
        MOVIE_FILENAME=${MOVIE_FILENAME}_audio.mpg
    else
        echo "else"
        AUDIO_PARAMS=
        MOVIE_FILENAME=${MOVIE_FILENAME}_noaudio.mpg
    fi
    ffmpeg ${AUDIO_PARAMS} -t ${MOVIE_LENGTH_IN_SECONDS}.04 -b 500 -i testmovies/tmp/${MOVIE_WIDTH}x${MOVIE_HEIGHT}_frame%07d.png -y -r ${FRAMERATE} -vcodec mpeg2video ${MOVIE_FILENAME} 
    
}

createImages 
createMovie 1 true
createMovie 10 true
createMovie 60 true
createMovie 1 false
createMovie 10 false
createMovie 60 false


# Cleanup temporary files
filecount=`ls testmovies/tmp | wc -l`
if [ $filecount -ge 1 ]
then
    echo "removing current images"
    rm testmovies/tmp/*
fi

