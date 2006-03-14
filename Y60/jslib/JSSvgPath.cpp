//=============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "JSVector.h"
#include "JSSvgPath.h"
#include "JSLine.h"
#include "JSSphere.h"

namespace jslib {

    JSConstIntPropertySpec * JSSvgPath::ConstIntProperties() {
        static JSConstIntPropertySpec myProperties[] = {
            {0}
        };
        return myProperties;
    }

    static JSBool
    toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Print path.");
        DOC_END;
        std::string myStringRep = std::string("SvgPath@") + asl::as_string(obj);
        JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
        *rval = STRING_TO_JSVAL(myString);
        return JS_TRUE;
    }

    static JSBool
    move(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Move to point.");
        DOC_PARAM("thePoint", "Destination point", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theRelativeFlag", "Relative-or-absolute position.", DOC_TYPE_BOOLEAN);
        DOC_END;

        ensureParamCount(argc, 1,2);

        asl::Vector3f myPosition;
        bool myRelativeFlag = false;

        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPosition)) {
            JS_ReportError(cx, "JSSvgPath::move: argument #1 must be a vector");
            return JS_FALSE;
        }
        if (argc > 1) {
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myPosition)) {
                JS_ReportError(cx, "JSSvgPath::move: argument #1 must be a boolean");
                return JS_FALSE;
            }
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::move: self is not a SvgPath");
            return JS_FALSE;
        }
        myNative->move(myPosition, myRelativeFlag);

        return JS_TRUE;
    }

    static JSBool
    line(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Draw line to point.");
        DOC_PARAM("thePoint", "End point", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theRelativeFlag", "Relative-or-absolute position.", DOC_TYPE_BOOLEAN);
        DOC_END;

        ensureParamCount(argc, 1,2);

        asl::Vector3f myPosition;
        bool myRelativeFlag = false;

        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPosition)) {
            JS_ReportError(cx, "JSSvgPath::line: argument #1 must be a vector");
            return JS_FALSE;
        }
        if (argc > 1) {
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myPosition)) {
                JS_ReportError(cx, "JSSvgPath::line: argument #1 must be a boolean");
                return JS_FALSE;
            }
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::line: self is not a SvgPath");
            return JS_FALSE;
        }
        myNative->line(myPosition, myRelativeFlag);

        return JS_TRUE;
    }

    static JSBool
    hline(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Draw horizontal line.");
        DOC_PARAM("theX", "X end point.", DOC_TYPE_FLOAT);
        DOC_PARAM("theRelativeFlag", "Relative-or-absolute position.", DOC_TYPE_BOOLEAN);
        DOC_END;

        ensureParamCount(argc, 1,2);

        float myXPosition;
        bool myRelativeFlag = false;

        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myXPosition)) {
            JS_ReportError(cx, "JSSvgPath::hline: argument #1 must be a float");
            return JS_FALSE;
        }
        if (argc > 1) {
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myRelativeFlag)) {
                JS_ReportError(cx, "JSSvgPath::hline: argument #1 must be a boolean");
                return JS_FALSE;
            }
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::hline: self is not a SvgPath");
            return JS_FALSE;
        }
        myNative->hline(myXPosition, myRelativeFlag);

        return JS_TRUE;
    }

    static JSBool
    vline(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Draw vertical line.");
        DOC_PARAM("theY", "Y end point.", DOC_TYPE_FLOAT);
        DOC_PARAM("theRelativeFlag", "Relative-or-absolute position.", DOC_TYPE_BOOLEAN);
        DOC_END;

        ensureParamCount(argc, 1,2);

        float myYPosition;
        bool myRelativeFlag = false;

        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myYPosition)) {
            JS_ReportError(cx, "JSSvgPath::vline: argument #1 must be a float");
            return JS_FALSE;
        }
        if (argc > 1) {
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myRelativeFlag)) {
                JS_ReportError(cx, "JSSvgPath::vline: argument #1 must be a boolean");
                return JS_FALSE;
            }
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::vline: self is not a SvgPath");
            return JS_FALSE;
        }
        myNative->vline(myYPosition, myRelativeFlag);

        return JS_TRUE;
    }

    static JSBool
    cbezier(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Draw cubic bezier curve.");
        DOC_PARAM("theStartAnchor", "Start point anchor.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theEndAnchor", "End point anchor.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theEnd", "End point.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theRelativeFlag", "Relative-or-absolute position.", DOC_TYPE_BOOLEAN);
        DOC_END;

        ensureParamCount(argc, 3,4);

        asl::Vector3f myStartAnchor, myEndAnchor, myEnd;
        bool myRelativeFlag = false;

        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myStartAnchor)) {
            JS_ReportError(cx, "JSSvgPath::cbezier: argument #1 must be a vector");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myEndAnchor)) {
            JS_ReportError(cx, "JSSvgPath::cbezier: argument #2 must be a vector");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[2]) || !convertFrom(cx, argv[2], myEnd)) {
            JS_ReportError(cx, "JSSvgPath::cbezier: argument #3 must be a vector");
            return JS_FALSE;
        }
        if (argc > 3) {
            if (JSVAL_IS_VOID(argv[3]) || !convertFrom(cx, argv[3], myRelativeFlag)) {
                JS_ReportError(cx, "JSSvgPath::cbezier: argument #4 must be a boolean");
                return JS_FALSE;
            }
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::cbezier: self is not a SvgPath");
            return JS_FALSE;
        }
        myNative->cbezier(myStartAnchor, myEndAnchor, myEnd, myRelativeFlag);

        return JS_TRUE;
    }


    static JSBool
    close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Close the path.");
        DOC_END;
        return Method<JSSvgPath::NATIVE>::call(&JSSvgPath::NATIVE::close,cx,obj,argc,argv,rval);
    }

    static JSBool
    revert(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Revert the path.");
        DOC_END;
        return Method<JSSvgPath::NATIVE>::call(&JSSvgPath::NATIVE::revert,cx,obj,argc,argv,rval);
    }

    static JSBool
    getNumElements(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Get number of elements in path.");
        DOC_RVAL("Number of elements.", DOC_TYPE_INTEGER);
        DOC_END;
        return Method<JSSvgPath::NATIVE>::call(&JSSvgPath::NATIVE::getNumElements,cx,obj,argc,argv,rval);
    }

    static JSBool
    getElement(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Get path element.");
        DOC_PARAM("theIndex", "Element index.", DOC_TYPE_INTEGER);
        DOC_RVAL("Element.", DOC_TYPE_OBJECT);
        DOC_END;

        ensureParamCount(argc, 1);

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::getElement: self is not a SvgPath");
            return JS_FALSE;
        }

        unsigned myIndex;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myIndex)) {
            JS_ReportError(cx, "JSSvgPath::getElement: argument #1 must be an int");
            return JS_FALSE;
        }

        asl::SvgPath::LineSegmentPtr myElement = myNative->getElement(myIndex);
        if (!myElement) {
            *rval = JSVAL_NULL;
        } else {
            *rval = as_jsval(cx, *(myElement.getNativePtr()));
        }

        return JS_TRUE;
    }

    static JSBool
    nearest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Find point on path closest to position.");
        DOC_PARAM("thePosition", "Position.", DOC_TYPE_VECTOR3F);
        DOC_RVAL("{point, element}", DOC_TYPE_OBJECT);
        DOC_END;

        ensureParamCount(argc, 1);

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::nearest: self is not a SvgPath");
            return JS_FALSE;
        }

        asl::Vector3f myPosition;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPosition)) {
            JS_ReportError(cx, "JSSvgPath::nearest: argument #1 must be a vector");
            return JS_FALSE;
        }

        asl::SvgPath::PathPoint3f myPathPoint = myNative->nearest(myPosition);

        JSObject * myReturnObject = JS_NewArrayObject(cx, 0, NULL);
        *rval = OBJECT_TO_JSVAL(myReturnObject);
        if (!JS_DefineProperty(cx, myReturnObject, "point", as_jsval(cx, static_cast<asl::Vector3f>(myPathPoint)), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;
        if (!JS_DefineProperty(cx, myReturnObject, "element", as_jsval(cx, myPathPoint.element), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;

        return JS_TRUE;
    }

    static JSBool
    normal(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Find left-vector of path at given position.");
        DOC_PARAM("thePosition", "Position.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theUpVector", "Up-vector.", DOC_TYPE_VECTOR3F);
        DOC_RVAL("{normal, nearest, element}", DOC_TYPE_OBJECT);
        DOC_END;

        ensureParamCount(argc, 1, 2);

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::normal: self is not a SvgPath");
            return JS_FALSE;
        }

        asl::Vector3f myPosition;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPosition)) {
            JS_ReportError(cx, "JSSvgPath::normal: argument #1 must be a vector");
            return JS_FALSE;
        }

        asl::SvgPath::PathNormal myPathNormal;
        if (argc > 1) {
            asl::Vector3f myUpVector;
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myUpVector)) {
                JS_ReportError(cx, "JSSvgPath::normal: argument #2 must be a vector");
                return JS_FALSE;
            }
            myPathNormal = myNative->normal(myPosition, myUpVector);
        } else {
            myPathNormal = myNative->normal(myPosition);
        }

        JSObject * myReturnObject = JS_NewArrayObject(cx, 0, NULL);
        *rval = OBJECT_TO_JSVAL(myReturnObject);
        if (!JS_DefineProperty(cx, myReturnObject, "normal", as_jsval(cx, myPathNormal.normal), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;
        if (!JS_DefineProperty(cx, myReturnObject, "nearest", as_jsval(cx, static_cast<asl::Vector3f>(myPathNormal.nearest)), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;
        if (!JS_DefineProperty(cx, myReturnObject, "element", as_jsval(cx, myPathNormal.nearest.element), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;

        return JS_TRUE;
    }

    static JSBool
    pointOnPath(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Find point on path by percentage of path length.");
        DOC_PARAM("thePercentage", "Percentage [0..1].", DOC_TYPE_FLOAT);
        DOC_PARAM("theUpVector", "Up-vector.", DOC_TYPE_VECTOR3F);
        DOC_RVAL("{point, element}", DOC_TYPE_OBJECT);
        DOC_END;

        ensureParamCount(argc, 1);

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::pointOnPath: self is not a SvgPath");
            return JS_FALSE;
        }

        float myPercentage;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPercentage)) {
            JS_ReportError(cx, "JSSvgPath::pointOnPath: argument #1 must be a float");
            return JS_FALSE;
        }

        asl::SvgPath::PathPoint3f myPathPoint = myNative->pointOnPath(myPercentage);

        JSObject * myReturnObject = JS_NewArrayObject(cx, 0, NULL);
        *rval = OBJECT_TO_JSVAL(myReturnObject);
        if (!JS_DefineProperty(cx, myReturnObject, "point", as_jsval(cx, static_cast<asl::Vector3f>(myPathPoint)), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;
        if (!JS_DefineProperty(cx, myReturnObject, "element", as_jsval(cx, myPathPoint.element), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;

        return JS_TRUE;
    }

    template <class T>
    static JSBool
    intersectHelper(JSContext * cx, JSObject * obj, uintN argc, jsval * argv, jsval * rval) {

        T myObject;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myObject)) {
            JS_ReportError(cx, "JSSvgPath::intersect: argument #1 must be an intersectable object");
            return JS_FALSE;
        }

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::intersect: self is not a SvgPath");
            return JS_FALSE;
        }
        std::vector<asl::SvgPath::PathPoint3f> myResult = myNative->intersect(myObject);

        // convert to simple arrays
        std::vector<asl::Vector3f> myPoints;
        std::vector<unsigned> myElements;
        for (unsigned i = 0; i < myResult.size(); ++i) {
            myPoints.push_back(myResult[i]);
            myElements.push_back(myResult[i].element);
        }

        JSObject * myReturnObject = JS_NewArrayObject(cx, 0, NULL);
        *rval = OBJECT_TO_JSVAL(myReturnObject);
        if (!JS_DefineProperty(cx, myReturnObject, "points", as_jsval(cx, myPoints), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;
        if (!JS_DefineProperty(cx, myReturnObject, "elements", as_jsval(cx, myElements), 0,0, JSPROP_ENUMERATE)) return JS_FALSE;

        return JS_TRUE;
    }

    static JSBool
    intersect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Intersect path with given element.");
        DOC_PARAM("theElement", "Element.", DOC_TYPE_OBJECT);
        DOC_RVAL("{points:[Point3f], elements:[int]}", DOC_TYPE_OBJECT);
        DOC_END;
        try {
            ensureParamCount(argc, 1);

            if (JSLineSegment::matchesClassOf(cx, argv[0])) {
                return intersectHelper<asl::LineSegment<float> >(cx, obj, argc, argv, rval);
            }
            if (JSLine::matchesClassOf(cx, argv[0])) {
                return intersectHelper<asl::Line<float> >(cx, obj, argc, argv, rval);
            }
            if (JSSphere::matchesClassOf(cx, argv[0])) {
                return intersectHelper<asl::Sphere<float> >(cx, obj, argc, argv, rval);
            }
        } HANDLE_CPP_EXCEPTION;
        return JS_FALSE;
    }

    static JSBool
    createPerpendicularPath(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Create a new SvgPath that is perpendicular to this path at the given position.");
        DOC_PARAM("thePosition", "Position.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theLength", "Length of new path.", DOC_TYPE_FLOAT);
        DOC_PARAM("theUpVector", "Up-vector.", DOC_TYPE_VECTOR3F);
        DOC_RVAL("Path", DOC_TYPE_OBJECT);
        DOC_END;
        try {
            ensureParamCount(argc, 2, 3);

            JSSvgPath::NATIVE * myNative = 0;
            if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
                JS_ReportError(cx, "JSSvgPath::createPerpendicularPath: self is not a SvgPath");
                return JS_FALSE;
            }

            asl::Vector3f myPosition;
            float myLength;
            if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPosition)) {
                JS_ReportError(cx, "JSSvgPath::createPerpendicularPath: argument #1 must be a vector");
                return JS_FALSE;
            }
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myLength)) {
                JS_ReportError(cx, "JSSvgPath::createPerpendicularPath: argument #2 must be a float");
                return JS_FALSE;
            }

            asl::SvgPath * myPath = 0;
            if (argc > 2) {
                asl::Vector3f myUpVector;
                if (JSVAL_IS_VOID(argv[2]) || !convertFrom(cx, argv[2], myUpVector)) {
                    JS_ReportError(cx, "JSSvgPath::createPerpendicularPath: argument #3 must be a vector");
                    return JS_FALSE;
                }
                myPath = myNative->createPerpendicularPath(myPosition, myLength, myUpVector);
            } else {
                myPath = myNative->createPerpendicularPath(myPosition, myLength);
            }

            if (!myPath) {
                *rval = JSVAL_NULL;
            } else {
                *rval = as_jsval(cx, asl::Ptr<asl::SvgPath>(myPath));
            }

            return JS_TRUE;
        } HANDLE_CPP_EXCEPTION;
    }

    static JSBool
    createSubPath(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
    {
        DOC_BEGIN("Create a new path along this path from start to end.");
        DOC_PARAM("theStart", "Start position.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theEnd", "End position.", DOC_TYPE_VECTOR3F);
        DOC_PARAM("theShortestPathFlag", "Find shortest path.", DOC_TYPE_BOOLEAN);
        DOC_RVAL("Path", DOC_TYPE_OBJECT);
        DOC_END;

        ensureParamCount(argc, 2, 3);

        JSSvgPath::NATIVE * myNative = 0;
        if (!convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative)) {
            JS_ReportError(cx, "JSSvgPath::createSubPath: self is not a SvgPath");
            return JS_FALSE;
        }

        asl::Vector3f myStart, myEnd;
        if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myStart)) {
            JS_ReportError(cx, "JSSvgPath::createSubPath: argument #1 must be a vector");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], myEnd)) {
            JS_ReportError(cx, "JSSvgPath::createSubPath: argument #2 must be a vector");
            return JS_FALSE;
        }

        asl::SvgPath * myPath = 0;
        if (argc > 2) {
            bool myShortestPathFlag;
            if (JSVAL_IS_VOID(argv[2]) || !convertFrom(cx, argv[2], myShortestPathFlag)) {
                JS_ReportError(cx, "JSSvgPath::createSubPath: argument #3 must be a boolean");
                return JS_FALSE;
            }
            myPath = myNative->createSubPath(myStart, myEnd, myShortestPathFlag);
        } else {
            myPath = myNative->createSubPath(myStart, myEnd);
        }

        if (!myPath) {
            *rval = JSVAL_NULL;
        } else {
            *rval = as_jsval(cx, asl::Ptr<asl::SvgPath>(myPath));
        }
 
        return JS_TRUE;
    }

    JSFunctionSpec * JSSvgPath::StaticFunctions() {
        static JSFunctionSpec myProperties[] = {{0}};
        return myProperties;
    }
 
    JSFunctionSpec * JSSvgPath::Functions() {
        static JSFunctionSpec myFunctions[] = {
            //{"name", native, nargs},
            {"toString", toString,0 },
            { "move",    move,    2 },
            { "line",    line,    2 },
            { "hline",   hline,   2 },
            { "vline",   vline,   2 },
            { "cbezier", cbezier, 3 },
            { "close",   close,   0 },
            { "revert",  revert,  0 },
            { "getNumElements", getNumElements, 0 },
            { "getElement",     getElement,     1 },
            { "nearest",        nearest,        1 },
            { "normal",         normal,         2 },
            { "pointOnPath",    pointOnPath,    1 },
            { "intersect",      intersect,      1 },
            { "createPerpendicularPath", createPerpendicularPath, 3 },
            { "createSubPath",           createSubPath,           3 },
            {0}
        };
        return myFunctions;
    }

    JSPropertySpec * JSSvgPath::StaticProperties() {
        static JSPropertySpec myProperties[] = {{0}};
        return myProperties;
    }

    JSPropertySpec * JSSvgPath::Properties()
    {
        static JSPropertySpec myProperties[] = {
            {"length", PROP_length, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},
            {"numelements", PROP_numelements, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},
            {"segmentlength", PROP_segmentlength, JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},
            {0}
        };
        return myProperties;
    }

    // getproperty handling
    JSBool
    JSSvgPath::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        switch (theID) {
        case PROP_length:
            *vp = as_jsval(cx, getNative().getLength());
            return JS_TRUE;
        case PROP_numelements:
            *vp = as_jsval(cx, getNative().getNumElements());
            break;
        case PROP_segmentlength:
            *vp = as_jsval(cx, getNative().getSegmentLength());
            break;
        default:
            JS_ReportError(cx,"JSSvgPath::getProperty: index %d out of range", theID);
            return JS_FALSE;
        }
        return JS_TRUE;
    };

    // set property
    JSBool
    JSSvgPath::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        jsval dummy;
        switch (theID) {
        case PROP_segmentlength:
            return Method<JSSvgPath::NATIVE>::call(&JSSvgPath::NATIVE::setSegmentLength, cx, obj, 1, vp, &dummy);
        default:
            JS_ReportError(cx,"JSSvgPath::setProperty: index %d out of range", theID);
            return JS_FALSE;
        }
        return JS_TRUE;
    }
    
    JSBool JSSvgPath::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Constructor a new SvgPath object");
        DOC_PARAM("thePathDefinition", "Path definition string", DOC_TYPE_STRING);
        DOC_PARAM("theSegmentLength", "BSpline segmentation length", DOC_TYPE_FLOAT);
        DOC_END;

        if (JSA_GetClass(cx, obj) != Class()) {
            JS_ReportError(cx, "Constructor for %s: bad object; did you forget a 'new'?", ClassName());
            return JS_FALSE;
        }

        std::string myPathDefinition = "";
        OWNERPTR mySvgPath;

        if (argc >= 1) {
            if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myPathDefinition)) {
                JS_ReportError(cx, "JSSvgPath: argument #1 must be a string");
                return JS_FALSE;
            }
        }

        if (argc > 1) {
            float mySegmentLength;
            if (JSVAL_IS_VOID(argv[1]) || !convertFrom(cx, argv[1], mySegmentLength)) {
                JS_ReportError(cx, "JSSvgPath: argument #2 must be a float");
                return JS_FALSE;
            }
            mySvgPath = OWNERPTR(new asl::SvgPath(myPathDefinition, mySegmentLength));
        } else {
            mySvgPath = OWNERPTR(new asl::SvgPath(myPathDefinition));
        }

        JSSvgPath * myNewObject = new JSSvgPath(mySvgPath, &*mySvgPath);
        JS_SetPrivate(cx, obj, myNewObject);
        return JS_TRUE;
    }

    JSObject * JSSvgPath::initClass(JSContext * cx, JSObject * theGlobalObject)
    {
        JSObject * myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions());
        DOC_CREATE(JSSvgPath);

        return myClass;
    }

    bool convertFrom(JSContext *cx, jsval theValue, JSSvgPath::NATIVE & theSvgPath) {
        if (JSVAL_IS_OBJECT(theValue)) {
            JSObject * myArgument;
            if (JS_ValueToObject(cx, theValue, &myArgument)) {
                if (JSA_GetClass(cx,myArgument) == JSClassTraits<asl::SvgPath>::Class()) {
                    theSvgPath = JSClassTraits<asl::SvgPath>::getNativeRef(cx, myArgument);
                    return true;
                }
            }
        }
        return false;
    }

    bool convertFrom(JSContext *cx, jsval theValue, JSSvgPath::NATIVE *& theSvgPath) {
        if (JSVAL_IS_OBJECT(theValue)) {
            JSObject * myArgument;
            if (JS_ValueToObject(cx, theValue, &myArgument)) {
                if (JSA_GetClass(cx,myArgument) == JSClassTraits<asl::SvgPath>::Class()) {
                    typedef JSClassTraits<asl::SvgPath>::ScopedNativeRef NativeRef;
                    NativeRef myObj(cx, myArgument);
                    theSvgPath = &myObj.getNative();
                    return true;
                }
            }
        }
        return false;
    }

    jsval as_jsval(JSContext *cx, JSSvgPath::OWNERPTR theOwner) {
        JSObject * myReturnObject = JSSvgPath::Construct(cx, theOwner, &*theOwner);
        return OBJECT_TO_JSVAL(myReturnObject);
    }

    jsval as_jsval(JSContext *cx, JSSvgPath::OWNERPTR theOwner, JSSvgPath::NATIVE * theSvgPath) {
        JSObject * myReturnObject = JSSvgPath::Construct(cx, theOwner, theSvgPath);
        return OBJECT_TO_JSVAL(myReturnObject);
    }

}
