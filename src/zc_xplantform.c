#include "zc_xplatform.h"
#include <Windows.h>

#ifdef _MSC_VER

zlogfd zlogopen(char *f,int m,int p) {
	HANDLE h = CreateFile(f,
		FILE_APPEND_DATA,
		ZLOG_OPEN_FLAGS,
		NULL, 
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	return(h);
}

int zlogwrite(zlogfd fd,char *data,int len) {
	int numb = 0;
	int status;

	status = WriteFile(fd,data,len,&numb,NULL);
	if(status == 0) return(-1);
	return(numb);
}

int zlogclose(zlogfd fd) {
	if(fd <= 0) return(0);
	if(CloseHandle(fd)) return(0);
	return(-1);
}
#endif
