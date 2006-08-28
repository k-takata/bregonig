#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	char msg[80];	// メッセージ領域
	BREGEXP *rxp = NULL;	// 必ずクリアしておくこと 
	// 変換文字サンプル
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "tr/A-Z0-9/a-zx/g";
	int ctr;
	if (ctr = BTrans(patern1,t1,t1+strlen(t1),&rxp,msg)) {
		printf("after(%d)=%s\n",ctr,rxp->outp);	// 変換した文字数と文字列
		printf("length=%d\n",rxp->outendp - rxp->outp);	// 変換後の文字数
	}

	if (rxp)				// コンパイルブロックの開放
		BRegfree(rxp);		// 忘れないように

}
