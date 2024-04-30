#pragma once
#include <string>
#include <fstream>
#include "./json.hpp"
#include <fstream>

using json = nlohmann::json;
 class ConnectionStatus
{
public:
	ConnectionStatus(std::string,int);
	~ConnectionStatus();

public:
	std::string m_status;
	int m_processId;
};

