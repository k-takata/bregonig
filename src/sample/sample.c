//
//  sample.c
//
// grep for bregexp version
//                     Author Tatsuo Baba
//
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include <bregexp.h>
int _tmain(int argc,TCHAR *argv[])
{
	TCHAR fname[512],line[4096],*p1;
	TCHAR msg[BREGEXP_MAX_ERROR_MESSAGE_LEN];
	FILE *fp;
	int len,ctr;
	BREGEXP *rxp = 0;
	TCHAR dmy[] = _T(" ");
	setlocale(LC_ALL, "");
	if (argc < 2) {
		_putts (_T("usage /regstr/ [file]\n  if omitted assume /usr/dict/words"));
		return 0;
	}
	_tcscpy(fname,_T("/usr/dict/words"));
	if (argc > 2)
		_tcscpy(fname,argv[2]);
	p1 = argv[1];
	fp = _tfopen(fname,_T("r"));
	if (!fp) {
		_tprintf (_T("file cant open  %s\n"),fname);
		return 0;
	}
	BMatch(p1,dmy,dmy+1,&rxp,msg);	// compile using dummy 
	if (msg[0]) {
		_tprintf (_T("parse error  %s\n"),msg);
		return 0;
	}
	ctr = 0;
	while(_fgetts(line,sizeof(line),fp)) {
		len = _tcslen(line);
		if (len && (BMatch(p1,line,line+len,&rxp,msg) > 0)) {
			ctr++;
			line[len-1] = 0;
			_putts(line);
		}
	}
	fclose(fp);

	_tprintf(_T("%ld lines(s) greped\n"),ctr);
	return 0;
}
