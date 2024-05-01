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
OVERLAPPED overlapped;

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


	//if (m_serverPipeHandle == INVALID_HANDLE_VALUE)
	//{
	//	write_log("uvnc_server pipe create failed");
	//	return;
	//}
}

void setupServicePipe()
{
	std::string pipeName="\\\\.\\pipe\\"+g_VNC_Service_Pipe;

	// 创建一个允许非管理员权限进程连接的管道
	PSECURITY_DESCRIPTOR pSD = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
		"D:P(A;;GA;;;WD)",  // 以字符串形式指定的安全描述符（允许任何用户组连接）
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
		write_log("waiting ccommand");
		ConnectNamedPipe(m_servicePipeHandle, NULL);
		read_pipe_command();
		DisconnectNamedPipe(m_servicePipeHandle);
		//write_log("waiting in new old pipe");
		//Sleep(1000);
	}

	//BOOL result = ConnectNamedPipe(m_servicePipeHandle,&overlapped);

	//if (result)
	//{
	//	std::cout << "Client connected." << std::endl;

	//	// 读取客户端数据

	//	read_pipe_command();
	//	int a = 100;
	//}
}

void setupServicePipe_New()
{
	OVERLAPPED overlapped;
	// 创建一个允许非管理员权限进程连接的管道
	PSECURITY_DESCRIPTOR pSD = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
		"D:P(A;;GA;;;WD)",  // 以字符串形式指定的安全描述符（允许任何用户组连接）
		SDDL_REVISION_1,
		&pSD,
		NULL))
	{
		std::cout << "Failed to convert security descriptor. Error code: " << GetLastError() << std::endl;
		return;
	}

	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = TRUE;


	// 创建命名管道
	m_servicePipeHandle = CreateNamedPipe(
		"\\\\.\\pipe\\MyPipe", // 管道名称
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // 管道访问模式
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // 管道类型和读取模式
		1, // 最大实例数
		4096, // 输出缓冲区大小
		4096, // 输入缓冲区大小
		0, // 默认超时时间
		&sa // 安全属性
	);

	if (m_servicePipeHandle == INVALID_HANDLE_VALUE) {
		//std::cout << "Failed to create named pipe. Error code: " << GetLastError() << std::endl;
		std::string msg = "Failed to create named pipe. Error code: "+GetLastError();
		write_log(msg);
		return;
	}


	// 异步等待客户端连接
	while (true)
	{
		write_log("waiting for command");
		ConnectNamedPipe(m_servicePipeHandle,NULL);
		
		
		read_pipe_command();
		write_log("received command:");
		write_log("cmd:" + m_currentVNCCommand->command + "\targs:" + m_currentVNCCommand->args + "\tid:" + m_currentVNCCommand->userId);
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
		m_serverPipeHandle,                   // 命名管道句柄
		jsonText.c_str(),        // 要写入的数据
		jsonText.length() + 1,   // 要写入的数据长度（包括 null 终止符）
		&bytesWritten,          // 实际写入的字节数
		NULL                    // 异步 I/O 句柄，NULL 表示同步操作
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

	m_currentVNCCommand->command = commandJson.at("command");
	m_currentVNCCommand->args = commandJson.at("arguments");
	m_currentVNCCommand->userId = commandJson.at("userId");
}

void waiting_for_command()
{
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

VOID CALLBACK CompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	if (dwErrorCode == 0 && dwNumberOfBytesTransfered > 0)
	{
		// 处理接收到的数据
		CHAR buffer[1024];
		memcpy(buffer, lpOverlapped->Pointer, dwNumberOfBytesTransfered);

		int commandLength = -1;
		get_command_length(buffer, commandLength);

		if (commandLength == -1)
		{
			write_log("read data error,command length get -1");
			return;
		}

		std::string cmd;
		for (int i = COMMAND_LENGTH; i < COMMAND_LENGTH + commandLength; i++)
		{
			cmd += buffer[i];
		}

		auto commandJson = json::parse(cmd);

		m_currentVNCCommand->command = commandJson.at("command");
		m_currentVNCCommand->args = commandJson.at("arguments");
		m_currentVNCCommand->userId = commandJson.at("userId");

		// 继续等待客户端写入数据
		ReadFileEx(m_servicePipeHandle, buffer, sizeof(buffer), lpOverlapped, CompletionRoutine);

		write_log("data received==============================================");
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

