// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetPost.h"
#include "MainDlg.h"
#include "HeaderDlg.h"
#include "../common/AutoLock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

RECT rcSrc;
int nStart, nEnd;
CMainDlg *g_pMainDlg = NULL;
CCriticalSection g_csGetPost;
/////////////////////////////////////////////////////////////////////////////
// CMainDlg dialog


CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDlg)
	m_strUrl = _T("http://");
	m_bDecodeUtf8 = FALSE;
	m_strPostValue = _T("");
	m_bMutiThread = FALSE;
	m_nThread = 100;
	//}}AFX_DATA_INIT
	m_nCurrentThreadCount = 0;
	((CGetPostApp *)AfxGetApp())->pMainDlg = this;
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDlg)
	DDX_Control(pDX, IDC_BUTTON_INSERT, m_btnInsert);
	DDX_Control(pDX, IDC_BUTTON_EXEC, m_btnExec);
	DDX_Control(pDX, IDC_EDIT_POST, m_editPost);
	DDX_Control(pDX, IDC_EDIT_HEADER, m_editHeader);
	DDX_Control(pDX, IDC_COMBO_MOTHOD, m_ComboType);
	DDX_Text(pDX, IDC_EDIT_URL, m_strUrl);
	DDX_Check(pDX, IDC_CHECK_UTF8TOANSI, m_bDecodeUtf8);
	DDX_Text(pDX, IDC_EDIT_POST, m_strPostValue);
	DDX_Check(pDX, IDC_CHECK_MUTI_THREAD, m_bMutiThread);
	DDX_Text(pDX, IDC_EDIT_THREAD, m_nThread);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	//{{AFX_MSG_MAP(CMainDlg)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_MOTHOD, OnSelchangeComboMothod)
	ON_BN_CLICKED(IDC_BUTTON_EXEC, OnButtonExec)
	ON_BN_CLICKED(IDC_BUTTON_INSERT, OnButtonInsert)
	ON_EN_KILLFOCUS(IDC_EDIT_POST, OnKillfocusEditPost)
	ON_EN_CHANGE(IDC_EDIT_HEADER, OnChangeEditHeader)
	ON_EN_CHANGE(IDC_EDIT_POST, OnChangeEditPost)
	ON_EN_CHANGE(IDC_EDIT_RETURN, OnChangeEditReturn)
	ON_EN_SETFOCUS(IDC_EDIT_POST, OnSetfocusEditPost)
	ON_COMMAND(ID_MENUITEM_COOKIE, OnMenuitemCookie)
	ON_COMMAND(ID_MENUITEM_REFERER, OnMenuitemReferer)
	ON_COMMAND(ID_MENUITEM_UA_BAIDU, OnMenuitemUaBaidu)
	ON_COMMAND(ID_MENUITEM_UA_GOOGLE, OnMenuitemUaGoogle)
	ON_COMMAND(ID_MENUITEM_UA_IE, OnMenuitemUaIe)
	ON_COMMAND(ID_MENUITEM_UA_CHROME, OnMenuitemUaChrome)
	ON_COMMAND(ID_MENUITEM_UA_ANDROID, OnMenuitemUaAndroid)
	ON_COMMAND(ID_MENUITEM_UA_IPHONE, OnMenuitemUaIphone)
	ON_COMMAND(ID_MENUITEM_FAKEIP, OnMenuitemFakeip)
	ON_BN_CLICKED(IDC_STATIC_HEADER, OnStaticHeader)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_STATIC_LOAD_SAVE, OnStaticLoadSave)
	ON_COMMAND(ID_MENUITEM_SAVE, OnMenuitemSave)
	ON_COMMAND(ID_MENUITEM_LOAD, OnMenuitemLoad)
	ON_WM_MEASUREITEM()
	ON_COMMAND(ID_MENUITEM_COPYALL, OnMenuitemCopyall)
	ON_COMMAND(ID_MENUITEM_BROWSER, OnMenuitemBrowser)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_MENUITEM_CLEAR, OnMenuitemClear)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_CHECK_MUTI_THREAD, OnCheckMutiThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CMainDlg)
EASYSIZE(IDC_EDIT_URL, ES_BORDER, ES_BORDER, ES_BORDER, IDC_COMBO_MOTHOD, 0)//��������
EASYSIZE(IDC_BUTTON_EXEC, IDC_EDIT_URL, ES_BORDER, ES_BORDER, IDC_COMBO_MOTHOD, 0)//��������
END_EASYSIZE_MAP
/////////////////////////////////////////////////////////////////////////////
// CMainDlg message handlers

BOOL CMainDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	INIT_EASYSIZE;
	g_pMainDlg = (CMainDlg *)((CGetPostApp *)AfxGetApp())->pMainDlg;
	// TODO: Add extra initialization here
	//��������
	m_ComboType.SendMessage(CB_SETITEMHEIGHT, -1, 15);
	m_ComboType.SendMessage(CB_SETITEMHEIGHT, 0, 17);
	m_ComboType.SetCurSel(0);
	
	m_infotip.Create(this);
	m_infotip.SetMaxTipWidth(450); 
	m_infotip.SetTipTextColor(RGB(0,170,64));
	m_infotip.AddTool(GetDlgItem(IDC_EDIT_URL), _T("http://www.example.com/"));
	m_infotip.AddTool(GetDlgItem(IDC_STATIC_HEADER), _T("�������ץȡ����Http Header��"));
	m_infotip.AddTool(GetDlgItem(IDC_STATIC_LOAD_SAVE), _T("������߱���������Ϣ"));
	m_infotip.AddTool(GetDlgItem(IDC_EDIT_HEADER), _T("Http Header��ÿ��һ������"));
	m_infotip.AddTool(GetDlgItem(IDC_CHECK_UTF8TOANSI), _T("���������Ϣ�������UTF8��ʽ���ݣ����Թ�ѡ������ת�����ݱ����ʽΪANSI"));
	m_infotip.AddTool(GetDlgItem(IDC_BUTTON_INSERT), 
		_T("�ڹ�괦���롰[\\0]����Post����ʱ�����Զ��޸�Ϊʮ�������ַ�0x00�����ܣ�����ϴ��ļ�ʱ�����ñ���·�����ļ������Զ��壬��ô�ύ0x00����ʵ�ֽضϴӶ��ϴ�ָ����׺�ļ�"));
	m_infotip.AddTool(GetDlgItem(IDC_CHECK_MUTI_THREAD), _T("��Ҫ���ڼ�Ⲣ�������µİ�ȫ���գ�����ʱ��5����"));


	m_editHeader.SetPromptColor(RGB(188,188,188));
	m_editHeader.SetPromptText(_T("����Ctrl��������������������Header"));
	
	((CEdit *)GetDlgItem(IDC_EDIT_POST))->SetLimitText(-1);
	((CEdit *)GetDlgItem(IDC_EDIT_RETURN))->SetLimitText(-1);
	
	GetDlgItem(IDC_EDIT_HEADER)->GetWindowRect(&rcSrc);//��ȡ�ؼ��������Ļ��λ��
	ScreenToClient(&rcSrc);//ת��Ϊ�Ի����ϵ����λ��
	RECT rc;
	rc.left = rcSrc.left;
	rc.right = rcSrc.right;
	rc.top = rcSrc.top;
	rc.bottom = rcSrc.bottom + 130;
	GetDlgItem(IDC_EDIT_HEADER)->MoveWindow(&rc);
	
	GetDlgItem(IDC_BUTTON_INSERT)->ShowWindow(SW_HIDE);
	OnSelchangeComboMothod();
	OnCheckMutiThread();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
	// TODO: Add your message handler code here
	if (GetParent()->m_hWnd != NULL && m_editHeader.m_hWnd != NULL)
	{
		RECT rc;
		GetParent()->GetClientRect(&rc);
		MoveWindow(&rc);
		Resize();
	}
}

void CMainDlg::Resize()
{
    CRect rcDlg;
	//GetParent()->GetClientRect(rcDlg);
	GetClientRect(rcDlg);

	if (m_editPost.GetStyle() & WS_VISIBLE)
	{
		CRect rcEditHeader;
		rcEditHeader.left    = rcDlg.left + 81;
		rcEditHeader.right   = rcDlg.right - 10;
		rcEditHeader.top     = rcDlg.top + 60;
		rcEditHeader.bottom  = rcEditHeader.top   + 155;
		this->MoveDlgItem(IDC_EDIT_HEADER, rcEditHeader, TRUE);
		
		CRect rcEditPost;
		rcEditPost.left       = rcDlg.left + 81;
		rcEditPost.right      = rcDlg.right - 10;
		rcEditPost.top        = rcEditHeader.bottom + 3;
		rcEditPost.bottom     = rcDlg.top + 380;
		this->MoveDlgItem(IDC_EDIT_POST, rcEditPost, TRUE);
		
		CRect rcEditReturn;
		rcEditReturn.left       = rcDlg.left + 81;
		rcEditReturn.right      = rcDlg.right - 10;
		rcEditReturn.top        = rcEditPost.bottom + 3;
		rcEditReturn.bottom     = rcDlg.bottom - 10;
		this->MoveDlgItem(IDC_EDIT_RETURN, rcEditReturn, TRUE);
	}
	else
	{
		CRect rcEditHeader;
		rcEditHeader.left       = rcDlg.left + 81;
		rcEditHeader.right      = rcDlg.right - 10;
		rcEditHeader.top        = rcDlg.top + 60;
		rcEditHeader.bottom     = rcDlg.top + 380;
		this->MoveDlgItem(IDC_EDIT_HEADER, rcEditHeader, TRUE);
		
		CRect rcEditReturn;
		rcEditReturn.left       = rcDlg.left + 81;
		rcEditReturn.right      = rcDlg.right - 10;
		rcEditReturn.top        = rcEditHeader.bottom + 3;
		rcEditReturn.bottom     = rcDlg.bottom - 10;
		this->MoveDlgItem(IDC_EDIT_RETURN, rcEditReturn, TRUE);
	}
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_MOUSEMOVE)//����������Ի����
		m_infotip.RelayEvent(pMsg);

	if (pMsg->message == WM_KEYDOWN)   
	{
		int nVirtKey = (int)pMsg->wParam;           
		if (nVirtKey == VK_RETURN)   
		{   
			//����ǻس�����������Ҫ��������,����ʲôҲ����
			if (GetFocus() != GetDlgItem(IDC_EDIT_HEADER) && GetFocus() != GetDlgItem(IDC_EDIT_POST))
				return TRUE;
		}   
		if (nVirtKey == VK_ESCAPE)   
		{   
			//�����ESC����������Ҫ��������,����ʲôҲ����
			return TRUE;
		}
	}
	else if(pMsg->message == WM_LBUTTONDOWN)
	{
		if(::GetKeyState(VK_CONTROL) < 0 && pMsg->hwnd == m_editHeader.m_hWnd)
		{
			if (m_editHeader.GetStyle() & ES_READONLY)
				return TRUE;
			m_editHeader.SetFocus();
			CNewMenu popup;
			popup.LoadMenu(IDR_MENU_HEADER);
			popup.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
			CNewMenu*	pM = (CNewMenu *)popup.GetSubMenu(0);
			CPoint	p;
			GetCursorPos(&p);
			pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
			popup.DestroyMenu();
			return TRUE;
		}
	}
	if(pMsg->message == WM_RBUTTONDOWN)
	{
		CPoint oPoint;
		GetCursorPos(&oPoint);
		if(WindowFromPoint(oPoint) == GetDlgItem(IDC_EDIT_RETURN))//�жϽ����Ƿ��ڿ���
		{
			CNewMenu OnLineMenu, *pSubMenu;
			OnLineMenu.LoadMenu(IDR_MENU_RETURN);
			OnLineMenu.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
			pSubMenu = (CNewMenu *)OnLineMenu.GetSubMenu(0);
			CPoint oPoint;
			GetCursorPos(&oPoint);
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, oPoint.x, oPoint.y, this);
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CMainDlg::MoveDlgItem(int nID, const CRect& rcPos, BOOL bRepaint)
{
    CWnd* pWnd = this->GetDlgItem(nID);
    if (NULL == pWnd)
        return;
    pWnd->MoveWindow(rcPos, bRepaint);
}

void CMainDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;	//�����������ʱ������
	CDialog::PostNcDestroy();
}

void CMainDlg::OnSelchangeComboMothod() 
{
	UpdateData();
	
	RECT rc;
	if (m_ComboType.GetCurSel() == 0)
	{
		rc.left = rcSrc.left;
		rc.right = rcSrc.right;
		rc.top = rcSrc.top;
		rc.bottom = rcSrc.bottom + 159;
		GetDlgItem(IDC_EDIT_HEADER)->MoveWindow(&rc);
		GetDlgItem(IDC_STATIC_POST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_POST)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_BUTTON_EXEC, "Get");
	}
	else
	{
		GetDlgItem(IDC_EDIT_HEADER)->MoveWindow(&rcSrc);
		GetDlgItem(IDC_STATIC_POST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_POST)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_BUTTON_EXEC, "Post");
	}
	PostMessage(WM_SIZE);
}

void CMainDlg::OnButtonExec() 
{
	UpdateData();

	if (m_bMutiThread)
	{
		m_nCurrentThreadCount = 0;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MutiMainThread, this, 0, NULL);
	}
	else
	{
		SetDlgItemText(IDC_EDIT_RETURN, "");
		HANDLE	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, this, 0, NULL);
		WaitForSingleObject(hThread, 100);
		CloseHandle(hThread);
	}
}

DWORD WINAPI CMainDlg::MainThread(LPVOID lparam)
{
	CMainDlg *pThis = (CMainDlg *)lparam;
	pThis->GetPost();
	pThis->OnChangeEditReturn();
	return -1;	
}

DWORD WINAPI CMainDlg::MutiMainThread(LPVOID lparam)
{
	CMainDlg *pThis = (CMainDlg *)lparam;

	pThis->GetDlgItem(IDC_BUTTON_EXEC)->EnableWindow(FALSE);
	DWORD dwStartTime = GetTickCount();
	while (1)
	{
		if (pThis->m_nCurrentThreadCount > pThis->m_nThread)
		{
			Sleep(200);
			continue;
		}
		if (GetTickCount() - dwStartTime > 5 * 60 * 1000)
		{
			break;
		}

		HANDLE	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MutiSubThread, pThis, 0, NULL);
		if (hThread != NULL)
		{
			g_csGetPost.Lock();
			pThis->m_nCurrentThreadCount++;	
			g_csGetPost.Unlock();
		}
	}
	pThis->GetDlgItem(IDC_BUTTON_EXEC)->EnableWindow(TRUE);

	return -1;	
}

DWORD WINAPI CMainDlg::MutiSubThread(LPVOID lparam)
{
	CMainDlg *pThis = (CMainDlg *)lparam;

	try
	{
		pThis->GetPost();
	}
	catch (...){}

	g_csGetPost.Lock();
	pThis->m_nCurrentThreadCount--;	
	g_csGetPost.Unlock();

	return -1;	
}

CString Utf8ToAnsi(CString szUtf8)
{
	TCHAR * pSrc = (TCHAR*)szUtf8.GetBuffer(0);
	
	int i = MultiByteToWideChar(CP_UTF8, 0, pSrc, -1, NULL, 0);  
	
	WCHAR * pszW = new WCHAR[i+1]; pszW[i] = 0;
	
	MultiByteToWideChar(CP_UTF8, 0, pSrc, -1, pszW, i);
	i = WideCharToMultiByte(CP_ACP, 0, pszW, -1, NULL, 0, NULL, NULL);   
	
	TCHAR * pszT = new TCHAR[i+1]; pszT[i] = 0;
	WideCharToMultiByte(CP_ACP, 0, pszW, -1, pszT, i, NULL, NULL);
	
	szUtf8 = pszT;
	
	delete [] pszT;
	delete [] pszW;
	
	return szUtf8;
}

BYTE *SetZero(CString strPostData, int &nLength)
{
	CString strBuffer;
	strBuffer = strPostData;
	strBuffer.Replace("[\\0]", "@");
	nLength = strBuffer.GetLength();
	BYTE *lpBuffer = new BYTE[nLength];
	memset(lpBuffer, 0, nLength);
	
	int nCurSize = 0;
	while (1)
	{
		int nPos = strPostData.Find("[\\0]", 0);
		if (nPos == -1)
		{
			memcpy(lpBuffer + nCurSize, strPostData, strPostData.GetLength());
			break;
		}
		memcpy(lpBuffer + nCurSize, strPostData.Mid(0, nPos), nPos + 1);
		nCurSize += (nPos + 1);
		strPostData.Delete(0, nPos + 4);
	}
	return lpBuffer;
}

CString intToString(int nIn)
{
	CString str;
	str.Format(_T("%d"), nIn);
	return str;
}

void CMainDlg::GetPost()
{
	DWORD dwFlags;
	CString strTemp, s, strHtml;
	CStringList sHeaderList;
	for (int i = 0; i < m_editHeader.GetLineCount(); i++)
	{
		int len = m_editHeader.LineLength(m_editHeader.LineIndex(i));
		m_editHeader.GetLine(i, strTemp.GetBuffer(len), len);
		strTemp.ReleaseBuffer(len);

		strTemp.TrimLeft(" ");
		strTemp.TrimRight(" ");
		strTemp.TrimLeft("\t");
		strTemp.TrimRight("\t");
					
		if (strTemp.GetLength() > 0 && strTemp.Find(": ", 0) != -1)
		{
			sHeaderList.AddTail(strTemp);			
		}
	}

	if (!m_bMutiThread)
		GetDlgItem(IDC_BUTTON_EXEC)->EnableWindow(FALSE);
	
	if (m_ComboType.GetCurSel() == 0)//Get
	{
		CHttpFile* pFile = NULL;
		CInternetSession Session(_T("session"), 0, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD);//���ò�����
		try
		{
			DWORD dwType = 0;
			CString strServer, strObject, indata;
			INTERNET_PORT wPort = 80;
			if (!AfxParseURL(m_strUrl, dwType, strServer, strObject, wPort))
				return;

			Session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 60 * 1000, 0);
			Session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 60 * 1000, 0);
			Session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 60 * 1000, 0);

			CHttpConnection *pHttpConnect = NULL;
			if (wPort == 443)
				pHttpConnect = Session.GetHttpConnection(strServer, INTERNET_FLAG_SECURE, wPort);
			else
				pHttpConnect = Session.GetHttpConnection(strServer, wPort);

			if( pHttpConnect )
			{
				if (wPort == 443)
				{
					pFile = (CHttpFile*)pHttpConnect->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1,
						NULL, NULL,INTERNET_FLAG_SECURE | INTERNET_FLAG_EXISTING_CONNECT |
						INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_DONT_CACHE
						);
					//get web server option
					pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
					dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
					//set web server option
					pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
				}
				else
				{
					pFile = pHttpConnect->OpenRequest( CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1, NULL, NULL,
						INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);//�������INTERNET_FLAG_NO_COOKIES���޷��Զ���cookie
				}
				if ( pFile )
				{
					for (int nListCount = 0; nListCount < sHeaderList.GetCount(); nListCount++)
					{
						pFile->AddRequestHeaders(sHeaderList.GetAt(sHeaderList.FindIndex(nListCount)));
					}
					pFile->SendRequest();

					// ���ص�HTML
					strHtml = "";
					pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strHtml, 0);
					strHtml += "*******************************************************************************************\r\n\r\n";
					
					while (pFile->ReadString(s) > 0)
					{
						strHtml += s;
						strHtml += "\r\n";
					}
					if (m_bDecodeUtf8)
						strHtml = Utf8ToAnsi(strHtml);
				}
			}
		}
		catch( CInternetException *e )
		{
			e->ReportError();
			DWORD dwError = e->m_dwError;
			strHtml += "\r\nGet���ݳ���\r\nCInternetException Error = ";
			strHtml += intToString(dwError);
			e->Delete();
		}
		catch (CFileException* e)
		{
			e->ReportError();
			DWORD dwError = e->m_cause;
			strHtml += "\r\nGet���ݳ���\r\nCFileException Error = ";
			strHtml += intToString(dwError);
			e->Delete();
		}
		if (pFile != NULL)
		{
			pFile->Close();
			delete pFile;
			pFile = NULL;
		}
		if (Session != NULL)
		{
			Session.Close();
			delete Session;
		}
	}
	else//Post
	{
		CHttpConnection* pServer = NULL;
		CHttpFile* pFile = NULL;
		try
		{
			int nRead = 0;
			DWORD dwRet;
			DWORD dwType = 0;
			CString strServer, strObject, indata;
			INTERNET_PORT wPort = 80;
			if (!AfxParseURL(m_strUrl, dwType, strServer, strObject, wPort))
				return;
			
			CInternetSession m_InetSession(_T("session"),
				0,
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL,
				NULL,
				INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD);    //���ò�����
			m_InetSession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 60 * 1000, 0);
			m_InetSession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 60 * 1000, 0);
			m_InetSession.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 60 * 1000, 0);
	
			if (wPort == 443)
				pServer = m_InetSession.GetHttpConnection(strServer, INTERNET_FLAG_SECURE, wPort);
			else
				pServer = m_InetSession.GetHttpConnection(strServer, wPort);

			if (wPort == 443)
			{
				pFile = (CHttpFile*)pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1,
					NULL, NULL,INTERNET_FLAG_SECURE | INTERNET_FLAG_EXISTING_CONNECT |
					INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_DONT_CACHE
					);
				//get web server option
				pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
				dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
				//set web server option
				pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwFlags);
			}
			else
			{
				pFile = pServer->OpenRequest( CHttpConnection::HTTP_VERB_POST,
					strObject,
					NULL,
					1,
					NULL,
					NULL,
					INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);//�������INTERNET_FLAG_NO_COOKIES���޷��Զ���cookie
			}
			
			for (int nListCount = 0; nListCount < sHeaderList.GetCount(); nListCount++)
			{
				pFile->AddRequestHeaders(sHeaderList.GetAt(sHeaderList.FindIndex(nListCount)));
			}

			int nLength = 0;
			BYTE *lpBuffer = SetZero(m_strPostValue, nLength);
			pFile->SendRequestEx(nLength);
			pFile->Write(lpBuffer, nLength);
			pFile->EndRequest();
			delete []lpBuffer;

			pFile->QueryInfoStatusCode(dwRet);
			pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strHtml, 0);
			strHtml += "*******************************************************************************************\r\n\r\n";
			if (dwRet == HTTP_STATUS_OK)
			{
				strHtml += "\r\n";
				while ((nRead = pFile->ReadString(s))>0)
				{
					strHtml += s;
					strHtml += "\r\n";
				}
				if (m_bDecodeUtf8)
					strHtml = Utf8ToAnsi(strHtml);
			}
		}
		catch (CInternetException* e)
		{
			e->ReportError();
			e->m_dwContext;
			DWORD dwError = e->m_dwError;
			strHtml += "\r\nPost���ݳ���\r\nCInternetException Error = ";
			strHtml += intToString(dwError);
		}
		catch (CFileException* e)
		{
			e->ReportError();
			DWORD dwError = e->m_cause;
			strHtml += "\r\nPost���ݳ���\r\nCFileException Error = ";
			strHtml += intToString(dwError);
			e->Delete();
		}
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
	}
	if (!m_bMutiThread)
	{
		SetDlgItemText(IDC_EDIT_RETURN, strHtml);
		GetDlgItem(IDC_BUTTON_EXEC)->EnableWindow(TRUE);
	}
}

void CMainDlg::OnButtonInsert() 
{
	UpdateData();
	m_editPost.GetFocus();				//��ȡ���뽹��
	m_editPost.SetSel(nStart, nEnd);	//�ָ����λ��
	m_editPost.ReplaceSel("[\\0]");		//����
}

void CMainDlg::OnKillfocusEditPost() 
{
	UpdateData();
	m_editPost.GetSel(nStart, nEnd);	//��ȡ�������λ��
	
	CWnd *pWnd = GetDlgItem(IDC_BUTTON_INSERT);
	if(GetFocus() != pWnd)				//�жϽ����ڲ���BUTTON��
		GetDlgItem(IDC_BUTTON_INSERT)->ShowWindow(SW_HIDE);
}

void CMainDlg::OnChangeEditHeader() 
{
	// ��ȡEDIT������ʾ������ 
	double nLineCount;
	TEXTMETRIC tm;
	CRect rect;
	GetDlgItem(IDC_EDIT_HEADER)->GetClientRect(&rect);
	CDC* pdc = GetDlgItem(IDC_EDIT_HEADER)->GetDC();
	::GetTextMetrics(pdc->m_hDC,&tm);
	GetDlgItem(IDC_EDIT_HEADER)->ReleaseDC(pdc);
	nLineCount = rect.bottom/(tm.tmHeight-1.5);
	//��̬��ʾ������
	int nLine = ((CEdit*)GetDlgItem(IDC_EDIT_HEADER))->GetLineCount();
	if( nLine > nLineCount)
		GetDlgItem(IDC_EDIT_HEADER)->ShowScrollBar(SB_VERT, TRUE);
	else
		GetDlgItem(IDC_EDIT_HEADER)->ShowScrollBar(SB_VERT, FALSE);
}

void CMainDlg::OnChangeEditPost() 
{
	// ��ȡEDIT������ʾ������ 
	double nLineCount;
	TEXTMETRIC tm;
	CRect rect;
	GetDlgItem(IDC_EDIT_POST)->GetClientRect(&rect);
	CDC* pdc = GetDlgItem(IDC_EDIT_POST)->GetDC();
	::GetTextMetrics(pdc->m_hDC,&tm);
	GetDlgItem(IDC_EDIT_POST)->ReleaseDC(pdc);
	nLineCount = rect.bottom/(tm.tmHeight-1.5);
	//��̬��ʾ������
	int nLine = ((CEdit*)GetDlgItem(IDC_EDIT_POST))->GetLineCount();
	if( nLine > nLineCount)
		GetDlgItem(IDC_EDIT_POST)->ShowScrollBar(SB_VERT, TRUE);
	else
		GetDlgItem(IDC_EDIT_POST)->ShowScrollBar(SB_VERT, FALSE);
}

void CMainDlg::OnChangeEditReturn() 
{
	// ��ȡEDIT������ʾ������ 
	double nLineCount;
	TEXTMETRIC tm;
	CRect rect;
	GetDlgItem(IDC_EDIT_RETURN)->GetClientRect(&rect);
	CDC* pdc = GetDlgItem(IDC_EDIT_RETURN)->GetDC();
	::GetTextMetrics(pdc->m_hDC,&tm);
	GetDlgItem(IDC_EDIT_RETURN)->ReleaseDC(pdc);
	nLineCount = rect.bottom/(tm.tmHeight-1.5);
	//��̬��ʾ������
	int nLine = ((CEdit*)GetDlgItem(IDC_EDIT_RETURN))->GetLineCount();
	if( nLine > nLineCount)
		GetDlgItem(IDC_EDIT_RETURN)->ShowScrollBar(SB_VERT, TRUE);
	else
		GetDlgItem(IDC_EDIT_RETURN)->ShowScrollBar(SB_VERT, FALSE);
}

void CMainDlg::OnSetfocusEditPost() 
{
	((CEdit *)GetDlgItem(IDC_EDIT_POST))->PostMessage(EM_SETSEL, 0, -1);
	GetDlgItem(IDC_BUTTON_INSERT)->ShowWindow(SW_SHOW);
}

void CMainDlg::OnMenuitemCookie() 
{
	m_editHeader.ReplaceSel(_T("Cookie: locale=zh;\r\n"));
}

void CMainDlg::OnMenuitemReferer() 
{
	m_editHeader.ReplaceSel(_T("Referer: http://www.baidu.com/\r\n"));
}

void CMainDlg::OnMenuitemUaBaidu() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Baiduspider+(+http://www.baidu.com/search/spider.htm)\r\n"));
}

void CMainDlg::OnMenuitemUaGoogle() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)\r\n"));
}

void CMainDlg::OnMenuitemUaIe() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)\r\n"));
}

void CMainDlg::OnMenuitemUaChrome() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"));
}

void CMainDlg::OnMenuitemUaAndroid() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Opera/9.80 (Android 3.2.1; Linux; Opera Tablet/ADR-1205181138; U; en-GB) Presto/2.10.254 Version/12.00\r\n"));
}

void CMainDlg::OnMenuitemUaIphone() 
{
	m_editHeader.ReplaceSel(_T("User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 5_1 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9B176 Safari/7534.48.3\r\n"));
}

CString GetIPAddr()
{
	CString strTemp;
	int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
	n1 = 49 + rand()%174;
	n2 = 15 + rand()%230;
	n3 = 10 + rand()%245;
	n4 = 2 + rand()%252;
	strTemp.Format("%d.%d.%d.%d", n1, n2, n3, n4);
	
	return strTemp;
}

#include "../common/InputDlg/InputDlg.h"
void CMainDlg::OnMenuitemFakeip() 
{
	UpdateData();
	
	CString strFakeIP;
	CInputDialog dlg;
	dlg.m_bRequired = FALSE;
	dlg.Init(_T("α��IP"), _T("������һ��IP��ַ����������Ļ������Զ�α��һ��IP:"), this);
	if (dlg.DoModal() != IDOK)   
		return;
	if (dlg.m_str.GetLength() == 0)
	{
		strFakeIP = GetIPAddr();
	}
	else
	{
		if (dlg.m_str.Find(".", 0) == -1)
		{
			::MessageBox(NULL, _T("�������IP��ʽ���������������룡"), _T("��ʾ"), MB_ICONINFORMATION);
			return;
		}
		else
			strFakeIP = dlg.m_str;
	}
	
	CString strHeader;
	GetDlgItemText(IDC_EDIT_HEADER, strHeader);
	strHeader.TrimLeft("\r\n");
	strHeader.TrimRight("\r\n");
	
	strHeader += "\r\n";
	strHeader += "X-Forwarded-For: ";
	strHeader += strFakeIP;
	
	strHeader += "\r\n";
	strHeader += "VIA: ";
	strHeader += strFakeIP;
	
	strHeader += "\r\n";
	strHeader += "CLIENT_IP: ";
	strHeader += strFakeIP;
	
	strHeader += "\r\n";
	strHeader += "REMOTE_ADDR: ";
	strHeader += strFakeIP;
	
	SetDlgItemText(IDC_EDIT_HEADER, strHeader);
}

void CMainDlg::OnStaticHeader() 
{
	// TODO: Add your control notification handler code here
	CHeaderDlg dlg;
	dlg.DoModal();
}

HBRUSH CMainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(pWnd->GetDlgCtrlID() == IDC_STATIC_HEADER || pWnd->GetDlgCtrlID() == IDC_STATIC_LOAD_SAVE)
		pDC->SetTextColor(RGB(0, 100, 255));
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CMainDlg::OnStaticLoadSave() 
{
	CNewMenu popup;
	popup.LoadMenu(IDR_MENU_LOADSAVE);
	popup.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
	CNewMenu*	pM = (CNewMenu *)popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
}

int IniGetInt(CString AppName,CString KeyName,int Default, CString strPath)
{
	return ::GetPrivateProfileInt(AppName, KeyName, Default, strPath);
}

CString IniGetString(CString AppName,CString KeyName,CString Default, CString strPath)
{
	TCHAR buf[1024 * 10];
	::GetPrivateProfileString(AppName, KeyName, Default, buf, sizeof(buf), strPath);
	return buf;
}

void CMainDlg::OnMenuitemSave() 
{
	UpdateData();
	CTime time = CTime::GetCurrentTime(); ///����CTime���� 
	CString strTime = time.Format("Config��%Y-%m-%d %H-%M-%S");
	
    CFileDialog dlg(FALSE, _T("txt"), strTime, OFN_OVERWRITEPROMPT, _T("*.txt|*.txt||"), NULL);
    if (dlg.DoModal() != IDOK)
        return;
	
	CString strTemp, strData;
	strData = "[Config]\r\nURL=";
	GetDlgItemText(IDC_EDIT_URL, strTemp);
	strData += strTemp;
	GetDlgItemText(IDC_EDIT_HEADER, strTemp);
	strTemp.Replace("\r\n", "\\r\\n");
	strData += "\r\nHeader=";
	strData += strTemp;
	
	GetDlgItemText(IDC_EDIT_POST, strTemp);
	strTemp.Replace("\r\n", "\\r\\n");
	strData += "\r\nPOSTVALUE=";
	strData += strTemp;
	
	strTemp.Format("%d", m_ComboType.GetCurSel());
	strData += "\r\nMode=";
	strData += strTemp;
	
	strData += "\r\nDecodeUtf8=";
	if (m_bDecodeUtf8)
		strData += "1";
	else
		strData += "0";
	
    CFile file;
    if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
    {
		file.Write(strData, strData.GetLength());
        file.Close();
        ::MessageBox(0, "������Ϣ�����ɹ�", "��ʾ", MB_ICONINFORMATION);
    }
}

void CMainDlg::OnMenuitemLoad() 
{
    CFileDialog dlg(TRUE, _T(""), "", OFN_OVERWRITEPROMPT, _T("*.txt|*.txt||"), NULL);
    if (dlg.DoModal() != IDOK)
        return;
	CString strPath = dlg.GetPathName();
	CString strBuffer;
	
	SetDlgItemText(IDC_EDIT_URL, IniGetString("Config", "URL", "", strPath));
	strBuffer = IniGetString("Config", "Header", "", strPath);
	strBuffer.Replace("\\r\\n", "\r\n");
	SetDlgItemText(IDC_EDIT_HEADER, strBuffer);
	strBuffer = IniGetString("Config", "POSTVALUE", "", strPath);
	strBuffer.Replace("\\r\\n", "\r\n");
	SetDlgItemText(IDC_EDIT_POST, strBuffer);
	
	int nType = IniGetInt("Config", "Mode", 0, strPath);
	m_ComboType.SetCurSel(nType);
	OnSelchangeComboMothod();
	
	CButton *pCh = (CButton *)GetDlgItem(IDC_CHECK_UTF8TOANSI);
	pCh->SetCheck(IniGetInt("Config", "DecodeUtf8", 0, strPath));
}

void CMainDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	lpMeasureItemStruct->itemHeight = 23;
	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CMainDlg::OnMenuitemCopyall() 
{
	UpdateData();
	CString strText;
	GetDlgItemText(IDC_EDIT_RETURN, strText);
    //�����ݱ��浽�����塣
    if (!strText.IsEmpty())
    {
        if (OpenClipboard())
        {
            EmptyClipboard();
            HGLOBAL hClipboardData = GlobalAlloc(GHND | GMEM_SHARE, (strText.GetLength() + 1) * sizeof(TCHAR));
            if (hClipboardData)
            {
                TCHAR* pszData = (TCHAR *)GlobalLock(hClipboardData);
                _tcscpy(pszData, strText);
                GlobalUnlock(hClipboardData);
                SetClipboardData(CF_TEXT, hClipboardData);
            }
            CloseClipboard();
        }
    }
}

void CMainDlg::OnMenuitemBrowser() 
{
	CString m_HtmlCode;
	GetDlgItemText(IDC_EDIT_RETURN, m_HtmlCode);
	if (m_HtmlCode.GetLength() == 0)
		return;
	CString strFenge;
	strFenge = "*******************************************************************************************";
	int nPos = m_HtmlCode.Find(strFenge, 0);
	if (nPos == -1)
		return;
	m_HtmlCode = m_HtmlCode.Right(m_HtmlCode.GetLength() - nPos - strFenge.GetLength());
	
	
	TCHAR strTempFile[MAX_PATH] = {0};
	memset(strTempFile, 0, sizeof(strTempFile));
	GetTempPath(MAX_PATH, strTempFile);
	lstrcat(strTempFile, _T("GetPost"));
	CreateDirectory(strTempFile, NULL);
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	wsprintf(strTempFile, _T("%s\\HTML_%d-%02d-%02d_%02d-%02d-%02d^%d.html"), strTempFile, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	HANDLE	hFile1 = CreateFile(strTempFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwBytesWrite = 0;
	USES_CONVERSION;
	WriteFile(hFile1, m_HtmlCode.GetBuffer(0), m_HtmlCode.GetLength(), &dwBytesWrite, NULL);
	CloseHandle(hFile1);
	
	ShellExecute(0, _T("open"), strTempFile, NULL, NULL, SW_SHOW);
}

BOOL CMainDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	CPoint pos;   
    GetCursorPos(&pos);  //��ȡ��ǰ���λ��
    CRect rc, rc1;   
    GetDlgItem(IDC_STATIC_HEADER)->GetWindowRect(&rc); //��ȡIDΪIDC_STATIC_HEADER��Static Text�ķ�Χ
    GetDlgItem(IDC_STATIC_LOAD_SAVE)->GetWindowRect(&rc1); //��ȡIDΪIDC_STATIC_LOAD_SAVE��Static Text�ķ�Χ
    if (rc.PtInRect(pos) || rc1.PtInRect(pos)) //�������������Χ֮��   
    {   
        SetCursor(LoadCursor(NULL, IDC_HAND)); //���ó���״   
        return TRUE; //�ǵ÷���TRUE   
    }
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CMainDlg::OnMenuitemClear() 
{
	SetDlgItemText(IDC_EDIT_RETURN, "");
}

void CMainDlg::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	UINT count;
	char filePath[MAX_PATH];
	count = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if(count)
	{
		int pathLen = DragQueryFile(hDropInfo, 0, filePath, sizeof(filePath));
		
		CString strPath = filePath;
		CString strBuffer;
		
		SetDlgItemText(IDC_EDIT_URL, IniGetString("Config", "URL", "", strPath));
		strBuffer = IniGetString("Config", "Header", "", strPath);
		strBuffer.Replace("\\r\\n", "\r\n");
		SetDlgItemText(IDC_EDIT_HEADER, strBuffer);
		strBuffer = IniGetString("Config", "POSTVALUE", "", strPath);
		strBuffer.Replace("\\r\\n", "\r\n");
		SetDlgItemText(IDC_EDIT_POST, strBuffer);
		
		int nType = IniGetInt("Config", "Mode", 0, strPath);
		m_ComboType.SetCurSel(nType);
		OnSelchangeComboMothod();
		
		CButton *pCh = (CButton *)GetDlgItem(IDC_CHECK_UTF8TOANSI);
		pCh->SetCheck(IniGetInt("Config", "DecodeUtf8", 0, strPath));
	}
	DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}

void CMainDlg::OnCheckMutiThread() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_bMutiThread)
	{
		GetDlgItem(IDC_STATIC_THREAD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_THREAD)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_STATIC_THREAD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_THREAD)->ShowWindow(SW_HIDE);
	}
}
