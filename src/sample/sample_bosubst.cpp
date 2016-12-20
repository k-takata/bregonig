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
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];	// メッセージ領域
	BREGEXP *rxp = NULL;	// 必ずクリアしておくこと 
	// 置換文字サンプル
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	int ctr;
	if ((ctr = BoSubst("(\\d\\d)-\\d{4}-\\d{4}", "$1-xxxx-xxxx", "g",
			t1,t1,t1+strlen(t1),NULL,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// 置換したパターン数と文字列
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// 置換後の文字数
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// 置換したパターン数
			_tprintf(_T("length=0\n"));		// 結果は空文字列
		}
	}

	// 同じ patternp と substp を指定した場合、再利用される
	if ((ctr = BoSubst("(\\d\\d)-\\d{4}-\\d{4}", "$1-xxxx-xxxx", "g",
			t1,t1,t1+strlen(t1),NULL,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// 置換したパターン数と文字列
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// 置換後の文字数
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// 置換したパターン数
			_tprintf(_T("length=0\n"));		// 結果は空文字列
		}
	}

	// patternp を再利用、新しい substp を指定
	// callback を指定
	if ((ctr = BoSubst(NULL, "$1-yyyy-zzzz", NULL,
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// 置換したパターン数と文字列
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// 置換後の文字数
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// 置換したパターン数
			_tprintf(_T("length=0\n"));		// 結果は空文字列
		}
	}

	// patternp と substp を再利用
	// callback を指定
	if ((ctr = BoSubst(NULL, NULL, NULL,
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// 置換したパターン数と文字列
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// 置換後の文字数
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// 置換したパターン数
			_tprintf(_T("length=0\n"));		// 結果は空文字列
		}
	}

	// 新しい patternp と、同じ substp を指定した場合、再利用されない
	// callback を指定
	if ((ctr = BoSubst("(\\d{3})-\\d{3}-\\d{4}", "$1-yyyy-zzzz", "g",
			t1,t1,t1+strlen(t1),callback,&rxp,msg)) > 0) {
		if (rxp->outp != NULL) {
			_tprintf(_T("after(%d)=%s\n"),ctr,rxp->outp);	// 置換したパターン数と文字列
			_tprintf(_T("length=%d\n"),rxp->outendp - rxp->outp);	// 置換後の文字数
		} else {
			_tprintf(_T("after(%d)\n"),ctr);	// 置換したパターン数
			_tprintf(_T("length=0\n"));		// 結果は空文字列
		}
	}

	if (rxp)			// コンパイルブロックの開放
		BRegfree(rxp);		// 忘れないように

}
