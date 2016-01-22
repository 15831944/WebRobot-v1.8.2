// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WebRobot.h"
#include "MainDlg.h"
#include "WebRobotDlg.h"
#include "PluginDlg.h"
#include "BrowserDlg.h"
#include "SettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TREECTRL_ABOUTME		_T("���ڳ���")
#define TREECTRL_GETURLS		_T("ץȡ����")
#define TREECTRL_FUNCTION		_T("���ܲ��")
#define TREECTRL_SETTING		_T("��������")

char m_strPluginPath[MAX_PATH] = {0};
CMainDlg* g_pMainDlg = NULL;
extern CWebRobotDlg* g_pWebRobotDlg;

typedef struct
{
	TCHAR	*pName;
	TCHAR	*pDescription;
	TCHAR	*pAbout;
	TCHAR	*pVersion;
}DLLINFO;

typedef	  char* (*GetInfo)();
extern PreTM m_PreTM;
CString strAboutWebRobot;
CString strGetUrlsText;
CString strSettingText;
/////////////////////////////////////////////////////////////////////////////
// CMainDlg dialog
IMPLEMENT_DYNCREATE(CMainDlg, CDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	((CWebRobotApp *)AfxGetApp())->m_pMainDlg = this;
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDlg)
	DDX_Control(pDX, IDC_STATIC_TEXT, m_static);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Control(pDX, IDC_TREE, m_TreeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	//{{AFX_MSG_MAP(CMainDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_TREE, OnDblclkTree)
	ON_NOTIFY(NM_CLICK, IDC_TREE, OnClickTree)
	ON_COMMAND(ID_MENUITEM_AUTOLOAD, OnMenuitemAutoload)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, OnRclickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_COMMAND(ID_MENUITEM_OPEN_PLUGIN_DIR, OnMenuitemOpenPluginDir)
	ON_COMMAND(ID_MENUITEM_REFRESH_PLUGIN_LIST, OnMenuitemRefreshPluginList)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, OnItemexpandingTree)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_NOTIFY(SPN_MAXMINPOS, IDC_SPLITTER, OnMaxMinInfo)
	ON_NOTIFY(SPN_DELTA,     IDC_SPLITTER, OnSplitter1Delta)
	ON_WM_MEASUREITEM()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_LIST_DROP_END, OnMessageShowDlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainDlg message handlers

//�ж��ļ��Ƿ����
BOOL FileExists(LPCTSTR lpszFileName)
{
	//�õ��ļ�����
	DWORD dwAttributes=GetFileAttributes(lpszFileName);
	//�������óɹ����ļ�����
	if(dwAttributes==0xffffffff)
		return false;
	//�����ļ�������
	else
		return true;
}

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
	g_pMainDlg = (CMainDlg *)((CWebRobotApp *)AfxGetApp())->m_pMainDlg;
	// TODO: Add extra initialization here
	m_ImageList.Create(16, 16,ILC_COLOR32,2,2); 
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_ABOUT));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_BROWSER));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_PLUGIN));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON_SETTING));
	::SendMessage(m_TreeCtrl.m_hWnd, TVM_SETITEMHEIGHT, 23, 0);
	
	m_ImageList1.Create(24, 24, ILC_COLORDDB | ILC_MASK, 0, 1);
	LoadMyBitmap(&m_ImageList1, IDB_BITMAP_PLUGIN);
	m_ListCtrl.InsertColumn(0, _T("�������"), LVCFMT_LEFT, 150);
	m_ListCtrl.InsertColumn(1, _T("˵��(���֧����ק����)"), LVCFMT_LEFT, 350);
	m_ListCtrl.InsertColumn(2, _T("����"), LVCFMT_LEFT, 50);
	m_ListCtrl.InsertColumn(3, _T("�汾"), LVCFMT_LEFT, 70);
	m_ListCtrl.InsertColumn(4, _T("�ļ���"), LVCFMT_LEFT, 120);
	m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT);
	m_ListCtrl.SetImageList(&m_ImageList1,LVSIL_SMALL);

	strAboutWebRobot = "<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"app:WM_APP_COMMAND1\"><font size=\"+13\" color=\"green\">WebRobot</font></a><br><br><br>";
	strAboutWebRobot += "&nbsp;&nbsp;&nbsp;&nbsp;��������һ����ģʽ�����簲ȫ��������Ҫ�����ɲ����ʵ�֡���������������Ŀ¼�µ�plugin�ļ�������в����<br>&nbsp;�����ļ����ֵ�������ļ�Ҳ���������ļ����ڡ�<br><br>";
	strAboutWebRobot += "&nbsp;&nbsp;&nbsp;&nbsp;����������WebRobot�����ڼ��Web��ȫ���ܣ����������Ƿ���;���Ƿ�ʹ�ñ�������������ֱ�ӻ�����ʧ�뱾���򿪷�����<br>&nbsp;�ء�����ʹ�ñ����ߵ�һ���𣬽���Ϊ���Ѿ����ܱ�����������<br><br>";
	strAboutWebRobot += "&nbsp;&nbsp;&nbsp;&nbsp;����ʹ�ñ�����Ĺ����������κ��������ʲô�õ�������飬��ӭɧ�š�<br><br><br>";
	strAboutWebRobot += "&nbsp;BY:����&nbsp;&nbsp;<a href=\"http://wpa.qq.com/msgrd?v=3&uin=243107&site=qq&menu=yes\">QQ:243107</a>";
	strAboutWebRobot += "<br><br>&nbsp;<a href=\"https://www.t00ls.net/index.php?from=WebRobot\"><font color=\"blue\">T00ls��̳</font></a>";
	strAboutWebRobot += "<br><br>&nbsp;<a href=\"http://webrobot.blog.51cto.com/\"><font color=\"blue\">My Blog</font></a><br>";
	m_static.SetWindowText(strAboutWebRobot);

	strGetUrlsText	= _T("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color=\"green\">ץȡ����</font><br><br><br>&nbsp;&nbsp;&nbsp;\
						 �������Դ������<br><br>&nbsp;&nbsp;&nbsp;&nbsp;�ȿ�����ȡ��ҳ���ӹ��ܣ�Ȼ���øò���Դ������������ҳ���Ϳ���ץȡ��ҳ����������ӡ�<br><br>&nbsp;&nbsp;&nbsp;&nbsp;���øù���ץȡ�������������վ���ӡ�");

	strSettingText	= _T("<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color=\"green\">��������</font><br><br><br>&nbsp;&nbsp;&nbsp;\
						 ����������á�");

	GetDlgItem(IDC_LIST)->ShowWindow(SW_HIDE);
	//Splitter
	CRect rc;
	CWnd* pWnd = GetDlgItem(IDC_SPLITTER);
	pWnd->GetWindowRect(rc);
	ScreenToClient(rc);
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rc, this, IDC_SPLITTER, SPS_VERTICAL|SPS_DELTA_NOTIFY);
    //  register windows for splitter
    m_wndSplitter.RegisterLinkedWindow(SPLS_LINKED_LEFT,     GetDlgItem(IDC_TREE));
    m_wndSplitter.RegisterLinkedWindow(SPLS_LINKED_RIGHT,    GetDlgItem(IDC_LIST));
    m_wndSplitter.RegisterLinkedWindow(SPLS_LINKED_RIGHT,    GetDlgItem(IDC_STATIC_TEXT));
    m_wndSplitter.Relayout();

	GetModuleFileName(NULL, m_strPluginPath, sizeof(m_strPluginPath));
	PathRemoveFileSpec(m_strPluginPath);
	lstrcat(m_strPluginPath, "\\plugin\\");
	LoadPlugins(m_strPluginPath);

	PostMessage(WM_SIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
    if (IsWindow(m_wndSplitter.GetSafeHwnd()))
	{
		LockWindowUpdate();
		if (m_ListCtrl.m_hWnd != NULL)
			Resize();
		UnlockWindowUpdate();
	}
}

void CMainDlg::MoveDlgItem(int nID, const CRect& rcPos, BOOL bRepaint)
{
    CWnd* pWnd = GetDlgItem(nID);
    if (NULL == pWnd)
        return;
    pWnd->MoveWindow(rcPos, bRepaint);
    CRect rcsp;
    m_wndSplitter.GetWindowRect(rcsp);
    ScreenToClient(rcsp);
}

void CMainDlg::Resize()
{
    CRect rcDlg;
    GetClientRect(rcDlg);
	
    CRect rcSplit;
    m_wndSplitter.GetWindowRect(rcSplit);
    ScreenToClient(rcSplit);
	rcSplit.top = rcDlg.top + 1;
    rcSplit.bottom = rcDlg.bottom;
    m_wndSplitter.MoveWindow(rcSplit, TRUE);
	
    CRect rcTreeCtrl;
    rcTreeCtrl.left    = rcDlg.left;
    rcTreeCtrl.right   = rcSplit.left - 1;
    rcTreeCtrl.top     = rcDlg.top + 1;
    rcTreeCtrl.bottom  = rcDlg.bottom;
    MoveDlgItem(IDC_TREE, rcTreeCtrl, TRUE);
	
	CRect rcList;
	rcList.left     = rcSplit.right + 1;
	rcList.right    = rcDlg.right;
	rcList.top      = rcDlg.top + 1;
	rcList.bottom   = rcDlg.bottom;
	MoveDlgItem(IDC_LIST, rcList, TRUE);

	CRect rcText;
	rcText.left     = rcSplit.right + 1;
	rcText.right    = rcDlg.right;
	rcText.top      = rcDlg.top + 1;
	rcText.bottom   = rcDlg.bottom;
	MoveDlgItem(IDC_STATIC_TEXT, rcText, TRUE);
	m_static.Reset();

	UpdateWindow();
}

int nIcon = 3;//���ͼ�꿪ʼ�ĵط����������nTreeIcon = 2;��һ��ֵ
void CMainDlg::InsertInfoToTreeCtrl(CString strName, int nData)														//���뵽TreeCtrl
{
	HTREEITEM hRoot = NULL;
	HTREEITEM hTempItem = NULL;
	if (strName == TREECTRL_ABOUTME || strName == TREECTRL_FUNCTION || strName == TREECTRL_SETTING)
	{
		hRoot = IsItemExist(TVI_ROOT, strName);//����֧
		if (hRoot == NULL)
		{
			int nTreeIcon = 0;
			if (strName == TREECTRL_ABOUTME)
				nTreeIcon = 0;
			else if (strName == TREECTRL_FUNCTION)
				nTreeIcon = 2;
			else
				nTreeIcon = 3;

			hRoot = m_TreeCtrl.InsertItem(strName, nTreeIcon, nTreeIcon, TVI_ROOT, TVI_LAST);//����֧
			m_TreeCtrl.SetItemData(hRoot, nData);
		}
	}
	else
	{
		hRoot = IsItemExist(TVI_ROOT, TREECTRL_FUNCTION);//����֧
		if (hRoot != NULL)
		{
			hTempItem = IsItemExist(hRoot, strName);
			if (hTempItem == NULL)
			{
				if (strName == TREECTRL_GETURLS)
					hTempItem = m_TreeCtrl.InsertItem(strName, 1, 1, hRoot, TVI_LAST);
				else
					hTempItem = m_TreeCtrl.InsertItem(strName, nIcon, nIcon, hRoot, TVI_LAST);

				if (hTempItem != NULL)
					m_TreeCtrl.SetItemData(hTempItem, nData);//���
			}
			m_TreeCtrl.Expand(hRoot, TVE_EXPAND);//չ�����
		}
	}
}

void CMainDlg::InsertInfoToListCtrl(CString strName, CString strDescription, CString strVersion, CString strFileName, CString strPath, CString strCrc32)		//���뵽ListCtrl
{
	nIcon++;
	int nCount = m_ListCtrl.GetItemCount();
	long nData = strtol(strCrc32, NULL, 16);

	m_ListCtrl.InsertItem(nCount, strName, 0);
	m_ListCtrl.SetItemText(nCount, 1, strDescription);

	if (GetFileAttributes(strPath) & FILE_ATTRIBUTE_READONLY)
		m_ListCtrl.SetItemText(nCount, 2, "��");
	else
	{
		m_ListCtrl.SetItemText(nCount, 2, "��");
		InsertInfoToTreeCtrl(strName, nData);
	}

	m_ListCtrl.SetItemText(nCount, 3, strVersion);
	m_ListCtrl.SetItemText(nCount, 4, strFileName);

	m_ListCtrl.SetItemData(nCount, nData);
}

HTREEITEM CMainDlg::IsItemExist(HTREEITEM hItemParent, CString const& strItem)
{
	BOOL bDoesItemExist=FALSE;
	HTREEITEM hExistItem = NULL;
	ASSERT(strItem.GetLength());
	HTREEITEM hChild = m_TreeCtrl.GetChildItem(hItemParent?hItemParent:TVI_ROOT);
	while (NULL != hChild && !bDoesItemExist)	
	{
		CString str = m_TreeCtrl.GetItemText(hChild);//��ȡ����ַ�����Ϣ
		if (0 == str.CompareNoCase(strItem))
		{
			bDoesItemExist = TRUE;
			hExistItem = hChild;
			break;
		}
		else
		{
			hChild = m_TreeCtrl.GetNextSiblingItem(hChild);
		}
	}
	return hExistItem;
}

CString CMainDlg::GetPluginPath(CString strName)
{
	CString strPath;
	strPath.Format("%s%s", m_strPluginPath, strName);
	return strPath;
}

void CMainDlg::LoadPlugins(LPCTSTR lpszPath)
{
	m_TreeCtrl.DeleteAllItems();
	m_ListCtrl.DeleteAllItems();
	InsertInfoToTreeCtrl(TREECTRL_ABOUTME, -1);
	InsertInfoToTreeCtrl(TREECTRL_FUNCTION, -2);
	InsertInfoToTreeCtrl(TREECTRL_GETURLS, -3);
	CStringArray sArrayPluginName;
	BOOL bExistSortInfo = TRUE;
	if (((CWebRobotApp *)AfxGetApp())->m_IniFile.GetString("Config", "PluginSort", "") == "")
		bExistSortInfo = FALSE;

	TCHAR szFind[MAX_PATH];
	lstrcpy(szFind, lpszPath);
	lstrcat(szFind, _T("*.*")); // �������ļ�
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(szFind, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
		return;
	do  
	{
		if (wfd.cFileName[0] == '.')
			continue; // ����������Ŀ¼
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}else
		{
			TCHAR szFile[MAX_PATH];
			wsprintf(szFile, _T("%s%s"), lpszPath, wfd.cFileName);
			LPTSTR pszExtension = PathFindExtension(szFile);
			if (stricmp(pszExtension, _T(".dll")) == 0)
			{
				if (!bExistSortInfo)//û�з���������Ϣ��ֱ�Ӱ�Ĭ�ϵ���
				{
					InsertPluginInfo(szFile, wfd.cFileName);
				}
				else
				{
					sArrayPluginName.Add(wfd.cFileName);
				}
			}
		}
	}while (FindNextFile(hFind, &wfd));  
	FindClose(hFind);

	if (bExistSortInfo)//����������Ϣ������Ϣ�ϵ�������
		InsertPluginInfoEx(sArrayPluginName);
	
	InsertInfoToTreeCtrl(TREECTRL_SETTING, -4);
	m_TreeCtrl.SetImageList(&m_ImageList, TVSIL_NORMAL);
}

void CMainDlg::InsertPluginInfo(LPCTSTR lpszPath, LPCTSTR lpszFileName)
{
	CString strName, strDescription, strVersion;
	HMODULE hModule = LoadLibrary(lpszPath);
	if (!hModule)
		return;

	GetInfo info = (GetInfo)GetProcAddress(hModule, "GetInfo");
	DLLINFO *pInfo = (DLLINFO*)info();
	strName = pInfo->pName;
	strDescription = pInfo->pDescription;
	strVersion = pInfo->pVersion;
	FreeLibrary(hModule);
	if (pInfo)
		HeapFree(GetProcessHeap(), 0, pInfo);//�ͷ�dll����GlobalAlloc������ڴ�

	HICON hicon = ExtractIcon(AfxGetInstanceHandle(), lpszPath, 0);
	m_ImageList.Add(hicon); 

	FileCRC32		m_CRC32;
	m_CRC32.Open(lpszPath);
	CString strCRC32  = m_CRC32.GetFileCRCString();
	m_CRC32.Close();

	strCRC32 = strCRC32.Left(5);

	InsertInfoToListCtrl(strName, strDescription, strVersion, lpszFileName, lpszPath, strCRC32);
}

void CMainDlg::InsertPluginInfoEx(CStringArray &sArray)
{
	CString strName, strPath;
	CStringArray sArrayPlugin, sArrayNewPlugin;
	CString strInfo = ((CWebRobotApp *)AfxGetApp())->m_IniFile.GetString("Config", "PluginSort", "");
	int nPos = 0;
	while (1)
	{
		int nPos1 = strInfo.Find("|", nPos);
		if (nPos1 == -1)
			break;
		strName = strInfo.Mid(nPos, nPos1 - nPos);
		if (strName.GetLength() != 0)
			sArrayPlugin.Add(strName);
		nPos = nPos1 + 1;
	}

	//��ȡ����ӵĲ��
	BOOL bFind = FALSE;
	for (int i = 0; i < sArray.GetSize(); i++)
	{
		bFind = FALSE;
		strName = sArray.GetAt(i);
		for (int j = 0; j < sArrayPlugin.GetSize(); j++)
		{
			if (strName == sArrayPlugin.GetAt(j))
			{
				bFind = TRUE;
				break;
			}
		}
		if (!bFind)//������ӽ����Ĳ��
			sArrayNewPlugin.Add(strName);
	}
	//���밴˳������Ĳ��
	for (i = 0; i < sArrayPlugin.GetSize(); i++)
	{
		strName = sArrayPlugin.GetAt(i);
		strPath = GetPluginPath(strName);
		if (FileExists(strPath))
		{
			InsertPluginInfo(strPath, strName);
		}
	}
	//����������ӽ����Ĳ��
	for (i = 0; i < sArrayNewPlugin.GetSize(); i++)
	{
		strName = sArrayNewPlugin.GetAt(i);
		InsertPluginInfo(GetPluginPath(strName), strName);
	}
}

void CMainDlg::OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	HTREEITEM hTreeSel = m_TreeCtrl.GetSelectedItem();
	if (hTreeSel == NULL)
		return;

	DWORD dwData = m_TreeCtrl.GetItemData(hTreeSel);
	if (dwData <= 0)//���ܲ��/δ֪����
	{
		return;
	}
	if (dwData == -3)//ץȡ����
	{
		CCoolTabCtrl::CPageItem* pItem = g_pWebRobotDlg->m_TabCtrl.AddPage(RUNTIME_CLASS(CBrowserDlg), IDD_BROWSER_DLG, " ץȡ���� ", IDI_ICON_BROWSER);
		g_pWebRobotDlg->m_TabCtrl.SetActivePage(g_pWebRobotDlg->m_TabCtrl.GetPageCount() - 1);
	}
	if (dwData == -4)//��������
	{
		CCoolTabCtrl::CPageItem* pItem = g_pWebRobotDlg->m_TabCtrl.AddPage(RUNTIME_CLASS(CSettingDlg), IDD_SETTING_DLG, " �������� ", IDI_ICON_SETTING);
		g_pWebRobotDlg->m_TabCtrl.SetActivePage(g_pWebRobotDlg->m_TabCtrl.GetPageCount() - 1);
	}


	CString strPath;
	int nItem = -1;
	for (int i = 0; i < m_ListCtrl.GetItemCount(); i++)
	{
		if (dwData == m_ListCtrl.GetItemData(i))
		{
			nItem = i;
			break;
		}
	}
	if (nItem == -1)
		return;

	strPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(i, 4));
	strPath.Replace("\\\\", "\\");
	LoadPlugin(m_ListCtrl.GetItemText(i, 0), strPath);

	*pResult = 0;
}

void CMainDlg::LoadCustomPlugin(CString strName, CString strTempTxtPath)
{
	CString strPluginPath;
	for (int i = 0; i < m_ListCtrl.GetItemCount(); i++)
	{
		if (m_ListCtrl.GetItemText(i, 0) == strName)
		{
			strPluginPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(i, 4));
			strPluginPath.Replace("\\\\", "\\");
			break;
		}
	}
	
	if (FileExists(strPluginPath))
	{
		LoadPlugin("ע����", strPluginPath, strTempTxtPath);
	}
}

void CMainDlg::LoadPlugin(CString strName, CString strPath, CString strTempTxtPath)
{
	BOOL bLoadTmpTxt = FALSE;
	if (strTempTxtPath != "")
		bLoadTmpTxt = TRUE;

	//�жϲ���Ƿ�򿪹�
	strName += " ";
	strName.Insert(0, " ");

	HINSTANCE hModule = NULL;
	typedef CDialog* (*lpFun)(CWnd *p);
	hModule = LoadLibrary(strPath);
	if (NULL == hModule)
	{
		AfxMessageBox(_T("dll����ʧ��"));
		return ;
	}
	PLoadTempTxt pLoadTxt = NULL;
	lpFun pShowDlg = (lpFun)GetProcAddress(hModule, "ShowDlg");
	m_PreTM = (PreTM)GetProcAddress(hModule, "PreTranslateMessage");
	if (bLoadTmpTxt)
	{
		pLoadTxt = (PLoadTempTxt)GetProcAddress(hModule, "LoadTempTxt");
		if (NULL == pLoadTxt)
		{
			AfxMessageBox(_T("dll�к���LoadTempTxtѰ��ʧ��"));
			return;
		}
	}
	if (NULL == pShowDlg)
	{
        AfxMessageBox(_T("dll�к���ShowDlgѰ��ʧ��"));
		return;
	}
	if (NULL == m_PreTM)
	{
        AfxMessageBox(_T("dll�к���PreTranslateMessageѰ��ʧ��"));
		return;
	}

	HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), strPath, 0);
	CCoolTabCtrl::CPageItem* pItem = g_pWebRobotDlg->m_TabCtrl.AddPage(RUNTIME_CLASS(CPluginDlg), IDD_PLUGIN_DLG, strName, hIcon);
	g_pWebRobotDlg->m_TabCtrl.SetActivePage(g_pWebRobotDlg->m_TabCtrl.GetPageCount() - 1);
	
	if (pShowDlg(pItem->m_pWnd))
	{
		if (bLoadTmpTxt)
		{
			pLoadTxt(strTempTxtPath);
		}
	}
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
	{   
		int nVirtKey = (int)pMsg->wParam;           
		if (nVirtKey == VK_RETURN)   
		{   
			//����ǻس�����������Ҫ��������,����ʲôҲ����
			return TRUE;
		}   
		else if (nVirtKey == VK_ESCAPE)   
		{   
			//�����ESC����������Ҫ��������,����ʲôҲ����
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CMainDlg::OnMaxMinInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    //  Get current pos of the child controls
    CRect rcTree;
    CRect rcList;
    m_TreeCtrl.GetWindowRect(rcTree);
    m_ListCtrl.GetWindowRect(rcList);
    ScreenToClient(rcTree);
    ScreenToClient(rcList);
    //  return the pos limit
    SPC_NM_MAXMINPOS* pNewMaxMinPos = (SPC_NM_MAXMINPOS*)pNMHDR;
    if (IDC_SPLITTER == pNMHDR->idFrom)
    {
        pNewMaxMinPos->lMin = rcTree.left + 150;
        pNewMaxMinPos->lMax = rcList.right - 250;
    }
}

void CMainDlg::OnSplitter1Delta(NMHDR* pNMHDR, LRESULT* pResult)
{
    //  this function just want to show you how to use the delta event
    *pResult = 0;
	
    SPC_NM_DELTA* pDelta = (SPC_NM_DELTA*)pNMHDR;
    if (NULL == pDelta)
        return;
	PostMessage(WM_SIZE);
}

void CMainDlg::OnClickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	UINT uFlags = 0;
	CPoint pt = GetCurrentMessage()->pt;   //��ȡ��ǰ�������Ϣ�������
	m_TreeCtrl.ScreenToClient(&pt);   //��������Ļ���꣬ת�������οؼ��Ŀͻ�������
	HTREEITEM hItem = m_TreeCtrl.HitTest(pt, &uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))//û�е���ڰ�ť��
	{
		DWORD dwData = m_TreeCtrl.GetItemData(hItem);
		if (dwData == -1)//���ڳ���
		{
			m_static.SetWindowText(strAboutWebRobot);

			if (!(m_static.GetStyle() & WS_VISIBLE))
			{
				GetDlgItem(IDC_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_SHOW);
			}
		}
		else if (dwData == -2)//���ܲ��
		{
			if (m_static.GetStyle() & WS_VISIBLE)
			{
				GetDlgItem(IDC_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_HIDE);
			}
		}
		if (dwData == -3)//ץȡ����
		{
			m_static.SetWindowText(strGetUrlsText);
			
			if (!(m_static.GetStyle() & WS_VISIBLE))
			{
				GetDlgItem(IDC_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_SHOW);
			}
		}
		if (dwData == -4)//��������
		{
			m_static.SetWindowText(strSettingText);
			
			if (!(m_static.GetStyle() & WS_VISIBLE))
			{
				GetDlgItem(IDC_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_SHOW);
			}
		}
		else if (dwData > 0)
		{
			int nItemList = -1;
			for (int i = 0; i < m_ListCtrl.GetItemCount(); i++)
			{
				if (dwData == m_ListCtrl.GetItemData(i))
				{
					nItemList = i;
					break;
				}
			}
			if (nItemList != -1)
			{
				CString strAbout, strPath;
				strPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(i, 4));
				strPath.Replace("\\\\", "\\");
				HMODULE hModule = LoadLibrary(strPath);
				if (!hModule)
					return;
				GetInfo info = (GetInfo)GetProcAddress(hModule, "GetInfo");
				DLLINFO *pInfo = (DLLINFO*)info();
				strAbout = pInfo->pAbout;
				FreeLibrary(hModule);
				if (pInfo)
					HeapFree(GetProcessHeap(), 0, pInfo);//�ͷ�dll����GlobalAlloc������ڴ�
				
				m_static.SetWindowText(strAbout);
				
				if (!(m_static.GetStyle() & WS_VISIBLE))
				{
					GetDlgItem(IDC_LIST)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_SHOW);
				}
			}
		}
	}

	*pResult = 0;
}

void CMainDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CNewMenu OnLineMenu, *pSubMenu;
	OnLineMenu.LoadMenu(IDR_MENU_PLUGIN);
	OnLineMenu.SetMenuDrawMode(CNewMenu::STYLE_XP_2003);
	pSubMenu = (CNewMenu *)OnLineMenu.GetSubMenu(0);
	CPoint oPoint;
	GetCursorPos(&oPoint);
	int	count = pSubMenu->GetMenuItemCount();
	if (m_ListCtrl.GetSelectedCount() == 0)
	{
		pSubMenu->DeleteMenu(1, MF_BYPOSITION);
	}
	else if (m_ListCtrl.GetSelectedCount() == 1)
	{
		POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
		int iCurrSel= m_ListCtrl.GetNextSelectedItem(pos);
		if (iCurrSel < 0)
			return;//δѡ�к�������
		CString strPath;
		strPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(iCurrSel, 4));
		strPath.Replace("\\\\", "\\");
		if (FileExists(strPath))
		{
			if (m_ListCtrl.GetItemText(iCurrSel, 2) == "��")
				pSubMenu->ModifyODMenu(ID_MENUITEM_AUTOLOAD, MF_BYCOMMAND, ID_MENUITEM_AUTOLOAD, "��Ϊ�Զ�����");
			else
				pSubMenu->ModifyODMenu(ID_MENUITEM_AUTOLOAD, MF_BYCOMMAND, ID_MENUITEM_AUTOLOAD, "��Ϊ���Զ�����");
		}
	}

	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, oPoint.x, oPoint.y, this);

	*pResult = 0;
}

void CMainDlg::OnMenuitemAutoload() 
{
	UpdateData();
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	int iCurrSel= m_ListCtrl.GetNextSelectedItem(pos);
	if (iCurrSel < 0)
		return;//δѡ�к�������
	CString strStatus, strPath;
	strStatus = m_ListCtrl.GetItemText(iCurrSel, 2);
	strPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(iCurrSel, 4));
	strPath.Replace("\\\\", "\\");
	if (FileExists(strPath))
	{
		if (strStatus == "��")
			SetFileAttributes(strPath, FILE_ATTRIBUTE_NORMAL);
		else
			SetFileAttributes(strPath, FILE_ATTRIBUTE_READONLY);

		LoadPlugins(m_strPluginPath);
	}
}

void CMainDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	int iCurrSel= m_ListCtrl.GetNextSelectedItem(pos);
	if (iCurrSel < 0)
		return;//δѡ�к�������

	CString strPath;
	strPath.Format("%s\\%s", m_strPluginPath, m_ListCtrl.GetItemText(iCurrSel, 4));
	strPath.Replace("\\\\", "\\");
	LoadPlugin(m_ListCtrl.GetItemText(iCurrSel, 0), strPath);
	*pResult = 0;
}

void CMainDlg::OnMenuitemOpenPluginDir() 
{
	::ShellExecute(NULL, _T("open"), m_strPluginPath, NULL, NULL, SW_SHOW);
}

void CMainDlg::OnMenuitemRefreshPluginList() 
{
	LoadPlugins(m_strPluginPath);
}

void CMainDlg::OnItemexpandingTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (pNMTreeView->hdr.code==TVN_ITEMEXPANDING
        && pNMTreeView->action==TVE_COLLAPSE)//��ֹ�ڵ�����
    {
        *pResult = 1;
		return;
    }
	*pResult = 0;
}

LRESULT CMainDlg::OnMessageShowDlg(WPARAM wParam, LPARAM lParam)
{
	CString strInfo;
	for (int i = 0; i < m_ListCtrl.GetItemCount(); i++)
	{
		strInfo += m_ListCtrl.GetItemText(i, 4);
		strInfo += "|";
	}
	((CWebRobotApp *)AfxGetApp())->m_IniFile.SetString("Config", "PluginSort", strInfo);
	LoadPlugins(m_strPluginPath);

	return 0;
}

void CMainDlg::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hSelect = m_TreeCtrl.GetSelectedItem();
	HTREEITEM hLight = m_TreeCtrl.GetDropHilightItem();
	if(hLight != NULL && hSelect != hLight)
		hSelect = hLight;
	if(hSelect == NULL)
		return;
	m_TreeCtrl.SelectItem(hSelect);

	*pResult = 0;
}

void CMainDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	lpMeasureItemStruct->itemHeight = 23;
	CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
