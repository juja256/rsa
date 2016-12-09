#include "rsa.h"
#include <iostream>

int main() {
    RsaAbonent A(256);
	
    RSA_PUBLIC_KEY pk = A.getPublicKey();
    l_dump(&(pk.n), 'h');

    L_NUMBER msg = RsaAbonent::StringToLongNumber("hello", 256);
    L_NUMBER msg2 = RsaAbonent::StringToLongNumber("hello2", 320);
    RSA_SIGNED_MSG s = A.Sign(msg);
    std::cout << "A: Message: " << RsaAbonent::LongNumberToString(s.msg) << "\n";
    std::cout << "A: Signature: ";
    l_dump(&(s.signature), 'h');

    RsaAbonent B(320);
    RSA_PUBLIC_KEY pk2 = B.getPublicKey();
    l_dump((&pk2.n), 'h');
    std::cout << "B: Verification Status: " << B.VerifySignedMsg(s, pk) << "\n";

    L_NUMBER encrypted = A.EncryptMsg(msg2, pk2);
    std::cout << "A: Encrypts '" << RsaAbonent::LongNumberToString(msg2) << "' : ";
    l_dump(&encrypted, 'h');

    L_NUMBER decrypted = B.Decrypt(encrypted);
    std::cout << "B: Decrypts: " << RsaAbonent::LongNumberToString(decrypted) << "\n";

    //L_NUMBER key_raw = RsaAbonent::StringToLongNumber("This is key", 256);
    //RSA_SIGNED_MSG keyA = A.SignKey(256, pk2);
    //l_dump(&(keyA.msg), 'h');
    //l_dump(&(keyA.signature), 'h');
    //L_NUMBER key = B.ObtainKey(keyA, pk);
    //l_dump(&key, 'h');
    return 0;
}
