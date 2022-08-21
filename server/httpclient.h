
#define HTTP_GET			1
#define HTTP_POST			2
#define HTTP_HEAD			3

//----------------------------------------------------

#define MAX_ENTITY_LENGTH	0x100000

//----------------------------------------------------

#define HTTP_SUCCESS					0
#define HTTP_ERROR_BAD_HOST				1
#define HTTP_ERROR_NO_SOCKET			2
#define HTTP_ERROR_CANT_CONNECT			3
#define HTTP_ERROR_CANT_WRITE			4
#define HTTP_ERROR_CONTENT_TOO_BIG		5
#define HTTP_ERROR_MALFORMED_RESPONSE	6

//----------------------------------------------------

#define CONTENT_TYPE_UNKNOWN		    0
#define CONTENT_TYPE_TEXT				1
#define CONTENT_TYPE_HTML				2

//----------------------------------------------------

#define USER_AGENT	"SAMP/0.3"
#define GET_FORMAT	"GET %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n" 
#define POST_FORMAT "POST %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: %u\r\n\r\n%s"
#define HEAD_FORMAT "HEAD %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n"

//----------------------------------------------------

#pragma pack(1)
typedef struct{
	unsigned short port;		/* remote port */
	int		rtype;				/* request type */
	char	host[257];			/* hostname */
	char 	file[4097];			/* GET/POST request file */
	char 	data[8193];			/* POST data (if rtype HTTP_POST) */
	char 	referer[257];		/* http referer. */
	char	request_head[16385];
} HTTP_REQUEST;

//----------------------------------------------------

#pragma pack(1)
typedef struct{
	char *			header;
	char *			response;
	unsigned int	header_len;
	unsigned long	response_len;
	unsigned int	response_code;
	unsigned int	content_type;
} HTTP_RESPONSE;

//----------------------------------------------------

class CHttpClient
{
private:

	int				m_iSocket;
	int				m_iError;
	HTTP_REQUEST	m_Request;
	HTTP_RESPONSE	m_Response;
	char			m_szBindAddress[257];
	int				m_bHasBindAddress;

	bool Connect(char *szHost, int iPort);
	void CloseConnection();
	bool Send(char *szData);
	int  Recv(char *szBuffer, int iBufferSize);

	void InitRequest(int iType, char *szURL, char *szPostData, char *szReferer);
	void HandleEntity();

	void Process();

public:

	int ProcessURL(int iType, char *szURL, char *szData, char *szReferer);

	bool GetHeaderValue(char *szHeaderName,char *szReturnBuffer, int iBufSize);
	int GetResponseCode() { return m_Response.response_code; };
	int GetContentType() { return m_Response.content_type; };
	char *GetResponseHeaders() { return m_Response.header; };
	char *GetDocument() { return m_Response.response; };
	int GetDocumentLength() { return m_Response.response_len; };
	int GetErrorCode() { return m_iError; };

	CHttpClient(char *szBindAddress);
	~CHttpClient();
};

//----------------------------------------------------
