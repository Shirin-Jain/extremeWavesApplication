#include "mbed.h"
#include "server.h"

int main()
{

	Server server = Server();

    
	while(true) 
	{

		server.waitForCommands();

	}

	return 0;
}





