#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <iomanip>

#include "chat.h"
#include "macros.h"

#define PORT 1414

char buff[4096];

//attributes
static bool mIsServer = true;
static bool mTerminate = false;

static RSA_t mPublicKey;          //LL
static RSA_t mPrivateKey;
static RSA_t rPublicKey;

static RSA_t mP = 1871;
static RSA_t mQ = 1873;
static RSA_t mN;
static RSA_t rN;
static RSA_t mPhiN;


static STR_FUNC logger;
static STR_FUNC messag;

static int* mStates;

#define LOG(...) sprintf_s(buff, __VA_ARGS__ ); logger(buff)
#define MSG(...) sprintf_s(buff, __VA_ARGS__ ); messag(buff)


SOCKET serverSocket = INVALID_SOCKET;
SOCKET connectionSocket = INVALID_SOCKET;

bool init_server() {
    using namespace std;

    cout << "Creating server socket .." << endl;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Failed to create server socket : " << WSAGetLastError() << endl;
        return false;
    }
    cout << "server socket created" << endl;


    cout << "binding server socket .." << endl;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Failed to bind server socket : " << WSAGetLastError() << endl;
        return false;
    }
    cout << "server socket bound" << endl;


    cout << "starting listening .." << endl;
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        cout << "Failed to listen : " << WSAGetLastError() << endl;
        return false;
    }
    cout << "listening started " << endl;
    LOG("waiting for connection");

    cout << "waiting for client on \"localhost:" << PORT << "\"" << endl;
    connectionSocket = accept(serverSocket, NULL, NULL);
    if (connectionSocket == INVALID_SOCKET) {
        cout << "no client connected" << endl;
        return false;
    }

    cout << "socket connected !" << endl;
    LOG("connected!");

    return true;
}

bool init_client() {

    using namespace std;

    cout << "Creating socket .." << endl;
    connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectionSocket == INVALID_SOCKET) {
        cout << "Failed to create socket : " << WSAGetLastError() << endl;
        return false;
    }
    cout << "server socket created" << endl;

    LOG("connecting ...");
    cout << "connecting socket .." << endl;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    addr.sin_port = htons(PORT);
    if (connect(connectionSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Failed to connect socket : " << WSAGetLastError() << endl;
        return false;
    }
    
    LOG("connected!");
    cout << "socket connected !" << endl;

    return true;
}


bool init() {
    using namespace std;

    cout << "[Init sequance]" << endl;

    cout << "RSA cfg .." << endl;

    mN = mP * mQ;
    mPhiN = (mP - 1) * (mQ - 1);
    auto keys = RSA::generateKeys(mP, mQ);
    mPublicKey = keys.first;
    mPrivateKey = keys.second;

    LOG("RSA { n : %lld , phi : %lld , pu-key : %lld , pr-key : %lld }", mN, mPhiN , mPublicKey , mPrivateKey);
    cout << "RSA { n: " << mN << " , phi: " << mPhiN << " , pu-key: " << mPublicKey << " , pr-key: " << mPrivateKey << " }" << endl;

    cout << "Loading Winsock .." << endl;

    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(version, &data)) {
        LOG("Error loading the Winsock library");
        cout << "Error loading the Winsock library" << endl;
        return false;
    }

    LOG("Winsock Loaded");
    cout << "Winsock Loaded" << endl;

    if (mIsServer) {
        if (!init_server()) return false;
    }
    else {
        if (!init_client()) return false;
    }
    return true;
}

char rbuff[20000];
void connection_tread() {
    using namespace std;

    RSA_t* data = (RSA_t*) malloc(sizeof(RSA_t) * 2);
    data[0] = mPublicKey;
    data[1] = mN;

    if (send(connectionSocket, (const char*)(data), sizeof(RSA_t) * 2 , 0) < 0) {
        cout << "Failed to send the public key" << endl;
        *mStates = 0;
        LOG("connection error");
        return;
    }

    int c = recv(connectionSocket, rbuff, 20000, 0);
    if (c != sizeof(RSA_t) * 2) {
        cout << "Error receiving the public key" << endl;
        *mStates = 0;
        LOG("connection error");
        return;
    }

    *mStates = 2;

    rPublicKey = ((RSA_t*)rbuff)[0];
    rN         = ((RSA_t*)rbuff)[1];

    LOG("Remote public key: %lld , N: %lld", rPublicKey , rN);
    cout << "Remote public key: " << rPublicKey << " , N : " << rN << endl;

    while (!mTerminate) {
        int c = recv(connectionSocket, rbuff, 20000, 0);
        if (c <= 0) {
            cout << "SOCKET ERROR" << endl;
            mTerminate = true; //disconnected
        }

        if (c > 0) {
            vector<RSA_t> data;
            for (int i = 0; i < c / sizeof(RSA_t); i++) {
                RSA_t* ptr = (RSA_t*)rbuff + i;
                data.push_back(*ptr);
            }
            
            cout << "Original: ";
            int k = 0;
            for (int i = 0; i < data.size(); i++) {
                if (k % 6 == 0) {
                    cout << endl << "\t";
                }
                cout << hex << setw(sizeof(RSA_t) * 2) << setfill('0') << data[i] << " ";
                k++;
            }
            cout << endl;
            string msg = RSA::decode(data, mN, mPrivateKey, mPhiN);
            cout << "Decrypted: " << msg << endl;
            const char* ptr = msg.c_str();
            
            MSG(ptr);
        }
    }
    
    //clean
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }

    if (connectionSocket != INVALID_SOCKET) {
        closesocket(connectionSocket);
        connectionSocket = INVALID_SOCKET;
    }

    
}

int thRunning = 0;
std::thread mThread;
char sbuff[20000];

void send(const char* ptr) {
    if (logger == nullptr)
        return;
    if (messag == nullptr)
        return;

    if (!thRunning)
        return;

    if (connectionSocket == INVALID_SOCKET)
        return;

    using namespace std;

    string str = ptr;

    auto vec = RSA::encode(str.c_str(), str.length(), rN, rPublicKey, -1);
    for (int i = 0; i < vec.size(); i++) {
        RSA_t* ptr = ((RSA_t*)(sbuff + i * sizeof(RSA_t)));
        *ptr = vec[i];
    }
    send(connectionSocket, sbuff, vec.size() * sizeof(RSA_t), 0);
}

void waitForChat(){
    if (mThread.joinable()) {
        mThread.join();
    }
}

void clean() {
    using namespace std;
    cout << "[Clean sequance]" << endl;

    if (serverSocket != INVALID_SOCKET) closesocket(serverSocket);
    if (connectionSocket != INVALID_SOCKET) closesocket(connectionSocket);

    WSACleanup();

    cout << "Cleaning done" << endl;
}

void start_seq() {
    thRunning = 1;
    *mStates = 1;
    if (!init()) {
        *mStates = 0;
        thRunning = 0;
        return;
    }
    
    connection_tread();
    *mStates = 0;
    thRunning = 0;
}

void start_chat(int& states, bool server, STR_FUNC _logger, STR_FUNC _msg , RSA_t p, RSA_t q) {
    
    if (mThread.joinable())
        mThread.join();

    mTerminate = false;
    mStates = &states;
    mIsServer = server;
    mP = p;
    mQ = q;
    ::logger = _logger;
    ::messag = _msg;
    
    std::cout << "Starting sequance" << std::endl;
    mThread = std::thread(start_seq);
}

void disconnect(){

    mTerminate = true;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }

    if (connectionSocket != INVALID_SOCKET) {
        closesocket(connectionSocket);
        connectionSocket = INVALID_SOCKET;
    }
}
