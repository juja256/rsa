#include "rsa.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <iostream>

static WORD base_pascal_seq_3[] = {1};
static WORD base_pascal_seq_5[] = {1};
#if ARCH == 64
static WORD base_pascal_seq_7[] = {1, 4, 2};
static WORD base_pascal_seq_11[] = {1, 4, 5, 9, 3};
static WORD base_pascal_seq_13[] = {1, 9, 3};
#else
static WORD base_pascal_seq_7[] = {1, 2, 4};
static WORD base_pascal_seq_11[] = {1, 9, 4, 3, 5};
static WORD base_pascal_seq_13[] = {1, 3, 9};
#endif

static void GenerateRandomForMR(L_NUMBER bound, L_NUMBER* out) {
    WORD len = bound.len;
    for (u32 i=0; i<len; i++) {
        out->words[i] = rand();
    }
    out->words[len - 1] %= bound.words[len - 1];
    if (!(out->words[len - 1])) out->words[len - 1]++;
}

static bool IsMutualPrimes(L_NUMBER a, L_NUMBER b) {
    bool fl;
    L_NUMBER t;
    l_init(&t, a.len);
    m_gcd(&a, &b, &t);
    if (t.words[0] == 1) fl = true;
    else fl = false;
    l_free(&t);
    return fl;
}

static bool PreDivisionTest(L_NUMBER p) {
    if (!(p.words[0] & 1)) return false; 
    WORD sum3 = 0;
    WORD sum5 = 0;
    WORD sum11 = 0;
    WORD sum7 = 0;
    WORD sum13 = 0;
    HALF* q = (HALF*)(p.words);
    for (u32 i=0; i<p.len*2; i++) {
        sum3 += q[i];
        sum5 += q[i];
        sum7 += q[i]*base_pascal_seq_7[i%3];
        sum11 += q[i]*base_pascal_seq_11[i%5];
        sum13 += q[i]*base_pascal_seq_13[i%3];
    }
    if ((sum3 % 3 == 0) || (sum5 % 5 == 0) || (sum7 % 7 == 0) || (sum11 % 11 == 0) || (sum13 % 13 == 0)) return false;
    return true;
}

bool MillerRabineTest(L_NUMBER p, WORD k) {
    if (!PreDivisionTest(p)) { return false; }
    bool fl = false;
    srand(time(NULL));
    u32 n=0;
    L_NUMBER unity;
    L_NUMBER unity_inv;
    L_NUMBER tmp;
    L_NUMBER m;
    L_NUMBER d={0,0};
    L_NUMBER a;
    l_init(&a, p.len);
    l_init(&m, 2*p.len);
    l_init(&tmp, p.len);
    l_init(&unity, p.len); unity.words[0] = 1;
    l_init(&unity_inv, p.len); l_sub(&p, &unity, &unity_inv);
    l_copy(&d, &p);
    l_sub(&d, &unity, &d);
    WORD s=0;
    while (!(d.words[0] & 1)) {
        l_shift_r(&d, 1, &d);
        s++;

    }
    m_pre_barret(p.len*2, &p, &m);
    for (u32 n=0; n<k; n++) {
        GenerateRandomForMR(p, &a);

        if (IsMutualPrimes(a, p)) {
            m_pow(&a, &d, &p, &m, &tmp);
            if ((l_cmp(&unity, &tmp) == 0) || (l_cmp(&unity_inv, &tmp) == 0)) { 
                fl = true;
                continue;
            } 
            for (u32 i=1; i<s; i++) {
                m_sqr(&tmp, &p, &m, &tmp);
                if (l_cmp(&unity, &tmp) == 0) {
                    fl = false;
                    break;
                }
                else if (l_cmp(&unity_inv, &tmp) == 0) {
                    fl = true;
                    break;
                }
            }
            if (fl) continue;
            else break;
        }
        else {
            fl = false;
            break;
        }
    }
    l_free(&unity); l_free(&unity_inv); l_free(&a); l_free(&tmp); l_free(&m); l_free(&d);
    return fl;
}

bool MillerRabineTest(L_NUMBER p, WORD k, L_NUMBER unity, L_NUMBER unity_inv, L_NUMBER m, L_NUMBER d, L_NUMBER a, L_NUMBER tmp) {
    if (!PreDivisionTest(p)) { return false; }
    bool fl = false;
    srand(time(NULL));
    u32 n=0;
    unity.words[0] = 1;
    l_sub(&p, &unity, &unity_inv);
    l_copy(&d, &p);
    l_sub(&d, &unity, &d);
    WORD s=0;
    while (!(d.words[0] & 1)) {
        l_shift_r(&d, 1, &d);
        s++;

    }
    m_pre_barret(p.len*2, &p, &m);
    for (u32 n=0; n<k; n++) {
        GenerateRandomForMR(p, &a);

        if (IsMutualPrimes(a, p)) {
            m_pow(&a, &d, &p, &m, &tmp);
            if ((l_cmp(&unity, &tmp) == 0) || (l_cmp(&unity_inv, &tmp) == 0)) { 
                fl = true;
                continue;
            } 
            for (u32 i=1; i<s; i++) {
                m_sqr(&tmp, &p, &m, &tmp);
                if (l_cmp(&unity, &tmp) == 0) {
                    fl = false;
                    break;
                }
                else if (l_cmp(&unity_inv, &tmp) == 0) {
                    fl = true;
                    break;
                }
            }
            if (fl) continue;
            else break;
        }
        else {
            fl = false;
            break;
        }
    }
    return fl;
}

std::string RsaAbonent::LongNumberToString(L_NUMBER l) {
    return std::string((const char*)l.words);
}

L_NUMBER RsaAbonent::StringToLongNumber(std::string s, WORD len) {
    L_NUMBER p; 
    l_init_by_len(&p, len);
    strcpy((char*)(p.words), s.c_str());
    return p;
}

L_NUMBER RsaAbonent::GenerateRandom(WORD len) {
    L_NUMBER n;
    l_init_by_len(&n, len);
    WORD blocks = (len % ARCH == 0) ? len/ARCH : len/ARCH+1;
    WORD h = (len-1) % ARCH;
    BYTE cur;
    BYTE* seq = (BYTE*)n.words;
    for (u32 i=0; i<blocks*ARCH/8; i++) {
        cur = this->generator(this->generator_seed);
        seq[i] = cur;
    }

    n.words[blocks - 1] &= (MAX_WORD >> (blocks*ARCH - len));
    if (!(n.words[blocks-1] & (1L << h))) n.words[blocks-1] ^= (1L << h);
    if (!(n.words[0] & 1)) n.words[0] ^= 1;
    return n;
}

L_NUMBER RsaAbonent::GenerateGoodPrime(WORD len) {
    u32 level = 5;
    L_NUMBER x = this->GenerateRandom(len-1);
    //l_dump(&x, 'h');
    L_NUMBER tmp;
    L_NUMBER unity;
    L_NUMBER two;
    L_NUMBER tmp2;
    l_init_by_len(&tmp2, len);
    l_init_by_len(&tmp, len); 
    l_init_by_len(&unity, len); unity.words[0] = 1;
    l_init_by_len(&two, len); two.words[0] = 2;
    L_NUMBER unity_inv; l_init_by_len(&unity_inv, len);
    L_NUMBER d; l_init_by_len(&d, len);
    L_NUMBER m; l_init_by_len(&m, len*2);
    L_NUMBER a; l_init_by_len(&a, len);
    while (1) {
        //if (MillerRabineTest(x, level, unity, unity_inv, m, d, a, tmp2)) {
        if (MillerRabineTest(x, level)) {
            //l_dump(&x, 'h');
            l_shift_l(&x, 1, &tmp);
            tmp.words[0]++;
            //l_dump(&tmp, 'h');
            //if (MillerRabineTest(tmp, level, unity, unity_inv, m, d, a, tmp2)) return tmp; 
            if (MillerRabineTest(tmp, level)) return tmp; 
            l_null(&tmp);
        }
        l_add(&x, &two, &x);
    }
    l_free(&unity_inv);
    l_free(&unity);
    l_free(&m);
    l_free(&d);
    l_free(&a);
    l_free(&tmp);
    l_free(&two);
}

void RsaAbonent::BuildRsa(WORD bit_len, WORD public_exp) {
    L_NUMBER e;
    l_init_by_len(&e, ARCH);
    e.words[0] = public_exp;
    this->public_key.e = e;
    L_NUMBER p = this->GenerateGoodPrime(bit_len/2);
    L_NUMBER q = this->GenerateGoodPrime(bit_len/2);
    L_NUMBER n;
    l_init_by_len(&n, bit_len);
    l_mul(&p, &q, &n);
    this->public_key.n = n;
    l_init_by_len(&(this->mu), bit_len*2);
    m_pre_barret(2*bit_len/ARCH, &n, &(this->mu));

    L_NUMBER totient;
    l_init_by_len(&totient, bit_len);
    p.words[0]--;
    q.words[0]--;
    l_mul(&p, &q, &totient);

    l_shift_r(&p, 1, &p);
    l_shift_r(&q, 1, &q);
    p.words[0]--;
    q.words[0]--;
    L_NUMBER power, unity;
    l_init_by_len(&unity, bit_len); unity.words[0] = 1; 
    l_init_by_len(&power, bit_len);
    l_mul(&p, &q, &power);
    l_sub(&power, &unity, &power);

    L_NUMBER mu2;
    L_NUMBER d;
    l_init_by_len(&mu2, 2*bit_len);
    m_pre_barret(2*bit_len/ARCH, &totient, &mu2);
    l_init_by_len(&d, bit_len);
    m_pow(&e, &power, &totient, &mu2, &d); // Computing private key
    this->private_key = d;
    //l_dump(&d, 'h');
    l_free(&p); l_free(&q); l_free(&totient); l_free(&mu2); l_free(&unity); l_free(&power); 
}

RsaAbonent::RsaAbonent(WORD bit_len) {
    srand(time(NULL));
    this->generator = EmbededGenerator;
    this->generator_seed = NULL;
    this->hash_function = NULL;
    this->BuildRsa(bit_len, 0x10001);
}

RsaAbonent::RsaAbonent(WORD bit_len, BYTE (*generator_)(void*), void* seed,
        L_NUMBER (*hf)(BYTE*, WORD), WORD public_exp): 
        generator(generator_), generator_seed(seed), hash_function(hf) {
    this->BuildRsa(bit_len, public_exp);
}

RsaAbonent::~RsaAbonent() {
    l_free(&(this->private_key));
    l_free(&(this->mu));
    l_free(&(this->public_key.n));
    l_free(&(this->public_key.e));
}

RSA_PUBLIC_KEY RsaAbonent::getPublicKey() {
    RSA_PUBLIC_KEY pub;
    pub.e.len = 0; pub.n.len = 0;
    l_copy(&(pub.n), &(this->public_key.n));
    l_copy(&(pub.e), &(this->public_key.e));
    return pub;
}

L_NUMBER RsaAbonent::Decrypt(L_NUMBER encrypted) {
    L_NUMBER dec;
    l_init(&dec, this->public_key.n.len);
    m_pow(&encrypted, &(this->private_key), &(this->public_key.n), &(this->mu), &dec);
    return dec; 
}

RSA_SIGNED_MSG RsaAbonent::Sign(L_NUMBER msg) {
    L_NUMBER h;
    RSA_SIGNED_MSG sig;
    sig.msg.len = 0;
    l_copy(&(sig.msg), &msg);
    l_init(&(sig.signature), this->public_key.n.len);
    if (this->hash_function) {
        BYTE* msg_ = (BYTE*)(msg.words);
        WORD len = msg.len * ARCH / 8;
        h = this->hash_function(msg_, len);
    }
    else {
        h = msg;
    }
    m_pow(&h, &(this->private_key), &(this->public_key.n), &(this->mu), &(sig.signature));
    return sig;
}

RSA_SIGNED_MSG RsaAbonent::Sign(BYTE* msg, WORD len) {
    L_NUMBER h;
    RSA_SIGNED_MSG sig;
    sig.msg.len = 0;
    l_init(&(sig.msg), (len % (ARCH/8) == 0) ? len/(ARCH/8) : len/(ARCH/8) + 1);
    memcpy((BYTE*)(sig.msg.words), msg, len);

    l_init(&(sig.signature), this->public_key.n.len);
    if (this->hash_function) {
        h = this->hash_function(msg, len);
    }
    else {
        h = sig.msg;
    }
    m_pow(&h, &(this->private_key), &(this->public_key.n), &(this->mu), &(sig.signature));
    return sig;
}

RSA_SIGNED_MSG RsaAbonent::SignKey(L_NUMBER key, RSA_PUBLIC_KEY abonentB) {
    RSA_SIGNED_MSG msg;
    L_NUMBER k1, S1, S, mu2;
    l_init(&k1, abonentB.n.len);
    l_init(&S1, abonentB.n.len);
    l_init(&S, this->public_key.n.len);
    l_init(&mu2, 2*abonentB.n.len);
    m_pre_barret(2*abonentB.n.len, &(abonentB.n), &mu2);
    m_pow(&key, &(abonentB.e), &(abonentB.n), &mu2, &k1);
    m_pow(&key, &private_key, &(public_key.n), &mu, &S);
    m_pow(&S, &(abonentB.e), &(abonentB.n), &mu2, &S1);
    msg.msg = k1;
    msg.signature = S1;
    l_free(&mu2);
    l_free(&S);
    return msg;
}

RSA_SIGNED_MSG RsaAbonent::SignKey(WORD len, RSA_PUBLIC_KEY abonentB) {
    L_NUMBER key = this->GenerateRandom(len);
    this->SignKey(key, abonentB);
}

bool RsaAbonent::VerifySignedMsg(RSA_SIGNED_MSG msg, RSA_PUBLIC_KEY pub, L_NUMBER (*hash_function)(BYTE*, WORD)) {
    bool fl = false;
    L_NUMBER mu2, m;
    l_init(&m, pub.n.len);
    l_init(&mu2, pub.n.len*2);
    m_pre_barret(2*pub.n.len, &(pub.n), &mu2);

    m_pow(&(msg.signature), &(pub.e), &(pub.n), &mu2, &m);
    if (hash_function) {
        L_NUMBER hash = hash_function((BYTE*)(msg.msg.words), msg.msg.len*ARCH/8);
        if (l_cmp(&hash, &m) == 0) fl = true;
        l_free(&hash);
    }
    else {
        if (l_cmp(&m, &(msg.msg)) == 0) fl = true;
    }
    l_free(&mu2);
    l_free(&m);
    return fl;
}

L_NUMBER RsaAbonent::EncryptMsg(L_NUMBER msg, RSA_PUBLIC_KEY pub) {
    L_NUMBER enc, mu2;
    l_init(&mu2, pub.n.len*2);
    m_pre_barret(2*pub.n.len, &(pub.n), &mu2);
    l_init(&enc, pub.n.len);
    m_pow(&msg, &(pub.e), &(pub.n), &mu2, &enc);
    l_free(&mu2);
    return enc;
}

L_NUMBER RsaAbonent::ObtainKey(RSA_SIGNED_MSG msg, RSA_PUBLIC_KEY pub) {
    L_NUMBER key = this->Decrypt(msg.msg);
    L_NUMBER signature = this->Decrypt(msg.signature);
    RSA_SIGNED_MSG key_signed;
    key_signed.signature = signature;
    key_signed.msg = key;
    if (this->VerifySignedMsg(key_signed, pub)) {
        l_free(&signature);
        return key;
    }
    else {
        l_free(&key);
        l_free(&signature);
        throw std::runtime_error("Key owner not verified!");
    }
}