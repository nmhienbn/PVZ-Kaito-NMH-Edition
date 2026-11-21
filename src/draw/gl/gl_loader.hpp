#pragma once

// Minimal OpenGL 2.1 function loader. Every GL symbol is resolved at runtime
// through SDL_GL_GetProcAddress, so the binary has no link dependency on
// -lGL / -lopengl32 (this machine ships only the libGL.so.1 runtime).
// GL types/constants needed by the sprite batch are defined here instead of
// including <SDL_opengl.h>, avoiding clashes with its prototypes.

#include <SDL.h>

#include <stddef.h>
#include <stdint.h>

namespace pvzgl {

typedef unsigned int GLenum_t;
typedef unsigned char GLboolean_t;
typedef unsigned int GLbitfield_t;
typedef int8_t GLbyte_t;
typedef short GLshort_t;
typedef int GLint_t;
typedef int GLsizei_t;
typedef unsigned char GLubyte_t;
typedef unsigned short GLushort_t;
typedef unsigned int GLuint_t;
typedef float GLfloat_t;
typedef double GLdouble_t;
typedef ptrdiff_t GLsizeiptr_t;
typedef char GLchar_t;

// Constants (values from the OpenGL 2.1 / glext headers).
enum
{
    PVZ_GL_FALSE = 0,
    PVZ_GL_TRUE = 1,
    PVZ_GL_TRIANGLES = 0x0004,
    PVZ_GL_UNSIGNED_BYTE = 0x1401,
    PVZ_GL_FLOAT = 0x1406,
    PVZ_GL_RGB = 0x1907,
    PVZ_GL_RGBA = 0x1908,
    PVZ_GL_COLOR_BUFFER_BIT = 0x4000,
    PVZ_GL_BLEND = 0x0BE2,
    PVZ_GL_DEPTH_TEST = 0x0B71,
    PVZ_GL_SCISSOR_TEST = 0x0C11,
    PVZ_GL_TEXTURE_2D = 0x0DE1,
    PVZ_GL_SRC_ALPHA = 0x0302,
    PVZ_GL_ONE_MINUS_SRC_ALPHA = 0x0303,
    PVZ_GL_TEXTURE_MAG_FILTER = 0x2800,
    PVZ_GL_TEXTURE_MIN_FILTER = 0x2801,
    PVZ_GL_TEXTURE_WRAP_S = 0x2802,
    PVZ_GL_TEXTURE_WRAP_T = 0x2803,
    PVZ_GL_NEAREST = 0x2600,
    PVZ_GL_LINEAR = 0x2601,
    PVZ_GL_CLAMP_TO_EDGE = 0x812F,
    PVZ_GL_TEXTURE0 = 0x84C0,
    PVZ_GL_ARRAY_BUFFER = 0x8892,
    PVZ_GL_STREAM_DRAW = 0x88E0,
    PVZ_GL_FRAGMENT_SHADER = 0x8B30,
    PVZ_GL_VERTEX_SHADER = 0x8B31,
    PVZ_GL_COMPILE_STATUS = 0x8B81,
    PVZ_GL_LINK_STATUS = 0x8B82,
    PVZ_GL_INFO_LOG_LENGTH = 0x8B84,
    PVZ_GL_MAX_TEXTURE_SIZE = 0x0D33,
};

// Function pointer typedefs (GL 1.1 + buffers/shaders up to 2.1).
typedef void (*PFNGLCLEARCOLOR)(GLfloat_t, GLfloat_t, GLfloat_t, GLfloat_t);
typedef void (*PFNGLCLEAR)(GLbitfield_t);
typedef void (*PFNGLVIEWPORT)(GLint_t, GLint_t, GLsizei_t, GLsizei_t);
typedef void (*PFNGLSCISSOR)(GLint_t, GLint_t, GLsizei_t, GLsizei_t);
typedef void (*PFNGLENABLE)(GLenum_t);
typedef void (*PFNGLDISABLE)(GLenum_t);
typedef void (*PFNGLBLENDFUNC)(GLenum_t, GLenum_t);
typedef void (*PFNGLDRAWARRAYS)(GLenum_t, GLint_t, GLsizei_t);
typedef void (*PFNGLREADPIXELS)(GLint_t, GLint_t, GLsizei_t, GLsizei_t,
                                GLenum_t, GLenum_t, void *);
typedef void (*PFNGLCOPYTEXSUBIMAGE2D)(GLenum_t, GLint_t, GLint_t, GLint_t,
                                       GLint_t, GLint_t, GLsizei_t, GLsizei_t);
typedef void (*PFNGLGETINTEGERV)(GLenum_t, GLint_t *);
typedef void (*PFNGLGENTEXTURES)(GLsizei_t, GLuint_t *);
typedef void (*PFNGLBINDTEXTURE)(GLenum_t, GLuint_t);
typedef void (*PFNGLTEXPARAMETERI)(GLenum_t, GLenum_t, GLint_t);
typedef void (*PFNGLTEXIMAGE2D)(GLenum_t, GLint_t, GLint_t, GLsizei_t, GLsizei_t,
                                GLint_t, GLenum_t, GLenum_t, const void *);
typedef void (*PFNGLTEXSUBIMAGE2D)(GLenum_t, GLint_t, GLint_t, GLint_t, GLsizei_t,
                                   GLsizei_t, GLenum_t, GLenum_t, const void *);
typedef void (*PFNGLDELETETEXTURES)(GLsizei_t, const GLuint_t *);
typedef void (*PFNGLACTIVETEXTURE)(GLenum_t);
typedef void (*PFNGLGENBUFFERS)(GLsizei_t, GLuint_t *);
typedef void (*PFNGLBINDBUFFER)(GLenum_t, GLuint_t);
typedef void (*PFNGLBUFFERDATA)(GLenum_t, GLsizeiptr_t, const void *, GLenum_t);
typedef void (*PFNGLBUFFERSUBDATA)(GLenum_t, ptrdiff_t, GLsizeiptr_t, const void *);
typedef void (*PFNGLDELETEBUFFERS)(GLsizei_t, const GLuint_t *);
typedef GLuint_t (*PFNGLCREATESHADER)(GLenum_t);
typedef void (*PFNGLSHADERSOURCE)(GLuint_t, GLsizei_t, const GLchar_t *const *, const GLint_t *);
typedef void (*PFNGLCOMPILESHADER)(GLuint_t);
typedef void (*PFNGLGETSHADERIV)(GLuint_t, GLenum_t, GLint_t *);
typedef void (*PFNGLGETSHADERINFOLOG)(GLuint_t, GLsizei_t, GLsizei_t *, GLchar_t *);
typedef GLuint_t (*PFNGLCREATEPROGRAM)(void);
typedef void (*PFNGLATTACHSHADER)(GLuint_t, GLuint_t);
typedef void (*PFNGLLINKPROGRAM)(GLuint_t);
typedef void (*PFNGLGETPROGRAMIV)(GLuint_t, GLenum_t, GLint_t *);
typedef void (*PFNGLGETPROGRAMINFOLOG)(GLuint_t, GLsizei_t, GLsizei_t *, GLchar_t *);
typedef void (*PFNGLUSEPROGRAM)(GLuint_t);
typedef void (*PFNGLDELETESHADER)(GLuint_t);
typedef void (*PFNGLDELETEPROGRAM)(GLuint_t);
typedef GLint_t (*PFNGLGETATTRIBLOCATION)(GLuint_t, const GLchar_t *);
typedef GLint_t (*PFNGLGETUNIFORMLOCATION)(GLuint_t, const GLchar_t *);
typedef void (*PFNGLUNIFORM1I)(GLint_t, GLint_t);
typedef void (*PFNGLUNIFORMMATRIX4FV)(GLint_t, GLsizei_t, GLboolean_t, const GLfloat_t *);
typedef void (*PFNGLVERTEXATTRIBPOINTER)(GLuint_t, GLint_t, GLenum_t, GLboolean_t,
                                         GLsizei_t, const void *);
typedef void (*PFNGLENABLEVERTEXATTRIBARRAY)(GLuint_t);
typedef void (*PFNGLDISABLEVERTEXATTRIBARRAY)(GLuint_t);

extern PFNGLCLEARCOLOR glClearColor;
extern PFNGLCLEAR glClear;
extern PFNGLVIEWPORT glViewport;
extern PFNGLSCISSOR glScissor;
extern PFNGLENABLE glEnable;
extern PFNGLDISABLE glDisable;
extern PFNGLBLENDFUNC glBlendFunc;
extern PFNGLDRAWARRAYS glDrawArrays;
extern PFNGLREADPIXELS glReadPixels;
extern PFNGLCOPYTEXSUBIMAGE2D glCopyTexSubImage2D;
extern PFNGLGETINTEGERV glGetIntegerv;
extern PFNGLGENTEXTURES glGenTextures;
extern PFNGLBINDTEXTURE glBindTexture;
extern PFNGLTEXPARAMETERI glTexParameteri;
extern PFNGLTEXIMAGE2D glTexImage2D;
extern PFNGLTEXSUBIMAGE2D glTexSubImage2D;
extern PFNGLDELETETEXTURES glDeleteTextures;
extern PFNGLACTIVETEXTURE glActiveTexture;
extern PFNGLGENBUFFERS glGenBuffers;
extern PFNGLBINDBUFFER glBindBuffer;
extern PFNGLBUFFERDATA glBufferData;
extern PFNGLBUFFERSUBDATA glBufferSubData;
extern PFNGLDELETEBUFFERS glDeleteBuffers;
extern PFNGLCREATESHADER glCreateShader;
extern PFNGLSHADERSOURCE glShaderSource;
extern PFNGLCOMPILESHADER glCompileShader;
extern PFNGLGETSHADERIV glGetShaderiv;
extern PFNGLGETSHADERINFOLOG glGetShaderInfoLog;
extern PFNGLCREATEPROGRAM glCreateProgram;
extern PFNGLATTACHSHADER glAttachShader;
extern PFNGLLINKPROGRAM glLinkProgram;
extern PFNGLGETPROGRAMIV glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOG glGetProgramInfoLog;
extern PFNGLUSEPROGRAM glUseProgram;
extern PFNGLDELETESHADER glDeleteShader;
extern PFNGLDELETEPROGRAM glDeleteProgram;
extern PFNGLGETATTRIBLOCATION glGetAttribLocation;
extern PFNGLGETUNIFORMLOCATION glGetUniformLocation;
extern PFNGLUNIFORM1I glUniform1i;
extern PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv;
extern PFNGLVERTEXATTRIBPOINTER glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAY glDisableVertexAttribArray;

// Loads every function above through SDL_GL_GetProcAddress.
// Returns false when any required entry point is missing.
bool init();

} // namespace pvzgl
