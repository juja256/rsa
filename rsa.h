#pragma once
#include "long_ar.h"
#include "generators.h"
#include <string>

typedef struct {
	L_NUMBER e;
	L_NUMBER n;
} RSA_PUBLIC_KEY;

typedef struct {
	L_NUMBER signature;
	L_NUMBER msg;
} RSA_SIGNED_MSG;

bool MillerRabineTest(L_NUMBER p, WORD k);

class RsaAbonent
{
private:
	L_NUMBER mu;
	L_NUMBER private_key;
	RSA_PUBLIC_KEY public_key;
	BYTE (*generator)(void*);
	L_NUMBER (*hash_function)(unsigned char*, WORD);
	void* generator_seed;
private:
	void BuildRsa(WORD bit_len, WORD public_exp);
	L_NUMBER GenerateRandom(WORD len);
	L_NUMBER GenerateGoodPrime(WORD len);
public:
	static std::string LongNumberToString(L_NUMBER l);
	static L_NUMBER StringToLongNumber(std::string s, WORD len);
	
	RsaAbonent(WORD bit_len);
	RsaAbonent(WORD bit_len, BYTE (*generator)(void*), void* seed, L_NUMBER (*hash_function)(BYTE*, WORD), WORD public_exp=0x10001);
	~RsaAbonent();
	RSA_PUBLIC_KEY getPublicKey();
	L_NUMBER Decrypt(L_NUMBER encrypted);
	RSA_SIGNED_MSG Sign(L_NUMBER msg);
	RSA_SIGNED_MSG Sign(BYTE* msg, WORD len);
	RSA_SIGNED_MSG SignKey(WORD len, RSA_PUBLIC_KEY abonentB);
	RSA_SIGNED_MSG SignKey(L_NUMBER key, RSA_PUBLIC_KEY abonentB);

	bool VerifySignedMsg(RSA_SIGNED_MSG msg, RSA_PUBLIC_KEY pub, L_NUMBER (*hash_function)(BYTE*, WORD)=NULL);
	L_NUMBER EncryptMsg(L_NUMBER msg, RSA_PUBLIC_KEY pub);
	L_NUMBER ObtainKey(RSA_SIGNED_MSG msg, RSA_PUBLIC_KEY pub);
};