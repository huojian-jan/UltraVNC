//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the program is not available from the place from
// which you received this file, check http://www.realvnc.com/ or contact
// the authors on info@realvnc.com for information on obtaining it.

// Log.cpp: implementation of the VNCLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdhdrs.h"
#include <io.h>
#include <iostream>
#include <fstream>
#include "vnclog.h"
#include "inifile.h"
#include "cstdlib"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int VNCLog::ToDebug = 1;
const int VNCLog::ToFile = 2;
const int VNCLog::ToConsole = 4;

static const int LINE_BUFFER_SIZE = 1024;

VNCLog::VNCLog()
	: m_tofile(true)
	, m_todebug(false)
	, m_toconsole(false)
	, m_mode(0)
	, m_level(0)
	, hlogfile(NULL)
	, m_append(false)
	, m_video(false)
	, m_lastLogTime(0)
{
	strcpy_s(m_filename, "");
	m_path[0] = 0;
}

void VNCLog::SetMode(int mode)
{
#ifdef SC_20
	return;
#endif
	m_mode = mode;
	if (mode & ToDebug)
		m_todebug = true;
	else
		m_todebug = false;

	if (mode & ToFile) {
		if (!m_tofile)
			OpenFile();
	}
	else {
		CloseFile();
		m_tofile = false;
	}

	if (mode & ToConsole) {
		if (!m_toconsole) {
			AllocConsole(); //lint !e534
			fclose(stdout);
			fclose(stderr);
#ifdef _MSC_VER3
			int fh = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), 0);
			_dup2(fh, 1);
			_dup2(fh, 2);
			_fdopen(1, "wt");
			_fdopen(2, "wt");
			printf("fh is %i\n", fh);
			fflush(stdout);
#endif
		}

		m_toconsole = true;

	}
	else {
		m_toconsole = false;
	}
}


void VNCLog::SetLevel(int level) {
	m_level = level;
}

void VNCLog::SetFile()
{
#ifdef SC_20
	return;
#endif
	char temp[512];
	//IniFile myIniFile;
	//myIniFile.ReadString("admin", "path", temp,512);

	std::string file;
	std::string file_path;

	GetLogPath(file);
	const size_t lastSeparatorPos = file.find_last_of("\\/");
	if (lastSeparatorPos != std::string::npos)
	{
		file_path = file.substr(0, lastSeparatorPos);
	}

	SetPath(file_path);
	strcpy(m_filename, file.c_str());
	m_append = true;
	if (m_tofile)
		OpenFile();
}

void VNCLog::GetLogPath(std::string& path)
{
	std::string log_path;
	std::string log_name = ".VNC_Server.log";
	std::size_t maxSize = 10 * 1024 * 1024;  // 10MB

	char* log_prefix = std::getenv("LOCALAPPDATA");
	if (log_prefix == nullptr)
	{
		return;
	}

	std::string date;
	GetCurrentDate(date);

	log_path += log_prefix;
	log_path += "\\ShadowBot\\log\\";
	log_path += date;
	log_path += log_name;

	path = GenerateLogPath(log_path, maxSize);
}


void VNCLog::GetCurrentDate(std::string& date)
{
	const std::time_t currentTime = std::time(nullptr);
	const std::tm* localTime = std::localtime(&currentTime);

	char buffer[9];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d", localTime);
	date = buffer;
}

std::string VNCLog::GenerateLogPath(const std::string& filePath, std::size_t maxSize, int suffix)
{
	struct stat buffer;
	if (stat(filePath.c_str(), &buffer) == 0) {
		if (buffer.st_size <= maxSize) {
			// 日志大小未超过最大限制，可以继续用
			return filePath;
		}
		else {
			// 日志大小超过最大限制，生成带后缀的新日志文件
			std::ostringstream oss;
			std::string newFilename = filePath;
			oss << suffix;
			if (suffix != 1)
			{
				const	size_t suffix_pos = filePath.find_last_of(".");
				if (suffix_pos != std::string::npos)
				{
					newFilename = filePath.substr(0, suffix_pos);
				}
			}

			newFilename = newFilename + "." + oss.str();
			return GenerateLogPath(newFilename, maxSize, suffix + 1);
		}
	}
	else {
		//日志文件是新的
		return filePath;
	}
}

void VNCLog::OpenFile()
{
#ifdef SC_20
	return;
#endif
	// Is there a file-name?
	if (strlen(m_filename) == 0)
	{
		m_todebug = true;
		m_tofile = false;
		Print(0, "Error opening log file\n");
		return;
	}

	m_tofile = true;

	// If there's an existing log and we're not appending then move it
	if (!m_append)
	{
		// Build the backup filename
		char* backupfilename = new char[strlen(m_filename) + 5];
		if (backupfilename)
		{
			strcpy_s(backupfilename, strlen(m_filename) + 5, m_filename);
			strcat_s(backupfilename, strlen(m_filename) + 5, ".bak");
			// Attempt the move and replace any existing backup
			// Note that failure is silent - where would we log a message to? ;)
			MoveFileEx(m_filename, backupfilename, MOVEFILE_REPLACE_EXISTING);
			delete[] backupfilename;
		}
	}

	CloseFile();

	// If filename is NULL or invalid we should throw an exception here
	hlogfile = CreateFile(
		m_filename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hlogfile == INVALID_HANDLE_VALUE) {
		// We should throw an exception here
		m_todebug = true;
		m_tofile = false;
		Print(0, "Error opening log file %s\n", m_filename);
	}
	if (m_append) {
		SetFilePointer(hlogfile, 0, NULL, FILE_END);
	}
	else {
		SetEndOfFile(hlogfile);
	}
}

// if a log file is open, close it now.
void VNCLog::CloseFile() {
#ifdef SC_20
	return;
#endif
	if (hlogfile != NULL) {
		CloseHandle(hlogfile);
		hlogfile = NULL;
	}
}

inline void VNCLog::ReallyPrintLine(const char* line)
{
#ifdef SC_20
	return;
#endif
	if (m_todebug) OutputDebugString(line);
	if (m_toconsole) {
		DWORD byteswritten;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), line, strlen(line), &byteswritten, NULL);
	};
	if (m_tofile && (hlogfile != NULL)) {
		DWORD byteswritten;
		WriteFile(hlogfile, line, strlen(line), &byteswritten, NULL);
	}
}

void VNCLog::ReallyPrint(const char* format, va_list ap)
{
#ifdef SC_20
	return;
#endif
	time_t current = time(0);
	if (current != m_lastLogTime) {
		m_lastLogTime = current;
		std::tm* localTime = std::localtime(&m_lastLogTime);
		char buffer[20];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
		ReallyPrintLine(buffer);
	}

	// - Write the log message, safely, limiting the output buffer size
	TCHAR line[(LINE_BUFFER_SIZE * 2) + 1]; // sf@2006 - Prevents buffer overflow
	TCHAR szErrorMsg[LINE_BUFFER_SIZE];
	DWORD  dwErrorCode = GetLastError();
	_vsnprintf(line, LINE_BUFFER_SIZE, format, ap);
	SetLastError(0);
	if (dwErrorCode != 0) {
		if (FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&szErrorMsg,
			LINE_BUFFER_SIZE, NULL) == 0)
		{
			sprintf_s(szErrorMsg, "error code 0x%08X", dwErrorCode);
		}
		strcat_s(line, " --");
		strcat_s(line, szErrorMsg);
	}
	ReallyPrintLine(line);
}

VNCLog::~VNCLog()
{
	try
	{
		CloseFile();
	}
	catch (...)
	{
	}
}

void VNCLog::GetLastErrorMsg(LPSTR szErrorMsg) const {

	DWORD  dwErrorCode = GetLastError();

	// Format the error message.
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&szErrorMsg,
		0, NULL);
}

void VNCLog::SetPath(const std::string& path)
{
	if (path.size() == 0)
	{
		char WORKDIR[MAX_PATH];
		if (GetModuleFileName(NULL, WORKDIR, MAX_PATH))
		{
			char* p = strrchr(WORKDIR, '\\');
			if (p == NULL) return;
			*p = '\0';
		}
		strcpy_s(m_path, WORKDIR);
	}
	else
		strcpy_s(m_path, path.c_str());
}
char* VNCLog::GetPath()
{
	if (strlen(m_path) == 0)
	{
		char WORKDIR[MAX_PATH];
		if (GetModuleFileName(NULL, WORKDIR, MAX_PATH))
		{
			char* p = strrchr(WORKDIR, '\\');
			if (p == NULL) return "";
			*p = '\0';
		}
		strcpy_s(m_path, WORKDIR);
	}

	return m_path;
}

void VNCLog::ClearAviConfig()
{
	char WORKDIR[MAX_PATH];
	strcpy_s(WORKDIR, m_path);
	strcat_s(WORKDIR, "\\");
	strcat_s(WORKDIR, "codec.cfg");
	DeleteFile(WORKDIR);
}
