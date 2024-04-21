#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <iomanip>

#include "RSA.h"
#include "macros.h"

void bRSA(RSA_t n) {
    if (n % 2 == 0) {
        printf("%lld : { %lld , %lld } \n", n, n / 2, (RSA_t) 2);
        return;
    }

    for (RSA_t i = 3; i * i < n; i += 2) {
        if (n % i == 0) {
            printf("%lld : { %lld , %lld } \n", n, n / i, i);
            return;
        }
    }
}

void breakRSA_test(const char* input_file) {
    INIT_TIMER;

    using namespace std;

    FILE* ptr, * output;

    fopen_s(&ptr, input_file, "r");
    fopen_s(&output, "output-break.txt", "w");

    if (ptr == nullptr || output == nullptr) {
        cout << "couldn't open files";
        return;
    }

    int count;

    fscanf_s(ptr, "%d", &count);
    fprintf_s(output, "%d", count);

    RSA_t n;
    for (int i = 0; i < count; i++) {
        fscanf_s(ptr, "%lld", &n);

        TIMER_LOG(bRSA(n));
        fprintf(output, "%lld : %lld\n", n, TIMER_GET_DELTA_MS);
    }

    fclose(ptr);
    fclose(output);
}

void encryptRSA_test(const char* input_file) {
    INIT_TIMER;

    using namespace std;

    FILE *ptr, *output;

    fopen_s(&ptr, input_file, "r");
    fopen_s(&output, "output.txt", "w");

    if (ptr == nullptr || output == nullptr) {
        cout << "couldn't open files";
        return;
    }

    int count;

    fscanf_s(ptr, "%d", &count);
    fprintf_s(output, "%d", count);

    string msg = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789~!@#$%^&*()_+}{:\"";

    RSA_t p, q, n, phi;
    for (int i = 0; i < count; i++) {
        fscanf_s(ptr, "%lld %lld", &p, &q);
        n = p * q;
        auto keys = RSA::generateKeys(p, q);
        phi = (p - 1) * (q - 1);
        printf_s("RSA { n = %lld , p = %lld , q = %lld , phi = %lld , public-key = %lld , private-key = %lld} \n", n, p, q, phi , keys.first , keys.second);
        TIMER_LOG(RSA::encode(msg, n, keys.first, phi));
        fprintf(output, "%lld : %lld\n", n, TIMER_GET_DELTA_MS);
    }

    fclose(ptr);
    fclose(output);
}

//args : -break input-break.txt
int main(int argc , char** argv) {
    using namespace std;
    if (argc < 2) {
        RSA_t p = 100823;
        RSA_t q = 104417;
        RSA_t n = p * q;
        printf_s("n = %lld , p(n) = %lld\n", n, ((q - 1) * (p - 1)));
        auto keys = RSA::generateKeys(p, q);
        printf_s("keys : { %lld , %lld } \n", keys.first, keys.second);
        RSA_t org = 'a';
        RSA_t enc = RSA::encrypt(org, n, keys.first);
        printf_s("msg : { 0x%016llx = %lld , 0x%016llx = %lld} \n", org , org , enc , enc);
        if (org != RSA::encrypt(enc, n, keys.second)) {
            printf_s("ERROR IN RSA\n");
        }
        else {
            printf_s("dec : { 0x%016llx = %lld }\n", RSA::encrypt(enc, n, keys.second) , RSA::encrypt(enc, n, keys.second));
        }
        return 1;
    }

    string str = argv[1];
    if (str == "-break") {
        breakRSA_test(argv[2]);
    }
    else if (str == "-enc") {
        encryptRSA_test(argv[2]);
    }

    return 0;
}