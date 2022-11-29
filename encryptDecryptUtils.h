#include "codec.h"

typedef struct paramsToJob{
	int key;
	int index;
	char* arr;
	void (*func)(char* s, int key);

}paramsToJob;

void encryptDecryptWrapper(void* args)
{
	struct paramsToJob prms = *((struct paramsToJob* ) args);
	char data[1] = {prms.arr[prms.index]}; 
	prms.func(data,prms.key);
	prms.arr[prms.index] = data[0];
}



