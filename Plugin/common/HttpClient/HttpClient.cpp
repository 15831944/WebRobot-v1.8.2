// HttpClient.cpp: implementation of the CHttpClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CHttpClient::CHttpClient()
{
	m_dwStatusCode = 417;
	m_dwWebPageSize = 0;
	m_strHtml.Empty();
	/*
	m_InetSession = new CInternetSession(_T("session"),
		0,
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD);    //���ò�����*/
}

CHttpClient::~CHttpClient()
{
}

//UTF8תANSI
void CHttpClient::Utf8ToAnsi(CString &strUTF8)
{
	//��ȡת��Ϊ���ֽں���Ҫ�Ļ�������С���������ֽڻ�����
	UINT nLen = MultiByteToWideChar(CP_UTF8,NULL,strUTF8,-1,NULL,NULL);
	WCHAR *wszBuffer = new WCHAR[nLen+1];
	nLen = MultiByteToWideChar(CP_UTF8,NULL,strUTF8,-1,wszBuffer,nLen);
	wszBuffer[nLen] = 0;
	
	nLen = WideCharToMultiByte(936,NULL,wszBuffer,-1,NULL,NULL,NULL,NULL);
	CHAR *szBuffer = new CHAR[nLen+1];
	nLen = WideCharToMultiByte(936,NULL,wszBuffer,-1,szBuffer,nLen,NULL,NULL);
	szBuffer[nLen] = 0;
	
	strUTF8 = szBuffer;
	//�����ڴ�
	delete []szBuffer;
	delete []wszBuffer;
}

typedef struct 
{
	CString strUrl;
	CString strPostData;
	CString strCookie;
	CString strUserAgent;
	int nMode;
	BOOL bGetHtml;
	BOOL bDecodeUtf8;
	CHttpClient *pThis;
} HttpClientParm;   

BOOL CHttpClient::GetData(CString strUrl, int nMode, CString strPostData, CString strCookie, CString strUserAgent, DWORD dwMillisecondsTimeout, DWORD &dwStatusCode, DWORD &dwWebPageSize, BOOL bGetHtml, BOOL bDecodeUtf8, CString &strHtml)
{
	HANDLE hThread = NULL; 
	DWORD dwThreadID = 0;
	DWORD dwExitCode = 0;
	dwStatusCode = 404;
	dwWebPageSize = 0;

	HttpClientParm *threadParm = new HttpClientParm;
	threadParm->strUrl = strUrl;
	threadParm->strPostData = strPostData;
	threadParm->strUserAgent = strUserAgent;
	threadParm->strCookie = strCookie;
	threadParm->nMode = nMode;
	threadParm->bGetHtml = bGetHtml;
	threadParm->bDecodeUtf8 = bDecodeUtf8;
	threadParm->pThis = this;
	
	hThread = CreateThread(
		NULL,            // Pointer to thread security attributes 
		0,               // Initial thread stack size, in bytes 
		(LPTHREAD_START_ROUTINE)WorkerFunction,  // Pointer to thread function 
		threadParm,     // The argument for the new thread
		0,               // Creation flags 
		&dwThreadID      // Pointer to returned thread identifier 
		);				 // Wait for the call to InternetConnect in worker function to complete
	
	if (WaitForSingleObject(hThread, dwMillisecondsTimeout) == WAIT_TIMEOUT)
	{
		// Wait until the worker thread exits
		//��ʱ�ˣ��ر�����
		//WaitForSingleObject(hThread, INFINITE);//���ȴ��ˣ��������߳��Լ���Ȼ��ʱ���أ�����ʱ����Ѿ�����������
		CloseHandle(hThread);
		return FALSE;
	}
	
	// The state of the specified object (thread) is signaled
	//�˺������óɹ�����TRUE,ʧ�ܷ���FALSE,ֻ��ʾ��������Ƿ���óɹ�����.
	//���ܸ��ݷ���ֵ���ж�һ���߳��Ƿ����,��Ҫ���� lpExitCode��ֵ��ȷ��.
	//���̼߳�������,��lpExitCode�д洢ָ���̵߳ķ���ֵ.
	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		CloseHandle(hThread);
		return FALSE;
	}

	CloseHandle(hThread);

	if (!dwExitCode)// Worker function failed
		return FALSE;

	dwStatusCode = m_dwStatusCode;
	dwWebPageSize = m_dwWebPageSize;
	strHtml = m_strHtml;
	
	return TRUE;
}

DWORD WINAPI CHttpClient::WorkerFunction(LPVOID vThreadParm)
{
	HttpClientParm* pThreadParm = (HttpClientParm*)vThreadParm;
	CHttpClient *pThis = pThreadParm->pThis;

	BOOL bReturn = TRUE;
	BOOL bContinue = TRUE;
	CHttpFile* pFile = NULL;
	CHttpConnection* pServer = NULL;
	CInternetSession m_InetSession(_T("session"), 0, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD);//���ò�����
	try
	{
		INTERNET_PORT wPort = 80;
		CString strServer, strObject, s, strHtml;
		DWORD dwType, dwFlags, dwLength = 0;
		
		if (!AfxParseURL(pThreadParm->strUrl, dwType, strServer, strObject, wPort))
			goto END;

		m_InetSession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5 * 1000, 0);
		m_InetSession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5 * 1000, 0);
		m_InetSession.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5 * 1000, 0);
		m_InetSession.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 5 * 1000, 0); //���ͳ�ʱ
		m_InetSession.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5 * 1000, 0); //���ճ�ʱ
		m_InetSession.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);//��ʱ���Դ�����Ĭ��ֵ��5
		
		if (wPort == 443)
		{
			pServer = m_InetSession.GetHttpConnection(strServer, INTERNET_FLAG_SECURE, wPort);
			if (pServer == NULL)
				goto END;
			
			pFile = (CHttpFile*)pServer->OpenRequest(pThreadParm->nMode, strObject, NULL, 1,
				NULL, NULL, INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_SECURE | INTERNET_FLAG_EXISTING_CONNECT |
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_EXISTING_CONNECT |
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_AUTO_REDIRECT);//��ֹ�Զ���ת
			if (pFile == NULL)
				goto END;

			pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
			dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
			pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
		}
		else
		{
			pServer = m_InetSession.GetHttpConnection(strServer, wPort);
			if (pServer == NULL)
				goto END;
			
			pFile = pServer->OpenRequest(pThreadParm->nMode, strObject, NULL, 1, NULL, NULL,
				INTERNET_FLAG_TRANSFER_ASCII | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_EXISTING_CONNECT |
				INTERNET_FLAG_NO_AUTO_REDIRECT);//��ֹ�Զ���ת
			if (pFile == NULL)
				goto END;
		}
		
		if (pThreadParm->strCookie > 8)
			pFile->AddRequestHeaders(pThreadParm->strCookie);
		
		if (pThreadParm->strUserAgent.GetLength() > 11)
			pFile->AddRequestHeaders(pThreadParm->strUserAgent);
		
		if (pThreadParm->nMode == 0)	//Post����
		{
			dwLength = pThreadParm->strPostData.GetLength();
			pFile->SendRequestEx(dwLength);
			pFile->Write(pThreadParm->strPostData, dwLength);
			pFile->EndRequest();
		}
		else 
		{
			pFile->SendRequest();//Get��HEAD
		}
		
		pFile->QueryInfoStatusCode(pThis->m_dwStatusCode);
		
		if (pThreadParm->nMode != CHttpConnection::HTTP_VERB_HEAD)
		{
			// ֻ��ȡҳ���С
			if(pFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, s)) 
				pThis->m_dwWebPageSize = atoi(s);
			else
			{
				while(pFile->ReadString(s) > 0)
					strHtml += s;
				
				pThis->m_dwWebPageSize = strHtml.GetLength();
			}
			
			//��ȡҳ������
			if (pThis->m_dwStatusCode >= 100 && pThis->m_dwStatusCode <= 505 && pThreadParm->bGetHtml)
			{
				while (pFile->ReadString(s))
				{
					strHtml += s;
					strHtml += "\r\n";
				}
				if (pThreadParm->bDecodeUtf8)
					pThis->Utf8ToAnsi(strHtml);
				
				pThis->m_strHtml = strHtml;
			}
		}
	}
	catch (CInternetException* e)
	{
		if (!pThreadParm->bGetHtml)//��ʱ��Ҳ����Ӧ��Ϊ404
			pThis->m_dwStatusCode = 404;
		bReturn = FALSE;
		e->Delete();
	}
	catch (CFileException* e)
	{
		if (!pThreadParm->bGetHtml)//��ʱ��Ҳ����Ӧ��Ϊ404
			pThis->m_dwStatusCode = 404;
		bReturn = FALSE;
		e->Delete();
	}
END:
	if (pFile != NULL)
	{
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	if (pServer != NULL)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	m_InetSession.Close();

	delete pThreadParm;
	return bReturn;
}
