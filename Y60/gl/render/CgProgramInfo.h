//=============================================================================
// Copyright (C) 1993-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _ac_renderer_CgProgramInfo_h_
#define _ac_renderer_CgProgramInfo_h_

#include "ShaderDescription.h"

#include <y60/scene/Viewport.h>
#include <y60/scene/Camera.h>
#include <y60/scene/Body.h>
#include <y60/scene/Light.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <string>
#include <vector>

namespace y60 {

   //=== Cg-GL specific parameter names  ============================================
    // these can be used with a _I, _T or _IT suffix to get the
    // inverted, transposed or interverted-transposed matrix
    static const char * CG_GL_MODELVIEW_PARAMETER           = "GL_MODELVIEW";
    static const char * CG_GL_PROJECTION_PARAMETER          = "GL_PROJECTION";
    static const char * CG_GL_TEXTURE_PARAMETER             = "GL_TEXTURE";
    static const char * CG_GL_MODELVIEW_PROJECTION_PARAMETER = "GL_MODELVIEW_PROJECTION";

    //=== AC CG automatic paramters  ============================================
    enum CgAutoParameterID {
        OBJECTWORLD,OBJECTWORLD_I,OBJECTWORLD_T,OBJECTWORLD_IT,
        CAMERA_POSITION, CAMERA_I, CAMERA_T, VIEWPROJECTION,
        POSITIONAL_LIGHTS, DIRECTIONAL_LIGHTS, SPOT_LIGHTS, 
        POSITIONAL_LIGHTS_DIFFUSE_COLOR, DIRECTIONAL_LIGHTS_DIFFUSE_COLOR, SPOT_LIGHTS_DIFFUSE_COLOR,
        POSITIONAL_LIGHTS_SPECULAR_COLOR, DIRECTIONAL_LIGHTS_SPECULAR_COLOR, SPOT_LIGHTS_SPECULAR_COLOR,
        AMBIENT_LIGHT_COLOR, TEXTURE_MATRICES,  
        SPOT_LIGHTS_CUTOFF,
        SPOT_LIGHTS_EXPONENT,
        SPOT_LIGHTS_DIRECTION,
        SPOT_LIGHTS_ATTENUATION,
        MAX_AUTO_PARAMETER
    };
    static const char * CgAutoParameterString[] = {
        "AC_OBJECTWORLD",
        "AC_OBJECTWORLD_I",
        "AC_OBJECTWORLD_T",
        "AC_OBJECTWORLD_IT",
        "AC_CAMERA_POSITION",
        "AC_CAMERA_I",
        "AC_CAMERA_T",
        "AC_VIEWPROJECTION",
        // light position
        "AC_POSITIONAL_LIGHTS",
        "AC_DIRECTIONAL_LIGHTS",
        "AC_SPOT_LIGHTS",
        // light diffuse color
        "AC_POSITIONAL_LIGHTS_DIFFUSE_COLOR",
        "AC_DIRECTIONAL_LIGHTS_DIFFUSE_COLOR",
        "AC_SPOT_LIGHTS_DIFFUSE_COLOR",
        // light specular color
        "AC_POSITIONAL_LIGHTS_SPECULAR_COLOR",
        "AC_DIRECTIONAL_LIGHTS_SPECULAR_COLOR",
        "AC_SPOT_LIGHTS_SPECULAR_COLOR",
        // ambient color
        "AC_AMBIENT_LIGHT_COLOR",
        "AC_TEXTURE_MATRICES",
        // spot parameters
        "AC_SPOT_LIGHTS_CUTOFF",
        "AC_SPOT_LIGHTS_EXPONENT",
        "AC_SPOT_LIGHTS_DIRECTION",
        "AC_SPOT_LIGHTS_ATTENUATION",
        0
    };

    class Renderer;

    void assertCg(const std::string & theWhere, const CGcontext theCgContext);

    class CgProgramInfo {
        public:
            CgProgramInfo(const ShaderDescription & theShader,
                    const CGcontext theCgContext);
            virtual ~CgProgramInfo();
            void load();
            CGprogram getCgProgramID() { return _myCgProgramID; }

            void enableProfile();
            void disableProfile();
            void enableTextures();
            void disableTextures();
            static CGprofile asCgProfile(const ShaderDescription & theShader);

            void bindMaterialParams(const MaterialBase & theMaterial);
            void bindBodyParams(const MaterialBase & theMaterial,
                        const y60::LightVector & theLightInstances,
                        const Viewport & theViewport,
                        const y60::Body & theBody,
                        const y60::Camera & theCamera);
            void bindOverlayParams();

            void bind();
            bool reloadIfRequired(const y60::LightVector & theLightInstances,
                                  const MaterialBase & theMaterial);
        private:
            static void setCgMaterialParameter(CGparameter & theCgParameter,
                    const dom::Node & theNode, const std::string & thePropertyName,
                    const MaterialBase & theMaterial);

            void createAndCompileProgram();

            void setCGGLParameters();

            struct CgProgramGlParam {
                CgProgramGlParam(const std::string & myParamName,
                                 CGparameter myParameter,
                                 CGGLenum myStateMatrixType,
                                 CGGLenum myTransform)
                    : _myParamName(myParamName),
                      _myParameter(myParameter),
                      _myStateMatrixType(myStateMatrixType),
                      _myTransform(myTransform)
                    {};
                std::string _myParamName;
                CGparameter _myParameter;
                CGGLenum _myStateMatrixType;
                CGGLenum _myTransform;
            };
            typedef std::vector<CgProgramGlParam> CgProgramGlParamVector;

            struct CgProgramAutoParam {
                CgProgramAutoParam() {}
                CgProgramAutoParam(const std::string & myName,
                                 CGparameter myParameter,
                                 CgAutoParameterID myID,
                                 CGtype myType)
                    : _myName(myName),
                      _myParameter(myParameter),
                      _myID(myID),
                      _myType(myType)
                    {};
                std::string         _myName;
                CGparameter         _myParameter;
                CgAutoParameterID   _myID;
                CGtype              _myType;
            };
            typedef std::map<int,CgProgramAutoParam> CgProgramAutoParams;


            struct CgProgramNamedParam {
                CgProgramNamedParam(const std::string & myParamName,
                                 CGparameter myParameter)
                    : _myParamName(myParamName),
                      _myParameter(myParameter)
                    {};
                std::string         _myParamName;
                CGparameter         _myParameter;
            };
            typedef std::vector<CgProgramNamedParam> CgProgramNamedParamVector;

            CgProgramInfo();
            CgProgramInfo(const CgProgramInfo &); // disable copy constructor
            CgProgramInfo & operator=(const CgProgramInfo &);     // disable assignment op

            void processParameters();
            void setCgVectorParameter(const CgProgramAutoParam & theParam, const asl::Vector3f & theValue);
            void setCgVectorParameter(const CgProgramAutoParam & theParam, const asl::Vector4f & theValue);
            void setCgMatrixParameter(const CgProgramAutoParam & theParam, const asl::Matrix4f & theValue);
            void setCgArrayVector3fParameter(const CgProgramAutoParam & theParam, const std::vector<asl::Vector3f> & theValue);
            void setCgArrayVector4fParameter(const CgProgramAutoParam & theParam, const std::vector<asl::Vector4f> & theValue);

            void setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam, const std::vector<asl::Vector3f> & theValue);
            void setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam, const std::vector<asl::Vector4f> & theValue);
            void setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam, const std::vector<float> & theValue);

            ShaderDescription           _myShader;
            CGprogram                   _myCgProgramID;
            CGcontext                   _myContext;
            std::string                 _myPathName;
            CgProgramGlParamVector      _myGlParams;       // GL-State to CG parameters
            CgProgramAutoParams         _myAutoParams;     // other automatic CG parameters (e.g. CameraPosition)
            CgProgramNamedParamVector   _myTextureParams;  // Texture parameters
            CgProgramNamedParamVector   _myMiscParams;     // material parameter
            
            std::map<int,int> _myUnsizedArrayAutoParamSizes;

            std::vector<const char *> _myCachedCompilerArgs;
            std::string _myCgProgramString;
    };

    typedef asl::Ptr<CgProgramInfo> CgProgramInfoPtr;
    typedef std::vector<CgProgramInfoPtr> CgProgramVector;
}

#endif
