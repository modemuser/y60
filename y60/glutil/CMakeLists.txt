
set(SOURCES
GLAlloc.cpp
GLBufferAdapter.cpp
GLContext.cpp
GLUtils.cpp
OffscreenBuffer.cpp
PixelEncodingInfo.cpp
RenderState.cpp
TextureCompressor.cpp
glExtensions.cpp
)

set(HEADERS
FrameBuffer.h
GLAlloc.h
GLBufferAdapter.h
GLContext.h
GLUtils.h
IGLContextManager.h
OffscreenBuffer.h
PixelEncodingInfo.h
RenderState.h
ScopedGLContext.h
TextureCompressor.h
VertexArray.h
VertexBufferObject.h
VertexMemory.h
VertexVector.h
glExtensions.h
)

set(TESTS
)

ac_add_library(
    y60glutil y60/glutil
    SOURCES ${SOURCES}
    HEADERS ${HEADERS}
    DEPENDS aslbase aslmath asldom aslzip
            y60base y60scene y60image y60video
    EXTERNS GLEW OPENEXR PAINTLIB CG
    TESTS   ${TESTS}
)
