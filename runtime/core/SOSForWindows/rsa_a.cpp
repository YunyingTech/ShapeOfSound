
/*
模块名称：登录加密模块
实现功能：对信息进行加密和生成密钥
完成作者：史家旭
测试人员：郭明皓 史家旭
审核人员：郭明皓 陈嘉晖
*/
#include "rsa_a.h"

#include<fstream>
#include<ostream>
#include <iostream>
#include<time.h>
#include <cmath>
using namespace std;

RSA::RSA() {
    ProduceKeys();
}
void RSA::Ecrept(const char* plain_file_in, const char* ecrept_file_out,
    long public_key, long share_key) {
    std::ifstream fin(plain_file_in);
    std::ofstream fout(ecrept_file_out, std::ofstream::app);
    if (!fin.is_open()) {
        std::cout << "open file failed" << std::endl;
        return;
    }
    const int NUM = 256;
    char buf[NUM];
    long buf_out[NUM];
    int cur_num;
    while (!fin.eof()) {
        fin.read(buf, NUM);
        cur_num = fin.gcount();
        for (int i = 0; i < cur_num; ++i) {
            buf_out[i] = Ecrept((long)buf[i], public_key, share_key);
        }
        fout.write((char*)buf_out, cur_num * sizeof(long));
    }
    fin.close();
    fout.close();
}
void RSA::DEcrept(const char* plain_file_in, const char* ecrept_file_out,
    long private_key, long share_key) {
    std::ifstream fin(plain_file_in);
    std::ofstream fout(ecrept_file_out, std::ofstream::app);
    if (!fin.is_open()) {
        std::cout << "open file failed" << std::endl;
        return;
    }
    const int NUM = 256;
    long buf[NUM];
    char buf_out[NUM];
    int cur_num;
    while (!fin.eof()) {
        fin.read((char*)buf, NUM * sizeof(long));
        cur_num = fin.gcount();
        cur_num /= sizeof(long);
        for (int i = 0; i < cur_num; ++i) {
            buf_out[i] = (char)Ecrept((long)buf[i], private_key, share_key);
        }
        fout.write(buf_out, cur_num);
    }
    fin.close();
    fout.close();
}

std::vector<long> RSA::Ecrept(std::string& str_in, long public_key, long share_key) {
    std::vector<long> vecout;
    for (const auto& e : str_in) {
        vecout.push_back(Ecrept(e, public_key, share_key));
    }
    return vecout;
}
std::string RSA::DEcrept(std::vector<long>& ecrept_str, long private_key, long share_key) {
    std::string strout;
    for (const auto& e : ecrept_str) {
        strout.push_back((char)Ecrept(e, private_key, share_key));
    }
    return strout;
}

void RSA::PrintInfo(std::vector<long>& ecrept_str) {
    for (const auto& e : ecrept_str) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
}


long RSA::Ecrept(long msg, long key, long share_key) {
    long msg_out = 1;
    long a = msg;
    long b = key;
    int c = share_key;
    while (b) {
        if (b & 1) {
            //msg_out = (A0*A1...Ai...An) % c
            msg_out = (msg_out * a) % c;
        }
        b >>= 1;
        a = (a * a) % c;
    }
    return msg_out;
}


long RSA::ProducePrime()
{
    const long TABLE[] = { 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 };
    srand(time(nullptr));
    long prime = TABLE[rand() % 11];
    // cout << prime << endl;
    return prime;
}


bool RSA::IsPrime(long prime) {
    if (prime < 2)
        return false;
    for (long i = 2; i <= sqrt(prime); ++i) {
        if (prime % i == 0)
            return false;
    }
    return true;
}


void RSA::ProduceKeys() {
    long prime1 = ProducePrime();
    long prime2 = ProducePrime();
    while (prime1 == prime2)
        prime2 = ProducePrime();
    _key.share_key = ProduceShareKey(prime1, prime2);
    long orla = ProduceOrla(prime1, prime2);
    _key.public_key = ProducePublicKey(orla);
    _key.private_key = ProducePrivateKey(_key.public_key, orla);
}


long RSA::ProduceShareKey(long prime1, long prime2) {
    return prime1 * prime2;
}


long RSA::ProduceOrla(long prime1, long prime2) {
    return (prime1 - 1) * (prime2 - 1);
}

long RSA::ProducePublicKey(long orla) {
    long public_key;
    srand(time(nullptr));
    while (1) {
        public_key = rand() % orla;
        if (public_key > 1 && ProduceGcd(public_key, orla) == 1)
            break;
    }
    return public_key;
}

long RSA::ProduceGcd(long public_key, long orla) {
    long residual;
    while (residual = public_key % orla) {
        public_key = orla;
        orla = residual;
    }
    return orla;
}

long RSA::ProducePrivateKey(long public_key, long orla) {
    //(public_key * private_key) % orla == 1
    long private_key = orla / public_key;
    while (1) {
        if ((public_key * private_key) % orla == 1)
            break;
        ++private_key;
    }
    return private_key;
}
