
#include <stdio.h>
#include <string.h>

#ifdef WIN32
# define SLEEP(x) { Sleep(x); }

# include <windows.h>
#else
# define SLEEP(x) { usleep(x * 1000); }

# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#include "httpclient.h"
#include "runutil.h"
#include "system.h"

//----------------------------------------------------

CHttpClient::CHttpClient(char *szBindAddress)
{
	memset(&m_Request,0,sizeof(HTTP_REQUEST));
	memset(&m_Response,0,sizeof(HTTP_RESPONSE));
	memset(m_szBindAddress,0,256);
	if (szBindAddress) {
		m_bHasBindAddress = 1;
		strcpy(m_szBindAddress,szBindAddress);
	}
	else {
		m_bHasBindAddress = 0;
	}
	m_iError = HTTP_SUCCESS; // Request is successful until otherwise indicated
	m_iSocket = (-1);

// Winsock init
#ifdef WIN32
	WORD				wVersionRequested;
	WSADATA				wsaData;
	wVersionRequested = MAKEWORD(2,2);
	WSAStartup(wVersionRequested,&wsaData);
#endif
}

//----------------------------------------------------

CHttpClient::~CHttpClient()
{
	CloseConnection();

	// Deallocate the response data memory
	if(m_Response.header) free(m_Response.header);
	if(m_Response.response) free(m_Response.response);
	
// Winsock cleanup
#ifdef WIN32
	WSACleanup();
#endif
}

//----------------------------------------------------

int CHttpClient::ProcessURL(int iType, char *szURL, char *szPostData, char *szReferer)
{
	InitRequest(iType,szURL,szPostData,szReferer);

	Process();

	return m_iError;
}

//----------------------------------------------------

bool CHttpClient::GetHeaderValue(char *szHeaderName,char *szReturnBuffer, int iBufSize)
{
	char *szHeaderStart;
	char *szHeaderEnd;
	int iLengthSearchHeader = strlen(szHeaderName);
	int iCopyLength;

	szHeaderStart = Util_stristr(m_Response.header,szHeaderName);
	if(!szHeaderStart) {
		return false;
	}
	szHeaderStart+=iLengthSearchHeader+1;

	szHeaderEnd = strchr(szHeaderStart,'\n');
	if(!szHeaderEnd) {
		szHeaderEnd = m_Response.header + strlen(m_Response.header); // (END OF STRING)
	}

	iCopyLength = szHeaderEnd - szHeaderStart;
	if(iBufSize < iCopyLength) {
		return false;
	}

	memcpy(szReturnBuffer,szHeaderStart,iCopyLength);
	szReturnBuffer[iCopyLength] = '\0';
	return true;
}

//----------------------------------------------------

bool CHttpClient::Connect(char *szHost, int iPort)
{
	struct sockaddr_in	sa;
	struct sockaddr_in	bind_sa;
	struct hostent		*hp;
	struct hostent		*bind_hp;

	// Hostname translation
	if((hp=(struct hostent *)gethostbyname(szHost)) == NULL ) {
		m_iError=HTTP_ERROR_BAD_HOST;
		return false;
	}

	// Prepare a socket	
	memset(&sa,0,sizeof(sa));
	memset(&bind_sa,0,sizeof(bind_sa));
	memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons((unsigned short)iPort);

	if(m_bHasBindAddress) {
		if((bind_hp=(struct hostent *)gethostbyname(m_szBindAddress)) == NULL ) {
			m_iError=HTTP_ERROR_BAD_HOST;
			return false;
		}
		memcpy(&bind_sa.sin_addr,bind_hp->h_addr,bind_hp->h_length);
		bind_sa.sin_family = bind_hp->h_addrtype;
		sa.sin_port = 0;
	}

	if((m_iSocket=socket(AF_INET,SOCK_STREAM,0)) < 0) {
		m_iError=HTTP_ERROR_NO_SOCKET;
		return false;
	}

	if (m_bHasBindAddress && bind(m_iSocket,(struct sockaddr *)&bind_sa,sizeof bind_sa) < 0) {
		m_iError=HTTP_ERROR_CANT_CONNECT;
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = 20000;
	timeout.tv_usec = 20000;
	setsockopt(m_iSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	// Try to connect
	if(connect(m_iSocket,(struct sockaddr *)&sa,sizeof sa) < 0)	{
		CloseConnection();
		m_iError=HTTP_ERROR_CANT_CONNECT;
		return false;
	}

	return true;
}

//----------------------------------------------------

void CHttpClient::CloseConnection()
{
#ifdef WIN32
	closesocket(m_iSocket);
#else
	close(m_iSocket);
#endif
}

//----------------------------------------------------

bool CHttpClient::Send(char *szData)
{
	if(send(m_iSocket,szData,strlen(szData),0) < 0) {
		m_iError = HTTP_ERROR_CANT_WRITE;
		return false;
	}
	return true;
}

//----------------------------------------------------

int CHttpClient::Recv(char *szBuffer, int iBufferSize)
{
	return recv(m_iSocket,szBuffer,iBufferSize,0);
}

//----------------------------------------------------

void CHttpClient::InitRequest(int iType, char *szURL, char *szPostData, char *szReferer)
{
	char		 port[129];		// port string
	char		 *port_char;	// position of ':' if any
	unsigned int slash_pos;		// position of first '/' numeric
	char		 *slash_ptr;
	char		 szUseURL[2049]={0}; // incase we have to cat something to it.

	memset(szUseURL,0,sizeof(szUseURL));
	strncpy(szUseURL,szURL,2048);

	m_Request.rtype = iType;
	strncpy(m_Request.referer,szReferer,256);

	if(iType==HTTP_POST) {
		strncpy(m_Request.data,szPostData,8192);
	}

	// Copy hostname from URL
	slash_ptr = strchr(szUseURL,'/');

	if(!slash_ptr) {
		strcat(szUseURL,"/");
		slash_ptr = strchr(szUseURL,'/');
	}

	slash_pos=(slash_ptr-szUseURL);
	if(slash_pos > 256) slash_pos = 256;
	memcpy(m_Request.host,szUseURL,slash_pos);
	m_Request.host[slash_pos]='\0';

	// Copy the rest of the url to the file string.
	strncpy(m_Request.file,strchr(szUseURL,'/'),4096);

	// Any special port used in the URL?
	if((port_char=strchr(m_Request.host,':'))!=NULL) {
		memset(port,0,sizeof(port));
		strncpy(port,port_char+1,sizeof(port)-1);
		*port_char='\0';
		m_Request.port = atoi(port);
	}
	else {
		m_Request.port = 80;
	}
}

//----------------------------------------------------

void CHttpClient::Process()
{
	int   header_len;

	if(!Connect(m_Request.host,m_Request.port)) {
		return;
	}

	// Build the HTTP Header
	switch(m_Request.rtype)
	{
		case HTTP_GET:
			header_len = strlen(m_Request.file)+strlen(m_Request.host)+
				strlen(GET_FORMAT)+strlen(m_Request.referer);
			if(header_len > 16384) return;
			sprintf(m_Request.request_head,GET_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host);
			break;

		case HTTP_HEAD:
			header_len = strlen(m_Request.file)+strlen(m_Request.host)+
				strlen(HEAD_FORMAT)+strlen(m_Request.referer);
			if(header_len > 16384) return;
			sprintf(m_Request.request_head,HEAD_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host);
			break;

		case HTTP_POST:
			header_len = strlen(m_Request.data)+strlen(m_Request.file)+
				strlen(m_Request.host)+strlen(POST_FORMAT)+
				strlen(USER_AGENT)+strlen(m_Request.referer);
			if(header_len > 16384) return;
			sprintf(m_Request.request_head,POST_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host,strlen(m_Request.data),m_Request.data);
			break;
	}

	if(!Send(m_Request.request_head)) {
		return;
	}

	HandleEntity();
}

//----------------------------------------------------

#define RECV_BUFFER_SIZE 4096

void CHttpClient::HandleEntity()
{
	int				bytes_total		= 0;
	int				bytes_read		= 0;
	char			buffer[RECV_BUFFER_SIZE];
	char			*head_end;
	char			*pcontent_buf;
	char			content_len_str[64];

	bool			header_got		= false;
	bool			has_content_len = false;
	int				content_len		= 0;

	memset(content_len_str,0,sizeof(content_len_str));
	memset(buffer,0,sizeof(buffer));

	m_Response.header = NULL;
	m_Response.response = NULL;
	m_Response.header_len = 0;
	m_Response.response_len = 0;

	while((bytes_read=Recv(buffer,RECV_BUFFER_SIZE)) > 0)
	{
		SLEEP(5);

		bytes_total+=bytes_read;
		m_Response.response=(char *)realloc(m_Response.response,bytes_total+1);
		if(m_Response.response == NULL) {
			bytes_total = 0;
			break;
		}
		memcpy(m_Response.response+(bytes_total-bytes_read),buffer,(unsigned int)bytes_read);

		if(!header_got)
		{
			if((head_end=strstr(m_Response.response,"\r\n\r\n"))!=NULL
				|| (head_end=strstr(m_Response.response,"\n\n"))!=NULL)
			{
				header_got=true;

				m_Response.header_len=(head_end- m_Response.response);
				m_Response.header=(char *)calloc(1,m_Response.header_len+1);
				memcpy(m_Response.header,m_Response.response,m_Response.header_len);
				m_Response.header[m_Response.header_len]='\0';

				if((*(m_Response.response+m_Response.header_len))=='\n') /* LFLF */
				{
					bytes_total-=(m_Response.header_len+2);
					memmove(m_Response.response,(m_Response.response+(m_Response.header_len+2)),bytes_total);
				}
				else /* assume CRLFCRLF */
				{
					bytes_total-=(m_Response.header_len+4);
					memmove(m_Response.response,(m_Response.response+(m_Response.header_len+4)),bytes_total);
				}

				/* find the content-length if available */
				if((pcontent_buf=Util_stristr(m_Response.header,"CONTENT-LENGTH:"))!=NULL)
				{
					has_content_len=true;

					pcontent_buf+=16;
					while(*pcontent_buf!='\n' && *pcontent_buf)
					{
						*pcontent_buf++;
						//content_len++;
					}

					pcontent_buf-=content_len;
					memcpy(content_len_str,pcontent_buf,content_len);

					if(content_len_str[content_len-1] == '\r') {
						content_len_str[content_len-1]='\0';
					}
					else {
						content_len_str[content_len]='\0';
					}

					content_len=atoi(content_len_str);
					if(content_len > MAX_ENTITY_LENGTH) {
						CloseConnection();
						m_iError = HTTP_ERROR_CONTENT_TOO_BIG;
						return;
					}
				}
			}
		}

		if(header_got && has_content_len)
			if(bytes_total>=content_len) break;
	}

	CloseConnection();

	if(m_Response.response)
		m_Response.response[bytes_total]='\0';

	// check the returned header
	if(bytes_total <= 0 || !header_got || Util_strnicmp(m_Response.header,"HTTP",4)) { // 'HTTP'
		m_iError = HTTP_ERROR_MALFORMED_RESPONSE;
		return;
	}

	// Now fill in the response code
	char response_code_str[4];
	strncpy(response_code_str,m_Response.header+9,3);
	response_code_str[3] = '\0';
	m_Response.response_code = atoi(response_code_str);

	// Copy over the document entity strings and sizes
	m_Response.response_len = bytes_total;

	//printf("Code: %u\n\n%s\n",m_Response.response_code,m_Response.header);

	// Try and determine the document type
	m_Response.content_type = CONTENT_TYPE_HTML; // default to html

	char szContentType[256];

	if(GetHeaderValue("CONTENT-TYPE:",szContentType,256) == true) {
		if(strstr(szContentType,"text/html") != NULL) {
			m_Response.content_type = CONTENT_TYPE_HTML;
		}
		else if(strstr(szContentType,"text/plain") != NULL) {
			m_Response.content_type = CONTENT_TYPE_TEXT;
		} else {
			m_Response.content_type = CONTENT_TYPE_UNKNOWN;
		}
	}
}

//----------------------------------------------------
