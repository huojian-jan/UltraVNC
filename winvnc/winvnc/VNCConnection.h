#pragma once
#include <string>

class VNCConnection
{
public:
	VNCConnection(std::string);
	~VNCConnection();

public:
	std::string m_status;
};

