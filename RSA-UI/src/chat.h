#pragma once
#include "RSA.h"

typedef void (*STR_FUNC)(const char*);

void start_chat(int& states , bool server , STR_FUNC logger, STR_FUNC msg, RSA_t p, RSA_t q);
void disconnect();
void send(const char* data);
void waitForChat();