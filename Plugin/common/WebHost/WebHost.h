/*########################################################################
	���ļ�����: 	WebHost.h
	����  �ơ�:		����web���ӵķ�װ��.
	----------------------------------------------------
	Remarks:	...
	----------------------------------------------------
	Author:		huawenNie
	Email:		nie173@vip.sina.com
				nie173@msn.com
	Created:	03/05/2004
  ########################################################################*/
#if !defined(AFX_WEBHOST_H__46FFF420_23C3_4356_A88D_AEBDA61EA186__INCLUDED_)
#define AFX_WEBHOST_H__46FFF420_23C3_4356_A88D_AEBDA61EA186__INCLUDED_
#include <vector>
using namespace std;
#include "URLSturct_.h"

class CWebHost
{
public:
   //���캯�� ����˵��
   //[ҳ�����],[����URL����]
   //[���ص�URL����],��ҳ��URL]
   //[��ҳ�ı���]
   CWebHost( const CString& m_str_webcode ,  vector<HyperLink>& m_vec_All_URL  ,HyperLink& str_URL);
   //�Ƿ��п�ܻ���ת
   BOOL mb_ifFream;
protected:

	//�������кϷ����� ����˵��
	//[ҳ�����],[����URL����],[���ص�URL����],[ץȡURL�Ƚ�ģ������],[����URL�Ƚ�ģ������]
    void OnRetrunWebContent(const  CString& m_str_webcode ,  vector<HyperLink>& m_vec_All_URL );
    //��ȡǰͷ��ָ����־������
	void OnGetWebPageURL(const CString& str_htmlcode, vector<HyperLink>& m_vec_All_URL, int pagesize, CString strFlagString, int nFlagStringLenth);
	//���Ƕ�״����URL
	void OnReturnFrameURL(const CString& str_htmlcode ,vector<HyperLink>& m_vec_All_URL) ;

	//��URLת���ɾ��Ե�ַ
	CString OnConversionURL(CString SeedURL,CString RelativeURL) ;
    
	//����Javascript���������Ҫ���URL ����˵��
    //[ҳ�����],[���ص�URL����]
	void OnAnalyseJavascrript(const CString& m_str_webcode ,  vector<HyperLink>& m_vec_All_URL  );
	
    //ȡURL��������
	CString OnGetLinkText(int iIn,const CString& htmlcode);
	
protected:
	 //��ҳ��URL
     CString str_Page_URL;
	 //��ҳ�ı���
	 CString str_pagetitle;
     HyperLink m_HyperLink;
	 //������ת
	 CString str_link;
};
#endif // !defined(AFX_WEBHOST_H__46FFF420_23C3_4356_A88D_AEBDA61EA186__INCLUDED_)