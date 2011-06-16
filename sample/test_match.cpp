#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	char msg[80];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// ���������T���v��
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "/(03|045)-(\\d{3,4})-(\\d{4})/";	// /(03|045)-(\d{3,4})-(\d{4})/
						// 03��045 �̓d�b�ԍ�������
						// �J�b�R() �́A���ꂼ��̔ԍ����L�����邱�Ƃ��Ӗ�����
	int pos = 0;	// �����|�W�V����
	while (BMatch(patern1,t1+pos,t1+strlen(t1),&rxp,msg)) {
		printf("data=%s\n",t1+pos);		// ��������镶��
		printf("found=%s\n",rxp->startp[0]);	// �}�b�`������
		printf("length=%d\n",rxp->endp[0] - rxp->startp[0]);	// �}�b�`������
		for (int i = 1;i <= rxp->nparens;i++) {		// �J�b�R���̃f�[�^ 
			printf("$%d = %s\n",i,rxp->startp[i]);
			printf("$%d length = %d\n",i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t1;		// ���̕����̌����ʒu
	}

	pos = 0;
	char t2[] = " abcdabce abcdabcd abcdabcf abcgabcg ";
	char patern2[] = "/abc(.)abc\\1/";	// �������ł̃p�^�[���L���̗�
	while(BMatch(patern2,t2+pos,t2+strlen(t2),&rxp,msg)) {
		printf("data=%s\n",t2);			// ��������镶��
		printf("found=%s\n",rxp->startp[0]);	// �}�b�`������
		printf("length=%d\n",rxp->endp[0] - rxp->startp[0]);	// �}�b�`������
		for (int i = 1;i <= rxp->nparens;i++) {		// �J�b�R���̃f�[�^ 
			printf("$%d = %s\n",i,rxp->startp[i]);
			printf("$%d length = %d\n",i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t2;	// ���̕����̌����ʒu
	}

	if (rxp)			// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
