// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ScanInjection.h"
#include "MainDlg.h"
#include "../common/AutoLock.h"
#include "../common/InputDlg/InputDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_STRING "���������"
#define QUOTE "'`([{^~"

#define COLUMN_STRING_0	"��������������"
#define COLUMN_STRING_1	"�仯��"
#define COLUMN_STRING_2	"��С�仯"
#define COLUMN_STRING_3	"�жϷ���"
#define COLUMN_STRING_4	"���"
#define COLUMN_STRING_5	"��⵽�Ĺؼ���"

typedef struct tagThreadParameter
{
	CString strTempURL;
	CMainDlg	*pThis;
}ThreadParameter,*LPThreadParameter;

CMainDlg *g_pMainDlg = NULL;
CCriticalSection g_csScanInjection;
#define WM_ON_INSERT_INJECT_RESULT (WM_USER + 100)
/////////////////////////////////////////////////////////////////////////////
// CMainDlg dialog


CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDlg)
	m_nTimeOut = ((CScanInjectionApp *)AfxGetApp())->m_IniFile.GetInt("ScanInjection", "nTimeOut", 10);
	m_nWebPageChangeRate = ((CScanInjectionApp *)AfxGetApp())->m_IniFile.GetInt("ScanInjection", "nWebPageChangeRate", 5);
	m_bInjectionCheckNormal = ((CScanInjectionApp *)AfxGetApp())->m_IniFile.GetInt("ScanInjection", "bInjectionCheckNormal", FALSE);
	m_nMaxThreadNum = ((CScanInjectionApp *)AfxGetApp())->m_IniFile.GetInt("ScanInjection", "nMaxThreadNum", 10);
	//}}AFX_DATA_INIT
	m_bScanning = FALSE;
	m_nThreadCount = 0;
	((CScanInjectionApp *)AfxGetApp())->m_pMyMainDlg = this;
}


void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDlg)
	DDX_Control(pDX, IDC_LIST_URL, m_ListCtrl);
	DDX_Control(pDX, IDC_LIST_RESULT, m_ListCtrlResult);
	DDX_Control(pDX, IDC_BUTTON_SCAN, m_btnScan);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, m_nTimeOut);
	DDX_Text(pDX, IDC_EDIT_WEBPAGECHANGERATE, m_nWebPageChangeRate);
	DDX_Check(pDX, IDC_CHECK_NORMAL, m_bInjectionCheckNormal);
	DDX_Text(pDX, IDC_EDIT_MAXTHREADNUM, m_nMaxThreadNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	//{{AFX_MSG_MAP(CMainDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_SCAN, OnButtonScanInjection)
	ON_COMMAND(ID_MENUITEM_VISIT, OnMenuitemVisit)
	ON_COMMAND(ID_MENUITEM_COPY, OnMenuitemCopy)
	ON_COMMAND(ID_MENUITEM_SAVE, OnMenuitemSave)
	ON_COMMAND(ID_MENUITEM_ADDURLTOLIST, OnMenuitemAddurltolist)
	ON_COMMAND(ID_MENUITEM_IMPORT, OnMenuitemImport)
	ON_COMMAND(ID_MENUITEM_DEL, OnMenuitemDel)
	ON_COMMAND(ID_MENUITEM_CLEAR, OnMenuitemClear)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_URL, OnRclickListUrl)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_RESULT, OnRclickListResult)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_URL, OnDblclkListUrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_RESULT, OnDblclkListResult)
	ON_COMMAND(ID_MENUITEM_VISIT_RESULT, OnMenuitemVisitResult)
	ON_COMMAND(ID_MENUITEM_COPY_RESULT, OnMenuitemCopyResult)
	ON_COMMAND(ID_MENUITEM_SAVE_RESULT, OnMenuitemSaveResult)
	ON_COMMAND(ID_MENUITEM_DEL_RESULT, OnMenuitemDelResult)
	ON_COMMAND(ID_MENUITEM_CLEAR_RESULT, OnMenuitemClearResult)
	ON_WM_DROPFILES()
	ON_WM_MEASUREITEM()
	ON_COMMAND(ID_MENUITEM_DELETE_TIMEOUT_URLS, OnMenuitemDeleteTimeoutUrls)
	ON_COMMAND(ID_MENUITEM_SCAN_TIMEOUT_URLS, OnMenuitemScanTimeoutUrls)
	//}}AFX_MSG_MAP
	ON_NOTIFY(SPN_MAXMINPOS, IDC_SPLITTER, OnMaxMinInfo)
	ON_NOTIFY(SPN_DELTA,     IDC_SPLITTER, OnSplitter1Delta)
	ON_MESSAGE(WM_ON_INSERT_INJECT_RESULT, OnMessageInsertInjectResult)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainDlg message handlers

void TransParentDC(CRect rect,CDC * pDC)
{
	CDC m_MemDC;
	m_MemDC.CreateCompatibleDC(pDC);
	CBitmap m_Bitmap;
	m_Bitmap.CreateCompatibleBitmap
		(pDC,rect.Width(),rect.Height());
	CBitmap *pOldBitmap = 
		m_MemDC.SelectObject(&m_Bitmap);
	m_MemDC.FillSolidRect(0,0,rect.Width(),
		rect.Height(),GetSysColor(COLOR_MENU));
	
	COLORREF cor =  pDC->GetPixel(0,0);
	for(int y = 0; y < rect.Height();y++)
	{
		for(int x = 0; x < rect.Width(); x++)
		{
			COLORREF ch = pDC->GetPixel(x,y);
			if(ch != cor)
				m_MemDC.SetPixelV(x,y,ch);
		}
	}
	pDC->BitBlt(0,0,rect.Width(),
		rect.Height(),&m_MemDC,0,0,SRCCOPY);
	m_MemDC.SelectObject(pOldBitmap);
	m_Bitmap.DeleteObject();
}

CSize CMainDlg::LoadMyBitmap(CImageList *imgList, UINT nID)//ʹbmpͼƬ͸��
{
	CDC * pDC = GetDC();
	CDC m_MemDC;
	m_MemDC.CreateCompatibleDC(pDC);
	CSize m_Size = pDC->GetTextExtent("ˢ��");
	ReleaseDC(pDC);
	CRect rect(0,0,60,32);
	CBitmap *pBitmap, *pOldBitmap;
	pBitmap = new CBitmap;
	pBitmap->LoadBitmap(nID);
	pOldBitmap = m_MemDC.SelectObject(pBitmap);
	TransParentDC(rect,&m_MemDC);
	m_MemDC.SelectObject(pOldBitmap);
	imgList->Add(pBitmap,GetSysColor(COLOR_MENU));
	pBitmap->DeleteObject();
	delete pBitmap;
	return m_Size;
}

BOOL CMainDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_pMainDlg = (CMainDlg *)((CScanInjectionApp *)AfxGetApp())->m_pMyMainDlg;
	// TODO: Add extra initialization here
	m_infotip.Create(this);
	m_infotip.SetMaxTipWidth(450); 
	m_infotip.SetTipTextColor(RGB(0,170,64));
	m_infotip.AddTool(GetDlgItem(IDC_EDIT_TIMEOUT), _T("��"));
	m_infotip.AddTool(GetDlgItem(IDC_EDIT_WEBPAGECHANGERATE), _T("���λ�ȡ��ҳ���С��Ƚϣ�������ڸ�ֵ���ж�Ϊ��ע�롣"));
	m_infotip.AddTool(GetDlgItem(IDC_CHECK_NORMAL), _T("�������ͨ�����͵�ע��ɼӿ����ٶȣ������ܻ���©�������͵�ע��㡣"));

	m_ImageList.Create(16, 16, ILC_COLORDDB | ILC_MASK, 0, 1);
	LoadMyBitmap(&m_ImageList, IDB_BITMAP_URL);
	m_ListCtrl.SetImageList(&m_ImageList, TVSIL_NORMAL);

	ListView_SetExtendedListViewStyle(m_ListCtrl.m_hWnd, LVS_EX_FULLROWSELECT);
	m_ListCtrl.InsertColumn(0, COLUMN_STRING, LVCFMT_LEFT, 600);
	m_ListCtrl.InsertColumn(1, _T("Hide Column(compare)"), TRUE, LVCFMT_LEFT, 0, -1);// ���ص�ListCtrl��
	m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_SUBITEMIMAGES);
	m_ListCtrl.SetImageList(&m_ImageList, LVSIL_SMALL);

	m_ImageListResult.Create(16, 16, ILC_COLORDDB | ILC_MASK, 0, 1);
	CBitmap bm;
	bm.LoadBitmap(IDB_BITMAP_RESULT);
	m_ImageListResult.Add(&bm, RGB(192, 192, 192));
	ListView_SetExtendedListViewStyle(m_ListCtrlResult.m_hWnd, LVS_EX_FULLROWSELECT);
	m_ListCtrlResult.InsertColumn(0, COLUMN_STRING_0, LVCFMT_LEFT, 350);
	m_ListCtrlResult.InsertColumn(1, COLUMN_STRING_1, LVCFMT_LEFT, 90);
	m_ListCtrlResult.InsertColumn(2, COLUMN_STRING_2, LVCFMT_LEFT, 90);
	m_ListCtrlResult.InsertColumn(3, COLUMN_STRING_3, LVCFMT_LEFT, 150);
	//m_ListCtrlResult.InsertColumn(4, COLUMN_STRING_4, LVCFMT_LEFT, 50);
	m_ListCtrlResult.InsertColumn(4, COLUMN_STRING_5, LVCFMT_LEFT, 250);
	//m_ListCtrlResult.InsertColumn(6, _T("Hide Column 1"), TRUE, LVCFMT_LEFT, 100, -1);// ���ص�ListCtrl��
	//m_ListCtrlResult.InsertColumn(7, _T("Hide Column 1"), TRUE, LVCFMT_LEFT, 100, -1);// ���ص�ListCtrl��
	
	m_ListCtrlResult.SetExtendedStyle(m_ListCtrlResult.GetExtendedStyle() | LVS_EX_SUBITEMIMAGES);
	m_ListCtrlResult.SetImageList(&m_ImageListResult, LVSIL_SMALL);

	m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, CRect(0, 0, 0, 0), this, 0x1200001);
	int strPartDim[2]= {300, -1};
	m_wndStatusBar.SetParts(2, strPartDim);

	//Splitter
	CRect rc;
	CWnd* pWnd = GetDlgItem(IDC_SPLITTER);
	pWnd->GetWindowRect(rc);
	ScreenToClient(rc);
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rc, this, IDC_SPLITTER, SPS_HORIZONTAL|SPS_DELTA_NOTIFY);
    //  register windows for splitter
    m_wndSplitter.RegisterLinkedWindow(SPLS_LINKED_UP,    GetDlgItem(IDC_LIST_URL));
    m_wndSplitter.RegisterLinkedWindow(SPLS_LINKED_DOWN,    GetDlgItem(IDC_LIST_RESULT));
    m_wndSplitter.Relayout();

	m_strUserAgent = ((CScanInjectionApp *)AfxGetApp())->m_IniFile.GetString("WebRobot", "UserAgent", "User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5");
	PostMessage(WM_SIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (GetParent()->m_hWnd != NULL && m_wndStatusBar.m_hWnd != NULL)
	{
		RECT rc;
		GetParent()->GetClientRect(&rc);
		MoveWindow(&rc);
		m_wndStatusBar.MoveWindow(&rc);
		Resize();
	}
}

void CMainDlg::MoveDlgItem(int nID, const CRect& rcPos, BOOL bRepaint)
{
    CWnd* pWnd = this->GetDlgItem(nID);
    if (NULL == pWnd)
        return;
    pWnd->MoveWindow(rcPos, bRepaint);
}

void CMainDlg::Resize()
{
	int nHeight = 0;
    CRect rcDlg;
	GetClientRect(rcDlg);
	
	if (m_ListCtrl.m_hWnd != NULL && m_ListCtrlResult.m_hWnd != NULL)
	{
		nHeight = (rcDlg.bottom - rcDlg.top)/2;
		CRect rcList;
		rcList.left    = rcDlg.left;
		rcList.right   = rcDlg.right - 2;
		rcList.top     = rcDlg.top + 38;
		rcList.bottom  = nHeight;
		this->MoveDlgItem(IDC_LIST_URL, rcList, TRUE);

		CRect rcListResult;
		rcListResult.left    = rcDlg.left;
		rcListResult.right   = rcDlg.right - 2;
		rcListResult.top     = nHeight + 4;
		rcListResult.bottom  = rcDlg.bottom - 23;
		this->MoveDlgItem(IDC_LIST_RESULT, rcListResult, TRUE);

		CRect rcSplit1;
		m_wndSplitter.GetWindowRect(rcSplit1);
		ScreenToClient(rcSplit1);
		rcSplit1.left = rcDlg.left;
		rcSplit1.right = rcDlg.right;
		rcSplit1.top = nHeight;
		rcSplit1.bottom = nHeight + 4;
		m_wndSplitter.MoveWindow(rcSplit1, TRUE);

		m_ListCtrl.SetColumnWidth(0, (rcDlg.right - rcDlg.left) - 25);
		m_ListCtrlResult.SetColumnWidth(0, (rcDlg.right - rcDlg.left - 355)/2);
		m_ListCtrlResult.SetColumnWidth(4, (rcDlg.right - rcDlg.left - 355)/2);
	}
}

void CMainDlg::StatusTextOut(int iPane,LPCTSTR ptzFormat, ...)
{
	TCHAR tzText[1024];
	va_list vlArgs;
	va_start(vlArgs, ptzFormat);
	wvsprintf(tzText, ptzFormat, vlArgs);
	va_end(vlArgs);
	
	m_wndStatusBar.SetText(tzText, iPane, 0);
}

inline BYTE toHex(const BYTE &x)
{
	return x > 9 ? x + 55: x + 48;
}

inline BYTE toByte(const BYTE &x)
{
	return x > 57? x - 55: x - 48;
}

CString URLDecode(CString sIn)
{
	CString sOut;
	const int nLen = sIn.GetLength() + 1;
	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf =(LPBYTE)sIn.GetBuffer(nLen);
	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen);
	
	if(pOutBuf)
	{
		pInTmp   = pInBuf;
		pOutTmp = pOutBuf;
		// do encoding
		while (*pInTmp)
		{
			if('%'==*pInTmp)
			{
				pInTmp++;
				*pOutTmp++ = (toByte(*pInTmp)%16<<4) + toByte(*(pInTmp+1))%16;//��4λ+��4λ
				pInTmp++;
			}
			else if('+'==*pInTmp)
				*pOutTmp++ = ' ';
			else
				*pOutTmp++ = *pInTmp;
			pInTmp++;
		}
		*pOutTmp = '\0';
		sOut.ReleaseBuffer();
	}
	sIn.ReleaseBuffer();
	
	return sOut;
}

CString URLEncode(CString sIn)
{
	CString sOut;
	const int nLen = sIn.GetLength() + 1;
	register LPBYTE pOutTmp = NULL;
	LPBYTE pOutBuf = NULL;
	register LPBYTE pInTmp = NULL;
	LPBYTE pInBuf =(LPBYTE)sIn.GetBuffer(nLen);
	//alloc out buffer
	pOutBuf = (LPBYTE)sOut.GetBuffer(nLen*3);
	
	if(pOutBuf)
	{
		pInTmp   = pInBuf;
		pOutTmp = pOutBuf;
		// do encoding
		while (*pInTmp)
		{
			if(isalnum(*pInTmp) || '-'==*pInTmp || '_'==*pInTmp || '.'==*pInTmp)
				*pOutTmp++ = *pInTmp;
			else if(isspace(*pInTmp))
				*pOutTmp++ = '+';
			else
			{
				*pOutTmp++ = '%';
				*pOutTmp++ = toHex(*pInTmp>>4);//��4λ
				*pOutTmp++ = toHex(*pInTmp%16);//��4λ
			}
			pInTmp++;
		}
		*pOutTmp = '\0';
		sOut.ReleaseBuffer();
	}
	sIn.ReleaseBuffer();
	
	return sOut;
}

void CMainDlg::OnButtonScanInjection() 
{
	UpdateData();
	if (!m_bScanning)
	{
		if (m_ListCtrl.GetItemCount() < 1)
		{
			StatusTextOut(1, "��ɨ��URL�б�Ϊ�գ�ע��ɨ����ֹ��");
			return;
		}
		
		((CScanInjectionApp *)AfxGetApp())->m_IniFile.SetInt("ScanInjection", "nTimeOut", m_nTimeOut);
		((CScanInjectionApp *)AfxGetApp())->m_IniFile.SetInt("ScanInjection", "nWebPageChangeRate", m_nWebPageChangeRate);
		((CScanInjectionApp *)AfxGetApp())->m_IniFile.SetInt("ScanInjection", "bInjectionCheckNormal", m_bInjectionCheckNormal);
		((CScanInjectionApp *)AfxGetApp())->m_IniFile.SetInt("ScanInjection", "nMaxThreadNum", m_nMaxThreadNum);

		m_hWorkThread = ThreadTemplate::StartThread<CMainDlg, DWORD>(this, ScanInjectionThread);
		if (m_hWorkThread == NULL)
		{
			::MessageBox(NULL, "����ɨ��ע���߳�ʧ�ܣ�", "��ʾ", MB_ICONINFORMATION);
			return;
		}
	}
	else
	{
		m_hTerminateWorkThread = ThreadTemplate::StartThread<CMainDlg, DWORD>(this, TerminateWorkThread);
		if (m_hTerminateWorkThread == NULL)
		{
			StatusTextOut(1, "����ɨ��ʧ�ܣ�");
			return;
		}
	}
}

DWORD CMainDlg::ScanInjectionThread()
{
	m_bScanning = TRUE;

	//��ȡSQL��������
	char strSQLErrFilePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, strSQLErrFilePath, sizeof(strSQLErrFilePath));
	PathRemoveFileSpec(strSQLErrFilePath);
	lstrcat(strSQLErrFilePath, "\\plugin\\");
	lstrcat(strSQLErrFilePath, "ScanInjection@sqlerr.txt");

	strArrayErrorWords.RemoveAll();
	CStdioFile file;
	CString strBuffer;
	file.Open(strSQLErrFilePath, CFile::modeRead);
	while(file.ReadString(strBuffer) > 0)
	{
		int nPos = strBuffer.Find("#####");
		if(nPos != -1)
			strBuffer.SetAt(nPos, 0x00);
		strBuffer.Format("%s", strBuffer);
		//ȥ���ո�
		strBuffer.TrimRight(" ");
		//ȥ��Tab
		strBuffer.TrimRight("\t");
		if(strBuffer.GetLength() > 0)
			strArrayErrorWords.Add(strBuffer);
	}
	file.Close();

	//���߳�ɨ��
	SetDlgItemText(IDC_BUTTON_SCAN, "ֹͣɨ��");
	StatusTextOut(1, "");
	m_nCurrentThreadCount = 0;
	CString strStatusBar;
	//�̴߳������
	ThreadParameter lparam;
	lparam.pThis = this;
	
	while (1)
	{
		if (m_ListCtrl.GetItemCount() == 0)
			break;
		lparam.strTempURL.Empty();
		lparam.strTempURL = m_ListCtrl.GetItemText(0, 0);
		m_ListCtrl.DeleteItem(0);
		ShowRowNum();
		//����ɨ���߳�
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DetectInjectURLThread, (LPVOID)&lparam, 0, NULL);
		
		g_csScanInjection.Lock();
		m_nCurrentThreadCount++;
		g_csScanInjection.Unlock();
		
		strStatusBar.Format("ʣ�� %d ��δ�������,��ǰ�߳���:%3d", m_ListCtrl.GetItemCount(), m_nCurrentThreadCount);
		StatusTextOut(0, "%s", strStatusBar);
		while (m_nCurrentThreadCount >= m_nMaxThreadNum)
		{
			Sleep(100);
		}
		Sleep(20);
	}
	
	//�ȴ�ɨ�����
	while(1)
	{
		if (m_nCurrentThreadCount == 0)
		{
			break;
		}
		strStatusBar.Format("ȫ�����Ӽ������,����%d���������ڱ���� ...", m_nCurrentThreadCount);
		StatusTextOut(0, "%s", strStatusBar);
		Sleep(500);
	}
	m_bScanning = FALSE;
	SetDlgItemText(IDC_BUTTON_SCAN, "ɨ��ע��");
	StatusTextOut(0, "ȫ��URL�����ϣ�");
	StatusTextOut(1, "");
	m_ListCtrl.DeleteAllItems();
	ShowRowNum();
	MessageBeep(MB_ICONEXCLAMATION);
	
	return 0;
}

DWORD CMainDlg::TerminateWorkThread()
{
	StatusTextOut(1, "���Ժ����ڵȴ�ȫ���߳��˳�...");
	GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);
	DWORD dwExitCode;
	if(m_hWorkThread != NULL)
	{
		//���߳�ֱ��ɨ������Ż��˳������Ա���ǿ�ƽ���
		WaitForSingleObject(m_hWorkThread, 200);
		if(GetExitCodeThread(m_hWorkThread, &dwExitCode))
		{
			if(dwExitCode == STILL_ACTIVE)
				TerminateThread(m_hWorkThread, dwExitCode);
		}
		m_hWorkThread = NULL;
	}
	
	//ɨ���߳������ʱ���ǣ���һ�����ӳ�ʱʱ��+����5�γ�ʱʱ��
	for(UINT i = 0; i < m_nThreadCount; i++)
	{
		if (m_hThread[i] != NULL)
		{
			DWORD dwExitCode;
			if(m_hThread[i] != NULL)
			{
				WaitForSingleObject(m_hThread[i], 100);
				if(GetExitCodeThread(m_hThread[i], &dwExitCode))
				{
					if(dwExitCode == STILL_ACTIVE)
						TerminateThread(m_hThread[i], dwExitCode);
				}
				m_hThread[i] = NULL;
			}
		}
	}
	m_nThreadCount = 0;
	m_bScanning = FALSE;
	SetDlgItemText(IDC_BUTTON_SCAN, "ɨ��ע��");
	StatusTextOut(0, "ɨ����ֹͣ��");
	StatusTextOut(1, "");
	GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);

	return 0;
}

DWORD WINAPI CMainDlg::DetectInjectURLThread(LPVOID lparam)
{
	ThreadParameter *par = (ThreadParameter *)lparam;
	CMainDlg *pThis = par->pThis;
	CString strUrl = par->strTempURL;
	
	ReturnData *lpReturnData = new ReturnData;
	lpReturnData->strUrl = strUrl;
	pThis->DetectURL(lpReturnData);
	
	g_csScanInjection.Lock();
	if (lpReturnData->bResult)
		pThis->PostMessage(WM_ON_INSERT_INJECT_RESULT, 0, (LPARAM)lpReturnData);//OnMessageInsertInjectResult
	pThis->m_nCurrentThreadCount--;
	g_csScanInjection.Unlock();
	
	return 0;
}

CString ShowPageSize(DWORD dw1, DWORD dw2)
{
	CString strShow;
	strShow.Format("%d|%d", dw1, dw2);
	return strShow;
}

LRESULT CMainDlg::OnMessageInsertInjectResult(WPARAM wParam, LPARAM lParam)
{
	ReturnData *par = (ReturnData *)lParam;
	
	int nCount = m_ListCtrlResult.GetItemCount();
	if (par->nRate < 0)//����ע��©����ʱ
	{
		m_ListCtrlResult.InsertItem(nCount, par->strUrl, 0);
		m_ListCtrlResult.SetItemText(nCount, 1, "��ʱ");
		m_ListCtrlResult.SetItemTextColor(nCount, RGB(128,128,128));
	}
	else
	{
		CString strShow;
		if (par->strErrWord.GetLength() > 0)//��⵽�˹ؼ���
		{
			m_ListCtrlResult.InsertItem(nCount, par->strUrl, 0);
			
			if (par->nMethod == 0)
			{
				strShow = "����⵽�ؼ���";
				m_ListCtrlResult.SetItemText(nCount, 1, " -");
				m_ListCtrlResult.SetItemText(nCount, 2, " -");
			}
			else
			{
				strShow.Format("%d%%", par->nRate);
				m_ListCtrlResult.SetItemText(nCount, 1, strShow);
				strShow = GetErrMethod(par->nMethod);
				m_ListCtrlResult.SetItemText(nCount, 2, ShowPageSize(par->lWebPageSize1, par->lWebPageSize2));
			}
			m_ListCtrlResult.SetItemText(nCount, 3, strShow);

			strShow = GetInjectType(par->nMethod);
			//m_ListCtrlResult.SetItemText(nCount, 4, strShow);
			m_ListCtrlResult.SetItemText(nCount, 4, par->strErrWord);
			//m_ListCtrlResult.SetItemText(nCount, 6, URLDecode(par->strURL1));
			//m_ListCtrlResult.SetItemText(nCount, 7, URLDecode(par->strURL2));
		}
		else
		{
			m_ListCtrlResult.InsertItem(nCount, par->strUrl, 0);
			strShow.Format("%d%%", par->nRate);
			m_ListCtrlResult.SetItemText(nCount, 1, strShow);
			m_ListCtrlResult.SetItemText(nCount, 2, ShowPageSize(par->lWebPageSize1, par->lWebPageSize2));
			strShow = GetErrMethod(par->nMethod);
			m_ListCtrlResult.SetItemText(nCount, 3, strShow);
			strShow = GetInjectType(par->nMethod);
			//m_ListCtrlResult.SetItemText(nCount, 4, strShow);
			m_ListCtrlResult.SetItemText(nCount, 4, par->strErrWord);
			//m_ListCtrlResult.SetItemText(nCount, 6, URLDecode(par->strURL1));
			//m_ListCtrlResult.SetItemText(nCount, 7, URLDecode(par->strURL2));
		}
	}
	
	ShowResultRowNum();

	delete par;
	return 0;
}

CString CMainDlg::GetErrMethod(int nMethod)
{
	CString strMethod;
	switch (nMethod)
	{
	case 0:
		strMethod += "";
		break;
	case 1:
		strMethod += "aND 8=8 | aND 8=3";
		break;
	case 2:
		strMethod += "aND 8=8 | aND 8=3";
		break;
	case 3:
		strMethod += "' aND '8'='8 | ' aND '8'='3";
		break;
	case 4:
		strMethod += "' aND '8'='8 | ' aND '8'='3";
		break;
	case 5:
		strMethod += "[Tab]aND[Tab]8=8 | [Tab]aND[Tab]8=3";
		break;
	case 6:
		strMethod += "[Tab]aND[Tab]8=8 | [Tab]aND[Tab]8=3";
		break;
	case 7:
		strMethod += "'[Tab]aND[Tab]'8'='8 | '[Tab]aND[Tab]'8'='3";
		break;
	case 8:
		strMethod += "'[Tab]aND[Tab]'8'='8 | '[Tab]aND[Tab]'8'='3";
		break;
	case 9:
		strMethod += "/**/aND/**/8=8 | /**/aND/**/8=3";
		break;
	case 10:
		strMethod += "/**/aND/**/8=8 | /**/aND/**/8=3";
		break;
	case 11:
		strMethod += "'/**/aND/**/'8'='8 | '/**/aND/**/'8'='3";
		break;
	case 12:
		strMethod += "'/**/aND/**/'8'='8 | '/**/aND/**/'8'='3";
		break;
	case 13:
		strMethod += "%' aND '8%'='8 | %' aND '8%'='3";
		break;
	case 14:
		strMethod += "%' aND '8%'='8 | %' aND '8%'='3";
		break;
	case 15:
		strMethod += "%'[Tab]aND[Tab]'8'='8 | %'[Tab]aND[Tab]'8%'='3";
		break;
	case 16:
		strMethod += "%'[Tab]aND[Tab]'8'='8 | %'[Tab]aND[Tab]'8%'='3";
		break;
	case 17:
		strMethod += "%'/**/aND/**/'8'='8 | %'/**/aND/**/'8%'='3";
		break;
	case 18:
		strMethod += "%'/**/aND/**/'8'='8 | %'/**/aND/**/'8%'='3";
		break;
	case 19:
		strMethod += "XoR 8=3 | XoR 8=8";
		break;
	case 20:
		strMethod += "XoR 8=3 | XoR 8=8";
		break;
	case 21:
		strMethod += "[Tab]XoR[Tab]8=3 | [Tab]XoR[Tab]8=8";
		break;
	case 22:
		strMethod += "[Tab]XoR[Tab]8=3 | [Tab]XoR[Tab]8=8";
		break;
	case 23:
		strMethod += "/**/XoR/**/8=3 | /**/XoR/**/8=8";
		break;
	case 24:
		strMethod += "/**/XoR/**/8=3 | /**/XoR/**/8=8";
		break;
	case 25:
		strMethod += "' XoR '8'='3 | ' XoR '8'='8";
		break;
	case 26:
		strMethod += "' XoR '8'='3 | ' XoR '8'='8";
		break;
	case 27:
		strMethod += "'[Tab]XoR[Tab]'8'='3 | '[Tab]XoR[Tab]'8'='8";
		break;
	case 28:
		strMethod += "'[Tab]XoR[Tab]'8'='3 | '[Tab]XoR[Tab]'8'='8";
		break;
	case 29:
		strMethod += "'/**/XoR/**/'8'='3 | '/**/XoR/**/'8'='8";
		break;
	case 30:
		strMethod += "'/**/XoR/**/'8'='3 | '/**/XoR/**/'8'='8";
		break;
	default:
		strMethod += "";
		break;
	}
	
	return strMethod;
}

CString CMainDlg::GetUseType(int nMethod)
{
	CString strTemp;
	switch (nMethod)
	{
	case 0:
		strTemp += "";
		break;
	case 1:
		strTemp += "AND";
		break;
	case 2:
		strTemp += "AND";
		break;
	case 3:
		strTemp += "AND";
		break;
	case 4:
		strTemp += "AND";
		break;
	case 5:
		strTemp += "AND";
		break;
	case 6:
		strTemp += "AND";
		break;
	case 7:
		strTemp += "AND";
		break;
	case 8:
		strTemp += "AND";
		break;
	case 9:
		strTemp += "AND";
		break;
	case 10:
		strTemp += "AND";
		break;
	case 11:
		strTemp += "AND";
		break;
	case 12:
		strTemp += "AND";
		break;
	case 13:
		strTemp += "AND";
		break;
	case 14:
		strTemp += "AND";
		break;
	case 15:
		strTemp += "AND";
		break;
	case 16:
		strTemp += "AND";
		break;
	case 17:
		strTemp += "AND";
		break;
	case 18:
		strTemp += "AND";
		break;
	case 19:
		strTemp += "XOR";
		break;
	case 20:
		strTemp += "XOR";
		break;
	case 21:
		strTemp += "XOR";
		break;
	case 22:
		strTemp += "XOR";
		break;
	case 23:
		strTemp += "XOR";
		break;
	case 24:
		strTemp += "XOR";
		break;
	case 25:
		strTemp += "XOR";
		break;
	case 26:
		strTemp += "XOR";
		break;
	case 27:
		strTemp += "XOR";
		break;
	case 28:
		strTemp += "XOR";
		break;
	case 29:
		strTemp += "XOR";
		break;
	case 30:
		strTemp += "XOR";
		break;
	default:
		strTemp += "";
		break;
	}
	
	return strTemp;
}

CString CMainDlg::GetInjectType(int nMethod)
{
	CString strMethod;
	switch (nMethod)
	{
	case 0:
		strMethod += "";
		break;
	case 1:
		strMethod += "������";
		break;
	case 2:
		strMethod += "������";
		break;
	case 3:
		strMethod += "�ı���";
		break;
	case 4:
		strMethod += "�ı���";
		break;
	case 5:
		strMethod += "������";
		break;
	case 6:
		strMethod += "������";
		break;
	case 7:
		strMethod += "�ı���";
		break;
	case 8:
		strMethod += "�ı���";
		break;
	case 9:
		strMethod += "������";
		break;
	case 10:
		strMethod += "������";
		break;
	case 11:
		strMethod += "�ı���";
		break;
	case 12:
		strMethod += "�ı���";
		break;
	case 13:
		strMethod += "������";
		break;
	case 14:
		strMethod += "������";
		break;
	case 15:
		strMethod += "������";
		break;
	case 16:
		strMethod += "������";
		break;
	case 17:
		strMethod += "������";
		break;
	case 18:
		strMethod += "������";
		break;
	case 19:
		strMethod += "������";
		break;
	case 20:
		strMethod += "������";
		break;
	case 21:
		strMethod += "������";
		break;
	case 22:
		strMethod += "������";
		break;
	case 23:
		strMethod += "������";
		break;
	case 24:
		strMethod += "������";
		break;
	case 25:
		strMethod += "�ı���";
		break;
	case 26:
		strMethod += "�ı���";
		break;
	case 27:
		strMethod += "�ı���";
		break;
	case 28:
		strMethod += "�ı���";
		break;
	case 29:
		strMethod += "�ı���";
		break;
	case 30:
		strMethod += "�ı���";
		break;
	default:
		strMethod += "";
		break;
	}
	
	return strMethod;
}

CString CMainDlg::ModifyParameter(int Mode, CString strURL)
{
	switch (Mode)
	{
	case 1:
		strURL += "%20aND%208%3D8";//" aND 8=8";
		break;
	case 2:
		strURL += "%20aND%208%3D3";//" aND 8=3";
		break;
	case 3:
		strURL += "%27%20aND%20%278%27%3D%278";//"' aND '8'='8";
		break;
	case 4:
		strURL += "%27%20aND%20%278%27%3D%273";//"' aND '8'='3";
		break;
	case 5:
		strURL += "%09aND%098%3D8";//"	aND	8=8";
		break;
	case 6:
		strURL += "%09aND%098%3D3";//"	aND	8=3";
		break;
	case 7:
		strURL += "%27%09aND%09%278%27%3D%278";//"'	aND	'8'='8";
		break;
	case 8:
		strURL += "%27%09aND%09%278%27%3D%273";//"'	aND	'8'='3";
		break;
	case 9:
		strURL += "/**/aND/**/8%3D8";//"/**/aND/**/8=8";
		break;
	case 10:
		strURL += "/**/aND/**/8%3D3";//"/**/aND/**/8=3";
		break;
	case 11:
		strURL += "%27/**/aND/**/%278%27%3D%278";//"'/**/aND/**/'8'='8";
		break;
	case 12:
		strURL += "%27/**/aND/**/%278%27%3D%273";//"'/**/aND/**/'8'='3";
		break;
	case 13:
		strURL += "%25%27%20aND%20%278%25%27%3D%278";//"%' aND '8%'='8";
		break;
	case 14:
		strURL += "%25%27%20aND%20%278%25%27%3D%273";//"%' aND '8%'='3";
		break;
	case 15:
		strURL += "%25%27%09aND%09%278%27%3D%278";//"%'	aND	'8'='8";
		break;
	case 16:
		strURL += "%25%27%09aND%09%278%25%27%3D%273";//"%'	aND	'8%'='3";
		break;
	case 17:
		strURL += "%25%27/**/aND/**/%278%27%3D%278";//"%'/**/aND/**/'8'='8";
		break;
	case 18:
		strURL += "%25%27/**/aND/**/%278%25%27%3D%273";//"%'/**/aND/**/'8%'='3";
		break;
	case 19:
		strURL += "%20XoR%208%3D8";//" XoR 8=8";
		break;
	case 20:
		strURL += "%20XoR%208%3D3";//" XoR 8=3";
		break;
	case 21:
		strURL += "%09XoR%098%3D8";//"	XoR	8=8";//tab
		break;
	case 22:
		strURL += "%09XoR%098%3D3";//"	XoR	8=3";//tab
		break;
	case 23:
		strURL += "/**/XoR/**/8%3D8";//"/**/XoR/**/8=8";
		break;
	case 24:
		strURL += "/**/XoR/**/8%3D3";//"/**/XoR/**/8=3";
		break;
	case 25:
		strURL += "%27%20XoR%20%278%27%3D%278";//"' XoR '8'='8";
		break;
	case 26:
		strURL += "%27%20XoR%20%278%27%3D%273";//"' XoR '8'='3";
		break;
	case 27:
		strURL += "%27%09XoR%09%278%27%3D%278";//"'	XoR	'8'='8";//tab
		break;
	case 28:
		strURL += "%27%09XoR%09%278%27%3D%273";//"'	XoR	'8'='3";//tab
		break;
	case 29:
		strURL += "%27/**/XoR/**/%278%27%3D%278";//"'/**/XoR/**/'8'='8";
		break;
	case 30:
		strURL += "%27/**/XoR/**/%278%27%3D%273";//"'/**/XoR/**/'8'='3";
		break;
	default:
		break;
	}
	
	return strURL;
}

CString CMainDlg::AddSingleQuotes(CString strURL)
{
	int nPos = 0;
	strURL.TrimRight(" ");
	strURL.TrimRight("\t");
	int nLenth = strURL.GetLength();
	//����̬ҳ��
	CString strTmpURL = strURL.Mid(nLenth - 6, 6);
	strTmpURL.MakeLower();
	int nPointPos	= strTmpURL.Find(".", 0);
	int nHtmPos		= strTmpURL.Find("htm", 0);
	if (nPointPos != -1 && nHtmPos != -1)
	{
		strURL.Insert(nLenth + nPointPos - 6, QUOTE);
	}
	else
	{
		strURL += QUOTE;
		while (1)
		{
			nPos = strURL.Find("&", nPos);
			if (nPos == -1)
				break;
			else
			{
				strURL.Insert(nPos, QUOTE);
			}
			nPos += 8;
		}
	}
	return strURL;
}

void CMainDlg::DetectURL(ReturnData *lpReturnData)
{
	lpReturnData->bResult	= FALSE;
	lpReturnData->nMethod	= 0;
	lpReturnData->nRate	=	0;
	lpReturnData->strErrWord = "";
	try
	{
		DWORD dwStatusCode = 0, dwWebPageSize = 0;
		CString strURL, strTemp, strOtherParameter, strNoParameterURL, strHtml;
		CStringList strList, strDetectURLList;
		strURL = lpReturnData->strUrl;

		//������ؼ���
		strTemp = AddSingleQuotes(lpReturnData->strUrl);
		
		CHttpClient httpClient;
		httpClient.GetData(strTemp, CHttpConnection::HTTP_VERB_GET, "", "", m_strUserAgent, m_nTimeOut * 1000, dwStatusCode, dwWebPageSize, TRUE, FALSE, strHtml);

		try
		{
			//��StringList���SQL����ؼ�������Ƚϣ�����ҵ��˵Ļ����Ϳ�����ӵ��б���
			CString strErr;
			for (int i = 0; i < strArrayErrorWords.GetSize(); i++)
			{
				strErr.Format("%s", strArrayErrorWords.GetAt(i));
				if (strHtml.Find(strErr, 0) != -1)
				{
					lpReturnData->strErrWord += strErr;
					lpReturnData->strErrWord += " | ";
				}
			}
		}
		catch (...){}
		
		if (lpReturnData->strErrWord.GetLength() > 0)
			lpReturnData->bResult = TRUE;
		
		// �任����λ��
		int nPos = strURL.Find("&", 0);
		strTemp = strURL;
		if (nPos == -1)
		{
			strList.AddTail(strURL);
		}
		else
		{
			int nQPos = strURL.Find("?", 0);
			nQPos++;
			strNoParameterURL = strURL.Mid(0, nQPos);
			strURL = strURL.Mid(nQPos, strURL.GetLength() - nQPos);
			strTemp = strNoParameterURL + strURL;
			strList.AddTail(strTemp);
			nPos = 0;
			int nCurPos = 0;
			while (1)
			{
				nCurPos = strURL.Find("&", nPos);
				if (nCurPos == -1)
				{
					break;
				}
				strTemp = strURL.Mid(nPos, nCurPos - nPos);
				strOtherParameter = strURL;
				strOtherParameter.Delete(nPos, nCurPos - nPos);
				nPos = nCurPos + 1;
				strTemp = strNoParameterURL + strOtherParameter + "&" + strTemp;
				strTemp.Replace("?&", "?");
				strTemp.Replace("&&", "&");
				strList.AddTail(strTemp);
			}
			
		}
		
		// �ֱ�Ϊÿ��URL������Ӳ����ַ��Լ��ע��
		for (int j = 0; j < strList.GetCount(); j++)
		{
			strTemp = strList.GetAt(strList.FindIndex(j));
			
			if (m_bInjectionCheckNormal)//�������ͨ������ע��
			{
				for (int i = 1; i < 3; i++)
				{
					strURL = ModifyParameter(i, strTemp);
					strDetectURLList.AddTail(strURL);
				}
			}
			else
			{
				for (int i = 1; i < 31; i++)
				{
					strURL = ModifyParameter(i, strTemp);
					strDetectURLList.AddTail(strURL);
				}
			}
		}
		// �����ӹ������ַ���URL���ж����Ƿ�Ϊ��ע������
		long lWebPageSize1 = 0, lWebPageSize2 = 0;
		for (int k = 0; k < strDetectURLList.GetCount(); k += 2)
		{
			strURL	= strDetectURLList.GetAt(strDetectURLList.FindIndex(k));
			strTemp	= strDetectURLList.GetAt(strDetectURLList.FindIndex(k + 1));
			
			lpReturnData->nRate = GetPageChangeRate(strURL, strTemp, lWebPageSize1, lWebPageSize2);// ��ȡҳ���С�仯��
			if (lpReturnData->nRate >= m_nWebPageChangeRate || lpReturnData->nRate < 0)//ҳ���С�仯�ʴ����Զ����������߼�ⳬʱ
			{
				lpReturnData->bResult = TRUE;
				lpReturnData->lWebPageSize1 = min(lWebPageSize1, lWebPageSize2);
				lpReturnData->lWebPageSize2 = max(lWebPageSize1, lWebPageSize2);
				if (m_bInjectionCheckNormal)//�������ͨ������ע��
				{
					lpReturnData->nMethod = 2;
				}
				else
				{
					lpReturnData->nMethod = k + 1;
					while (lpReturnData->nMethod > 30)
						lpReturnData->nMethod -= 30;
				}
				
				lpReturnData->strURL1 = strURL;
				lpReturnData->strURL2 = strTemp;
				break;//һ���з���ע�����������ӳ��֣���������⣬ֱ���ж���Ϊ��ע������
			}
		}
	}
	catch (...){}
}

__inline VOID MyOutputDebugString(LPCTSTR ptzFormat, ...)
{
	va_list vlArgs;
	TCHAR tzText[1024];
	va_start(vlArgs, ptzFormat);
	wvsprintf(tzText, ptzFormat, vlArgs);
	OutputDebugString(tzText);
	va_end(vlArgs);
}

int CMainDlg::GetPageChangeRate(CString strURL1, CString strURL2, long &lWebPageSize1, long &lWebPageSize2)
{
	int nRate = 0;
	try
	{
		CString strHtml;
		DWORD dwStatusCode = 0, dwWebPageSize1 = 0, dwWebPageSize2 = 0;
		lWebPageSize1 = 0, lWebPageSize2 = 0;

		CHttpClient httpClient1;
		if (!httpClient1.GetData(strURL1, CHttpConnection::HTTP_VERB_GET, "", "", m_strUserAgent, m_nTimeOut * 1000, dwStatusCode, dwWebPageSize1, FALSE, FALSE, strHtml))
			lWebPageSize1 = -1;//��ʱ
		else
			lWebPageSize1 = dwWebPageSize1;
		
		CHttpClient httpClient2;
		if (!httpClient2.GetData(strURL2, CHttpConnection::HTTP_VERB_GET, "", "", m_strUserAgent, m_nTimeOut * 1000, dwStatusCode, dwWebPageSize2, FALSE, FALSE, strHtml))
			lWebPageSize2 = -1;//��ʱ
		else
			lWebPageSize2 = dwWebPageSize2;
	}
	catch (...){
		lWebPageSize1 = -1;//��ʱ
		lWebPageSize2 = -1;//��ʱ
	}
	
	if (lWebPageSize1 == -1 || lWebPageSize2 == -1)
	{
		nRate = -1;//��ʱ��־
	}
	else if (lWebPageSize1 == 0 && lWebPageSize2 == 0)
	{
		nRate = 0;
	}
	else if (lWebPageSize1 == 0 && lWebPageSize2 != 0)
	{
		nRate = 10;
	}
	else if (lWebPageSize1 != 0 && lWebPageSize2 == 0)
	{
		nRate = 10;
	}
	else if (lWebPageSize1 > 0 && lWebPageSize1 == lWebPageSize2)
	{
		nRate = 0;
	}
	else
	{
		float nPageChangeRate = (float)(max(lWebPageSize1, lWebPageSize2) - min(lWebPageSize1, lWebPageSize2))/min(lWebPageSize1, lWebPageSize2);
		nRate = nPageChangeRate * 100;
		if (nRate > 100)
			nRate = 100;
	}

	return nRate;
}

void CMainDlg::ShowRowNum()
{
	CString strColumnHeaderText;
	strColumnHeaderText.Format("%s - (%d)", COLUMN_STRING, m_ListCtrl.GetItemCount());
    LVCOLUMN lvCom = {0};
    char szBuffer[256] = {0};
    strcpy(szBuffer, strColumnHeaderText.GetBuffer(0));
    lvCom.mask = LVCF_TEXT;
    lvCom.cchTextMax = 256;
    lvCom.pszText = szBuffer;
    m_ListCtrl.SetColumn(0, &lvCom);
}

void CMainDlg::ShowResultRowNum()
{
	CString strColumnHeaderText;
	strColumnHeaderText.Format("%s - (%d)", COLUMN_STRING_0, m_ListCtrlResult.GetItemCount());
    LVCOLUMN lvCom = {0};
    char szBuffer[256] = {0};
    strcpy(szBuffer, strColumnHeaderText.GetBuffer(0));
    lvCom.mask = LVCF_TEXT;
    lvCom.cchTextMax = 256;
    lvCom.pszText = szBuffer;
    m_ListCtrlResult.SetColumn(0, &lvCom);
}

void CMainDlg::OnMenuitemVisit() 
{
	UpdateData();
	try
	{
		POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
		while(pos)
		{
			int	nItem = m_ListCtrl.GetNextSelectedItem(pos);
			CString strURL = m_ListCtrl.GetItemText(nItem, 0);
			ShellExecute(0, "open", strURL, NULL, NULL, SW_SHOW);
		}
	}
	catch (...){}
}

void CMainDlg::OnMenuitemCopy() 
{
    int nItem;
    CString strText(_T(""));
    POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	
    //��ȡ����ѡ����Ŀ�����ݡ�
    while (pos)
    {
        nItem = m_ListCtrl.GetNextSelectedItem(pos);
        strText += m_ListCtrl.GetItemText(nItem, 0) + _T("\r\n");
    }
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

void CMainDlg::OnMenuitemSave() 
{
	UpdateData();
	CTime time = CTime::GetCurrentTime(); ///����CTime���� 
	CString strTime = time.Format("%Y-%m-%d %H-%M-%S");
	strTime.Insert(0, "URL��");
	
    CFileDialog dlg(FALSE, _T("txt"), strTime, OFN_OVERWRITEPROMPT, _T("*.txt|*.txt||"), NULL);
    if (dlg.DoModal() != IDOK)
        return;
	
    CFile file;
	CString strData;
    if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
    {
		for (int i = 0; i < m_ListCtrl.GetItemCount(); i++)
		{
            strData.Empty();
            strData.Format(_T("%s\r\n"), m_ListCtrl.GetItemText(i, 0));
            file.Write(strData, strData.GetLength());
		}
        file.Close();
        ::MessageBox(0, "�б����ݵ����ɹ�", "��ʾ", MB_ICONINFORMATION);
    }
}

CString CMainDlg::GetKeyboardText()
{
	HGLOBAL   hglb; 
	LPTSTR    lptstr;
	if (!IsClipboardFormatAvailable(CF_TEXT)) 
		return ""; 
	if (!OpenClipboard()) 
		return ""; 
	
	hglb = GetClipboardData(CF_TEXT); 
	if (hglb != NULL) 
	{ 
		lptstr = (char *)GlobalLock(hglb); 
		if (lptstr != NULL)
		{
			GlobalUnlock(hglb);
			CloseClipboard();
			if (strstr(lptstr, "://") != NULL && strstr(lptstr, "?") != NULL)
				return lptstr;
		}
	}
	return "";
}

void CMainDlg::OnMenuitemAddurltolist() 
{
	CInputDialog	dlg;
	dlg.Init(_T("���URL"), _T("���Ҫ����URL���б�:"), this);
	dlg.m_str = GetKeyboardText();
	if (dlg.DoModal() != IDOK)   
		return;
	
	CString strUrl = dlg.m_str;
	strUrl.TrimLeft(" ");
	strUrl.TrimLeft("\t");
	strUrl.TrimRight(" ");
	strUrl.TrimRight("\t");
	
	if (strUrl.GetLength() < 1)
	{
		::MessageBox(NULL, "URL����Ϊ�գ�", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	
	if (strUrl.Find("?", 0) == -1)
	{
		::MessageBox(NULL, "URL��������һ����?����", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	
	//URL��ͷ������"/"��URL��������һ��"."��URL�ﲻ�ܳ���"///"����URL�ĳ��˿�ͷ��"http://"����"//",����Ͳ�������"//"�ˣ������URL�ﲻ�ܳ��ַ�б��
	if (strUrl.GetAt(0) == '//' || strUrl.Find(".", 0) == -1 || strUrl.Find("///", 0) != -1 || strUrl.Find("\\", 0) != -1)
	{
		::MessageBox(NULL, "URL��ʽ����", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	CString strTemp = strUrl.Mid(0, 8);
	if (strTemp.Find("://", 0) == -1 || strTemp.Find("http", 0) == -1)//�ҵ���"://",��û�ҵ�"http",URLҲ�ǲ��Ϸ���
	{
		::MessageBox(NULL, "URL��ʽ����", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	
	m_ListCtrl.InsertItem(0, strUrl, 0);
}

CString CMainDlg::RemoveURLParameters(CString strURL)
{
	int nPos = 0;
	strURL.TrimRight(" ");
	strURL.TrimRight("\t");
	int nLenth = strURL.GetLength();
	CString strTmpURL;
	strTmpURL.Format("%s", strURL);
	int nQMarkPos = strURL.Find("?", 0);
	if (nQMarkPos == -1)//�ж��Ƿ�����ʺ�
	{
		return strURL;
	}
	else
	{
		int nLenthChange = 0;
		int nConnectionSymbolPos = nQMarkPos;//���ӷ�λ��
		int nEqualMark = nQMarkPos;//���ں�λ��
		while (1)
		{
			nEqualMark = strTmpURL.Find("=", nConnectionSymbolPos);
			
			if (nEqualMark == -1)
			{
				strTmpURL.Format("%s", strTmpURL.Mid(0, nConnectionSymbolPos));
				break;
			}
			else
			{
				//�����ӷ�
				nConnectionSymbolPos = strTmpURL.Find("&", nEqualMark);
				if (nConnectionSymbolPos == -1)//�ж��Ƿ�������ӷ�
				{
					nConnectionSymbolPos = nEqualMark + 1;
					strTmpURL.Format("%s", strTmpURL.Mid(0, nEqualMark + 1));
					//goto END;
				}
				else
				{
					nLenthChange = strTmpURL.GetLength();
					strTmpURL.Format("%s%s", strTmpURL.Mid(0, nEqualMark + 1), strTmpURL.Mid(nConnectionSymbolPos, strTmpURL.GetLength()));
					nLenthChange -= strTmpURL.GetLength();
					nConnectionSymbolPos -= nLenthChange;
				}
			}
		}
	}
	return strTmpURL;
}

void CMainDlg::ImportUrlFile(CString strPath)
{
	if (m_bScanning)
	{
		StatusTextOut(1, "����ɨ��ע�룬���Ժ��ٵ���URL���ӣ�");
		return;
	}
	
	if (strPath.GetLength() == 0)
	{
		CFileDialog dlg(TRUE, _T(""), "", OFN_OVERWRITEPROMPT, _T("*.txt|*.txt||"), NULL);
		if (dlg.DoModal() != IDOK)
			return;
		strPath = dlg.GetPathName();
	}

	CStringArray sArray;
	try
	{
		CStdioFileEx file;
		CString strBuffer;
		file.Open(strPath, CFile::modeRead);
		while(file.ReadString(strBuffer) > 0)
		{
			if(strBuffer.GetLength() > 0)
			{
				strBuffer.Format("%s", strBuffer);
				if (strBuffer.Mid(0, 4).CompareNoCase("http") != 0)
					continue;
				
				int nPos = strBuffer.Find('#');
				if(nPos != -1)
					strBuffer.SetAt(nPos, 0x00);
				strBuffer.TrimRight(" ");
				strBuffer.TrimRight("\t");
				strBuffer.Format("%s", strBuffer);
				if (strBuffer.Find("?", 0) == -1)
					continue;
				sArray.Add(strBuffer);
			}
		}
		file.Close();
	}
	catch (...)
	{
		::MessageBox(0, "���ݵ������", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	
	StatusTextOut(0, "���ڵ����б����Ժ�...");
	int nCount = sArray.GetSize();
	if (nCount < 1)
	{
		StatusTextOut(0, "Ҫ������б�Ϊ�գ�");
		return;
	}
	
	int i = 0;
	CString strTemp;
	BOOL bAddToList = TRUE;
	for (int nListPos = 0; nListPos < sArray.GetSize(); nListPos++)
	{
		bAddToList = TRUE;
		strTemp = sArray.GetAt(nListPos);
		if (strTemp.Find("?", 0) == -1)
			continue;
		
		CString strNoParametersURL = RemoveURLParameters(strTemp);
		for (i = 0; i < m_ListCtrl.GetItemCount(); i++)
		{
			if (strNoParametersURL == m_ListCtrl.GetItemText(i, 1))
			{
				bAddToList = FALSE;
				break;
			}
		}
		if (bAddToList)
		{
			m_ListCtrl.InsertItem(0, strTemp);
			m_ListCtrl.SetItemText(0, 1, strNoParametersURL);
		}
	}
	
	ShowRowNum();
	
	//�������
	CTime time = CTime::GetCurrentTime(); ///����CTime���� 
	CString strTime = time.Format("%Y-%m-%d %H:%M:%S");
	StatusTextOut(0, "%s �����б�ɹ���", strTime);

	strPath.MakeLower();
	if (strPath.Find("\\temp\\", 0) != -1)
	{
		DeleteFile(strPath);
	}
}

DWORD CMainDlg::RemoveDuplicateThread()
{
	ImportUrlFile(m_strTempImportFilePath);
	return 0;
}

void CMainDlg::OnMenuitemImport() 
{
	m_strTempImportFilePath = "";
	m_hWorkThread = ThreadTemplate::StartThread<CMainDlg, DWORD>(this, RemoveDuplicateThread);
	if (m_hWorkThread == NULL)
	{
		::MessageBox(NULL, "���������б��߳�ʧ�ܣ�", "��ʾ", MB_ICONINFORMATION);
		return;
	}
}

void CMainDlg::LoadTxt(CString strTxtPath)
{
	m_strTempImportFilePath = strTxtPath;
	m_hWorkThread = ThreadTemplate::StartThread<CMainDlg, DWORD>(this, RemoveDuplicateThread);
	if (m_hWorkThread == NULL)
	{
		::MessageBox(NULL, "���������б��߳�ʧ�ܣ�", "��ʾ", MB_ICONINFORMATION);
		return;
	}
}

void CMainDlg::OnMenuitemDel() 
{
	for (int i = m_ListCtrl.GetItemCount() - 1; i >= 0; i--)
	{
		if (m_ListCtrl.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			m_ListCtrl.DeleteItem(i);
	}
	StatusTextOut(0, "");
	ShowRowNum();
}

void CMainDlg::OnMenuitemClear() 
{
	m_ListCtrl.DeleteAllItems();
	StatusTextOut(0, "");
	
	ShowRowNum();
}

void CMainDlg::OnRclickListUrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CNewMenu OnLineMenu, *pSubMenu;
	OnLineMenu.LoadMenu(IDR_MENU_TOP);
	OnLineMenu.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
	pSubMenu = (CNewMenu *)OnLineMenu.GetSubMenu(0);
	CPoint oPoint;
	GetCursorPos(&oPoint);
	int	count = pSubMenu->GetMenuItemCount();
	if (m_ListCtrl.GetSelectedCount() == 0)
	{
		for (int i = count - 1; i >= 0; i--)
		{
			if (i == count - 4 || i == count - 5)
				continue;
			pSubMenu->DeleteMenu(i, MF_BYPOSITION);
		}
	}
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, oPoint.x, oPoint.y, this);
	
	*pResult = 0;
}

void CMainDlg::OnRclickListResult(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CNewMenu OnLineMenu, *pSubMenu;
	OnLineMenu.LoadMenu(IDR_MENU_BOTTOM);
	OnLineMenu.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
	pSubMenu = (CNewMenu *)OnLineMenu.GetSubMenu(0);
	CPoint oPoint;
	GetCursorPos(&oPoint);
	int	count = pSubMenu->GetMenuItemCount();
	if (m_ListCtrlResult.GetSelectedCount() == 0)
	{
		for (int i = count - 1; i >= 0; i--)
		{
			pSubMenu->DeleteMenu(i, MF_BYPOSITION);
		}
	}
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, oPoint.x, oPoint.y, this);
	
	*pResult = 0;
}

void CMainDlg::OnDblclkListUrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
		while(pos)
		{
			int	nItem = m_ListCtrl.GetNextSelectedItem(pos);
			CString strURL = m_ListCtrl.GetItemText(nItem, 0);
			ShellExecute(0, "open", strURL, NULL, NULL, SW_SHOW);
		}
	}
	catch (...){}	
	*pResult = 0;
}

void CMainDlg::OnDblclkListResult(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();
	try
	{
		POSITION pos = m_ListCtrlResult.GetFirstSelectedItemPosition();
		while(pos)
		{
			int	nItem = m_ListCtrlResult.GetNextSelectedItem(pos);
			CString strURL = m_ListCtrlResult.GetItemText(nItem, 0);
			ShellExecute(0, "open", strURL, NULL, NULL, SW_SHOW);
		}
	}
	catch (...){}

	*pResult = 0;
}

void CMainDlg::OnMenuitemVisitResult() 
{
	UpdateData();
	try
	{
		POSITION pos = m_ListCtrlResult.GetFirstSelectedItemPosition();
		while(pos)
		{
			int	nItem = m_ListCtrlResult.GetNextSelectedItem(pos);
			CString strURL = m_ListCtrlResult.GetItemText(nItem, 0);
			ShellExecute(0, "open", strURL, NULL, NULL, SW_SHOW);
		}
	}
	catch (...){}
}

void CMainDlg::OnMenuitemCopyResult() 
{
    int nItem;
    CString strText;
    POSITION pos = m_ListCtrlResult.GetFirstSelectedItemPosition();
	
    //��ȡ����ѡ����Ŀ�����ݡ�
    while (pos)
    {
        nItem = m_ListCtrlResult.GetNextSelectedItem(pos);
        strText += m_ListCtrlResult.GetItemText(nItem, 0) + _T("\r\n");
    }
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

void CMainDlg::OnMenuitemSaveResult() 
{
	CTime time = CTime::GetCurrentTime(); ///����CTime���� 
	CString strTime = time.Format("%Y-%m-%d %H-%M-%S");
	strTime.Insert(0, "Inject��");
	
    CFileDialog dlg(FALSE, _T("txt"), strTime, OFN_OVERWRITEPROMPT, _T("*.txt|*.txt||"), NULL);
    if (dlg.DoModal() != IDOK)
        return;
	
    CFile file;
	CString strData;
    if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
    {
		for (int i = 0; i < m_ListCtrlResult.GetItemCount(); i++)
		{
            strData.Empty();
            strData.Format(_T("%s\r\n"), m_ListCtrlResult.GetItemText(i, 0));
            file.Write(strData, strData.GetLength());
		}
        file.Close();
        ::MessageBox(0, "�б����ݵ����ɹ�", "��ʾ", MB_ICONINFORMATION);
    }
}

void CMainDlg::OnMaxMinInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    //  Get current pos of the child controls
    CRect rcList;
    CRect rcList1;
    m_ListCtrl.GetWindowRect(rcList);
    m_ListCtrlResult.GetWindowRect(rcList1);
    ScreenToClient(rcList);
    ScreenToClient(rcList1);
    //  return the pos limit
    SPC_NM_MAXMINPOS* pNewMaxMinPos = (SPC_NM_MAXMINPOS*)pNMHDR;
    if (IDC_SPLITTER == pNMHDR->idFrom)
    {
        pNewMaxMinPos->lMin = rcList.top + 50;
        pNewMaxMinPos->lMax = rcList1.bottom - 50;
    }
}

void CMainDlg::OnSplitter1Delta(NMHDR* pNMHDR, LRESULT* pResult)
{
    //  this function just want to show you how to use the delta event
    *pResult = 0;
	
    SPC_NM_DELTA* pDelta = (SPC_NM_DELTA*)pNMHDR;
    if (NULL == pDelta)
        return;
}

void CMainDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;	//�����������ʱ������
	CDialog::PostNcDestroy();
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

		m_strTempImportFilePath = filePath;
		m_hWorkThread = ThreadTemplate::StartThread<CMainDlg, DWORD>(this, RemoveDuplicateThread);
		if (m_hWorkThread == NULL)
		{
			::MessageBox(NULL, "���������б��߳�ʧ�ܣ�", "��ʾ", MB_ICONINFORMATION);
			return;
		}
	}
	DragFinish(hDropInfo);
	CDialog::OnDropFiles(hDropInfo);
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_MOUSEMOVE)//����������Ի����
		m_infotip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CMainDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	lpMeasureItemStruct->itemHeight = 23;
	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CMainDlg::OnMenuitemDelResult() 
{
	for (int i = m_ListCtrlResult.GetItemCount() - 1; i >= 0; i--)
	{
		if (m_ListCtrlResult.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			m_ListCtrlResult.DeleteItem(i);
	}
	ShowResultRowNum();
	StatusTextOut(0, "");
	StatusTextOut(1, "");
}

void CMainDlg::OnMenuitemClearResult() 
{
	m_ListCtrlResult.DeleteAllItems();
	
	ShowResultRowNum();
	StatusTextOut(0, "");
	StatusTextOut(1, "");
}

void CMainDlg::OnMenuitemDeleteTimeoutUrls() 
{
	// TODO: Add your command handler code here
	if (m_bScanning)
	{
		::MessageBox(NULL, "��ȴ���ǰɨ�����֮���ٽ��б�������", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	CString strBuffer;
	for (int i = m_ListCtrlResult.GetItemCount() - 1; i >= 0; i--)
	{
		strBuffer = m_ListCtrlResult.GetItemText(i, 1);
		if (strBuffer == "��ʱ")// �ҵ���ʱ���ӱ�־
		{
			m_ListCtrlResult.DeleteItem(i);
		}
	}
	ShowResultRowNum();
	
	StatusTextOut(0, "��ɾ�����г�ʱ���ӣ�");
	StatusTextOut(1, "");
}

void CMainDlg::OnMenuitemScanTimeoutUrls() 
{
	// TODO: Add your command handler code here
	if (m_bScanning)
	{
		::MessageBox(NULL, "��ȴ���ǰɨ�����֮���ٽ��б�������", "��ʾ", MB_ICONINFORMATION);
		return;
	}
	
	int nItemData = 0;
	CString strBuffer, strCurrentSite = "", strTemp;
	int nCount = 0;
	for (int i = m_ListCtrlResult.GetItemCount() - 1; i >= 0; i--)
	{
		strBuffer = m_ListCtrlResult.GetItemText(i, 1);
		if (strBuffer.Find("��ʱ", 0) != -1)// �ҵ���ʱ���ӱ�־
		{
			strBuffer = m_ListCtrlResult.GetItemText(i, 0);
			nCount = m_ListCtrl.GetItemCount();
			m_ListCtrl.InsertItem(nCount, strBuffer);
			m_ListCtrlResult.DeleteItem(i);
		}
	}
	ShowRowNum();
	ShowResultRowNum();
	OnButtonScanInjection();
	StatusTextOut(0, "");
	StatusTextOut(1, "");
}
