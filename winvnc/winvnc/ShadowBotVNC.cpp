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
		vnclog.Print(0, ex.what());
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
		m_serverPipeHandle,                   // �����ܵ����
		jsonText.c_str(),        // Ҫд�������
		jsonText.length() + 1,   // Ҫд������ݳ��ȣ����� null ��ֹ����
		&bytesWritten,          // ʵ��д����ֽ���
		NULL                    // �첽 I/O �����NULL ��ʾͬ������
	);

	if (!result)
	{
		vnclog.Print(9, VNCLOG("status send failed"));
		return;
	}

	const std::string msg = "status:" + status + "send successfully\n";
	vnclog.Print(9, msg.c_str());
	DisconnectNamedPipe(m_serverPipeHandle);
}

