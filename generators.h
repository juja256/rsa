#ifndef GENERATORS_H
#define GENERATORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "long_ar.h"
typedef unsigned char BIT;
typedef unsigned char BYTE;

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */
#define ITALIC "\033[3m"

BYTE EmbededGenerator(void* seed);

BYTE LehmerHighGenerator(void* seed);

BYTE LehmerLowGenerator(void* seed);

BIT L20Generator(void* seed);

BIT L89Generator(void* seed);

BIT GeffeGenerator(void* seed);

BIT WolframGenerator(void* seed);

BYTE LibrarianGenerator(void* seed);

BIT BMGenerator(void* seed);

BYTE BMByteGenerator(void* seed);

BIT BBSGenerator(void* seed);

BYTE BBSByteGenerator(void* seed);

void ByteGenGenerateSequence(BYTE (*generator)(void*), void* seed, BYTE* out, WORD size);

void BitGenGenerateSequence(BIT (*generator)(void*), void* seed, BYTE* out, WORD size);

BIT UniformnessTest(BYTE* seq, WORD size);

BIT IndependanceTest(BYTE* seq, WORD size);

BIT HomogeneousnessTest(BYTE* seq, WORD size);

#ifdef __cplusplus
}
#endif

#endif