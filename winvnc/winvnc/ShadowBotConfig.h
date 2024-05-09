#pragma once
#include<string>
#include <Windows.h>
#include "VNCConnection.h"
#include "./json.hpp"
#include "vnclog.h"
//#include "stdhdrs.h"
//#include"rfb.h"

#define LL_INTERR	0
#define LL_INTINFO	9
#define VNCLOG(s)	(__FUNCTION__ " : " s)

using json = nlohmann::json;

extern HANDLE m_serverPipeHandle;
extern VNCLog vnclog;
void connect2ShadowBot();
void sendStatus(const std::string& status);


class ShadowBotConfig
{
public:
	static ShadowBotConfig* getInstance();
	void setAuthPassword(const std::string&);
	void setCompressLevel(const int value);
	void setQualityLevel(const int value);
	void setEncoder(const std::string& value);
	void setPipeName(const std::string& value);

	void getAuthPassword(std::string&);
	void getCompressLevel(int&);
	void getQualityLevel(int&);
	void getEncoder(int& encoder);
	void getPipeName(std::string& pipeName) ;

private:
	ShadowBotConfig();
private:
	static ShadowBotConfig* s_instance;
	std::string m_auth_password;
	int m_pref_CompressLevel;
	int m_pref_QualityLevel;
	std::string m_encoder;
	std::string m_pipeName;
};

