#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "bregexp.h"
//#include "bregpool.h"

#ifdef _K2REGEXP_
int __stdcall callback(int kind, int value, int index)
{
	_tprintf("value %d, index %d\n", value, index);
	return 1;
}
#endif


int main()
{
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// �u�������T���v��
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	TCHAR patern1[] = _T("s/(\\d\\d)-\\d{4}-\\d{4}/$1-xxxx-xxxx/g");
	int ctr;
#ifdef _K2REGEXP_
	if ((ctr = BSubst(patern1,t1,t1,t1+strlen(t1),&rxp,msg, callback)) > 0) {
#else
	if ((ctr = BSubst(patern1,t1,t1+_tcslen(t1),&rxp,msg)) > 0) {
#endif
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �u����̕�����
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// �u�������p�^�[����
			_tprintf(_T("length=0\n"));		// ���ʂ͋󕶎���
		}
	}

	if (rxp)			// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
