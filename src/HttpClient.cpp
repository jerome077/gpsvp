/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "HttpClient.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <string.h>
#ifndef UNDER_WINE
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <netdb.h>
	typedef int HANDLE;
#	define closesocket close
#endif

const char * method = "GET";

struct CHttpRequest::Data
{
	Data(std::wstring * pwstrHttpStatus) : _pwstrHttpStatus(pwstrHttpStatus), _h(0) {}
	void Request(const std::string & uri, const std::string & user_agent);
	std::string _error;
	std::string _protocol;
	std::string _code;
	std::string _description;
	int _size;
	std::vector<char> _response;
	std::wstring * _pwstrHttpStatus;
	HANDLE _h;
	void Error(const char * descr);
	int _incoming;
	int _outgoing;
};

void CHttpRequest::Data::Error(const char * descr)
{
	_error = descr;
	if (_pwstrHttpStatus)
	{
		wchar_t buffer[1000];
#ifndef UNDER_WINE
		wsprintf(buffer, L"%S (%d)", descr, WSAGetLastError());
#endif // UNDER_WINE
		*_pwstrHttpStatus = buffer;
	}
}

bool CHttpRequest::bSocketsInitialized = false;

void CHttpRequest::InitSocketsIfNecessary()
{
	if (!bSocketsInitialized)
	{
#ifndef UNDER_WINE
		WSADATA wsaData;
		int wsaRes = WSAStartup(MAKEWORD(1, 1), &wsaData);
		bSocketsInitialized = (0 == wsaRes);
#endif // UNDER_WINE
	}
}

void CHttpRequest::CleanupSocketsIfNecessary()
{
	if (bSocketsInitialized)
	{
#ifndef UNDER_WINE
		WSACleanup();
#endif // UNDER_WINE
	}
}

CHttpRequest::CHttpRequest(std::wstring * pwstrHttpStatus) : _data(new Data(pwstrHttpStatus))
{
}

CHttpRequest::~CHttpRequest()
{
	delete _data;
}

void CHttpRequest::Request(const std::string & uri, const std::string & user_agent)
{
	_data->Request(uri, user_agent);
}

bool CHttpRequest::m_useProxy;
std::string CHttpRequest::m_proxyHost;
std::string CHttpRequest::m_proxyPort;
std::string CHttpRequest::m_proxyAuth;

// assign proxy variables to static members of CHttpRequest
void CHttpRequest::SetProxy(std::wstring prox) 
{
	const wchar_t* wbuf=prox.c_str();
	char buf[101];
	int i;
	i=-1;
	do {
		i++;
		buf[i]=(char) wbuf[i];
	} while (buf[i]!=0 && i<100);
	std::string proxy(buf); // std::wstring to std::string - conversion  may work bad for non-ascii characters!

	std::string server;
	std::string credentials;
	int nauth = proxy.find("@", 0);
	if (nauth!=std::string::npos && nauth!=0) { // there is xxxxx@ at the start
		credentials = proxy.substr(0,nauth);
		const char *s = credentials.c_str();
		int k,v;
		char  ans[100];
		int l = strlen(s);
		const char *cod="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		for (k=0;6*k<8*l;k++) // base64 encoding
		{
			v=s[6*k/8]*256+s[6*k/8+1];
			ans[k]=cod[(v & (63 <<( 10-(6*k)%8)))>>(10-((6*k)%8))];
		}
		while (k%4!=0) {ans[k]='='; k++;}; // resulting length must be 4x
		ans[k]=0;
		m_proxyAuth = std::string(ans);
	}
	else {
		m_proxyAuth=""; // no authentification on proxy
		nauth=-1;
	}

	int p = proxy.find(":", nauth+1);
	int finish = (int) proxy.length();
	CHttpRequest::m_proxyPort = "8080";
	if (p != std::string::npos && p < finish) {
		CHttpRequest::m_proxyPort = proxy.substr(p + 1, finish - p - 1);
		CHttpRequest::m_proxyHost = proxy.substr(nauth+1,p-nauth-1);
	} else {
		CHttpRequest::m_proxyHost = proxy.substr(nauth+1,finish-nauth-1);
	}
	
}

void CHttpRequest::Data::Request(const std::string & uri, const std::string & user_agent)
{
	_outgoing = 0;
	_incoming = 0;
	_size = 0;
	InitSocketsIfNecessary();
	std::string server;
	if (uri.find("http://") != 0) {
		Error("Cannot find http://");
		return;
	}
	int start = strlen("http://");
	int finish = uri.find("/", start);
	if (finish == std::string::npos) {
		Error("Cannot find /");
		return;
	}
	std::string location  = uri.substr(finish);
	int p = uri.find(":", start);
	std::string port = "80";
	if (p != std::string::npos && p < finish) {
		port = uri.substr(p + 1, finish - p - 1);
		finish = p;
	}
	
	std::string host = uri.substr(start, finish - start);

	int wsaRes;
	addrinfo * pAddrInfo;
	std::string full_data;

	if (!CHttpRequest::m_useProxy || CHttpRequest::m_proxyHost.length()==0) // if no proxy - use usual http query
	{
		full_data = std::string() + method + " " + uri.substr(finish) + " HTTP/1.0\r\nHost: " + host + "\r\nUser-Agent: " + user_agent + "\r\n";
		if (wsaRes = getaddrinfo(host.c_str(), port.c_str(), 0, &pAddrInfo)) {
			Error("Cannot resolve name");
			return;
		}
	} else {
		full_data = std::string() + method + " " + uri.c_str() + " HTTP/1.0\r\nHost: " + host + "\r\nUser-Agent: " + user_agent + "\r\n";
		if (wsaRes = getaddrinfo(CHttpRequest::m_proxyHost.c_str(), CHttpRequest::m_proxyPort.c_str(), 0, &pAddrInfo)) {
			Error("Cannot resolve proxy name");
			return;
		}
	}

	int s;
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		Error("Cannot create socket");
		return;
	}

	int r = connect(s, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen);

	if (r != 0) {
		Error("Cannot connect");
		closesocket(s);
		return;
	}

	if (CHttpRequest::m_useProxy && CHttpRequest::m_proxyAuth.length()>0) // if proxy needs authorisation, add it to headers
	{
		// it works without Proxy-Connection: Keep-Alive too.
		full_data=full_data+"Proxy-Authorization: Basic "+CHttpRequest::m_proxyAuth.c_str()+"\r\n\r\n";
		//full_data=full_data+"Proxy-Authorization: Basic "+CHttpRequest::m_proxyAuth.c_str()+"\r\nProxy-Connection: Keep-Alive\r\n\r\n";
	} else {
		full_data+="\r\n";
	}

	r = send(s, full_data.c_str(), full_data.length(), 0);
	_outgoing += r;
	if (r != full_data.length()) {
		Error("Cannot send header");
		closesocket(s);
		return;
	}
	
	const int maxdata = 100000;
	std::auto_ptr<char> apBuffer(new char[maxdata + 1]);
	char * buffer = apBuffer.get();
	int rr = 0;
	do {
		r = recv(s, buffer + rr, maxdata - rr, 0);
		_incoming += r;
		rr += r;
	} while (r > 0 && rr < maxdata);

	if (r < 0) {
		Error("Error receiving");
		closesocket(s);
		return;
	}
	if (rr == 0) {
		Error("Nothing received");
		closesocket(s);
		return;
	}
	buffer[rr] = 0;
	const char * delim1 = strchr(buffer, ' ');
	if (!delim1) {
		Error("First space not found");
		closesocket(s);
		return;
	}
	const char * delim2 = strchr(delim1 + 1, ' ');
	if (!delim2) {
		Error("Second space not found");
		closesocket(s);
		return;
	}
	const char * lineend = strstr(buffer, "\r\n");
	if (lineend < delim2) {
		Error("No two spaces in first line found");
		closesocket(s);
		return;
	}
	const char * body = strstr(buffer, "\r\n\r\n");
	if (!body) {
		Error("Body not found");
		closesocket(s);
		return;
	}
	// const char * contentlength = strstr(buffer, "\r\nContent-length: ");
	// if (!contentlength) {
	//	Error("Content-length not found");
	//	std::cout << buffer;
	//	return;
	// }
	_protocol = std::string(buffer, delim1 - buffer);
	_code = std::string(delim1 + 1, delim2 - delim1 - 1);
	_description = std::string(delim2 + 1, lineend - delim2 - 1);
	_size = (buffer + rr) - (body + 4);
	_response.resize(_size);
	if (_size)
		memcpy(&_response[0], body + 4, _size);
	if (_code != "200") {
		Error((_code + " " + _description).c_str());
		closesocket(s);
		return;
	}
	// std::cout << _response.c_str() << "\n";
	closesocket(s);
}

bool CHttpRequest::IsGood() const 
{
	return _data->_error.length() == 0;
}

const char * CHttpRequest::GetErrorDescr() const 
{
	return _data->_error.c_str();
}

const char * CHttpRequest::GetResult() const
{
	if (_data->_size)
		return &_data->_response[0];
	else
		return 0;
}

int CHttpRequest::GetSize() const
{
	return _data->_size;
}

int CHttpRequest::GetIncoming() const
{
	return _data->_incoming;
}

int CHttpRequest::GetOutgoing() const
{
	return _data->_outgoing;
}

