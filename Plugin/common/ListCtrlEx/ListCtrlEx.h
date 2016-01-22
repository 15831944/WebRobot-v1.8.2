#if !defined(AFX_LISTCTRLEX_H__INCLUDED_)
#define AFX_LISTCTRLEX_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlEx.h : header file
//
#include <afxtempl.h>       // MFC core and standard components

#define _CCS_DES

#define WM_COLUMN_CHANGED		WM_USER+5	//�ı��еĿ��ʱ���͸���Ϣ
#define WM_ITEM_CHANGED			WM_USER+8	//ĳһ��༭�ı����ݺ��͸���Ϣ
#define WM_LISTBUTTON_CLICKED	WM_USER+10	//�б��еİ�ť���������͸���Ϣ

#define CS_NONE			0x00000000	//�޿ؼ�
#define CS_EDIT			0x00000001	//�༭��
#define CS_COMBOBOX		0x00000002	//��Ͽ�
#define CS_BUTTON		0x00000004	//��ť


/*=========================================================================================
CListCtrlEx Class Version 3.00
Author:luxintian
Company:�������ܿƼ��������޹�˾
Tel:13737075336
E-Mail:luxintian@sina.com
Date:2005-05-07
All right reserved.
��Ĺ��ܣ�
�����Ǽ̳���MFC���е�CListCtrl�࣬�ڸ����������һ�¹���
1�������޸ĸ����������ɫ
2�������޸ĸ���ı�����ɫ
3������б�ͷ���б�������Ը������ݽ�������
4���������ı��������Ͽ�༭�������Ҫʹĳһ���ܱ��༭�������SetEdit����(Ĭ���ǲ����༭��)

�޸����̣�
1��ȥ����CArray<COLORREF,COLORREF> m_aryBKColor��CArray<COLORREF,COLORREF> m_aryTextColor
2����ΪITEMCOLOR�ṹ��
3���ڲ������ʱ���ITEMCOLOR�ṹʵ���ĵ�ַ������LVITEM�ṹ�еĲ���lParam
4��2005��6��22�������������ڴ�й©��Bug,�����OnDestroy()�����ں��������
   ��ɾ���Ѿ�ΪITEMCOLOR�ṹ�����ռ䡣
5��2006��12��27�������WM_COLUMN_CHANGED�Զ�����Ϣ������Ϣ���ڸı��еĿ�ȵ�ʱ���͸���Ϣ��������
   ����������Ӹ���Ϣ�Ĵ��������Ϳ��Դ����ڸı��еĿ��ʱ���¼���
6��2007��3��28���������InsertColumn�����е�nFormat�����ĵĿ����ԣ�֮ǰֻ�Ǿ��п��ã�����ʹ��LVCFMT_LEFT��
   LVCFMT_CENTER��LVCFMT_RIGHT��ʹÿһ�е�����룬���к��Ҷ��롣
7��2007��6��3�������SetItemText��GetItemText���������أ��ú����ĵڶ��������������б�ͷ�����ƣ���
   LPCTSTR���͵Ĳ���
8��2007��6��4�������SetColumnWidth��GetColumnWidth���������أ��ú����ĵڶ��������������б�ͷ�����ƣ���
   LPCTSTR���͵Ĳ���
9��2007��6��7���޸���SetItemTextColor��SetItemBKColor��SetItemText�е����ݣ�ʹ�����ˢ�´���
10��2007��9��25��ɾ����OnDestroy������ɾ����InsertItem����ʹ����OnInsertitem������Ӹ��Ӵ��룻
    ɾ����DeleteItem��DeleteAllItems����ʹ����OnDeleteitem������Ӹ��Ӵ��룻
    �޸���tagITEMCOLOR�ṹΪCListCtrlEx�ĳ�Ա�ṹ
11��2007��9��7����ԭ���Ļ��������CListCellCb���CListCellEd�࣬ʹ�б�ؼ��Ա༭��
12��2007��10��17��������б��ӡ���ܣ�ʹ��PrintReport������GetPrintPages�������ڶԻ���he��ͼ�зֱ��������
    ��ͬ�汾��PrintReport����������ͼ�а�PrintReport�����ŵ�OnPrint�����У����Ǳ�����OnBeginPrinting������
	����GetPrintPages�������Ҫ��ӡ��ҳ��������pInfo->SetMaxPage��������ҳ��,�ô�ӡ����ֻ�����ڹ̶��ṹ��ʽ,
	���Ҫ��ӡ������ʽ,����ʹ�øú���,ֻ���Լ�д������
13��2008��1��3����������б��а�ť����������InsertColumn������ʹ��CS_BUTTON��ʽ����ʵ�֣����ڵ����ťʱ��
	�����巢��WM_LISTBUTTON_CLICKED�Զ�����Ϣ
14��2008��3��24�������˴�ӡ�е����������⣬ÿһ�еĸ߶���double���͵ı�������ʾ��
15��2008��4��18����������XP��ʽ�Ľ����±�ͷ��ʾ�쳣���⣬ԭ���Ǳ�ͷ�����ػ�ķ������£������������������ػ棬
	���Ǽ��롰shell32.dll���е�λͼ��Ϊ��ͷ����ͼ��
------------------------
ע�⣺
WM_ITEM_CHANGED��ϢΪ�б��޸ĺ󷢳�����Ϣ��WPARAMΪ���޸ĵĵ���������LPARAMΪ���޸���������
����ÿؼ��������ʱ������Ҫ���ļ���ͷ��Ӻ궨�壺#define _CCS_DES
����ÿؼ���������ʱ������Ҫ���ļ���ͷ��Ӻ궨�壺#define _CCS_RUN
*///=======================================================================================
//////////////////////////////////////////////////////////////////////////
// CListCellBT window

class CListCellBT : public CButton
{
	// Construction
public:
	CListCellBT(int nItem,int nSubItem);
	
	// Attributes
public:
	
	// Operations
public:
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCellBT)
	//}}AFX_VIRTUAL
	
	// Implementation
public:
	virtual ~CListCellBT();
	
	// Generated message map functions
protected:
	int			m_nItem;
    int			m_nSubItem;
	
	//{{AFX_MSG(CListCellBT)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnClicked();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////

// CListCellEd window

class CListCellEd : public CEdit
{
	// Construction
public:
	CListCellEd(int nItem, int nSubItem, CString strInitTex);
	void SetListItemText();
	
	// Attributes
public:
	
	// Operations
public:
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCellEd)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	
	// Implementation
public:
	virtual ~CListCellEd();
	
protected:
	int			m_nItem;
    int			m_nSubItem;
    CString		m_strInitText;
    BOOL		m_bEscape;
	
	// Generated message map functions
	//{{AFX_MSG(CListCellEd)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CListCellCb window

class CListCellCb : public CComboBox
{
	// Construction
public:
	CListCellCb();
	CListCellCb(int nItem,int nSubItem,CString strWindowText);
	// Attributes
public:
	
	// Operations
public:
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCellCb)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	
	// Implementation
public:
	virtual ~CListCellCb();
	
	// Generated message map functions
protected:	
	int m_iRowIndex;// Index of the item in the list control	
	int m_iColumnIndex;// Index of the subitem in the list control		
	CString m_strWindowText;// Previous selected string value in the combo control
	
	//{{AFX_MSG(CListCellCb)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CListHeader window

class CListHeader : public CHeaderCtrl
{
	// Construction
public:
	CListHeader();
	
	// Attributes
public:
	
	// Operations
public:
	
protected:
	int m_nUpBit;
	int m_nDownBit;
	CImageList m_ImgList;
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListHeader)
	//}}AFX_VIRTUAL
	
	// Implementation
public:
	void SetSortArrow( const int iSortColumn, const BOOL bSortAscending );
	virtual ~CListHeader();
	
	// Generated message map functions
protected:
	virtual BOOL OnChildNotify(UINT msg, WPARAM wp, LPARAM lp, LRESULT* pRes);	
	BOOL m_bSortAscending;
	int m_iSortColumn;	
	//{{AFX_MSG(CListHeader)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx window

class CListCtrlEx : public CListCtrl
{
	//������
	typedef struct tagITEMCOLOR
	{
		tagITEMCOLOR(){dwData=0;crBK=NULL;crText=NULL;bEdit=FALSE;}
		DWORD		dwData;	//���ڱ���CLuListVCtrl::SetItemData/CLuListVCtrl::GetItemData�����ݱ���LVITEM�ṹ�еĲ���lParam
		COLORREF	crBK;	//������ı�����ɫ
		COLORREF	crText;	//�������������ɫ
		BOOL		bEdit;	//�Ƿ���Ա༭
	} ITEMCOLOR,*LPITEMCOLOR;
	//������ʽ��Ϣ
	typedef struct tagSUBITEMINFO
	{
		tagSUBITEMINFO(){m_dwData=0;m_dwColumnType=0;m_parComboString=NULL;}
		DWORD			m_dwData;
		DWORD			m_dwColumnType;
		CStringArray*	m_parComboString;
	} SUBITEMINFO,*LPSUBITEMINFO;
	
// Construction
public:
	CListCtrlEx();
	virtual ~CListCtrlEx();
		
// Operations
public:
		
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlEx)
	protected:
	//}}AFX_VIRTUAL
		
#ifdef _CCS_DES
	virtual void PreSubclassWindow();		
#endif

public:

#ifdef _CCS_RUN
	BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );	
#endif
	
	
// Implementation
public:
	//������Ҫ��ӡ��ҳ��
	int GetPrintPages(CDC* pDC);
	//��ӡ�б�����,��ͼ������(strTitle:�������,strHeader:ҳü)	
	void PrintReport(CDC* pDC,CPrintInfo* pInfo,CString strTitle,CString strHeader=_T(""));
	//��ӡ�б�����,�Ի�������(strTitle:�������,strHeader:ҳü)
	void PrintReport(CString strTitle,CString strHeader=_T(""));
	//������(nStyle:����ʽ;pStringArray:�ַ�������ָ��;��������ͬMFC)
	int InsertColumn( int nCol, const LVCOLUMN* pColumn, DWORD nStyle = CS_NONE,CStringArray * pStringArray = NULL );
	//������(nStyle:����ʽ;pStringArray:�ַ�������ָ��;��������ͬMFC)	
	int InsertColumn( int nCol, LPCTSTR lpszColumnHeading, int nFormat = LVCFMT_LEFT, int nWidth = -1, DWORD nStyle = CS_NONE ,CStringArray * pStringArray = NULL, int nSubItem = -1 );
	//ɾ����(nCol:������)
	BOOL DeleteColumn( int nCol );
	//��ʾ����ؼ�(nItem:������;nSubItem:����)
	void ShowSubCtrl(int nItem, int nSubItem);
	//���ָ�����Ƿ���Ա༭(nItem:������,�����Ƿ��ܱ��༭)
	BOOL CanEdit(int nItem);
	//ʹָ����ɷ��Ա༭(nItem:������;bEdit:�Ƿ��ܱ��༭�����Ҫʹ��һ��༭������ô˺���)
	void SetEdit(int nItem,BOOL bEdit = TRUE);
	//��ȡָ�����������ɫ(nItem:������)
	COLORREF GetItemTextColor(int nItem) const;
	//����ָ�����������ɫ(nItem:������;crTextColor:������ɫ)
	BOOL SetItemTextColor(int nItem,COLORREF crTextColor);
	//��ȡָ����ı�����ɫ(nItem:������)
	COLORREF GetItemBKColor(int nItem) const;
	//����ָ����ı�����ɫ(nItem:������;crBKColor:������ɫ)
	BOOL SetItemBKColor(int nItem,COLORREF crBKColor);
	//��ȡ�������(nItem:������)
	DWORD GetItemData( int nItem ) const;
	//�����������(nItem:������,dwData:������)
	BOOL SetItemData( int nItem, DWORD dwData );
	//�������ı�(nItem:������;nSubItem:����;lpszText:�ı�)
	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText );
	//�������ı�(nItem:������;lpszSubItem:��������;lpszText:�ı�)	
	BOOL SetItemText( int nItem, LPCTSTR lpszSubItem, LPCTSTR lpszText );
	//��ȡ���ı�(nItem:������;nSubItem:����;lpszText:�ı�;nLen:�ı�����)
	int GetItemText( int nItem, int nSubItem, LPTSTR lpszText, int nLen );
	//��ȡ���ı�(nItem:������;lpszSubItem:����������;lpszText:�ı�;nLen:�ı�����)	
	int GetItemText( int nItem, LPCTSTR lpszSubItem, LPTSTR lpszText, int nLen );
	//��ȡ���ı�(nItem:������;nSubItem:����;��������ı�)
	CString GetItemText( int nItem, int nSubItem );
	//��ȡ���ı�(nItem:������;lpszSubItem:����������;��������ı�)	
	CString GetItemText( int nItem, LPCTSTR lpszSubItem );
	//�����еĿ��(nCol:������;cx:���)
	BOOL SetColumnWidth( int nCol, int cx );
	//�����еĿ��(lpszCol:����������;cx:���)	
	BOOL SetColumnWidth( LPCTSTR lpszCol, int cx );
	//��ȡ�еĿ��(nCol:������;���ؿ��)
	int GetColumnWidth( int nCol );
	//��ȡ�еĿ��(lpszCol:����������;���ؿ��)
	int GetColumnWidth( LPCTSTR lpszCol );	

	void DisableSort();

	// Generated message map functions
protected:
	void PrintPage(int nPage,CDC* pDC,CRect rcPrint, CString strTitle, CString strHeader=_T(""));	
	BOOL m_bSortAscending;
	CArray<LPSUBITEMINFO,LPSUBITEMINFO> m_arColumnInfo;	
	int m_iSortColumn;//��ѡ����
	CListHeader m_ctlHeader;
	BOOL m_bDisableSort;
	
	int HitTestEx (CPoint& Point, int& nSubItem);	
	static bool IsNumber( LPCTSTR pszText );
	static int NumberCompare( LPCTSTR pszNumber1, LPCTSTR pszNumber2 );
	static bool IsDate( LPCTSTR pszText );
	static int DateCompare( const CString& strDate1, const CString& strDate2 );
	static int CALLBACK CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData );
	
	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);		
	afx_msg void OnInsertitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLEX_H__INCLUDED_)
