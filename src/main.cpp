#include "mbed.h"

int main()
{

    
	while(true) 
	{
		printf("Hello World\n");
		ThisThread::sleep_for(1s);
	}

	return 0;
}


