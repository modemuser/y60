
set(SOURCES
AnimationBase.cpp
AnimationBuilder.cpp
AnimationCharacter.cpp
AnimationClip.cpp
AnimationManager.cpp
Body.cpp
BodyBuilder.cpp
BuilderBase.cpp
Camera.cpp
CameraBuilder.cpp
Canvas.cpp
CanvasBuilder.cpp
CharacterBuilder.cpp
ClipBuilder.cpp
Combiner.cpp
ElementBuilder.cpp
Geometry.cpp
ImageBuilder.cpp
JointBuilder.cpp
Light.cpp
LightBuilder.cpp
LightSource.cpp
LightSourceBuilder.cpp
MaterialBase.cpp
MaterialBuilder.cpp
MovieBuilder.cpp
Primitive.cpp
Scene.cpp
SceneBuilder.cpp
SceneOptimizer.cpp
Shape.cpp
ShapeBuilder.cpp
Texture.cpp
TextureBuilder.cpp
TextureManager.cpp
TextureUnit.cpp
TransformBuilder.cpp
TransformBuilderBase.cpp
TransformHierarchyFacade.cpp
Viewport.cpp
WorldBuilder.cpp
WorldBuilderBase.cpp
Y60Decoder.cpp
)

set(HEADERS
Animation.h
AnimationBase.h
AnimationBuilder.h
AnimationCharacter.h
AnimationClip.h
AnimationManager.h
Body.h
BodyBuilder.h
BuilderBase.h
Camera.h
CameraBuilder.h
Canvas.h
CanvasBuilder.h
CharacterBuilder.h
ClipBuilder.h
Combiner.h
ElementBuilder.h
Facades.h
Geometry.h
IFrameBuffer.h
IProgressNotifier.h
ISceneDecoder.h
IShader.h
ImageBuilder.h
IncludeFacade.h
JointBuilder.h
Light.h
LightBuilder.h
LightSource.h
LightSourceBuilder.h
LodFacade.h
MaterialBase.h
MaterialBuilder.h
MaterialParameter.h
MaterialTypes.h
MemoryResourceManager.h
MovieBuilder.h
Overlay.h
Primitive.h
PropertyListFacade.h
Record.h
ResourceManager.h
Scene.h
SceneBuilder.h
SceneOptimizer.h
Shape.h
ShapeBuilder.h
Texture.h
TextureBuilder.h
TextureManager.h
TextureTags.h
TextureUnit.h
TransformBuilder.h
TransformBuilderBase.h
TransformHierarchyFacade.h
VertexData.h
VertexDataImpl.h
VertexMemoryBase.h
Viewport.h
WorldBuilder.h
WorldBuilderBase.h
Y60Decoder.h
)

set(TESTS
AnimationBuilder
Camera
LightSourceBuilder
Lights
Scene
ShapeBuilder
)

if(OSX)
endif(OSX)

if(LINUX)
endif(LINUX)

if(WIN32)
endif(WIN32)

ac_add_library(
    y60scene y60/scene
    SOURCES ${SOURCES}
    HEADERS ${HEADERS}
    DEPENDS aslbase asldom aslxpath aslmath aslzip y60base y60image y60video
    EXTERNS GLEW OPENEXR PAINTLIB
    TESTS   ${TESTS}
)
