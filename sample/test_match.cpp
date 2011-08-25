#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// ���������T���v��
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	TCHAR patern1[] = _T("/(03|045)-(\\d{3,4})-(\\d{4})/");	// /(03|045)-(\d{3,4})-(\d{4})/
						// 03��045 �̓d�b�ԍ�������
						// �J�b�R() �́A���ꂼ��̔ԍ����L�����邱�Ƃ��Ӗ�����
	int pos = 0;	// �����|�W�V����
	while (BMatch(patern1,t1+pos,t1+_tcslen(t1),&rxp,msg) > 0) {
		_tprintf(_T("data=%s\n"),t1+pos);		// ��������镶��
		_tprintf(_T("found=%s\n"),rxp->startp[0]);	// �}�b�`������
		_tprintf(_T("length=%d\n"),rxp->endp[0] - rxp->startp[0]);	// �}�b�`������
		for (int i = 1;i <= rxp->nparens;i++) {		// �J�b�R���̃f�[�^ 
			_tprintf(_T("$%d = %s\n"),i,rxp->startp[i]);
			_tprintf(_T("$%d length = %d\n"),i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t1;		// ���̕����̌����ʒu
	}

	pos = 0;
	TCHAR t2[] = _T(" abcdabce abcdabcd abcdabcf abcgabcg ");
	TCHAR patern2[] = _T("/abc(.)abc\\1/");	// �������ł̃p�^�[���L���̗�
	while(BMatch(patern2,t2+pos,t2+_tcslen(t2),&rxp,msg) > 0) {
		_tprintf(_T("data=%s\n"),t2);			// ��������镶��
		_tprintf(_T("found=%s\n"),rxp->startp[0]);	// �}�b�`������
		_tprintf(_T("length=%d\n"),rxp->endp[0] - rxp->startp[0]);	// �}�b�`������
		for (int i = 1;i <= rxp->nparens;i++) {		// �J�b�R���̃f�[�^ 
			_tprintf(_T("$%d = %s\n"),i,rxp->startp[i]);
			_tprintf(_T("$%d length = %d\n"),i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t2;	// ���̕����̌����ʒu
	}

	if (rxp)			// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
