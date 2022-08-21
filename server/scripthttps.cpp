
#include "main.h"

BOOL StartThread(void *params);

//----------------------------------------------------------------------------------

CScriptHttps::CScriptHttps()
{
	for (int i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		pHttps[i] = NULL;
	}
}

//----------------------------------------------------------------------------------

BOOL CScriptHttps::New(int iIndex, int iType, PCHAR szURL, PCHAR szData, PCHAR szBindAddress, bool bSilent, AMX* amx, PCHAR szCallback)
{
	int iSlot;

	for (iSlot = 0; iSlot != MAX_HTTP_REQUESTS; iSlot++)
	{
		if (pHttps[iSlot] == NULL) break;
	}
	if (iSlot == MAX_HTTP_REQUESTS) return FALSE;

	pHttps[iSlot] = new ScriptHttp_s(szBindAddress);
	pHttps[iSlot]->iState = SCRIPT_HTTP_STATE_WAITING;
	pHttps[iSlot]->iIndex = iIndex;
	pHttps[iSlot]->iType = iType;

	if(szURL && strlen(szURL))
	{
		pHttps[iSlot]->url = szURL;
		if(szData)
			pHttps[iSlot]->data = szData;
		pHttps[iSlot]->bSilent = bSilent;
		pHttps[iSlot]->pAMX = amx;

		if(szCallback)
			strcpy(pHttps[iSlot]->szCallback,szCallback);
		else
			pHttps[iSlot]->szCallback[0] = '\0';

		StartThread(pHttps[iSlot]);
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------------

#ifdef WIN32
DWORD WINAPI HttpThread(LPVOID arg)
#else
void* HttpThread(void* arg)
#endif
{
	ScriptHttp_s* pInfo = (ScriptHttp_s*)arg;

	if(pInfo && pInfo->pHttpClient)
	{
		if(pNetGame)
			pInfo->dwTime = pNetGame->GetTime();

		pInfo->iState = SCRIPT_HTTP_STATE_PROCESSING;
		
		pInfo->pHttpClient->ProcessURL(pInfo->iType,
			(char*)pInfo->url.c_str(), (char*)pInfo->data.c_str(), "sa-mp.com");
		
		pInfo->iState = SCRIPT_HTTP_STATE_PROCESSED;
		pInfo->field_79 = 0;
	}
	return 0;
}

//----------------------------------------------------------------------------------

BOOL StartThread(void *params)
{
#ifdef WIN32
	DWORD dwThreadId = 0;
	HANDLE hHandle = CreateThread(NULL, 0, HttpThread, params, 0, &dwThreadId);
	if (hHandle && hHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hHandle);
		return TRUE;
	}
	return FALSE;
#else
	pthread_attr_t attr;
	pthread_t thr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	return pthread_create(&thr, &attr, HttpThread, params) == 0;
#endif
}

//----------------------------------------------------------------------------------

void CScriptHttps::Process()
{
	for(int i = 0; i < MAX_HTTP_REQUESTS; i++)
	{
		if(pHttps[i] && pHttps[i]->iState == SCRIPT_HTTP_STATE_PROCESSED)
		{
			if(!pHttps[i]->bSilent && pHttps[i]->pAMX)
			{
				int idx;
				cell ret = 0;
				char szData[] = "";
				int iResponseCode;

				if(!amx_FindPublic(pHttps[i]->pAMX, pHttps[i]->szCallback, &idx))
				{
					cell amx_addr, *amx_phys;
						
					if (pHttps[i]->pHttpClient->GetErrorCode())
					{
						amx_PushString(pHttps[i]->pAMX, &amx_addr, &amx_phys, szData, 0, 0);
						iResponseCode = pHttps[i]->pHttpClient->GetErrorCode();
					}
					else
					{
						amx_PushString(pHttps[i]->pAMX, &amx_addr, &amx_phys, pHttps[i]->pHttpClient->GetDocument(), 0, 0);
						iResponseCode = pHttps[i]->pHttpClient->GetResponseCode();
					}
					amx_Push(pHttps[i]->pAMX, iResponseCode);
					amx_Push(pHttps[i]->pAMX, pHttps[i]->iIndex);
					amx_Exec(pHttps[i]->pAMX, &ret, idx);
					amx_Release(pHttps[i]->pAMX, amx_addr);
				}
			}

			delete pHttps[i];
			pHttps[i] = NULL;
		}
	}
}

//----------------------------------------------------------------------------------
// EOF
