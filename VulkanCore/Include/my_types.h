#ifndef MY_TYPES_H
#define MY_TYPES_H

#ifndef _WIN64
#include <unistd.h>
#endif

#include <cstdint>

typedef unsigned int uint; //unsigned é sem sinal pois só armazena valores positivos
typedef unsigned short ushort; // um dado inteiro de 16bytes, um int que aceita menos espaço de numeros
typedef unsigned char uchar;  // normalmente usado para manipular buffers de dados ou dados binarios, aceita do numero 0 a 255

typedef uint8_t u8; // um alias para o tipo uint8_t da lib stdint.h - inteiro sem sinal de 1byte/8bits
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

#endif