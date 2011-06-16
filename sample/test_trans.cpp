#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	char msg[80];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// �ϊ������T���v��
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "tr/A-Z0-9/a-zx/g";
	int ctr;
	if (ctr = BTrans(patern1,t1,t1+strlen(t1),&rxp,msg)) {
		printf("after(%d)=%s\n",ctr,rxp->outp);	// �ϊ������������ƕ�����
		printf("length=%d\n",rxp->outendp - rxp->outp);	// �ϊ���̕�����
	}

	if (rxp)				// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
