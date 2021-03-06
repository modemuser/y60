//=============================================================================
// Copyright (C) 2011, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

/*jslint nomen:false plusplus:false*/
/*globals use, spark, Logger, Vector2f, getImageSize, ShapeStretcher, Vector2i*/

use("spark/add-ons/ShapeStretcher.js");

spark.StretchyImage = spark.ComponentClass("StretchyImage");

spark.StretchyImage.Constructor = function (Protected) {
    var Public = this;
    var Base   = {};
    Public.Inherit(spark.Image);

    /////////////////////
    // Private Members //
    /////////////////////

    var _myImageSize      = null;
    var _myShapeStretcher = null;
    
    function _addEdgeProperty(theAcessorName, theEdgeName) {
        Public.__defineGetter__(theAcessorName, function () {
            return _myShapeStretcher.edges[theEdgeName];
        });

        Public.__defineSetter__(theAcessorName, function (theValue) {
            _myShapeStretcher.edges[theEdgeName] = theValue;
            _myShapeStretcher.updateGeometry(_myImageSize, true, Public.origin);
            _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
        });
    }
    
    function _addCropProperty(theAcessorName, theEdgeName) {
        Public.__defineGetter__(theAcessorName, function () {
            return _myShapeStretcher.crop[theEdgeName];
        });

        Public.__defineSetter__(theAcessorName, function (theValue) {
            _myShapeStretcher.crop[theEdgeName] = theValue;
            _myShapeStretcher.updateGeometry(_myImageSize, true, Public.origin);
            _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
        });
    }
    
    function _resetShapeStretcher() {
        _myShapeStretcher.setupGeometry(_myImageSize, Public.origin);
        _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
    }

    function _applySize(theValue) {
        _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
    }

    ////////////////////
    // Public Methods //
    ////////////////////
    
    _addEdgeProperty('edgeTop',    'top');
    _addEdgeProperty('edgeBottom', 'bottom');
    _addEdgeProperty('edgeLeft',   'left');
    _addEdgeProperty('edgeRight',  'right');

    _addCropProperty('cropTop',    'top');
    _addCropProperty('cropBottom', 'bottom');
    _addCropProperty('cropLeft',   'left');
    _addCropProperty('cropRight',  'right');

    Public.__defineGetter__("edges", function () {
        return [_myShapeStretcher.edges.left,
                _myShapeStretcher.edges.bottom,
                _myShapeStretcher.edges.right,
                _myShapeStretcher.edges.top];
    });
    
    Public.__defineSetter__("edges", function (theEdges) {
        _myShapeStretcher.edges.left   = theEdges[0];
        _myShapeStretcher.edges.bottom = theEdges[1];
        _myShapeStretcher.edges.right  = theEdges[2];
        _myShapeStretcher.edges.top    = theEdges[3];
        _myShapeStretcher.updateGeometry(_myImageSize, true, Public.origin);
        _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
    });

    Public.__defineGetter__("crop", function () {
        return [_myShapeStretcher.crop.left,
                _myShapeStretcher.crop.bottom,
                _myShapeStretcher.crop.right,
                _myShapeStretcher.crop.top];
    });
    
    Public.__defineSetter__("crop", function (theEdges) {
        _myShapeStretcher.crop.left   = theEdges[0];
        _myShapeStretcher.crop.bottom = theEdges[1];
        _myShapeStretcher.crop.right  = theEdges[2];
        _myShapeStretcher.crop.top    = theEdges[3];
        _myShapeStretcher.updateGeometry(_myImageSize, true, Public.origin);
        _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
    });

    Protected.__defineGetter__("shapeStretcher", function () {
            return _myShapeStretcher;
    });

    Protected.__defineGetter__("imageSize", function () {
            return _myImageSize;
    });

    Base.realize = Public.realize;
    Public.realize = function () {
        Base.realize();
        var myShapeStretcherCtor = spark.ShapeStretcher.Factory.getShapeStretcherFromAttribute(
                Protected.getString("shapeStretcher", "default"));
        _myShapeStretcher = new myShapeStretcherCtor(Protected.shape);
        
        _myImageSize  = getImageSize(Public.image);
        Base.imageSetter = Public.__lookupSetter__("image");
        Public.__defineSetter__("image", function (theImage) {
            Base.imageSetter(theImage);
            _myImageSize = getImageSize(theImage);
            _myShapeStretcher.setupGeometry(_myImageSize, Public.origin);
            _myShapeStretcher.updateGeometry(Public.size, false, Public.origin);
        });
        var pToOverride = {height:true, width:true, originX:true, originY:true, originZ:true};
        for (var p in pToOverride) {
            js.array.filter(Public._properties_, function (theItem, i, theArray) {
                return theItem.name === p;
            })[0].handler = _applySize;
        }
        _myShapeStretcher.initialize(Public.node?Public.node:new Node("<StretchyImage/>"));
        _resetShapeStretcher();
    };
};
