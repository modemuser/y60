//============================================================================
//
// Copyright (C) 2002-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "SimWater.h"
#include "AGPBufferAllocator.h"

#include <y60/JSVector.h>
#include <y60/AbstractRenderWindow.h>

#include <y60/GLAlloc.h>

const int SIMULATION_WIDTH = 480;
const int SIMULATION_HEIGHT = 240;

const std::string DATA_DIR("data");
const char * const ourCubeSides[6] = {"right","left","top","bottom","front","back"};

using namespace std;
using namespace asl;
using namespace jslib;
using namespace y60;

SimWater::SimWater(DLHandle theDLHandle) :
    PlugInBase(theDLHandle),
    IRendererExtension("SimWater"),
    _mySimulationSize( SIMULATION_WIDTH + 2 , SIMULATION_HEIGHT + 2),
    _mySimulationOffset( 1, 1 ),
    _myDisplaySize(SIMULATION_WIDTH / 4 , SIMULATION_HEIGHT / 2),
    _myDisplayOffset(0 , 0), // TODO: doesn't change anything ... find out why?
    _myWaterDamping(0.9993),
    _myRunSimulationFlag( true ),
    _myIntegrationsPerFrame( 3 ),
    _myTimeStep( 0.1 ),
    _myViewportSize(0, 0),
    _myFloormapCounter(0),
    _myCubemapCounter(0)
{
//    initGLExtensions(0);
}

SimWater::~SimWater() {
}

void 
SimWater::onUpdateSettings(dom::NodePtr theConfiguration) {
};

void 
SimWater::onGetProperty(const std::string & thePropertyName,
        PropertyValue & theReturnValue) const
{
    AC_TRACE << "SimWater::onGetProperty(): " << thePropertyName;
    if (thePropertyName == "activeFloormap") {
        theReturnValue.set( _myWaterRepresentation->getCurrentTextureIndex( WaterRepresentation::floormaps ));
        return;
    }
    if (thePropertyName == "floormapCount") {
        theReturnValue.set( _myFloormapCounter );
        return;
    }
    if (thePropertyName == "activeCubemap") {
        theReturnValue.set( _myWaterRepresentation->getCurrentTextureIndex( WaterRepresentation::cubemaps ));
        return;
    }
    if (thePropertyName == "cubemapCount") {
        theReturnValue.set( _myCubemapCounter );
        return;
    }
    if (thePropertyName == "integrationsPerFrame") {
        theReturnValue.set( _myIntegrationsPerFrame );
        return;
    }
    if (thePropertyName == "reflectionAlphaBias") {
        theReturnValue.set( _myWaterRepresentation->getReflectionAlphaBias() );
        return;
    }
    if (thePropertyName == "reflectionAlphaScale") {
        theReturnValue.set( _myWaterRepresentation->getReflectionAlphaScale() );
        return;
    }
    if (thePropertyName == "timestep") {
        theReturnValue.set( _myTimeStep);
        return;
    }
    if (thePropertyName == "simulationOffset") {
        theReturnValue.set( _mySimulationOffset);
        return;
    }
    if (thePropertyName == "displayOffset") {
        theReturnValue.set( _myDisplayOffset);
        return;
    }
    if (thePropertyName == "simulationSize") {
        theReturnValue.set( Vector2i(SIMULATION_WIDTH, SIMULATION_HEIGHT));
        return;
    }
    
    AC_WARNING << "SimWater::onGetProperty(): Unknown property '" << thePropertyName << "'.";
};

void 
SimWater::onSetProperty(const std::string & thePropertyName,
        const PropertyValue & thePropertyValue)
{
    AC_TRACE << "SimWater::onSetProperty()";
    if (thePropertyName == "activeFloormap") {
        short myIndex = short(thePropertyValue.get<int>());
        _myWaterRepresentation->activateTexture( WaterRepresentation::floormaps, myIndex );
        return;
    }
    if (thePropertyName == "activeCubemap") {
        short myIndex = short(thePropertyValue.get<int>());
        _myWaterRepresentation->activateTexture( WaterRepresentation::cubemaps, myIndex );
        return;
    }
    if (thePropertyName == "integrationsPerFrame") {
        short myIterationCount = short(thePropertyValue.get<int>());
        _myIntegrationsPerFrame = myIterationCount;
        return;
    }
    if (thePropertyName == "reflectionAlphaBias") {
        float myValue = thePropertyValue.get<float>();
        _myWaterRepresentation->setReflectionAlphaBias( myValue );
        return;
    }
    if (thePropertyName == "reflectionAlphaScale") {
        float myValue = thePropertyValue.get<float>();
        _myWaterRepresentation->setReflectionAlphaScale( myValue );
        return;
    }
    if (thePropertyName == "timestep") {
       _myTimeStep = thePropertyValue.get<float>();
       return;
    }
    if (thePropertyName == "simulationOffset") {
        _mySimulationOffset = thePropertyValue.get<Vector2i>();
        _myWaterRepresentation->setDataOffset( _mySimulationOffset );
        return;
    }
    if (thePropertyName == "displayOffset") {
        _myDisplayOffset = thePropertyValue.get<Vector2i>();
        _myWaterRepresentation->setDisplayOffset( _myDisplayOffset );
        return;
    }

    AC_WARNING << "SimWater::onSetProperty(): Unknown property '" << thePropertyName << "'.";
};

void 
SimWater::onStartup(jslib::AbstractRenderWindow * theWindow)  {
    AC_PRINT << "SimWater::onStartup()";
    initGLExtensions(0, true, false);
    initializeGLMemoryExtensions();
    _myViewportSize[0] = theWindow->getWidth();
    _myViewportSize[1] = theWindow->getHeight();

    _myWaterSimulation = WaterSimulationPtr(
            new WaterSimulation( _mySimulationSize, _myWaterDamping ));
    _myWaterSimulation->init();
    _myWaterSimulation->resetDamping( false );

    _myWaterRepresentation = WaterRepresentationPtr( new WaterRepresentation() );
    dom::NodePtr myCanvas = theWindow->getCanvas();
    _myWaterRepresentation->init( _myWaterSimulation, _myDisplaySize[0], _myDisplaySize[1],
            _mySimulationOffset[0], _mySimulationOffset[1], 
            _mySimulationSize[0], _mySimulationSize[1],
            theWindow->getWidth(), theWindow->getHeight(),
            _myDisplayOffset[0], _myDisplayOffset[1],
            BufferAllocatorPtr( new AGPBufferAllocator ) );

    _myStartTime.setNow();
};

void
SimWater::loadTexturesFromConfig(const dom::Node & theConfig, WaterRepresentation::TextureClass theClassID) {
    try {
        string myClassName = WaterRepresentation::TextureClassName(theClassID);
        int numFloorTextures = theConfig(myClassName).childNodesLength("file");
        for (int i = 0; i < numFloorTextures; ++i) {
            AC_PRINT << "texture: " << i;
            const dom::Node & myFileNode = theConfig(myClassName)("file",i); 
            string fileName = DATA_DIR + "/" + myFileNode["name"].nodeValue();
            short objectID = asl::as<short>(myFileNode["objectid"].nodeValue());
            bool loadOK = _myWaterRepresentation->loadTexture(theClassID,objectID,fileName.c_str());
            assert(loadOK);
        }
    } catch (const asl::Exception & ex) {
        AC_ERROR << "Water:: in loadTexturesFromConfig():" << endl;
        AC_ERROR << ex.what() << " AT " << ex.where() << endl;
        AC_ERROR << "Aborting" << endl;
        exit(1);
    } catch (...) {
        AC_ERROR << "#ERROR: Water:: in loadTexturesFromConfig():" << endl;
        AC_ERROR << "Aborting" << endl;
        exit(1);
    }
}


bool 
SimWater::onSceneLoaded(jslib::AbstractRenderWindow * theWindow) {
    AC_PRINT << "SimWater::onSceneLoaded()";

    return true;
};

void 
SimWater::handle(jslib::AbstractRenderWindow * theWindow, y60::EventPtr theEvent) {
    AC_DEBUG << "SimWater::handle()";
};

void 
SimWater::onFrame(jslib::AbstractRenderWindow * theWindow , double t) {
    AC_DEBUG << "SimWater::onFrame()";

    _myViewportSize[0] = theWindow->getWidth();
    _myViewportSize[1] = theWindow->getHeight();
    
    asl::Time myCurrentTime;
    myCurrentTime.setNow();
    double myRunTime = myCurrentTime - _myStartTime;

    if (_myRunSimulationFlag) {
        if (_myIntegrationsPerFrame == 1) {
            _myWaterSimulation->simulationStep(_myTimeStep);
        } else {
            _myWaterSimulation->simulationMultiStep(_myTimeStep, _myIntegrationsPerFrame, 5);
        }
        assert( _myWaterRepresentation);

        _myWaterRepresentation->preRender();
    }

};


void
SimWater::setWaterProjection() {
    
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

    float   left = (-((float)(_myDisplaySize[0])) / 2.f) + _myDisplayOffset[0];
    float   right = (((float)_myDisplaySize[0]) / 2.f-1.f) + _myDisplayOffset[0];
    float   top = (-((float)_myDisplaySize[1]) / 2.f) + _myDisplayOffset[1];
    float   bottom = (((float)_myDisplaySize[1]) / 2.f) + _myDisplayOffset[1];

    //cerr << "l: " << left << " r: " << right << " t: " << top << " b: " << bottom << endl;
	glOrtho( left, right, top, bottom, -100, 10000);
}

void 
SimWater::onPreRender(jslib::AbstractRenderWindow * theRenderer) {
    AC_DEBUG << "SimWater::onPreRender()";
};

void 
SimWater::onPostRender(jslib::AbstractRenderWindow * theRenderer) {
    AC_DEBUG << "SimWater::onPostRender()";

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );

    {
        glDisable(GL_CULL_FACE); // XXX the whole water thingy seems to be backfacing
        glDisable(GL_LIGHTING);

        setWaterProjection();

        glMatrixMode( GL_MODELVIEW );

        glLoadIdentity();

        // camera transformation
        glTranslatef( 0, 0, 0.0);
        glScalef(1.0, 1.0, 1.0);
        glRotatef(0, 1.,0.,0.);
        glRotatef(0, 0.,1.,0.);
        glRotatef(0, 0.,0.,1.);

        glColor4f(1, 1, 1, 1);

        assert(_myWaterRepresentation);

        _myWaterRepresentation->render();
    }

    glPopClientAttrib();
    glPopAttrib();

    CHECK_OGL_ERROR;
};

Vector2i 
SimWater::convertMouseCoordsToSimulation( const Vector2i & theMousePos ) {
    Vector2i myResult;
    myResult[0] = max( 0, min( _myDisplaySize[0] - 1, 
                (theMousePos[0] * _myDisplaySize[0] / _myViewportSize[0])+ _myDisplayOffset[0]));
    myResult[1] = max (0, min( _myDisplaySize[1] - 1,
                ((_myViewportSize[1] - theMousePos[1])  * _myDisplaySize[1] / _myViewportSize[1])+ _myDisplayOffset[1]));
    return myResult + _mySimulationOffset;
}

int 
SimWater::addFloormap(const std::string & theFilename) {
    bool loadOk = _myWaterRepresentation->loadTexture(WaterRepresentation::floormaps, _myFloormapCounter, theFilename.c_str());
    _myWaterRepresentation->activateTexture(WaterRepresentation::floormaps, _myFloormapCounter );
    return _myFloormapCounter++;
}

int 
SimWater::addCubemap(const std::string theFilenames[]) {
    bool loadOk = _myWaterRepresentation->loadCubeMapTexture(WaterRepresentation::cubemaps, _myCubemapCounter,
            theFilenames);
    _myWaterRepresentation->activateTexture(WaterRepresentation::cubemaps, _myCubemapCounter );
    return _myCubemapCounter++;
}

void 
SimWater::reset() {
    _myWaterSimulation->reset();
}

void 
SimWater::splash(const asl::Vector2i & thePosition, int theMagnitude, int theRadius) {
     Vector2i myCoord = convertMouseCoordsToSimulation( thePosition );
     _myWaterSimulation->sinoidSplash(myCoord[0], myCoord[1], theMagnitude, theRadius);
}

const char *
SimWater::ClassName() {
    static const char * myClassName = "SimWater";
    return myClassName;
}


static JSBool
Splash(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;

    asl::Vector2i myPosition;
    convertFrom(cx, argv[0], myPosition);

    int myMagnitude;
    convertFrom(cx, argv[1], myMagnitude);

    int myRadius;
    convertFrom(cx, argv[2], myRadius);

    asl::Ptr<SimWater> myNative = getNativeAs<SimWater>(cx, obj);
    if (myNative) {
        myNative->splash(myPosition, myMagnitude, myRadius);
    } else {
        assert(myNative);
    }

    return JS_TRUE;
}

//=== Wrapper Functions ===============================

static JSBool
Reset(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;

    asl::Ptr<SimWater> myNative = getNativeAs<SimWater>(cx, obj);
    if (myNative) {
        myNative->reset();
    } else {
        assert(myNative);
    }

    return JS_TRUE;
}

static JSBool
AddFloormap(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;

    std::string myFilename;
    convertFrom(cx, argv[0], myFilename);

    asl::Ptr<SimWater> myNative = getNativeAs<SimWater>(cx, obj);
    if (myNative) {
        myNative->addFloormap( myFilename );
    } else {
        assert(myNative);
    }

    return JS_TRUE;
}

static JSBool
AddCubemap(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;

    std::string myFilenames[6];
    for (unsigned i = 0; i < 6; ++i) {
        convertFrom(cx, argv[i], myFilenames[i]);
    }

    asl::Ptr<SimWater> myNative = getNativeAs<SimWater>(cx, obj);
    if (myNative) {
        myNative->addCubemap( myFilenames );
    } else {
        assert(myNative);
    }

    return JS_TRUE;
}


JSFunctionSpec *
SimWater::Functions() {
    static JSFunctionSpec myFunctions[] = {
        {"addFloormap", AddFloormap, 1},
        {"addCubemap", AddCubemap, 6},
        {"splash", Splash, 3},
        {"reset", Reset, 0},
        {0}
    };
    return myFunctions;
}

