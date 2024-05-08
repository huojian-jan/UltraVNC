#include "ShadowBotVNC.h"

HANDLE m_serverPipeHandle;

void connect2ShadowBot()
{
	std::string pipeName = "\\\\.\\pipe\\" + g_pipeName;
	try
	{
		m_serverPipeHandle = CreateFile(
			pipeName.c_str(),
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
		vnclog.Print(LL_INTERR, VNCLOG("%s",ex.what()));
	}
}


void sendStatus(const std::string &status)
{
	vnclog.Print(LL_INTINFO, VNCLOG("send status:%s\n",status));
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
		vnclog.Print(LL_INTINFO, VNCLOG("status send failed"));
		return;
	}

	vnclog.Print(LL_INTINFO, VNCLOG("status:%s send successfully\n", status));
	DisconnectNamedPipe(m_serverPipeHandle);
}

