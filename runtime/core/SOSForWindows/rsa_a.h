/*
模块名称：登录加密模块
实现功能：对信息进行加密和生成密钥
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓 陈嘉晖
*/
#ifndef RSA_A_H
#define RSA_A_H
#include<vector>
#include <string>

using namespace std;

struct Key {
    long long share_key;
    long long public_key;
    long long private_key;
};
class RSA
{
public:

        RSA();
        RSA(long long share_key, long long public_key) {
            _key.share_key = share_key;
            _key.public_key = public_key;
        }
        Key GetKey() {
            return _key;
        }
        void Ecrept(const char* plain_file_in, const char* ecrept_file_out,
            long public_key, long share_key);
        void DEcrept(const char* plain_file_in, const char* ecrept_file_out,
            long private_key, long share_key);

        std::vector<long> Ecrept(std::string& str_in, long public_key, long share_key);
        std::string DEcrept(std::vector<long>& ecrept_str, long private_key, long share_key);

        void PrintInfo(std::vector<long>& ecrept_str);
    private:
        long ProducePrime();
        bool IsPrime(long prime);
        void ProduceKeys();
        long ProduceShareKey(long prime1, long prime2);
        long ProduceOrla(long prime1, long prime2);
        long ProducePublicKey(long orla);
        long ProduceGcd(long public_key, long orla);
        long ProducePrivateKey(long public_key, long orla);
        long Ecrept(long msg, long key, long share_key);
        public:
        Key _key;
};

#endif // RSA_A_H
