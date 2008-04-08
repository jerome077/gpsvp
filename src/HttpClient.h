#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>

class CHttpRequest
{
private:
	struct Data;
	Data * _data;
public:
	CHttpRequest(std::wstring * pwstrHttpStatus);
	~CHttpRequest();
	void Request(const std::string & uri, const std::string & user_agent);
	bool IsGood() const;
	const char * GetErrorDescr() const;
	const char * GetResult() const;
	int GetSize() const;
	int GetOutgoing() const;
	int GetIncoming() const;
};

#endif HTTPCLIENT_H
