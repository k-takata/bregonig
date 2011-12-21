#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "bregexp.h"
//#include "bregpool.h"


int main()
{
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];	// メッセージ領域
	BREGEXP *rxp = NULL;	// 必ずクリアしておくこと 
	// 検索文字サンプル
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	TCHAR patern1[] = _T("/(03|045)-(\\d{3,4})-(\\d{4})/");	// /(03|045)-(\d{3,4})-(\d{4})/
						// 03か045 の電話番号を検索
						// カッコ() は、それぞれの番号を記憶することを意味する
	int pos = 0;	// 検索ポジション
	while (BMatch(patern1,t1+pos,t1+_tcslen(t1),&rxp,msg) > 0) {
		_tprintf(_T("data=%s\n"),t1+pos);		// 検索される文字
		_tprintf(_T("found=%s\n"),rxp->startp[0]);	// マッチ文字列
		_tprintf(_T("length=%d\n"),rxp->endp[0] - rxp->startp[0]);	// マッチ文字数
		for (int i = 1;i <= rxp->nparens;i++) {		// カッコ内のデータ 
			_tprintf(_T("$%d = %s\n"),i,rxp->startp[i]);
			_tprintf(_T("$%d length = %d\n"),i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t1;		// 次の文字の検索位置
	}

	pos = 0;
	TCHAR t2[] = _T(" abcdabce abcdabcd abcdabcf abcgabcg ");
	TCHAR patern2[] = _T("/abc(.)abc\\1/");	// 検索中でのパターン記憶の例
	while(BMatch(patern2,t2+pos,t2+_tcslen(t2),&rxp,msg) > 0) {
		_tprintf(_T("data=%s\n"),t2);			// 検索される文字
		_tprintf(_T("found=%s\n"),rxp->startp[0]);	// マッチ文字列
		_tprintf(_T("length=%d\n"),rxp->endp[0] - rxp->startp[0]);	// マッチ文字数
		for (int i = 1;i <= rxp->nparens;i++) {		// カッコ内のデータ 
			_tprintf(_T("$%d = %s\n"),i,rxp->startp[i]);
			_tprintf(_T("$%d length = %d\n"),i,rxp->endp[i]-rxp->startp[i]);
		}
		pos = rxp->endp[0] - t2;	// 次の文字の検索位置
	}

	if (rxp)			// コンパイルブロックの開放
		BRegfree(rxp);		// 忘れないように

}
