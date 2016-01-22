/*########################################################################
	���ļ�����: 	WebHost.cpp
	����  �ơ�:		����web���ӵķ�װ��.
	----------------------------------------------------
	Remarks:	...
	----------------------------------------------------
	Author:		huawenNie
	Email:		nie173@vip.sina.com
	MSN:		nie173@msn.com
	Created:	03/05/2004
  ########################################################################*/

#include "stdafx.h"
#include "WebHost.h"

#define FLAGSTRING "~!@#$%^&"
//���캯��
// str_URL:��Ҫ��ȡ��URL
// m_vec_All_URL:���ڷ��ػ�ȡ��������URL
// m_str_webcode:URL�������Ĵ���
CWebHost::CWebHost(const CString& m_str_webcode, vector<HyperLink>& m_vec_All_URL, HyperLink& str_URL)
{
   str_Page_URL = str_URL.str_Hyperlink;
   str_pagetitle = str_URL.str_HyperlinkText;
   mb_ifFream = FALSE;
   OnRetrunWebContent(m_str_webcode, m_vec_All_URL);
}

//=============================================================
//
// �������кϷ�����
//
//=============================================================
void CWebHost::OnRetrunWebContent(const CString& str_htmlcode, vector<HyperLink>& m_vec_All_URL)
{
	// ����JAVASCRIPT����
	OnAnalyseJavascrript(str_htmlcode, m_vec_All_URL);
	//����FrameǶ�״����URL�����ӣ�http://bq.caihong.com/,����á�src=���Ϳ��Ի�ȡ���ˣ������������ȥ������
	//OnReturnFrameURL(str_htmlcode, m_vec_All_URL);
/*
//����URL��
��javascript��
http://www.qidian.com/
�� href=��
http://jgfw.mohurd.gov.cn/
http://china-images.ubuntu.com/-----Ŀ¼����
�� HREF=��
http://www.atrain.cn/
�� url=��
http://www.mohurd.gov.cn/wsbsdtnew/
�� URL=��

�� src=��
http://jjz.mohurd.gov.cn/
�� SRC=��
http://www.abchina.com/cn/
�� action=��
http://jjz.mohurd.gov.cn/#
--------------------------------------------
������ҳ��
http://jjz.mohurd.gov.cn/images/infoview/com.trs.editor/local/postArticle.htm
var href='';

http://www.5173.com/

http://www.mtime.com/community/



*/

// 	if (str_htmlcode.Find(" action=\"", 0) != -1)
// 	{
// 		//::MessageBox(NULL, str_Page_URL, "��ʾ", MB_ICONINFORMATION);
// 		ShellExecute(0, "open", str_Page_URL, NULL, NULL, SW_SHOW);
// 	}
	int pagesize = str_htmlcode.GetLength();

	//��ȡ��href=��ָ������ӣ���ȡ��ת��url��
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "href=", 5);
	//��ȡ��HREF=��ָ������ӣ���ȡ��ת��url��
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "HREF=", 5);

	//��ȡ��url=��ָ������ӣ���ȡ��ת��url��
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "url=", 4);
	//��ȡ��URL=��ָ������ӣ���ȡ��ת��url��
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "URL=", 4);

    //��ȡ��src=��ָ�������
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "src=", 4);
    //��ȡ��SRC=��ָ�������
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "SRC=", 4);

    //��ȡ��action=��ָ�������
	OnGetWebPageURL(str_htmlcode, m_vec_All_URL, pagesize, "action=", 7);
}
//////////////////////////////////////////////////////////////////////////
//����Javascript���������Ҫ���URL ����˵��
//[ҳ�����],[����URL����],[���ص�URL����]
void CWebHost::OnAnalyseJavascrript(const CString& str_htmlcode ,  vector<HyperLink>& m_vec_All_URL  )
{
	CStringArray str_javascript;
	int pos1 = 0;
	int pos2 = 0;
	//int iLinkTextpos=0;
	//��ȡҳ������javascipt����
	int isafety=0;
	while( pos1 != -1&&isafety<100 )
	{
		pos1 = str_htmlcode.Find( "<script" , pos1);
		if( pos1!=-1 )
		{
			pos2 = str_htmlcode.Find( "</script>" , pos1);
			pos2 +=9;
			str_javascript.Add( str_htmlcode.Mid(pos1, pos2-pos1));
			pos1 = pos2-1;
		}
		isafety++;
	}

	//����ҳ��Javascrript�����URL
	CString str_link;
	for(int a = 0 ; a < str_javascript.GetSize() ; a++)
	{
		
		pos1=0;
		pos2=0;
		for(int b=0;pos1!=-1&&b<str_javascript[a].GetLength();b++)
		{
			pos1 = str_javascript[a].Find( "('" , pos1 );
			if( pos1!=-1 )
			{
				pos1 +=1 ;
				pos2 = str_javascript[a].Find("')",pos1);
				pos2 -=pos1;
				
				str_link = str_javascript[a].Mid(pos1,pos2);
				
				if(str_link.Find(".htm",0)!=-1||str_link.Find(".html",0)!=-1||
					str_link.Find(".php",0)!=-1||str_link.Find(".asp",0)!=-1||
					str_link.Find(".xml",0)!=-1||str_link.Find("http://",0)!=-1)
				{
					str_link.Replace(" ","");
					str_link.Replace("'","");
					str_link.Replace("\""," ");
					str_link.Replace("&amp;", "&");
					str_link.TrimLeft(" ");
					//,���Ŵ���
					if(str_link.Find(",",0)!=-1)
					{
						str_link=str_link.Left(str_link.Find(",",0));
					}
					//=�Ŵ���
					if(str_link.Find("=",0)!=-1)
					{
						str_link=str_link.Right(str_link.Find("=",0));
					}
					//>����
					if(str_link.Find(">",0)!=-1)
					{
						str_link=str_link.Left(str_link.Find(">",0));
					}

					//-----------------------------------------------------------------
					if(str_link.Find("http://",0) < -1)//URL�������Ե�ַ�Ļ�
					{ 
						str_link = OnConversionURL (str_Page_URL, str_link);
					}	
					
					if(str_link.GetLength() > 5)
					{
						m_HyperLink.str_Hyperlink=str_link;
						m_HyperLink.str_HyperlinkText.Empty();
						m_vec_All_URL.push_back( m_HyperLink );
					}
				}
			}
		}
	}
}
//====================================================
//
// ../��ʾ����һ��
// /��ʾ��Ŀ¼�µ�
// XX.htm��ʾ��ǰĿ¼�µ�
// ��URLת���ɾ��Ե�ַ
CString CWebHost::OnConversionURL(CString sURL,CString str_fafURL)
{
	if(sURL.Find("/",8)<0)
	{
		sURL +="/";
	}
	str_fafURL.Replace("'","");
	CString str_activeURL;
	int int_j = 0;
	int i=0;
	str_activeURL = str_fafURL;
	if(str_fafURL.Find("../",0)!=-1&&str_fafURL[0]!='/')
	{
		while( i<=str_fafURL.GetLength() )
		{
			if( str_fafURL[i] == '.' && str_fafURL[i+1] == '.' && str_fafURL[i+2] == '/' )
			{ int_j++;}
			i++;
		}
		if(str_fafURL[0]=='/')
		{
			str_fafURL.Delete(0,1);
		}
		str_fafURL.Replace("../","");
		i=0;
		int int_i=0;
		while( i <= sURL.GetLength() )
		{
			if( sURL[i]=='/' )
			{ 
				int_i++;
			}
			i++;
		}
		int_i -= int_j;

		if( int_i<3 )
		{
			int_i = 3;
		}

		int int_cour=0;
		for( i=0; i<=sURL.GetLength(); i++)
		{
			if( sURL[i]=='/' )
			{ 
				int_cour++;
			}
			if( int_cour==int_i )
			{			 
				sURL= sURL.Left(i+1);
				break;
			}
		}
		//�ݴ���
		if( sURL[sURL.GetLength()-1]!='/' )
		{	
			sURL +="/";
		}
		sURL += str_fafURL;
		//   sURL += "[ת��] 1";
		return sURL;
	}
	else
	{
		if( str_fafURL[0] =='/' )
		{
			int int_b = 0 ;
			for( int a=0; int_b<3 && a<sURL.GetLength(); a++)
			{
				if( sURL[a]=='/' )
			 {
				 int_b++;
			 }
			 if( int_b==3 )
			 {
				 sURL = sURL.Left(a);
				 break;
			 }
			}
			sURL += str_fafURL;
			//sURL += "[ת��] 2";
		}
		else
		{
			for( int i=sURL.GetLength() - 1; i> 0 ; i-- )
			{
				if( sURL[i] =='/' )
				{
					sURL = sURL.Left( i+1 );
					break;
				}
			}
			sURL += str_fafURL;
			// sURL += "[ת�� 3]";
		}
		//	 sURL += "[ת��]";
		return sURL;
	}
}

void CWebHost::OnGetWebPageURL(const CString& str_htmlcode, vector<HyperLink>& m_vec_All_URL, int pagesize, CString strFlagString, int nFlagStringLenth)
{
	//������ת
	int tag0 = 0;
	int tag1 = 0;
	int tag3 = 0;
	int tag4 = 0;
	int i;
	int iLinkTextpos = 0;
	for( i = 0; i < pagesize && tag0 != -1; i++)
	{
		tag0 = str_htmlcode.Find(strFlagString, tag0);
		if(tag0 != -1)
		{
			tag0 += nFlagStringLenth;
		}
		tag1=str_htmlcode.Find(">", tag0);
		iLinkTextpos = tag1;
		tag1 -= tag0;
		str_link = str_htmlcode.Mid(tag0, tag1);
		str_link.Replace("'", "\"");
		str_link.Replace("\";", " ");
		str_link.Replace("\"\"", FLAGSTRING);
		str_link.Replace("\n"," ");
		str_link.Replace("\"", " ");
		str_link.TrimLeft(" ");
		str_link.Replace("&amp;", "&");
		str_link.Replace("/./", "/");

		if(!str_link.IsEmpty())
		{
			tag3 = str_link.Find(" ", 0);
			if(tag3 != -1)
				str_link = str_link.Left(tag3);

			if (str_link.Find(FLAGSTRING, 0) != -1)
				continue;

			CString strTemp;
			strTemp = str_link;
			strTemp.MakeLower();
			if (strTemp.Find("javascript:", 0) != -1)
				continue;

			if(str_link.Left(7) != "http://" && str_link.Left(8) != "https://")//URL�������Ե�ַ�Ļ�
			{ 
				str_link=OnConversionURL(str_Page_URL, str_link);
			}
			if(/*!tag4 && */str_link.GetLength() > 5)//URLδ������
			{
				mb_ifFream=TRUE;
				m_HyperLink.str_Hyperlink=str_link;
				m_HyperLink.str_HyperlinkText=OnGetLinkText(iLinkTextpos+1,str_htmlcode);
				m_vec_All_URL.push_back(m_HyperLink);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

//ȡURL��������
CString CWebHost::OnGetLinkText(int iIn,const CString& htmlcode)
{
	CString str_return;
	int ipos1=0;
	int ipos2=0;
	int iSafety=0;
	ipos1=htmlcode.Find("</a>",iIn);
    ipos2 = htmlcode.Find("<a>",iIn);
    if(ipos2>0)
     {
       if(ipos2<ipos1)
          ipos1 = ipos2;
     }
	if(ipos1>-1)
	{
        str_return =htmlcode.Mid(iIn,ipos1-iIn);
		if(str_return.Find("href=",0)<0)
		{
			while(ipos1>-1&&iSafety<500)
			{
				iSafety++;
				//
				ipos1 = str_return.Find("<",0);
				ipos2 = str_return.Find(">",ipos1);
				if(ipos1>-1&&ipos2>-1)
				{
					str_return.Delete(ipos1,ipos2-ipos1+1);
				}
				else
				{
                    ipos1 =-1;
				}
			}
		}
		else
		{
			str_return.Empty();
		}
	}
    
	if(str_return.GetLength()==1||str_return.GetLength()==2)
	{
		str_return =str_pagetitle;
	}
	return str_return;
}
//����Ƕ�״����URL
void CWebHost::OnReturnFrameURL(const CString& str_htmlcode ,vector<HyperLink>& m_vec_All_URL  )
{
   CStringArray str_javascript;
   int pos1 = 0;
   int pos2 = 0;
   str_javascript.RemoveAll();
   //��ȡҳ������ frame ����
    CString str_link;
   while( pos1 != -1 )
   {
    pos1 = str_htmlcode.Find( "<frame" , pos1 );
    if( pos1!=-1 )
    {
	pos2 = str_htmlcode.Find( "</frameset>" , pos1);
    pos1 += 6;
    pos2 -= pos1;
	str_javascript.Add( str_htmlcode.Mid( pos1 , pos2 ) );
    }
   }
   for(int a = 0 ; a < str_javascript.GetSize() ; a++)
   {
	   
       //AfxMessageBox( str_javascript[a] );
	   pos1=0;
	   pos2=0;
	   for(int b=0;pos1!=-1&&b<str_javascript[a].GetLength();b++)
	   {
		   
           pos1 =str_javascript[a].Find( "src" , pos1 );
		   if( pos1!=-1 )
		   {
			   pos1 = str_javascript[a].Find( "=" , pos1 );
			   pos2 = str_javascript[a].Find( ">" , pos1 );
			   pos1 += 1;
			   pos2 -= pos1;
               str_link = str_javascript[a].Mid(pos1,pos2);
			  
               if(str_link.Find("http://",0)<0 
				   && str_link.Find(">",0)<0 && str_link.Find("<",0)<0 )//URL�������Ե�ַ�Ļ�
				 { 
				   
			      str_link = OnConversionURL ( str_Page_URL , str_link);
				 }		  
			    str_link.Replace( "\'" , "");
				
				if( str_link.GetLength() > 5)//URLδ������
				{
					mb_ifFream =TRUE;		  
				m_HyperLink.str_Hyperlink=str_link;
                m_HyperLink.str_HyperlinkText.Empty();
			    m_vec_All_URL.push_back(m_HyperLink);
				}
		   }
	   }
   }
}

//====================================================
//URLģ��,��4��ͨ���,$ ��ʾ һ������,^ ��ʾһ������,# ��ʾһ���ַ�,!��ʾһ���ַ� �ı�
//�ж�URL�Ƿ���
// http://news.sina.com.cn
//====================================================


