/*jslint nomen: false, plusplus: false, bitwise: false*/
/*globals use, spark, OffscreenRenderArea, Modelling, window, Node, Vector3f,
          BaseViewer, LightManager, product, Matrix4f, Point3f, Logger,
          fileExists, getDirectoryPart, print, adjustNodeId, Renderer*/

/**
 * Wrapper for the Y60 offscreen renderer.
 */

spark.Canvas = spark.ComponentClass("Canvas");

spark.Canvas.Constructor = function (Protected) {
    var Public = this;
    var Base = {};
    
    Public.Inherit(spark.ResizableRectangle);
    BaseViewer.prototype.Constructor(Public, []);
    Public.Inherit(spark.CallbackWrapper);
    
    var _myRenderArea = null;
    var _myWorld = null;
    var _myViewport = null;
    var _myCanvasNode = null;
    var _onPreViewportFunc = null;
    var _onPostViewportFunc = null;
    var _myRenderFlag = true;
    
    var PICK_RADIUS = 0.01;
    var _sampling = 1;
    
    /////////////////////
    // Private Methods //
    /////////////////////
    
    function onKey(theEvent) {
        var myState = (theEvent.type === "keybord-key-down");
        var myShiftFlag = (theEvent.modifiers === spark.Keyboard.SHIFT) ||
                          (theEvent.modifiers === spark.Keyboard.ALT_SHIFT) ||
                          (theEvent.modifiers === spark.Keyboard.CTRL_SHIFT) ||
                          (theEvent.modifiers === spark.Keyboard.CTRL_ALT_SHIFT);
        var myAltFlag = (theEvent.modifiers === spark.Keyboard.ALT) ||
                        (theEvent.modifiers === spark.Keyboard.ALT_SHIFT) ||
                        (theEvent.modifiers === spark.Keyboard.CTRL_ALT) ||
                        (theEvent.modifiers === spark.Keyboard.CTRL_ALT_SHIFT);
        // this works because keyboard modifiers are manipulated bitwise
        var myCtrlFlag = (theEvent.modifiers >= spark.Keyboard.CTRL);
        
        if(Public.getLightManager()) {
            Public.getLightManager().onKey(theEvent.key, myState, myShiftFlag);
        }
        var myMover = Public.getMover(_myViewport);
        if (myMover) {
            myMover.onKey(theEvent.key, myState, 0, 0, myShiftFlag, myCtrlFlag, myAltFlag);
        }
    }
    
    ///////////////////////
    // Getters / Setters //
    ///////////////////////
    
    Public.__defineGetter__("world", function () {
        return _myWorld;
    });
    
    Public.__defineGetter__("viewport", function () {
        return _myViewport;
    });
    
    Public.__defineSetter__("onPreViewportFunc", function(theCallback) {
        _onPreViewportFunc = theCallback;
    });
    
    Public.__defineSetter__("onPostViewportFunc", function(theCallback) {
        _onPostViewportFunc = theCallback;
    });
    
    Public.__defineSetter__("render", function(theBoolean) {
        _myRenderFlag = theBoolean;
    });
    
    ////////////////////
    // Public Methods //
    ////////////////////
    
    Public.convertToCanvasCoordinates = function (theX, theY) {
        var transformMatrix;
        var intersecPointOnCanvas = null;
        // assumptions:
        //  - as the rectangle has no depth, there should always only appear ONE intersection
        //  - the Canvas is topmost intersection, if not, this handler would not have been called by spark
        var myIntersection = Public.stage.picking.pickIntersection(theX, theY);
        if (myIntersection) {
            transformMatrix = new Matrix4f(Public.innerSceneNode.globalmatrix);
            transformMatrix.invert();
            intersecPointOnCanvas = product(myIntersection.info.intersections[0].position,
                                            transformMatrix);
            intersecPointOnCanvas.y = Public.height - intersecPointOnCanvas.y;
        }
        return intersecPointOnCanvas;
    };
    
    Base.realize = Public.realize;
    Public.realize = function () {
        var myCamera, myWorldId, myCanvasId;
        _sampling    = Protected.getNumber("sampling", 1);
        var myWidth  = Protected.getNumber("width", 100);
        var myHeight = Protected.getNumber("height", 100);
        
        _myRenderArea = new OffscreenRenderArea();
        _myRenderArea.renderingCaps = Public.getDefaultRenderingCapabilites() | Renderer.FRAMEBUFFER_SUPPORT;
        _myRenderArea.multisamples = _sampling;
        
        var myImage = Modelling.createImage(window.scene,
                                            myWidth, myHeight, "BGRA");
        myImage.name = Public.name + "_canvasImage";
        var myTexture = Modelling.createTexture(window.scene, myImage);
        myTexture.wrapmode = "clamp_to_edge";
        
        var myFlipMatrix = myTexture.matrix;
        myFlipMatrix.makeScaling(new Vector3f(1, -1, 1));
        myFlipMatrix.translate(new Vector3f(0, 1, 0));
        
        var myMaterial = Modelling.createUnlitTexturedMaterial(window.scene, myTexture);
        myMaterial.transparent = true;
        
        var myAutoCreateTag = Protected.getBoolean("autoCreate", true);
        var mySceneFile = Protected.getString("sceneFile", "");
        
        if (mySceneFile) {
            // Create (merge) world from scene file
            var myDom = spark.Canvas.prepareMerge(mySceneFile);
            
            // XXX assumption: only one world exist/ is handled
            myWorldId  = myDom.find(".//worlds/world").id;
            myCanvasId = myDom.find(".//canvases/canvas").id;
            
            spark.Canvas.mergeScenes(window.scene, myDom);
            
            _myCanvasNode = window.scene.dom.find(".//canvases/canvas[@id='" + myCanvasId + "']");
            _myViewport = _myCanvasNode.find(".//viewport");
            myCamera    = window.scene.dom.getElementById(myWorldId).find(".//camera");
            
            _myWorld = window.scene.dom.getElementById(myWorldId);
            _myWorld.name = Public.name + "-world";
        } 
        else {
            _myCanvasNode = Node.createElement("canvas");
            _myCanvasNode.name = Public.name + "-canvas";
            window.scene.canvases.appendChild(_myCanvasNode);
            
            _myViewport = Node.createElement("viewport");
            _myViewport.name = Public.name + "-viewport";
            _myCanvasNode.appendChild(_myViewport);
            
            if(myAutoCreateTag) {
                // Setup default world
                _myWorld = Node.createElement("world");
                _myWorld.name = Public.name + "-world";
                window.scene.worlds.appendChild(_myWorld);

                myCamera = Node.createElement("camera");
                _myWorld.appendChild(myCamera);
                spark.setupCameraOrtho(myCamera, myWidth, myHeight);
                
                _myViewport.camera = myCamera.id;
                _myCanvasNode.backgroundcolor[3] = 0.0;
            }
            else {
                _myCanvasNode.backgroundcolor = new Vector4f(1,1,1,1);
            }
        }
        
        if(_myWorld) {
            var myLightManager = new LightManager(window.scene, _myWorld);
            Public.setLightManager(myLightManager);
        }
        
        Public.setupWindow(_myRenderArea, false);
        
        _myCanvasNode.target = myTexture.id;
        _myRenderArea.setSceneAndCanvas(window.scene, _myCanvasNode);

        Public.setCanvas(_myCanvasNode);
        
        Public.parent.stage.addEventListener(spark.StageEvent.FRAME, Public.onFrame);
        Public.stage.addEventListener(spark.KeyboardEvent.KEY_DOWN, onKey);
        Public.stage.addEventListener(spark.KeyboardEvent.KEY_UP, onKey);
        Public.addEventListener(spark.MouseEvent.MOVE, Public.onMouseMotion);
        Public.addEventListener(spark.MouseEvent.BUTTON_DOWN, Public.onMouseButtonDown);
        Public.addEventListener(spark.MouseEvent.BUTTON_UP, Public.onMouseButtonUp);
        
        Base.realize(myMaterial);
    };
    
    Base.setActiveCamera = Public.setActiveCamera;
    Public.setActiveCamera = function(theCamera, theViewport) {
        if(!Public.getLightManager()) {
            var myWorld = theCamera.parentNode;
            while(myWorld && myWorld.nodeName != "world") {
                myWorld = myWorld.parentNode;
            }
            var myLightManager = new LightManager(window.scene, myWorld);
            Public.setLightManager(myLightManager);
            myLightManager.setupDefaultLighting(_myCanvasNode);
        }
        
        Base.setActiveCamera(theCamera, theViewport);
    };
    
    Base.onPreViewport = Public.onPreViewport;
    Public.onPreViewport = function(theViewport) {
        Base.onPreViewport(theViewport);
        if (_onPreViewportFunc) {
            _onPreViewportFunc();
        }
    };
    
    Base.onPostViewport = Public.onPostViewport;
    Public.onPostViewport = function(theViewport) {
        if (_onPostViewportFunc) {
            _onPostViewportFunc();
        }
        Base.onPostViewport(theViewport);
    };
    
    Public.pickBody = function (theX, theY) {
        var pickedBody     = null,
            canvasPosition = Public.convertToCanvasCoordinates(theX, theY);
        if (canvasPosition) {
            pickedBody = Public.picking.pickBodyBySweepingSphereFromBodies(
                                canvasPosition.x, canvasPosition.y,
                                PICK_RADIUS, _myWorld, _myViewport);
        }
        return pickedBody;
    };
    
    Base.onFrame = Public.onFrame;
    Public.onFrame = function (theEvent) {
        Base.onFrame(theEvent.currenttime);
        if(_myRenderFlag) {
            _myRenderArea.renderToCanvas();
        }
    };
    
    Base.onMouseMotion = Public.onMouseMotion;
    Public.onMouseMotion = function (theEvent) {
        var canvasPosition = Public.convertToCanvasCoordinates(theEvent.stageX, theEvent.stageY);
        if (canvasPosition) {
            Base.onMouseMotion(canvasPosition.x, canvasPosition.y);
        }
    };
    
    Base.onMouseButton = Public.onMouseButton;
    Public.onMouseButton = function (theButton, theState, theX, theY) {
        var canvasPosition = Public.convertToCanvasCoordinates(theX, theY);
        if (canvasPosition) {
            Base.onMouseButton(theButton, theState, canvasPosition.x, canvasPosition.y);
        }
    };
};

spark.Canvas.prepareMerge = function prepareMerge(theSceneFilePath) {
    theSceneFilePath = searchFile(theSceneFilePath);
    var myTargetDir, myTexSrcPath, myNewPath;
    /* adjust ids:
     *  - world, canvas, viewport ids need to be unique as we need duplicates
     *  - camera id needs to be unique, is linked in viewport, so viewport.camera needs update too
     *  -> !!! current assumption: only one World, Camera, Canvas and Viewport per scene !!!
     *  - all other duplicate ids are supposed to be same model as already exists and will share shape etc
     *
     * adjust file path of textures within scene DOM
     *  - path is originally set relative to Model file location
     *  -> due to scene merge path needs to be updated (relative to application directory)
     */
    
    Logger.info("Loading spark file " + theSceneFilePath);
    if (!fileExists(theSceneFilePath)) {
        throw new Error("spark file '" + theSceneFilePath + "' does not exist.");
    }
    
    var myDom = new Node();
    
    myDom.parseFile(theSceneFilePath);
    
    adjustNodeId(myDom.find(".//world"), true);
    adjustNodeId(myDom.find(".//canvases/canvas"), false);
    var myViewport = myDom.find(".//viewport");
    adjustNodeId(myViewport, false);
    adjustNodeId(myDom.find(".//camera"), false);
    myViewport.camera = myDom.find(".//camera").id;
    
    // REWRITE TEXTURE SRC PATH
    var imageNode = myDom.find(".//images");
    for (var i = 0; i < imageNode.childNodesLength(); ++i) {
        myTargetDir = getDirectoryPart(theSceneFilePath);
        myTexSrcPath = imageNode.childNodes[i].src;
        if (myTexSrcPath.charAt(0) !== "/") {
            myTexSrcPath = myTexSrcPath.replace(/^\.\//, "");
            myNewPath = myTargetDir + myTexSrcPath;
            if (!fileExists(myNewPath)) {
                Logger.warning("Could not find texture within path '" + myNewPath + "'");
            } else {
                imageNode.childNodes[i].src = myNewPath;
            }
        }
    }
    return myDom;
};

spark.Canvas.mergeScenes = function (theTargetScene, theModelDom) {
    var childNode, receivingNode, childChildNode;
    var mySceneNode = theModelDom.firstChild;
    
    while (mySceneNode.childNodesLength()) {
        childNode = mySceneNode.firstChild;
        receivingNode = theTargetScene.dom.getNodesByTagName(childNode.nodeName)[0];
        while (childNode.childNodesLength()) {
            childChildNode = childNode.removeChild(childNode.firstChild);
            try {
                receivingNode.appendChild(childChildNode);
            } catch (err) {
                print("--> Node '" + childChildNode.nodeName + "' with id='" + childChildNode.id + "' already exists but is not needed. Don't worry."); 
            }
        }
        mySceneNode.removeChild(mySceneNode.firstChild);
    }
};