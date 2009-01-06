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

#ifndef _scene_Viewport_h_
#define _scene_Viewport_h_

#include <y60/base/CommonTags.h>
#include <y60/base/NodeNames.h>
#include <y60/base/DataTypes.h>
#include <asl/dom/Facade.h>
#include <asl/dom/AttributePlug.h>
#include <asl/math/Frustum.h>

namespace y60 {

    //                  theTagName                  theType        theAttributeName                      theDefault
    DEFINE_ATTRIBUT_TAG(ViewportOrientationTag,     std::string,   ORIENTATION_ATTRIB,              LANDSCAPE_ORIENTATION);
    DEFINE_ATTRIBUT_TAG(CameraTag,                  std::string,   CAMERA_ATTRIB,                   "");
    DEFINE_ATTRIBUT_TAG(Size2DTag,                  asl::Vector2f, VIEWPORT_SIZE_ATTRIB,            asl::Vector2f(1,1));
    DEFINE_ATTRIBUT_TAG(ViewportLeftTag,            int,           VIEWPORT_LEFT_ATTRIB,            0);
    DEFINE_ATTRIBUT_TAG(ViewportTopTag,             int,           VIEWPORT_TOP_ATTRIB,             0);
    DEFINE_ATTRIBUT_TAG(ViewportHeightTag,          unsigned,      VIEWPORT_HEIGHT_ATTRIB,          0);
    DEFINE_ATTRIBUT_TAG(ViewportWidthTag,           unsigned,      VIEWPORT_WIDTH_ATTRIB,           0);

    DEFINE_ATTRIBUT_TAG(ViewportBackfaceCullingTag, bool,          VIEWPORT_BACKFACECULLING_ATTRIB, true);
    DEFINE_ATTRIBUT_TAG(ViewportWireframeTag,       bool,          VIEWPORT_WIREFRAME_ATTRIB,       false);
    DEFINE_ATTRIBUT_TAG(ViewportLightingTag,        bool,          VIEWPORT_LIGHTING_ATTRIB,        true);
    DEFINE_ATTRIBUT_TAG(ViewportFlatshadingTag,     bool,          VIEWPORT_FLATSHADING_ATTRIB,     false);
    DEFINE_ATTRIBUT_TAG(ViewportCullingTag,         bool,          VIEWPORT_CULLING_ATTRIB,         true);
    DEFINE_ATTRIBUT_TAG(ViewportDebugCullingTag,    bool,          VIEWPORT_DEBUGCULLING_ATTRIB,    false);
    DEFINE_ATTRIBUT_TAG(ViewportTexturingTag,       bool,          VIEWPORT_TEXTURING_ATTRIB,       true);
    DEFINE_ATTRIBUT_TAG(ViewportAlphaTestTag,       bool,          VIEWPORT_ALPHATEST_ATTRIB,
    true);
    DEFINE_ATTRIBUT_TAG(ViewportDrawGlowTag,        bool,          VIEWPORT_GLOW_ATTRIB,            false);
    DEFINE_ATTRIBUT_TAG(ViewportDrawNormalsTag,     bool,          VIEWPORT_DRAWNORMALS_ATTRIB,     false);

    class Viewport :
        public dom::Facade,
        public IdTag::Plug,
		public NameTag::Plug,
		public CameraTag::Plug,
        public Position2DTag::Plug,
        public Size2DTag::Plug,
		public ViewportOrientationTag::Plug,
		public ViewportBackfaceCullingTag::Plug,
		public ViewportWireframeTag::Plug,
		public ViewportLightingTag::Plug,
		public ViewportTexturingTag::Plug,
		public ViewportFlatshadingTag::Plug,
		public ViewportCullingTag::Plug,
		public ViewportDebugCullingTag::Plug,
        public ViewportAlphaTestTag::Plug,
        public ViewportDrawNormalsTag::Plug,
        public ViewportDrawGlowTag::Plug,
        public dom::DynamicAttributePlug<ViewportTopTag, Viewport>,
        public dom::DynamicAttributePlug<ViewportLeftTag, Viewport>,
        public dom::DynamicAttributePlug<ViewportHeightTag, Viewport>,
        public dom::DynamicAttributePlug<ViewportWidthTag, Viewport>,
        public ResizePolicyTag::Plug
    {
        public:
            Viewport(dom::Node & theNode) :
              dom::Facade(theNode),
                  IdTag::Plug(theNode),
                  NameTag::Plug(theNode),
                  Position2DTag::Plug(theNode),
                  Size2DTag::Plug(theNode),
		          CameraTag::Plug(theNode),
                  ViewportOrientationTag::Plug(theNode),
                  ViewportBackfaceCullingTag::Plug(theNode),
                  ViewportWireframeTag::Plug(theNode),
                  ViewportLightingTag::Plug(theNode),
                  ViewportFlatshadingTag::Plug(theNode),
                  ViewportTexturingTag::Plug(theNode),
                  ViewportCullingTag::Plug(theNode),
                  ViewportDebugCullingTag::Plug(theNode),
                  ViewportAlphaTestTag::Plug(theNode),
                  ViewportDrawGlowTag::Plug(theNode),
                  ViewportDrawNormalsTag::Plug(theNode),
                  dom::DynamicAttributePlug<ViewportTopTag, Viewport>(this, &Viewport::getTop),
                  dom::DynamicAttributePlug<ViewportLeftTag, Viewport>(this, &Viewport::getLeft),
                  dom::DynamicAttributePlug<ViewportHeightTag, Viewport>(this, &Viewport::getHeight),
                  dom::DynamicAttributePlug<ViewportWidthTag, Viewport>(this, &Viewport::getWidth),
                  ResizePolicyTag::Plug( theNode )
              {
              }
            IMPLEMENT_DYNAMIC_FACADE(Viewport);
            /// returns the distance between bottom of viewport and bottom of
            //  canvas. Suitable for glViewport.
            int getLower() const; 
            void applyAspectToCamera();
        private:
            bool getTop(int & theTop) const;
            bool getLeft(int & theLeft) const;
            bool getHeight(unsigned & theHeight) const;
            bool getWidth(unsigned & theWidth) const;

    };

    typedef asl::Ptr<Viewport, dom::ThreadingModel> ViewportPtr;
}

#endif