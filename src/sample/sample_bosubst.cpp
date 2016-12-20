#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <tchar.h>
#include "bregexp.h"

int __stdcall callback(int kind, int value, ptrdiff_t index)
{
    _tprintf("callback: %d, %d, %d\n", kind, value, (int) index);
	return 1;
}


int main()
{
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// �u�������T���v��
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	int ctr;
	if ((ctr = BoSubst("(\\d\\d)-\\d{4}-\\d{4}", "$1-xxxx-xxxx", "g",
			t1,t1,t1+strlen(t1),NULL,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �u����̕�����
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// �u�������p�^�[����
			_tprintf(_T("length=0\n"));		// ���ʂ͋󕶎���
		}
	}

	// ���� patternp �� substp ���w�肵���ꍇ�A�ė��p�����
	if ((ctr = BoSubst("(\\d\\d)-\\d{4}-\\d{4}", "$1-xxxx-xxxx", "g",
			t1,t1,t1+strlen(t1),NULL,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �u����̕�����
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// �u�������p�^�[����
			_tprintf(_T("length=0\n"));		// ���ʂ͋󕶎���
		}
	}

	// patternp ���ė��p�A�V���� substp ���w��
	// callback ���w��
	if ((ctr = BoSubst(NULL, "$1-yyyy-zzzz", NULL,
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �u����̕�����
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// �u�������p�^�[����
			_tprintf(_T("length=0\n"));		// ���ʂ͋󕶎���
		}
	}

	// patternp �� substp ���ė��p
	// callback ���w��
	if ((ctr = BoSubst(NULL, NULL, NULL,
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// �u����̕�����
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// �u�������p�^�[����
			_tprintf(_T("length=0\n"));		// ���ʂ͋󕶎���
		}
	}

	// �V���� patternp �ƁA���� substp ���w�肵���ꍇ�A�ė��p����Ȃ�
	// callback ���w��
	if ((ctr = BoSubst("(\\d{3})-\\d{3}-\\d{4}", "$1-yyyy-zzzz", "g",
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
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
