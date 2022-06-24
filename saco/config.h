
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_CONFIG_STRSIZE 256
#define MAX_CONFIG_ENTRIES 512
#define MAX_CONFIG_DIRNAME 40

//----------------------------------------------------

enum CONFIG_DATATYPE {
	CONFIG_DATATYPE_NONE,
	CONFIG_DATATYPE_INT,
	CONFIG_DATATYPE_STRING,
	CONFIG_DATATYPE_FLOAT,
};

//----------------------------------------------------

typedef struct _CONFIG_ENTRY
{
	char szDirectiveName[MAX_CONFIG_DIRNAME+1];
	BOOL bReadOnly;
	CONFIG_DATATYPE DataType;
	int iDirectiveData;
	float fDirectiveData;
	char *szDirectiveData;
} CONFIG_ENTRY;

//----------------------------------------------------

class CConfig
{
private:
	CONFIG_ENTRY	m_Entries[MAX_CONFIG_ENTRIES];
	BOOL			m_bEntrySlotState[MAX_CONFIG_ENTRIES];
	CHAR			m_szConfigFile[MAX_PATH+1];
	int				m_iNumEntries;

	void RecalcEntrySize();

	int FindConfigEntry(char *szDirectiveName);
	int CreateConfigEntry(char *szDirectiveName);
	void AddConfigEntry(char * szName, char * wsData);

	int ReadConfigFile();
	int WriteConfigFile();
	
public:
	CConfig(char *szConfigFile);
	~CConfig();

	bool IsConfigEntryExist(char *szDirectiveName);

	CONFIG_DATATYPE GetConfigEntryType(char *szDirectiveName);
	CONFIG_ENTRY* GetConfigEntryAtIndex(int iIndex);
	BOOL RemoveConfigEntry(char *szDirectiveName);

	BOOL SetIntConfigEntry(char *szDirectiveName, int iData, BOOL bReadOnly=FALSE);
	BOOL SetStringConfigEntry(char *szDirectiveName, char *szData, BOOL bReadOnly=FALSE);
	BOOL SetFloatConfigEntry(char *szDirectiveName, float fData, BOOL bReadOnly=FALSE);

	int GetIntConfigEntry(char *szDirectiveName);
	char* GetStringConfigEntry(char *szDirectiveName);
	float GetFloatConfigEntry(char *szDirectiveName);
};

//----------------------------------------------------

