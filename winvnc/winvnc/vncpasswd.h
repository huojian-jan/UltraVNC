//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
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
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncPasswd

// This header provides helpers for handling encrypted password data.
// The password handling routines found in vncauth.h should not be used directly

class vncPasswd;

#if (!defined(_WINVNC_VNCPASSWD))
#define _WINVNC_VNCPASSWD

#include "stdhdrs.h"
#include "ShadowBotConfig.h"

#ifdef _MSC_VER
extern "C" {
#include "vncauth.h"
}
#else
#include "vncauth.h"
#endif

// Password handling helper class
class vncPasswd
{
public:

	// Password decryptor!
	class ToText
	{
	public:
		inline ToText(const char encrypted[MAXPWLEN], bool secure)
		{
			plaintext = vncDecryptPasswd((char*)encrypted, secure);

#ifdef __SHADOWBOT_BUILD__
			//�������ж�ȡ���룬�浽SettingsManager�У������SettingsManager�л�ȡ����
			std::string cmdPasswd;
			ShadowBotConfig::getInstance()->getAuthPassword(cmdPasswd);

			size_t cpy_len = std::min<size_t>(strlen(cmdPasswd.c_str()), MAXPWLEN);
			strncpy(plaintext, cmdPasswd.c_str(), cpy_len);

			size_t diff_size = MAXPWLEN - cpy_len;
			memset(plaintext + cpy_len, '\0', diff_size * sizeof(char));
#endif // __SHADOWBOT_BUILD__

		}
		inline ~ToText()
		{
			if (plaintext != NULL)
			{
				ZeroMemory(plaintext, strlen(plaintext));
				free(plaintext);
			}
		}
		inline operator const char* () const { return plaintext; };
	private:
		char* plaintext;
	};

	class FromText
	{
	public:
		inline FromText(char* unencrypted, bool secure)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("PASSWD : FromText called\n"));
			vncEncryptPasswd(unencrypted, encrypted, secure);
			// ZeroMemory(unencrypted, strlen(unencrypted));
		}
		inline ~FromText()
		{
		}
		inline operator const char* () const { return encrypted; };
	private:
		char encrypted[MAXPWLEN];
	};

	class FromClear
	{
	public:
		inline FromClear(bool secure)
		{
			//vnclog.Print(LL_INTINFO, VNCLOG("PASSWD : FromClear called\n"));
			vncEncryptPasswd((char*)"", encrypted, secure);
		}
		inline ~FromClear()
		{
		}
		inline operator const char* () const { return encrypted; };
	private:
		char encrypted[MAXPWLEN];
	};
};

#endif