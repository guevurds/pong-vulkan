#pragma once

#ifndef _WIN64
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include <time.h>

#ifndef MY_VULKAN
#include <GL/glew.h>
#endif

#include "my_types.h"

using namespace std;

bool ReadFile(const char* fileName, string& outFile); // & indica que espera receber uma refencia, para poder modificar a variavel recebida.
char* ReadBinaryFile(const char* pFileName, int& size); //para ler arquivo binario e retornar o ponteiro

void WriteBinaryFile(const char* pFileName, const void* pData, int size);

void MyError(const char* pFileName, uint line, const char* msg, ...);
// void MyFileError(const char* pFileName, uint line, const char* pFileError);

#define MY_ERROR0(msg) MyError(__FILE__, __LINE__, msg)
#define MY_ERROR(msg, ...) MyError(__FILE__, __LINE__, msg, __VA_ARGS__) //... permite um numero variado de parametros e __VA_ARGS__ captura esse numero variavel após o msg 
// #define MY_FILE_ERROR(FileError) MyFileError(__FILE__, __LINE__, FileError)

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0] * a.size()))

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifdef _WIN64
#define SNPRINTF _snprintf_s
#define VSNPRINTF _vsnprintf_s // versão segura da função _vsnprintf (evita overglow de buffer) 
#define RANDOM rand
#define SRANDOM srand((unsigned)time(NULL)) // time(NULL)=tempo atual em segundos (unsined) garante que o tempo atual seja positivo srand(seed) gera um numero aleatorio, se o numero for o mesmo a sequencia de numeros aleatorios sera sempre a mesma
#pragma warning (disable: 4566) // desabilita uma instrução especifica do compilador o codigo 4566 referece normalmente a avisos sobre strings literais Unicode
#else 
#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf
#define RANDOM random
#define SRANDOM srandom(getpid())
#endif

#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_OGL_VALUE 0xffffffff

#define NUM_CUBE_MAP_FACES 6 // numero de faces do cubo?

#define SAFE_DELETE(p) if(p) {delete p; p=NULL;} //isso serve para objetos?

// long long GetCurrentTimeMillis();


#define ASSIMP_LOAD_FLAGS ( aiProcess_JoinIdenticalVertices |    \
  aiProcess_Triangulate |              \
  aiProcess_GenSmoothNormals |         \
  aiProcess_LimitBoneWeights |         \
  aiProcess_SplitLargeMeshes |         \
  aiProcess_ImproveCacheLocality |     \
  aiProcess_RemoveRedundantMaterials | \
  aiProcess_FindDegenerates |          \
  aiProcess_FindInvalidData |          \
  aiProcess_GenUVCoords |              \
  aiProcess_CalcTangentSpace) // flags definidas na lib Assimp que controlam diferentes tipos de processamento

#define NOT_IMPLEMENTED printf("Not implement case in %s:%d\n", __FILE__, __LINE__); exit(0);


// acredito que n é necessario usar a parte a baixo no vulkan em nenhum momento
// #ifndef MY_VULKAN
// #define GLExitIfError                                                          \
// {                                                                               \
//     GLenum Error = glGetError();                                                \
//                                                                                 \
//     if (Error != GL_NO_ERROR) {                                                 \
//         printf("OpenGL error in %s:%d: 0x%x\n", __FILE__, __LINE__, Error);     \
//         exit(0);                                                                \
//     }                                                                           \
// }

// #define GLCheckError() (glGetError() == GL_NO_ERROR)

// void gl_check_error(const char* function, const char *file, int line);

// #define CHECK_GL_ERRORS

// #ifdef CHECK_GL_ERRORS
// #define GCE gl_check_error(__FUNCTION__, __FILE__, __LINE__);
// #else
// #define GCE
// #endif

// void glDebugOutput(GLenum source,
//                    GLenum type,
//                    unsigned int id,
//                    GLenum severity,
//                    GLsizei length,
//                    const char *message,
//                    const void *userParam);
// #endif

// string GetDirFromFilename(const string& Filename);

// #define MAX_BONES (200)

// #define CLAMP(Val, Start, End) std::min(std::max((Val), (Start)), (End));

// int GetGLMajorVersion();
// int GetGLMinorVersion();

// int IsGLVersionHigher(int MajorVer, int MinorVer);