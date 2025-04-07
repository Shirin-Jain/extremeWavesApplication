#include "matrixTransfer.h"
#include "mbed.h"

Client::Client()
{
    return;
}

void Client::init()
{

    return;
}

// initliazie membor functions in one

bool Client::askForCommand()
{

    uint8_t *input;

    printf("Type your command \n");
    scanf("%d", &input);

    printf("Enter matrixID \n");

    if (input != 2 or input != 4)
    { // make commands defines
        printf("Enter m \n");
        printf("Enter n \n");
    }

    if (input == 8)
    {
        printf("Enter value \n");
    }

    if (input == 1)
    {
        print("Enter matrix \n") // allocate data before hand based on matrix;
    }

    printf("Not a valid command\n");

    // format args and find args length;
    //  serialize args;
    // might not need to create an actual packet;

    // sned command
    // save checksum

    return true; // if sent
}

void Client::waitRepsonse()
{

    uint8_t buf[MAX_SERIALBUFFER_SIZE]; // find good size
    int16_t size = 0;
    if (serial.readable())
    {
        size = serial.read(buf, MAX_SERIALBUFFER_SIZE);
    }

    if (size < 0)
    {
        size = 0;
    }

    for (size_t offset = 0; offset < (uint16_t)size; offset++)
    {
        bool packetedCompleted = parser.processByte(buf[offset]);

        if (packetedCompleted)
        {
            if (parser.completedPacket.isValid())
            {

                RPCPacket inputPacket = parser.completedPacket;

                if( 0 == commandChecksum){
                    if(commandId == 0){
                        //check for success
                        // if fail print error message - some for fail, non exitsent, idk

                        if(commandID == 2){
                            //print matrix;

                        }

                        if(commandId == 16){
                            //print value
                        }




                    }

                }
            }
        }
    }
}
