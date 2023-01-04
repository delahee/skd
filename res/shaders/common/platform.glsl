#ifndef _HBC_GL_PLATFORM
#define _HBC_GL_PLATFORM

#ifdef PASTA_NX // define NX specfic
    #define UNIFORM
    #define BEGIN_UNIFORM_BLOCK uniform MainUBlock {
    #define END_UNIFORM_BLOCK };
#else // define PC
    #define UNIFORM uniform
    #define BEGIN_UNIFORM_BLOCK
    #define END_UNIFORM_BLOCK
#endif

#endif // _HBC_GL_PLATFORM