#include "generators.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "long_ar.h"
#include <math.h>
#include <string.h>

#define NUM_OF_INTERVALS 100

const double norm_quantile_099 = 2.33;
const double norm_quantile_09 = 1.28;
const double norm_quantile_095 = 1.65;

BYTE EmbededGenerator(void* seed) {
    return (BYTE)rand();
}

BYTE LehmerHighGenerator(void* seed) {
    unsigned int* state = (unsigned int*)seed;
    *state = ((1<<16)+1) * (*state) + 119;
    return ((*state) & 0xFF000000) >> 24;
}

BYTE LehmerLowGenerator(void* seed) {
    unsigned int* state = (unsigned int*)seed;
    *state = ((1<<16)+1) * (*state) + 119;
    return (*state) & 0xFF;
}

BIT L20Generator(void* s) { 
    int* state = (int*)s;
    int t = ((*state) ^ (*state >> 11) ^ (*state >> 15) ^ (*state >> 17)) & 1;
    BIT ret = *state & 1;
    *state >>= 1;
    *state ^= (-t ^ *state) & (1 << 19);
    return ret;
}

BIT L89Generator(void* s) {
    L_NUMBER* state = (L_NUMBER*)s;
    WORD t = (state->words[0] ^ (state->words[0] >> 51)) & 1;
    BIT ret = state->words[0] & 1;
    l_shift_r(state, 1, state);
    state->words[1] ^= (-t ^ state->words[1]) & (1 << 24);
    return ret;
}

BIT GeffeGenerator(void* state) {
    int* state_x = &(((int*)state)[0]);
    int* state_y = &(((int*)state)[1]);
    int* state_s = &(((int*)state)[2]);
    int x = ((*state_x) ^ (*state_x >> 2)) & 1;
    int y = ((*state_y) ^ (*state_y >> 1) ^ (*state_y >> 3) ^ (*state_y >> 4)) & 1;
    int s = ((*state_s) ^ (*state_s >> 3)) & 1;

    BIT ret = (x&s) ^ (y&(1^s));

    *state_x >>= 1;
    *state_y >>= 1;
    *state_s >>= 1;

    *state_x ^= (-x ^ *state_x) & (1<<10);
    *state_y ^= (-y ^ *state_y) & (1<<8);
    *state_s ^= (-s ^ *state_s) & (1<<9);
    return ret;
}

BIT WolframGenerator(void* s) {
    unsigned* state = (unsigned*)s;
    BIT ret = *state & 1;
    unsigned r_l = *state << 1;
    r_l ^= ((0x80000000 & *state) >> 31);
    unsigned r_r = *state >> 1;
    r_r ^= ((*state & 1) << 31);
    *state = r_l ^ (r_r | *state);
    return ret;
}

BYTE LibrarianGenerator(void* s) {
    char** c = (char**)s;
    char cur = **c;
    (*c)++;
    return (BYTE)cur; 
}

BIT BMGenerator(void* seed) {
    L_NUMBER* a = &(((L_NUMBER*)seed)[0]);
    L_NUMBER* p = &(((L_NUMBER*)seed)[1]);
    L_NUMBER* q = &(((L_NUMBER*)seed)[2]);
    L_NUMBER* mu = &(((L_NUMBER*)seed)[3]);
    L_NUMBER* t = &(((L_NUMBER*)seed)[4]);
    m_pow(a, t, p, mu, t);
    return (l_cmp(q, t) == 1) ? 1 : 0;
}

BYTE BMByteGenerator(void* seed) {
    int i=0;
    L_NUMBER* a = &(((L_NUMBER*)seed)[0]);
    L_NUMBER* p = &(((L_NUMBER*)seed)[1]);
    L_NUMBER* mu = &(((L_NUMBER*)seed)[2]);
    L_NUMBER* t = &(((L_NUMBER*)seed)[3]);
    L_NUMBER* q_table = &(((L_NUMBER*)seed)[4]);
    q_table++;
    m_pow(a, t, p, mu, t);
    while ((i<=255) && (l_cmp(t, &q_table[i]) == 1)) {
        i++;
    }
    return (BYTE)(i-1);
}

BIT BBSGenerator(void* seed) {
    L_NUMBER* n = &(((L_NUMBER*)seed)[0]);
    L_NUMBER* mu = &(((L_NUMBER*)seed)[1]);
    L_NUMBER* r = &(((L_NUMBER*)seed)[2]);
    m_sqr(r, n, mu, r);
    return r->words[0] & 1;
}

BYTE BBSByteGenerator(void* seed) {
    L_NUMBER* n = &(((L_NUMBER*)seed)[0]);
    L_NUMBER* mu = &(((L_NUMBER*)seed)[1]);
    L_NUMBER* r = &(((L_NUMBER*)seed)[2]);
    m_sqr(r, n, mu, r);
    return r->words[0] & 0xFF;
}

void ByteGenGenerateSequence(BYTE (*generator)(void*), void* seed, BYTE* out, WORD size) {
    srand(time(NULL));
    BYTE b;
    for (u32 i=0; i < size; ++i) {
        b = generator(seed);
        out[i] = b;
    }
}

void BitGenGenerateSequence(BIT (*generator)(void*), void* seed, BYTE* out, WORD size) {
    WORD size_bits = size*8;
    BIT b;
    for (u32 i = 0; i < size_bits; ++i)
    {
        b = generator(seed);
        //printf("%d %d\n", i, b);
        out[i/8] ^= b << (i%8);
    }
}

BIT UniformnessTest(BYTE* seq, WORD size) {
    WORD freq_table[256];
    double mean = size/256.0;
    BIT r=0;
    double khi_sqr = 0;
    double khi_sqr_quantile_099 = sqrt(2*255)*norm_quantile_099 + 255;
    double khi_sqr_quantile_095 = sqrt(2*255)*norm_quantile_095 + 255;
    double khi_sqr_quantile_09 = sqrt(2*255)*norm_quantile_09 + 255;

    memset(freq_table, 0, 256*sizeof(WORD));
    for (u32 i=0; i<size; i++) {
        freq_table[seq[i]]++;
    }
    for (u32 i=0; i<256; i++) {
        khi_sqr += (freq_table[i] - mean)*(freq_table[i] - mean) / mean;
    }

    
    if (khi_sqr <= khi_sqr_quantile_09) {
        r++;
        printf(GREEN"Uniformness Test passed on false positive rate 0.9\n"RESET
                "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_09);
    }
    else if (khi_sqr <= khi_sqr_quantile_095) {
        r++;
        printf(GREEN"Uniformness Test passed on false positive rate 0.95\n"RESET
                "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_095);
    }
    else if (khi_sqr <= khi_sqr_quantile_099) {
        r++;
        printf(GREEN"Uniformness Test passed on false positive rate 0.99\n"RESET
                "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_099);
    }
    else {
        printf(RED"Uniformness Test failed on false positive rates 0.99, 0.95, 0.9\n"RESET
                "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_099);
    }

    return r;
}

BIT IndependanceTest(BYTE* seq, WORD size) {
    WORD freq_table[256][256];
    WORD first_place[256];
    WORD second_place[256];
    BIT r=0;
    double khi_sqr = 0;
    double khi_sqr_quantile_099 = sqrt(2*255*255)*norm_quantile_099 + 255*255;
    double khi_sqr_quantile_095 = sqrt(2*255*255)*norm_quantile_095 + 255*255;
    double khi_sqr_quantile_09 = sqrt(2*255*255)*norm_quantile_09 + 255*255;

    memset(freq_table, 0, sizeof(WORD)*65536);
    memset(first_place, 0, sizeof(WORD)*256);
    memset(second_place, 0, sizeof(WORD)*256);
    for (u32 i=0; i<size/2; i++) {
        freq_table[seq[2*i]][seq[2*i+1]]++;

    }
    for (u32 i=0; i<256; i++) {
        for (u32 j=0; j<256; j++) {
            first_place[i] += freq_table[i][j];
        }
    }
    for (u32 i=0; i<256; i++) {
        for (u32 j=0; j<256; j++) {
            second_place[i] += freq_table[j][i];
        }
    }

    for (u32 i=0; i<256; i++) 
        for (u32 j=0; j<256; j++) {
            if (first_place[i]*second_place[j] != 0)
                khi_sqr += ((double)(freq_table[i][j]*freq_table[i][j])/(first_place[i]*second_place[j]));
        }
    khi_sqr--;
    khi_sqr *= size/2;

    if (khi_sqr <= khi_sqr_quantile_09) {
        r++;
        printf(GREEN"Independance Test passed on false positive rate 0.9\n"RESET
               "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_09);
    }
    else if (khi_sqr <= khi_sqr_quantile_095) {
        r++;
        printf(GREEN"Independance Test passed on false positive rate 0.95\n"RESET
               "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_095);
    }
    else if (khi_sqr <= khi_sqr_quantile_099) {
        r++;
        printf(GREEN"Independance Test passed on false positive rate 0.99\n"RESET
               "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_099);
    }
    else {
        printf(RED"Independance Test failed on false positive rates 0.99, 0.95, 0.9\n"RESET
            "KhiSquare Statistic/Quantile: %f/%f\n", khi_sqr, khi_sqr_quantile_099);
    }
    return r;
}

BIT HomogeneousnessTest(BYTE* seq, WORD size) {
    WORD freq_table[NUM_OF_INTERVALS][256];
    WORD m = size/NUM_OF_INTERVALS;
    WORD n = m*NUM_OF_INTERVALS;
    WORD freq_total[256];
    memset(freq_total, 0, 256*sizeof(WORD));
    memset(freq_table, 0, sizeof(WORD)*256*NUM_OF_INTERVALS);

    BIT r=0;
    double khi_sqr = 0;
    double khi_sqr_quantile_099 = sqrt(2*255*(NUM_OF_INTERVALS-1))*norm_quantile_099 + 255*(NUM_OF_INTERVALS-1);
    double khi_sqr_quantile_095 = sqrt(2*255*(NUM_OF_INTERVALS-1))*norm_quantile_095 + 255*(NUM_OF_INTERVALS-1);
    double khi_sqr_quantile_09 = sqrt(2*255*(NUM_OF_INTERVALS-1))*norm_quantile_09 + 255*(NUM_OF_INTERVALS-1);

    for (u32 i=0; i<n; i++) {
        freq_table[i/m][seq[i]]++;
    }
    /*for (u32 i=0; i<256; i++)
        for (u32 j=0; j<NUM_OF_INTERVALS; j++) 
            freq_total[i] += freq_table[j][i];*/
    for (u32 i=0; i<n; i++)
        freq_total[seq[i]]++;

    for (u32 i=0; i<NUM_OF_INTERVALS; i++)
        for (u32 j=0; j<256; j++)
            if (freq_total[j])
                khi_sqr += ((double)(freq_table[i][j]*freq_table[i][j]))/(freq_total[j]*m);
    khi_sqr--;
    khi_sqr *= n;

    if (khi_sqr <= khi_sqr_quantile_09) {
        r++;
        printf(GREEN"Homogeneousness Test passed on false positive rate 0.9\n"RESET
               "Number of intervals: %d\n"
               "KhiSquare Statistic/Quantile: %f/%f\n", NUM_OF_INTERVALS, khi_sqr, khi_sqr_quantile_09);
    }
    else if (khi_sqr <= khi_sqr_quantile_095) {
        r++;
        printf(GREEN"Homogeneousness Test passed on false positive rate 0.95\n"RESET
               "Number of intervals: %d\n"
               "KhiSquare Statistic/Quantile: %f/%f\n", NUM_OF_INTERVALS, khi_sqr, khi_sqr_quantile_095);
    }
    else if (khi_sqr <= khi_sqr_quantile_099) {
        r++;
        printf(GREEN"Homogeneousness Test passed on false positive rate 0.99\n"RESET
               "Number of intervals: %d\n"
               "KhiSquare Statistic/Quantile: %f/%f\n", NUM_OF_INTERVALS, khi_sqr, khi_sqr_quantile_099);
    }
    else {
        printf(RED"Homogeneousness Test failed on false positive rates 0.99, 0.95, 0.9 and Number of intervals - %d\n"RESET
                  "KhiSquare Statistic/Quantile: %f/%f\n", NUM_OF_INTERVALS, khi_sqr, khi_sqr_quantile_099);
    }
    return r;
}