#pragma once
#include<string>
#include <Windows.h>
#include "VNCConnection.h"
#include "./json.hpp"
#include "vnclog.h"

#define LL_INTERR	0
#define LL_INTINFO	9
#define VNCLOG(s)	(__FUNCTION__ " : " s)

using json = nlohmann::json;

extern HANDLE m_serverPipeHandle;
extern VNCLog vnclog;

const static std::string g_pipeName = "D10D041F-06BE-4E83-A538-B333428ED3C1-serverPipe";

void connect2ShadowBot();
void sendStatus(const std::string &status);

