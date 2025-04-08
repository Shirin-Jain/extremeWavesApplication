#include "mbed.h"
#include "client.h"

int main()
{

	Client client = Client();

    
	while(true) 
	{

		if(client.askForCommand()){
			client.waitResponse();
		}

	}

	return 0;
}









