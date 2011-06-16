#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	TCHAR msg[80];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// �ϊ������T���v��
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	TCHAR patern1[] = _T("tr/A-Z0-9/a-zx/g");
	int ctr;
	if ((ctr = BTrans(patern1,t1,t1+_tcslen(t1),&rxp,msg)) > 0) {
		_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �ϊ������������ƕ�����
		_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �ϊ���̕�����
	}

	if (rxp)				// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
