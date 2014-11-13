#include "FreeRTOS.h"
#include "semphr.h"
#include "hellofilesystem.h"

static xSemaphoreHandle _mutex = 0;

#define LOCK() \
	if (!_mutex) _mutex = xSemaphoreCreateMutex();\
	xSemaphoreTake(_mutex,portMAX_DELAY);\

#define UNLOCK()\
	xSemaphoreGive(_mutex);

FRESULT hello_mount (BYTE drv,FATFS *fs) {

	FRESULT res;
	LOCK();
	res = f_mount(drv,fs);
	UNLOCK();
	return res;
}
FRESULT hello_open ( FIL *fp, const char *path, BYTE mode) {
	FRESULT res;
	LOCK();
	res = f_open(fp,path,mode);
	UNLOCK();
	return res;
}
FRESULT hello_read (FIL *fp, void *buff,WORD btr,WORD *br  ) {
	FRESULT res;
	LOCK();
	res = f_read(fp,buff,btr,br);
	UNLOCK();
	return res;
}
FRESULT hello_write (  FIL *fp, const void *buff,WORD btw,WORD *bw) {
	FRESULT res;
	LOCK();
	res = f_write(fp,buff,btw,bw);
	UNLOCK();
	return res;
}

FRESULT hello_lseek(FIL *fp,DWORD ofs) {
	FRESULT res;
	LOCK();
	res = f_lseek(fp,ofs);
	UNLOCK();
	return res;
}
FRESULT hello_close (FIL* fp) {
	FRESULT res;
	LOCK();
	res = f_close(fp);
	UNLOCK();
	return res;
}
FRESULT hello_opendir (DIR* dirobj, const char* path) {
	FRESULT res;
	LOCK();
	res = f_opendir(dirobj,path);
	UNLOCK();
	return res;
}
FRESULT hello_readdir (DIR * dirobj, FILINFO * finfo) {
	FRESULT res;
	LOCK();
	res = f_readdir(dirobj,finfo);
	UNLOCK();
	return res;
}
FRESULT hello_stat (const char* path, FILINFO * finfo) {
	FRESULT res;
	LOCK();
	res = f_stat(path,finfo);
	UNLOCK();
	return res;
}
FRESULT hello_getfree (const char * drv, DWORD * nclust, FATFS ** fatfs) {
	FRESULT res;
	LOCK();
	res = f_getfree(drv,nclust,fatfs);
	UNLOCK();
	return res;
}



FRESULT hello_sync (FIL*finfo) {
	FRESULT res;
	LOCK();
	res = f_sync(finfo);
	UNLOCK();
	return res;
}
FRESULT hello_unlink (const char*path) {
	FRESULT res;
	LOCK();
	res = f_unlink(path);
	UNLOCK();
	return res;
}
FRESULT    hello_mkdir (const char*path) {
	FRESULT res;
	LOCK();
	res = f_mkdir(path);
	UNLOCK();
	return res;
}
FRESULT hello_chmod (const char*path, BYTE value, BYTE mask) {
	FRESULT res;
	LOCK();
	res = f_chmod(path,value,mask);
	UNLOCK();
	return res;
}
FRESULT hello_rename (const char* path_old, const char*path_new) {
	FRESULT res;
	LOCK();
	res = f_rename(path_old,path_new);
	UNLOCK();
	return res;
}
FRESULT hello_mkfs (BYTE drv, BYTE partition, BYTE allocsize) {
	FRESULT res;
	LOCK();
	res = f_mkfs(drv,partition,allocsize);
	UNLOCK();
	return res;
}
FRESULT hello_append(const char* file_name, const unsigned char* content, int length) {
	FRESULT res;
	LOCK();
	res = f_append(file_name,content,length);
	UNLOCK();
	return res;
}

