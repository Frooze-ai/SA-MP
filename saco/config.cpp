
#include "main.h"

//----------------------------------------------------------

CConfig::CConfig(char* szConfigFile)
{
	for(int i=0; i!=MAX_CONFIG_ENTRIES; i++)
	{
		memset(&m_Entries[i],0,sizeof(CONFIG_ENTRY));
		m_bEntrySlotState[i] = FALSE;
	}

	m_iNumEntries=0;

	if(szConfigFile && strlen(szConfigFile))
	{
		strcpy(m_szConfigFile,szConfigFile);

		ReadConfigFile();
	}
}

//----------------------------------------------------------

CConfig::~CConfig()
{
	int i=0;
	while(i!=MAX_CONFIG_ENTRIES) {
		if(m_bEntrySlotState[i]) {
			if(m_Entries[i].szDirectiveData) {
				free(m_Entries[i].szDirectiveData);
			}
		}
		m_bEntrySlotState[i] = FALSE;
		i++;
	}
}

//----------------------------------------------------------

void CConfig::RecalcEntrySize()
{
	int iLastIndex=0;
	int i=0;
	while(i < MAX_CONFIG_ENTRIES) {
		if(m_bEntrySlotState[i]) {
			iLastIndex = i;
		}
		i++;
	}
	m_iNumEntries = iLastIndex + 1;
}

//----------------------------------------------------------

int CConfig::FindConfigEntry(char *szName)
{
	strlwr(szName);

	int i=0;

	if(m_iNumEntries <= 0) return (-1);

	while(i < m_iNumEntries) {
		if(m_bEntrySlotState[i]) {
			if(!strcmp(szName, m_Entries[i].szDirectiveName)) {
				return i;
			}
		}
		i++;
	}

	return (-1);
}

//----------------------------------------------------------

bool CConfig::IsConfigEntryExist(char *szDirectiveName)
{
	return FindConfigEntry(szDirectiveName) >= 0;
}

//----------------------------------------------------------

int CConfig::CreateConfigEntry(char* szName)
{
	int iIndex;

	for(iIndex = 0; iIndex < MAX_CONFIG_ENTRIES; iIndex++)
	{
		if(!m_bEntrySlotState[iIndex]) break;
	}
	
	if (iIndex == MAX_CONFIG_ENTRIES) return (-1);

	if(strlen(szName) <= MAX_CONFIG_DIRNAME)
	{
		strlwr(szName);

		CONFIG_ENTRY* pEntry = &m_Entries[iIndex];
		strcpy(pEntry->szDirectiveName,szName);
		pEntry->bReadOnly = FALSE;
		pEntry->DataType = CONFIG_DATATYPE_NONE;
		m_bEntrySlotState[iIndex] = TRUE;

		RecalcEntrySize();

		return iIndex;
	}
	return (-1);
}

//----------------------------------------------------------

int CConfig::GetIntConfigEntry(char *szDirectiveName)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex >= 0) {
		if(m_Entries[iIndex].DataType == CONFIG_DATATYPE_INT) {
			return m_Entries[iIndex].iDirectiveData;
		}
	}
	return 0;
}

//----------------------------------------------------------

char* CConfig::GetStringConfigEntry(char *szDirectiveName)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex >= 0)
	{
		if(m_Entries[iIndex].DataType == CONFIG_DATATYPE_STRING)
		{
			return m_Entries[iIndex].szDirectiveData;
		}
	}
	return NULL;
}

//----------------------------------------------------------

float CConfig::GetFloatConfigEntry(char *szDirectiveName)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex >= 0)
	{
		if(m_Entries[iIndex].DataType == CONFIG_DATATYPE_FLOAT)
		{
			return m_Entries[iIndex].fDirectiveData;
		}
	}
	return 0.0f;
}

//----------------------------------------------------------

BOOL CConfig::RemoveConfigEntry(char *szDirectiveName)
{
	int iIndex = FindConfigEntry(szDirectiveName);

	if(iIndex < 0) return FALSE;

	if(m_Entries[iIndex].szDirectiveData)
		free(m_Entries[iIndex].szDirectiveData);
	memset(&m_Entries[iIndex],0,sizeof(CONFIG_ENTRY));
	m_bEntrySlotState[iIndex] = FALSE;

	RecalcEntrySize();

	return TRUE;
}

//----------------------------------------------------------

CONFIG_DATATYPE CConfig::GetConfigEntryType(char *szDirectiveName)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex >= 0) {
		return m_Entries[iIndex].DataType;
	}
	return CONFIG_DATATYPE_NONE;
}

//----------------------------------------------------------

CONFIG_ENTRY* CConfig::GetConfigEntryAtIndex(int iIndex)
{
	if(iIndex >= 0 && iIndex < MAX_CONFIG_ENTRIES) {
		if(m_bEntrySlotState[iIndex]) {
			return &m_Entries[iIndex];
		}	
	}
	return NULL;
}

//----------------------------------------------------------

CONFIG_DATATYPE DetermineDataType(char *szData)
{
	if(szData && strlen(szData))
	{
		if(szData[0] == '"' && szData[strlen(szData)-1] == '"')
			return CONFIG_DATATYPE_STRING;
		else if(strchr(szData,'.') != NULL)
			return CONFIG_DATATYPE_FLOAT;
		else
			return CONFIG_DATATYPE_INT;
	}
	return CONFIG_DATATYPE_NONE;
}

//----------------------------------------------------------

int CConfig::WriteConfigFile()
{
	FILE *fWriteFile = fopen(m_szConfigFile,"w");

	if(!fWriteFile) {
		return 0;
	}

	int i=0;
	while(i != MAX_CONFIG_ENTRIES) {
		if(m_bEntrySlotState[i]) {
			switch(m_Entries[i].DataType) {
				case CONFIG_DATATYPE_INT:
					fprintf(fWriteFile,"%s=%d\n",
						m_Entries[i].szDirectiveName,m_Entries[i].iDirectiveData);
					break;
				case CONFIG_DATATYPE_STRING:
					if(m_Entries[i].szDirectiveData) {
						fprintf(fWriteFile,"%s=\"%s\"\n",
							m_Entries[i].szDirectiveName,m_Entries[i].szDirectiveData);
					}
					break;
				case CONFIG_DATATYPE_FLOAT:
					fprintf(fWriteFile,"%s=%f\n",
						m_Entries[i].szDirectiveName,m_Entries[i].fDirectiveData);
					break;
			}
		}
		i++;
	}

	fclose(fWriteFile);
	return 1;
}

//----------------------------------------------------------

BOOL CConfig::SetIntConfigEntry(char *szDirectiveName, int iData, BOOL bReadOnly)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex < 0)
	{
		iIndex = CreateConfigEntry(szDirectiveName);
		if(iIndex < 0)
		{
			return FALSE;
		}
	}

	CONFIG_ENTRY *pEntry = &m_Entries[iIndex];

	if(bReadOnly)
	{
		pEntry->bReadOnly = TRUE;
	}
	else if(pEntry->bReadOnly)
	{
		return FALSE;
	}

	pEntry->iDirectiveData = iData;
	pEntry->DataType = CONFIG_DATATYPE_INT;

	WriteConfigFile();

	return TRUE;
}

//----------------------------------------------------------

BOOL CConfig::SetStringConfigEntry(char *szDirectiveName, char *szData, BOOL bReadOnly)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex < 0)
	{
		iIndex = CreateConfigEntry(szDirectiveName);
		if(iIndex < 0)
		{
			return FALSE;
		}
	}

	CONFIG_ENTRY *pEntry = &m_Entries[iIndex];

	if(bReadOnly)
	{
		pEntry->bReadOnly = TRUE;
	}
	else if(pEntry->bReadOnly)
	{
		return FALSE;
	}

	pEntry->DataType = CONFIG_DATATYPE_STRING;
	if (pEntry->szDirectiveData)
		free(pEntry->szDirectiveData);
	pEntry->szDirectiveData = (char *)calloc(1,strlen(szData)+1);
	strcpy(pEntry->szDirectiveData,szData);

	WriteConfigFile();

	return TRUE;
}

//----------------------------------------------------------

BOOL CConfig::SetFloatConfigEntry(char *szDirectiveName, float fData, BOOL bReadOnly)
{
	int iIndex = FindConfigEntry(szDirectiveName);
	if(iIndex < 0)
	{
		iIndex = CreateConfigEntry(szDirectiveName);
		if(iIndex < 0)
		{
			return FALSE;
		}
	}

	CONFIG_ENTRY *pEntry = &m_Entries[iIndex];

	if(bReadOnly)
	{
		pEntry->bReadOnly = TRUE;
	}
	else if(pEntry->bReadOnly)
	{
		return FALSE;
	}

	pEntry->fDirectiveData = fData;
	pEntry->DataType = CONFIG_DATATYPE_FLOAT;

	WriteConfigFile();

	return TRUE;
}

//----------------------------------------------------------

void CConfig::AddConfigEntry(char * szName, char * wsData)
{
	CONFIG_DATATYPE DataType = DetermineDataType(wsData);

	switch(DataType) {
		case CONFIG_DATATYPE_INT:
			SetIntConfigEntry(szName,atoi(wsData));
			break;
		case CONFIG_DATATYPE_STRING:
		{
			DWORD dwLen = strlen(wsData);
			strncpy(wsData,wsData+1,dwLen-1);
			wsData[dwLen-2]='\0';
			SetStringConfigEntry(szName,wsData);
			break;
		}
		case CONFIG_DATATYPE_FLOAT:
			SetFloatConfigEntry(szName,atof(wsData));
			break;
	}
}

//----------------------------------------------------------

int CConfig::ReadConfigFile()
{
	char	szReadBuffer[MAX_CONFIG_STRSIZE];
	char	szDirectiveName[MAX_CONFIG_STRSIZE];
	char	szDirectiveData[MAX_CONFIG_STRSIZE];

	char	*szReadPtr;
	int		iDirectiveLength;
	int		iDirectiveDataLength;

	FILE	*fReadFile = fopen(m_szConfigFile,"r");

	if(!fReadFile) {
		return 0;
	}

	while(!feof(fReadFile)) {
		fgets(szReadBuffer,MAX_CONFIG_STRSIZE,fReadFile);
		szReadPtr = szReadBuffer;
		iDirectiveLength = 0;
		iDirectiveDataLength = 0;

		while(*szReadPtr == ' ' || *szReadPtr == '\t') szReadPtr++;

		if( *szReadPtr == '\0' || *szReadPtr == ';' || 
			*szReadPtr == '\n' || *szReadPtr == '[' ) {

			continue;
		}

		while( *szReadPtr != '\0' && 
			 *szReadPtr != ' ' &&
			 *szReadPtr != '=' &&
			 *szReadPtr != '\n' &&
			 *szReadPtr != '\t' &&
			 *szReadPtr != ';' ) {
			szDirectiveName[iDirectiveLength] = toupper(*szReadPtr);
			iDirectiveLength++;
			szReadPtr++;
		}

		if(iDirectiveLength == 0) {
			continue;
		}

		szDirectiveName[iDirectiveLength] = '\0';

		while(*szReadPtr == ' ' || *szReadPtr == '\t') szReadPtr++;

		if(*szReadPtr != '=') {
			continue;
		}

		*szReadPtr++;

		while(*szReadPtr == ' ' || *szReadPtr == '\t') szReadPtr++;

		while( *szReadPtr != '\0' && 
			 *szReadPtr != '\n' ) {
			szDirectiveData[iDirectiveDataLength] = *szReadPtr;
			iDirectiveDataLength++;
			szReadPtr++;
		}

		if(iDirectiveDataLength == 0) {
			continue;
		}

		szDirectiveData[iDirectiveDataLength] = '\0';

		iDirectiveDataLength--;
		while(szDirectiveData[iDirectiveDataLength] == ' '  ||
			  szDirectiveData[iDirectiveDataLength] == '\t' ||
			  szDirectiveData[iDirectiveDataLength] == '\r')
		{
			szDirectiveData[iDirectiveDataLength] = '\0';
			iDirectiveDataLength--;
		}

		AddConfigEntry(szDirectiveName,szDirectiveData);
	}

	fclose(fReadFile);
	return 1;
}

//----------------------------------------------------------
