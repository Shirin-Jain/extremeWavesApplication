#include "mbed.h"

int main()
{

    
	while(true) 
	{

        int input = -1;
		printf("Waiting for command\n");


        scanf("%d", &input);

        printf("Input: %d \n", input);


		ThisThread::sleep_for(1s);
	}

	return 0;
}





