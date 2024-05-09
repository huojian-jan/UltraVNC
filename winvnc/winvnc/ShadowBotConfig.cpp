#include "ShadowBotConfig.h"

HANDLE m_serverPipeHandle;

ShadowBotConfig* ShadowBotConfig::s_instance = NULL;
ShadowBotConfig* config = ShadowBotConfig::getInstance();

ShadowBotConfig::ShadowBotConfig()
{

}

ShadowBotConfig* ShadowBotConfig::getInstance()
{
	if (!s_instance)
		s_instance = new ShadowBotConfig;
	return s_instance;
}

void ShadowBotConfig::setAuthPassword(const std::string& passwd)
{
	m_auth_password = passwd;
}

void ShadowBotConfig::setCompressLevel(const int value)
{
	m_pref_CompressLevel = value;
}

void ShadowBotConfig::setQualityLevel(const int value)
{
	m_pref_QualityLevel = value;
}

void ShadowBotConfig::setEncoder(const std::string& value)
{
	m_encoder = value;
}


void ShadowBotConfig::setPipeName(const std::string& value)
{
	m_pipeName = value;
}

void ShadowBotConfig::getAuthPassword(std::string& passwd)
{
	passwd = m_auth_password;
}

void ShadowBotConfig::getCompressLevel(int& compress)
{
	compress = m_pref_CompressLevel;
}

void ShadowBotConfig::getQualityLevel(int& quality)
{
	quality = m_pref_QualityLevel;
}

void ShadowBotConfig::getEncoder(int& encoder)
{
	if (m_encoder == "rfbEncodingRaw")
	{
		//encoder = rfbEncodingRaw;
		encoder = 0;
	}
	else if (m_encoder == "rfbEncodingCopyRect")
	{
		//encoder = rfbEncodingCopyRect;
		encoder = 1;
	}
	else if (m_encoder == "rfbEncodingRRE")
	{
	//encoder = rfbEncodingRRE;
	encoder = 2;
	}
	else if (m_encoder == "rfbEncodingCoRRE")
	{
		//encoder = rfbEncodingCoRRE;
		encoder = 4;
	}
	else if (m_encoder == "rfbEncodingHextile")
	{
		//encoder = rfbEncodingHextile;
		encoder = 5;
	}
	else if (m_encoder == "rfbEncodingZlib")
	{
		//encoder = rfbEncodingZlib;
		encoder = 6;
	}
	else if (m_encoder == "rfbEncodingTight")
	{
		//encoder = rfbEncodingTight;
		encoder = 7;
	}
	else if (m_encoder == "rfbEncodingZlibHex")
	{
		//encoder = rfbEncodingZlibHex;
		encoder = 8;
	}
	else if (m_encoder == "rfbEncodingUltra")
	{
		//encoder = rfbEncodingUltra;
		encoder = 10;
	}
	else if (m_encoder == "rfbEncodingUltra2")
	{
		//encoder = rfbEncodingUltra2;
		encoder = 10;
	}
	else if (m_encoder == "rfbEncodingZRLE")
	{
		//encoder = rfbEncodingZRLE;
		encoder = 16;
	}
	else if (m_encoder == "rfbEncodingZYWRLE")
	{
		//encoder = rfbEncodingZYWRLE;
		encoder = 17;
	}
	else if (m_encoder == "rfbEncodingXZ")
	{
		//encoder = rfbEncodingXZ;
		encoder = 18;
	}
	else if (m_encoder == "rfbEncodingXZYW")
	{
		//encoder = rfbEncodingXZYW;
		encoder = 19;
	}
	else if (m_encoder == "rfbEncodingZstd")
	{
		//encoder = rfbEncodingZstd;
		encoder = 25;
	}
	else if (m_encoder == "rfbEncodingTightZstd")
	{
		//encoder = rfbEncodingTightZstd;
		encoder = 26;
	}
	else if (m_encoder == "rfbEncodingZstdHex")
	{
		//encoder = rfbEncodingZstdHex;
		encoder = 27;
	}
	else if (m_encoder == "rfbEncodingZSTDRLE")
	{
		//encoder = rfbEncodingZSTDRLE;
		encoder = 28;
	}
	else if (m_encoder == "rfbEncodingZSTDYWRLE")
	{
		//encoder = rfbEncodingZSTDYWRLE;
		encoder = 29;
	}
	else
	{
		//非法的编码器，用默认编码器
		encoder = 0;
	}
}

void ShadowBotConfig::getPipeName(std::string& pipeName) 
{
	pipeName = m_pipeName;
}

void connect2ShadowBot()
{
	std::string pipeName;
	config->getPipeName(pipeName);

	std::string fullPipeName = "\\\\.\\pipe\\" +pipeName;
	try
	{
		m_serverPipeHandle = CreateFile(
			fullPipeName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);
	}
	catch (const std::exception& ex)
	{
		vnclog.Print(LL_INTERR, ex.what());
	}
}


void sendStatus(const std::string &status)
{
	const std::string info = "send status:" + status;
	vnclog.Print(9,info.c_str());
	connect2ShadowBot();

	json data;
	data["status"] = status;

	std::string jsonText = data.dump(4);
	DWORD bytesWritten = 0;

	bool result = WriteFile(
		m_serverPipeHandle,                   // 命名管道句柄
		jsonText.c_str(),        // 要写入的数据
		jsonText.length() + 1,   // 要写入的数据长度（包括 null 终止符）
		&bytesWritten,          // 实际写入的字节数
		NULL                    // 异步 I/O 句柄，NULL 表示同步操作
	);

	if (!result)
	{
		vnclog.Print(9, VNCLOG("status send failed"));
		return;
	}

	const std::string msg = "status:" + status + "send successfully\n";
	vnclog.Print(LL_INTINFO, msg.c_str());
	DisconnectNamedPipe(m_serverPipeHandle);
}

