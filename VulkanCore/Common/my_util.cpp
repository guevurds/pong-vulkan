//  não foi completamente implementado, implementei apenas oq foi precisa ser usado, transcrito de ogldev_util.cpp
#include <iostream>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#include "my_util.h"

bool ReadFile(const char* pFileName, string& outFile) {
  ifstream f(pFileName); 

  bool ret = false;

  if(f.is_open()) {
    string line;
    while(getline(f, line)) {
      outFile.append(line);
      outFile.append("\n");
    }

    f.close();

    ret = true;
  }
  else {
    MY_ERROR0(pFileName);
  }

  return ret;
}

#ifdef _WIN32

char* ReadBinaryFile  (const char* pFilename, int& size) {
  FILE* f = NULL;

  errno_t err = fopen_s(&f, pFilename, "rb");

  if(!f) {
    char buf[256] = { 0 };
    strerror_s(buf, sizeof(buf), err);//?
    MY_ERROR("Error opening '%s': %s\n", pFilename, buf);
    exit(0);
  }

  struct stat stat_buf;
  int error = stat(pFilename, &stat_buf); //?

  if(error) {
    char buf[256] = {0};
    strerror_s(buf, sizeof(buf), err);
    MY_ERROR("Error getting file stats: %s\n", buf);
    return NULL;
  }

  size = stat_buf.st_size;

  char* p = (char*)malloc(size);
  assert(p);

  size_t bytes_read = fread(p, 1, size, f);

  if(bytes_read != size) {
    char buf[256] = {0};
    strerror_s(buf, sizeof(buf), err);
    MY_ERROR("Read file error file: %s\n", buf);
  }

  fclose(f);

  return p;
}

void WriteBinaryFile(const char* pFilename, const void* pData, int size) { //?
  FILE* f =NULL;

  errno_t err = fopen_s(&f, pFilename, "wb");

  if(!f) {
    MY_ERROR("Error opening '%s'\n", pFilename);
    exit(0);
  }

  size_t bytes_written = fwrite(pData, 1, size, f);

  if(bytes_written != size) {
    MY_ERROR0("Error write file\n");
    exit(0);
  }

  fclose(f);
}
#else
//deveria escrever aqui o pra linux 
#endif
 


void MyError(const char* pFileName, uint line, const char* format, ...) {
  char msg[1000];
  va_list args; // tipo que armazena lista de argumentos variadicos
  va_start(args, format); // inicializa a lista para acessar os parametros variadiadicos passados para a função, iniciando do argumento format
  VSNPRINTF(msg, sizeof(msg), format, args); // formata a mensagem de erro com o macro
  va_end(args);

  #ifdef _WIN32
    char msg2[1000];
    _snprintf_s(msg2, sizeof(msg2), "%s:%d: %s", pFileName, line, msg);
    MessageBoxA(NULL, msg2, NULL, 0); // exibe a mensagem numa caixa de mensagem do windows
  #else 
    fprintf(stderr, "%s:%d: %s", pFileName, line, msg);
  #endif
}