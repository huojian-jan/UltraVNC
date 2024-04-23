#include "ConnectionStatus.h"

ConnectionStatus::ConnectionStatus(std::string status):m_status(status)
{
}

void
ConnectionStatus::ToJson(std::string& status_txt)
{
	json j;
	j["status"] = m_status;

	status_txt = j.dump();
}

ConnectionStatus::~ConnectionStatus()
{	
}

