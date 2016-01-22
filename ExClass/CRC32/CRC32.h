
#if !defined(AFX_CRC_H__530315EE_21FB_4B60_AB75_B65D72708BE7__INCLUDED_)
#define AFX_CRC_H__530315EE_21FB_4B60_AB75_B65D72708BE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//��һ����ֵ��16����תΪ�ַ�����
//dec			Ϊ��ת����ֵ��
//StrBuf		Ϊ���շ����ַ�����������
//formatwidth	Ϊ�����(λ��)��ʽ������λ����ǰ��0
static char *decToHex(unsigned long dec, char *StrBuf, int formatwidth)
{
	char hexStr[17] = "0123456789ABCDEF";
	char Result[21] = {"0"};
	int i=0, j=0, k=0;
	do{
		int t = dec % 16;
		dec = dec / 16;
		Result[i] = hexStr[t];
		i++;
	}while(dec > 0);
	if(i < formatwidth)
		k += formatwidth - i;
	while(j<k)
		StrBuf[j++] = '0';
	while(i > 0)
	{
		StrBuf[k] = Result[i-1];
		k++;
		i--;
	}
	StrBuf[k] = 0;
	return StrBuf;
}

class CRC32
{
private:
	unsigned long CRCVAL;
	unsigned long Crc32Table[256];
	void MakeTable();//����CRC32�����

protected:

	char CRCString[20];

public:
	CRC32();
	int GetCrc32(unsigned char * csData, unsigned long dwSize);//��ȡcrc32ֵ
	char *GetCrc32Str(unsigned char * csData, unsigned long dwSize);

};


#endif // !defined(AFX_CRC_H__530315EE_21FB_4B60_AB75_B65D72708BE7__INCLUDED_)
