#include "client.h"
#include "serialib.h"
#include "stdio.h"
#include <string>

int main()
{
	printf("What serial port :\n");
	char tmp[20];
	scanf("%19s", tmp);
	std::string port = tmp;

	printf("Connecting to port: %s\n", port.c_str());

	Client client = Client(port, MT_BAUD_RATE);

	while(true) 
	{

		if(client.askForCommand()){
			client.waitResponse();
		}

	}

	return 0;
}









