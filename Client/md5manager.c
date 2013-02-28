#include <stdio.h>
#include <openssl/md5.h>
#include "md5manager.h"

int getMD5checkSum(const char *filename, char* buffer)
{
	unsigned char c[MD5_DIGEST_LENGTH];
	int i;
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	//char buffer[32];

	if (inFile == NULL) {
		printf ("%s can't be opened.\n", filename);
		return 0;
	}

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
	MD5_Update (&mdContext, data, bytes);
	MD5_Final (c,&mdContext);
	FILE *f = tmpfile();//fopen("log.txt", "a");  
	for(i = 0; i < MD5_DIGEST_LENGTH; i++){fprintf(f,"%02x", c[i]);};
	fclose (inFile);
	rewind(f);
	fgets (buffer,33,f);
	fclose(f);  
	buffer[34]='\0';
	return 1;
}
