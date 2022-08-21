
#ifndef SAMPSRV_HTTPS_H
#define SAMPSRV_HTTPS_H

#include "httpclient.h"

#define MAX_HTTP_REQUESTS 200

//----------------------------------------------------

#define SCRIPT_HTTP_STATE_NONE			0
#define SCRIPT_HTTP_STATE_WAITING		1
#define SCRIPT_HTTP_STATE_PROCESSING	2
#define SCRIPT_HTTP_STATE_PROCESSED		3

//----------------------------------------------------

#pragma pack(1)
struct ScriptHttp_s
{
	CHttpClient* pHttpClient;
	int iState;
	int iIndex;
	int iType;
	std::string url;
	std::string data;
	bool bSilent;
	AMX* pAMX;
	char szCallback[40];
	DWORD dwTime;
	int field_79;

	ScriptHttp_s(char *szBindAddress)
	{
		pHttpClient = new CHttpClient(szBindAddress);
		iState = SCRIPT_HTTP_STATE_NONE;
		iIndex = 0;
		iType = 0;
		bSilent = true;
		memset(szCallback,0,sizeof(szCallback));
		dwTime = 0;
		field_79 = 0;
	}

	~ScriptHttp_s()
	{
		SAFE_DELETE(pHttpClient);
	}
};

//----------------------------------------------------

class CScriptHttps
{
private:
	ScriptHttp_s *pHttps[MAX_HTTP_REQUESTS];

public:
	CScriptHttps();

	BOOL New(int iIndex, int iType, PCHAR szURL, PCHAR szData, PCHAR szBindAddress, bool bSilent, AMX* amx, PCHAR szCallback);

	void Process();
};

//----------------------------------------------------

#endif
