#include "mbed.h"
#include "server.h"
#include "matrixTransfer.h"

int main()
{
	Server server = Server(CONSOLE_TX, CONSOLE_RX, MT_BAUD_RATE);

	while (true)
	{

		server.waitForCommands();
	}

	return 0;
}
