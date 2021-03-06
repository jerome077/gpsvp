﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

// ---------------------------------------------------------------

class CHttpRequest
{
private:
	static bool bSocketsInitialized;
	struct Data;
	Data * _data;
public:
	static bool m_useProxy; //use proxy flag, is syncronized with Use Proxy menu option
	static std::string m_proxyHost;
	static std::string m_proxyPort;
	static std::string m_proxyAuth; // base64-encoded std::string "user:password"
	
	CHttpRequest(std::wstring * pwstrHttpStatus);
	~CHttpRequest();
	void Request(const std::string & uri, const std::string & user_agent);
	bool IsGood() const;
	const char * GetErrorDescr() const;
	const char * GetResult() const;
	int GetSize() const;
	int GetOutgoing() const;
	int GetIncoming() const;
	static void InitSocketsIfNecessary();
	static void CleanupSocketsIfNecessary();
	static void SetProxy(std::wstring proxy);
};

// ---------------------------------------------------------------

#endif // HTTPCLIENT_H
