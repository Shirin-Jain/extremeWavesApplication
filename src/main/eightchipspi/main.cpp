#include "mbed.h"

int main()
{

    
	while(true) 
	{

		uint8_t input[3];
		RPCPacket chipPackets[8][128]; 

		printf("Waiting for command\n");

		getCommand(input);

        printf("Input: %d \n", input);


		ThisThread::sleep_for(1s);
	}

	return 0;
}


bool getCommand(uint8_t command[3]){ // pretty sure this needs to be a pointer

	scanf("%d", &command);

	//code for seeing if comand is valid;

	return true;

}

void computePackets(uint8_t command[3], RPCPacket packets[8][128] ){

	//code for computing packets that takes 5 us

	// set evreything to some number rn



}

struct RPCPacket{
	uint8_t packet[5];
};

struct Chip{
	Chip(); //constructor that takes spi pins

	void sendData(RPCPacket packets[128]);

	//data members should just be spi pins



};

//code for picking chip selects





