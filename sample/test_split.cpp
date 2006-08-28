#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bregexp.h"
#include "bregpool.h"


int main()
{
	static BregPool bpool(8);
	char msg[80];
	char t1[] = " Yokohama 045-222-1111  Osaka 06-5555-6666  Tokyo 03-1111-9999 ";
	char patern1[] = "/ *\\d{2,3}-\\d{3,4}-\\d{4} */";
	BREGEXP *rxp = bpool.Get(patern1);
	int splitcnt = BSplit(patern1,t1,t1+strlen(t1),0,&rxp,msg);
	if (splitcnt > 0 ) {
		int i = 0;
		for (int j = 0;j < splitcnt;j++) {
			int len = rxp->splitp[i+1] - rxp->splitp[i];
			char *tp = (char*)rxp->splitp[i];
			char ch = tp[len]; // save delmitter
			tp[len] = 0;	// set stopper
			printf("len=%d [%d]=%s\n",len,j,tp);
			tp[len] = ch;	// restore the char
			i += 2;
		}
	}
}
