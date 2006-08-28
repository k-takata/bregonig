//
//  sample.c
//
// grep for bregexp version
//                     Author Tatsuo Baba
//
#include <stdio.h>
#include <bregexp.h>
int main(int argc,char *argv[])
{
	char fname[512],line[4096];
	char msg[80],*p1;	
    FILE *fp;
	int len,ctr;
	BREGEXP *rxp = 0;
	char dmy[] = " ";
    if (argc < 2) {
		puts ("usage /regstr/ [file]\n  if omitted assume /usr/dict/words");
		return 0;
	}
	strcpy(fname,"/usr/dict/words");
	if (argc > 2)
		strcpy(fname,argv[2]);
	p1 = argv[1];
    fp = fopen(fname,"r");
    if (!fp) {
		printf ("file cant open  %s\n",fname);
		return 0;
	}
   	BMatch(p1,dmy,dmy+1,&rxp,msg);	// compile using dummy 
    if (msg[0]) {
		printf ("parse error  %s\n",msg);
		return 0;
	}
    ctr = 0;
    while(fgets(line,sizeof(line),fp)) {
		len = strlen(line);
   		if (len && BMatch(p1,line,line+len,&rxp,msg)) {
            ctr++;
            line[len-1] = 0;
			puts(line);
		}
	}
	fclose(fp); 

	printf("%ld lines(s) greped\n",ctr);
	return 0;
}
