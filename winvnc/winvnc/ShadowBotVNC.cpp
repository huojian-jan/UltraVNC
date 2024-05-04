#include "ShadowBotVNC.h"

VNC_Command::VNC_Command()
{
}

VNC_Command::~VNC_Command()
{

}

VNC_Command* m_currentVNCCommand;
HANDLE m_serverPipeHandle;
HANDLE m_servicePipeHandle;

std::mutex cmd_mtx;
std::condition_variable cmd_cond_vr;

void connect2ShadowBot()
{
	std::string pipeName = "\\\\.\\pipe\\" + g_VNC_Server_Pipe;

	if (m_serverPipeHandle == nullptr)
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
}

void setupServicePipe()
{
	std::string pipeName="\\\\.\\pipe\\"+g_VNC_Service_Pipe;

	// ����һ������ǹ���ԱȨ�޽������ӵĹܵ�
	PSECURITY_DESCRIPTOR pSD = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
		"D:P(A;;GA;;;WD)",  // ���ַ�����ʽָ���İ�ȫ�������������κ��û������ӣ�
		SDDL_REVISION_1,
		&pSD,
		NULL))
	{
		std::cout << "Failed to convert security descriptor. Error code: " << GetLastError() << std::endl;
		return;
	}

	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = TRUE;

	m_servicePipeHandle = CreateNamedPipe(
		pipeName.c_str(),
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE,
		0,
		&sa
	);

	if (m_servicePipeHandle == INVALID_HANDLE_VALUE)
	{
		write_log("uvnc_service pipe setup failed");
		return;
	}

	while (true)
	{
		write_log("waiting for command...");

		ConnectNamedPipe(m_servicePipeHandle, NULL);
		read_pipe_command();
		DisconnectNamedPipe(m_servicePipeHandle);
	}
}



void sendStatus(const std::string status)
{
	write_log("going to send status:" + status);
	connect2ShadowBot();

	int pid=GetCurrentProcessId();
	json data;
	data["status"] = status;
	data["processId"] = pid;

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
		write_log("status send failed");
		return;
	}

	write_log("status:" + status + "send successfully");
}

void read_pipe_command()
{
	char buffer[PIPE_BUFFER_SIZE];
	DWORD bytesRead;

	ReadFile(m_servicePipeHandle, buffer, sizeof(buffer), &bytesRead, NULL);

	int commandLength = -1;
	get_command_length(buffer,commandLength);

	if (commandLength == -1)
	{
		write_log("read data error,command length get -1");
		return;
	}

	std::string cmd;
	for (int i = COMMAND_LENGTH; i < COMMAND_LENGTH +commandLength; i++)
	{
		cmd += buffer[i];
	}

	auto commandJson = json::parse(cmd);

	{
		std::lock_guard<std::mutex> lock(cmd_mtx);
		if (m_currentVNCCommand == nullptr)
		{
			m_currentVNCCommand = new VNC_Command;
		}

		m_currentVNCCommand->command = commandJson.at("command");
		m_currentVNCCommand->args = commandJson.at("arguments");
		m_currentVNCCommand->userId = commandJson.at("userId");
		
		cmd_cond_vr.notify_one();
	}
}


void get_command_length(char* buffer, int& length)
{
	std::string commandLenStr;
	for (int i = 0; i < COMMAND_LENGTH; i++)
	{
		commandLenStr += buffer[i];
	}

	try
	{
		length = std::stoi(commandLenStr);
	}
	catch (std::exception ex)
	{
		write_log(ex.what());
	}
}


void write_log(const std::string& info)
{
	const std::string prefixPath = std::getenv("LocalAppData");
	//const 	std::string log_path = prefixPath +"\\ShadowBot\\users\\Assistant\\vnc_service_log.log";
	const 	std::string log_path = "C:\\Users\\think\\AppData\\Local\\ShadowBot\\users\\Assistant\\vnc_service_log.log";
	std::fstream file(log_path, std::ios::app);
	if (file)
	{
		file << info;
		file << "\n";
		file.close();
	}
}

