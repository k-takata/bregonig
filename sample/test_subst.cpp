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
	char msg[80];	// メッセージ領域
	BREGEXP *rxp = NULL;	// 必ずクリアしておくこと 
	// 置換文字サンプル
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "s/(\\d\\d)-\\d{4}-\\d{4}/$1-xxxx-xxxx/g";
	int ctr;
#ifdef _K2REGEXP_
	if (ctr = BSubst(patern1,t1,t1,t1+strlen(t1),&rxp,msg, callback)) {
#else
	if (ctr = BSubst(patern1,t1,t1+strlen(t1),&rxp,msg)) {
#endif
		printf("after(%d)=%s\n",ctr,rxp->outp);	// 置換したパターン数と文字列
		printf("length=%d\n",rxp->outendp - rxp->outp);	// 置換後の文字数
	}

	if (rxp)			// コンパイルブロックの開放
		BRegfree(rxp);		// 忘れないように

}
