#include "gl_loader.hpp"

#include <cstdio>

namespace pvzgl {

PFNGLCLEARCOLOR glClearColor = nullptr;
PFNGLCLEAR glClear = nullptr;
PFNGLVIEWPORT glViewport = nullptr;
PFNGLSCISSOR glScissor = nullptr;
PFNGLENABLE glEnable = nullptr;
PFNGLDISABLE glDisable = nullptr;
PFNGLBLENDFUNC glBlendFunc = nullptr;
PFNGLDRAWARRAYS glDrawArrays = nullptr;
PFNGLREADPIXELS glReadPixels = nullptr;
PFNGLCOPYTEXSUBIMAGE2D glCopyTexSubImage2D = nullptr;
PFNGLGETINTEGERV glGetIntegerv = nullptr;
PFNGLGENTEXTURES glGenTextures = nullptr;
PFNGLBINDTEXTURE glBindTexture = nullptr;
PFNGLTEXPARAMETERI glTexParameteri = nullptr;
PFNGLTEXIMAGE2D glTexImage2D = nullptr;
PFNGLTEXSUBIMAGE2D glTexSubImage2D = nullptr;
PFNGLDELETETEXTURES glDeleteTextures = nullptr;
PFNGLACTIVETEXTURE glActiveTexture = nullptr;
PFNGLGENBUFFERS glGenBuffers = nullptr;
PFNGLBINDBUFFER glBindBuffer = nullptr;
PFNGLBUFFERDATA glBufferData = nullptr;
PFNGLBUFFERSUBDATA glBufferSubData = nullptr;
PFNGLDELETEBUFFERS glDeleteBuffers = nullptr;
PFNGLCREATESHADER glCreateShader = nullptr;
PFNGLSHADERSOURCE glShaderSource = nullptr;
PFNGLCOMPILESHADER glCompileShader = nullptr;
PFNGLGETSHADERIV glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOG glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAM glCreateProgram = nullptr;
PFNGLATTACHSHADER glAttachShader = nullptr;
PFNGLLINKPROGRAM glLinkProgram = nullptr;
PFNGLGETPROGRAMIV glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOG glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAM glUseProgram = nullptr;
PFNGLDELETESHADER glDeleteShader = nullptr;
PFNGLDELETEPROGRAM glDeleteProgram = nullptr;
PFNGLGETATTRIBLOCATION glGetAttribLocation = nullptr;
PFNGLGETUNIFORMLOCATION glGetUniformLocation = nullptr;
PFNGLUNIFORM1I glUniform1i = nullptr;
PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv = nullptr;
PFNGLVERTEXATTRIBPOINTER glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAY glDisableVertexAttribArray = nullptr;

namespace {

int gMissing = 0;

void *load(const char *name)
{
    void *p = SDL_GL_GetProcAddress(name);
    if (p == nullptr)
    {
        printf("pvzgl: missing GL function %s\n", name);
        gMissing++;
    }
    return p;
}

#define LOAD(Name) Name = reinterpret_cast<decltype(Name)>(load(#Name))

} // namespace

bool init()
{
    // Not strictly required (SDL binds its default GL library on demand),
    // but makes failures explicit and early.
    SDL_GL_LoadLibrary(nullptr);

    LOAD(glClearColor);
    LOAD(glClear);
    LOAD(glViewport);
    LOAD(glScissor);
    LOAD(glEnable);
    LOAD(glDisable);
    LOAD(glBlendFunc);
    LOAD(glDrawArrays);
    LOAD(glReadPixels);
    LOAD(glCopyTexSubImage2D);
    LOAD(glGetIntegerv);
    LOAD(glGenTextures);
    LOAD(glBindTexture);
    LOAD(glTexParameteri);
    LOAD(glTexImage2D);
    LOAD(glTexSubImage2D);
    LOAD(glDeleteTextures);
    LOAD(glActiveTexture);
    LOAD(glGenBuffers);
    LOAD(glBindBuffer);
    LOAD(glBufferData);
    LOAD(glBufferSubData);
    LOAD(glDeleteBuffers);
    LOAD(glCreateShader);
    LOAD(glShaderSource);
    LOAD(glCompileShader);
    LOAD(glGetShaderiv);
    LOAD(glGetShaderInfoLog);
    LOAD(glCreateProgram);
    LOAD(glAttachShader);
    LOAD(glLinkProgram);
    LOAD(glGetProgramiv);
    LOAD(glGetProgramInfoLog);
    LOAD(glUseProgram);
    LOAD(glDeleteShader);
    LOAD(glDeleteProgram);
    LOAD(glGetAttribLocation);
    LOAD(glGetUniformLocation);
    LOAD(glUniform1i);
    LOAD(glUniformMatrix4fv);
    LOAD(glVertexAttribPointer);
    LOAD(glEnableVertexAttribArray);
    LOAD(glDisableVertexAttribArray);
    return gMissing == 0;
}

} // namespace pvzgl
