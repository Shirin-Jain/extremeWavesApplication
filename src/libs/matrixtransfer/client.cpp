#include "mbed.h"
#include "client.h"
#include "matrixTransfer.h"

Client::Client()
{
    init();
}

void Client::init()
{
    serial = BufferedSerial(CONSOLE_TX, CONSOLE_RX, 115200); // look this up
    parser = Parser();
}

// initliazie membor functions in one

bool Client::askForCommand()
{

    uint8_t *id;

    Packet rpc = Packet();

    uint16_t payloadLength;
    uint16_t payloadChecksum;




    uint16_t headerChecksum;
    uint8_t* payload;

    printf("Type your command \n");
    scanf("%d", &id);

    rpc.id = *id;  // add check for valid command


    uint16_t *matrixID;
    printf("Enter matrixID \n");
    scanf("%d", &matrixID);

    if (*id == READ_MATRIX or *id == DELETE_MATRIX)
    { 
        rpc.payloadLength = sizeof(Matrix::matrixID);
        rpc.payload = (uint8_t*) malloc(payloadLength); // makesure this works

        memcpy(payload, matrixID, sizeof(matrixID));

        rpc.payloadChecksum = fletcher16(rpc.header, payloadChecksum);
        rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));

        //send response

    }


    uint16_t *m;
    uint16_t *n;


    printf("Enter m and n \n");
    scanf("%d %d", &m, &n);


    if (*id == READ_CELL)
    { 

        rpc.payloadLength = sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n);

        rpc.payload = (uint8_t*) malloc(payloadLength); // makesure this works

        memcpy(payload, matrixID, sizeof(matrixID));
        memcpy(payload + 2, m, sizeof(m));
        memcpy(payload + 4, n, sizeof(n));

        rpc.payloadChecksum = fletcher16(rpc.header, payloadChecksum);
        rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));

    }

    if (*id == WRITE_CELL)
    {   
        float* value;

        printf("Enter value \n");
        scanf("%f", &value);

        rpc.payloadLength = sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n) + sizeof(float);

        rpc.payload = (uint8_t*) malloc(payloadLength); // makesure this works

        memcpy(payload, matrixID, sizeof(matrixID));
        memcpy(payload + 2, m, sizeof(m));
        memcpy(payload + 4, n, sizeof(n));
        memcpy(payload + 6, value, sizeof(float));

        rpc.payloadChecksum = fletcher16(rpc.header, payloadChecksum);
        rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));


    }

    if(*id == WRITE_MATRIX){

        uint8_t* memFlag;

        printf("Enter 1 for volatile memory and 2 for permanent memory \n"); // should make a define for this

        scanf("%d", memFlag);

        uint32_t matrixLength = *m * *n; 

        float* values = (float*)(malloc(matrixLength * sizeof(float)));  // double check this works

        printf("Enter %d floats: \n", matrixLength);
        for (int i = 0; i < matrixLength; ++i) {
            scanf("%f", &values[i]);  
        }


        rpc.payloadLength = sizeof(memFlag) + sizeof(Matrix::matrixID) +sizeof(Matrix::m) + sizeof(Matrix::n) + sizeof(values);
        rpc.payload = (uint8_t*) malloc(payloadLength);

        payload[0] = *memFlag;
        memcpy(payload + 1, matrixID, sizeof(matrixID));
        memcpy(payload + 3, m, sizeof(m));
        memcpy(payload + 5, n, sizeof(n));
        memcpy(payload + 7, values, sizeof(values));

        rpc.payloadChecksum = fletcher16(rpc.header, payloadChecksum);
        rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));

        sendResponse(&rpc);
    }

    return false;

}



void Client::sendResponse(Packet* packet){

    //test byprinting this shit out
    //also not response

    serial.write(packet->header, sizeof(Packet::header));
    serial.write(&(packet->headerChecksum), sizeof(Packet::headerChecksum)); // double check this works
    serial.write(packet->payload, packet->payloadLength);
}

void Client::waitResponse()
{

    uint8_t buf[MAX_SERIAL_BUFFER]; // find good size
    int16_t size = 0;

    bool responseFound = false;
    Packet responsePacket;

    printf("Waiting for response ...\n");

    while(!responseFound) { // add time out too


        if (serial.readable())
        {
            size = serial.read(buf, MAX_SERIAL_BUFFER);
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
                    if(parser.completedPacket.payloadChecksum == commandChecksum){
                        responsePacket = parser.completedPacket;
                        responseFound == true;
                        break;

                    }


                
                }
            }
        }

    }

   

    if(responsePacket.payload[0] == SUCCESS_FLAG){
        printf("Success \n");


        if(commandId == READ_CELL){
            float value;
            memcpy(&value, responsePacket.payload +1, sizeof(float));
    
            printf("Cell : %f", value);
        }
    
    
        if(commandId == READ_MATRIX){ // repsose format - error flag( 1byte), 2 byte mID, 2 byte m, 2 byte n, etc
            
            uint16_t matrixId;
            memcpy(&matrixId, responsePacket.payload + 1, sizeof(uint16_t));
            uint16_t m;
            memcpy(&m, responsePacket.payload + 3, sizeof(uint16_t));
            uint16_t n;
            memcpy(&n, responsePacket.payload + 5, sizeof(uint16_t));
    
            uint32_t index = 0;
    
            printf("Matrix [%d]: ", matrixId);
    
            for(int i = 0; i < m; i++){
    
                for(int j = 0; j < n; j ++){
    
                    printf("%f ", *reinterpret_cast<float*>(responsePacket.payload + 7 + index));
                    index++;
    
                }
    
                printf("/n");
            }

        }
    


    } else if (responsePacket.payload[0] == FAIL_FLAG){
        printf("Command failure \n");

    }
    

    


    
}
