/*************************************************************************
    > File Name: jiami.c
    > Author: 
    > Mail: 
    > Created Time: 2019年01月03日 星期四 15时44分08秒
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "md5.c"
 
int main(int argc, char *argv[])
{
	int i;
    char name[15];
    FILE *fp=fopen("test.txt","r");
	unsigned char encrypt[30];
    fscanf(fp,"%s %s",name,encrypt);
	unsigned char decrypt[16];
	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,encrypt,strlen((char *)encrypt));
	MD5Final(&md5,decrypt);
    fclose(fp);
    FILE *fp1=fopen("test.txt","w");
    fprintf(fp1,"%s ",name);
	for(i=0;i<16;i++)
	{
		printf("%02x",decrypt[i]);
	}
    fclose(fp1);
	return 0;
}

