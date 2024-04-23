#pragma once
#include <string>
#include <fstream>
#include "./json.hpp"
#include <fstream>

using json = nlohmann::json;
 class ConnectionStatus
{
public:
	ConnectionStatus(std::string);
	~ConnectionStatus();

public:
	void ToJson(std::string &status_txt);

	public:
		std::string m_status;
	
};

