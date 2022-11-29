#include "codec.h"
#include <stdio.h>
#include <stdlib.h>
#include "thpool.h"

int main(int argc, char *argv[])
{
	printf("length of args %s \n",argv[1]);
	char data[] = "m";
	int key = 12;

	encrypt(data,key);
	printf("encripted data: %s\n",data);

	decrypt(data,key);
	printf("decripted data: %s\n",data);

	return 0;
}
