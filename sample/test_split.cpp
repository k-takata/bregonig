#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "bregexp.h"
#include "bregpool.h"


int main()
{
	static BregPool bpool(8);
	TCHAR msg[80];
	TCHAR t1[] = _T(" Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ");
	TCHAR patern1[] = _T("/ *\\d{2,3}-\\d{3,4}-\\d{4} */");
	BREGEXP *rxp = bpool.Get(patern1);
	int splitcnt = BSplit(patern1,t1,t1+_tcslen(t1),0,&rxp,msg);
	if (splitcnt > 0 ) {
		int i = 0;
		for (int j = 0;j < splitcnt;j++) {
			int len = rxp->splitp[i+1] - rxp->splitp[i];
			TCHAR *tp = (TCHAR*)rxp->splitp[i];
			TCHAR ch = tp[len]; // save delmitter
			tp[len] = 0;	// set stopper
			_tprintf(_T("len=%d [%d]=%s\n"),len,j,tp);
			tp[len] = ch;	// restore the char
			i += 2;
		}
	}
}
