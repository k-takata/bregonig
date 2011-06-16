#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bregexp.h"
//#include "bregpool.h"

#ifdef _K2REGEXP_
int __stdcall callback(int kind, int value, int index)
{
	printf("value %d, index %d\n", value, index);
	return 1;
}
#endif


int main()
{
	char msg[80];	// ���b�Z�[�W�̈�
	BREGEXP *rxp = NULL;	// �K���N���A���Ă������� 
	// �u�������T���v��
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "s/(\\d\\d)-\\d{4}-\\d{4}/$1-xxxx-xxxx/g";
	int ctr;
#ifdef _K2REGEXP_
	if (ctr = BSubst(patern1,t1,t1,t1+strlen(t1),&rxp,msg, callback)) {
#else
	if (ctr = BSubst(patern1,t1,t1+strlen(t1),&rxp,msg)) {
#endif
		printf("after(%d)=%s\n",ctr,rxp->outp);	// �u�������p�^�[�����ƕ�����
		printf("length=%d\n",rxp->outendp - rxp->outp);	// �u����̕�����
	}

	if (rxp)			// �R���p�C���u���b�N�̊J��
		BRegfree(rxp);		// �Y��Ȃ��悤��

}
