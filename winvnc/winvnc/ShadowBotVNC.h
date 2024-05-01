#pragma once
#include<string>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <thread>
#include <mutex>
#include <sddl.h>
#include "ConnectionStatus.h"
#include "./json.hpp"

#define COMMAND_LENGTH 4
#define  PIPE_BUFFER_SIZE 1024
using json = nlohmann::json;

class VNC_Command
{
public:
	VNC_Command();
	~VNC_Command();

public:
	std::string command;
	std::string args;
	std::string userId;
};

extern VNC_Command* m_currentVNCCommand;
extern HANDLE m_serverPipeHandle;
extern HANDLE m_servicePipeHandle;
extern 	OVERLAPPED overlapped;

//const static std::string g_VNC_Server_Pipe = "D10D041F-06BE-4E83-A538-B333428ED3C1-serverPipe";
const static std::string g_VNC_Server_Pipe = "serverPipe";
//const static std::string g_VNC_Service_Pipe = "33F9112E-CEB4-4407-842A-385BD7232CA1-servicePipe";
const static std::string g_VNC_Service_Pipe = "huojian";


 void connect2ShadowBot();
void setupServicePipe();
void setupServicePipe_New();
void sendStatus(const std::string status);
void read_pipe_command();
void waiting_for_command();
void get_command_length(char* buffer,int &length);

VOID CALLBACK CompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

void write_log(const std::string& info);

