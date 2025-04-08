#include "client.h"
#include "serialib.h"
#include "matrixTransfer.h"
#include <string>

Client::Client(std::string port, uint32_t baud)
{

    int status = serial.openDevice(port.c_str(), baud);
    if (status != 1) {
        printf("Serial error: %d\n", status);
    }
    serial.DTR(true);
    serial.RTS(false);
    init();
}

void Client::init()
{
    parser = Parser();
}

// initliazie membor functions in one

bool Client::askForCommand()
{

    uint16_t id;

    Packet rpc = Packet();

    bool success = false;


    uint16_t headerChecksum;

    printf("Type your command:\n");
    scanf("%hd", &id);

    rpc.id = id;  // add check for valid command
    commandId = id;

    uint8_t m;
    uint8_t n;
    
    uint16_t matrixID;
    printf("Enter matrixID:\n");
    scanf("%hd", &matrixID);

    if (id == READ_MATRIX or id == DELETE_MATRIX)
    { 
        rpc.payloadLength = sizeof(Matrix::matrixID);
        rpc.payload = (uint8_t*) malloc(rpc.payloadLength); // makesure this works

        memcpy(rpc.payload, &matrixID, sizeof(matrixID));
        success = true;
    } else {
        printf("Enter m:\n");
        scanf("%hhd", &m);
    
        printf("Enter n:\n");
        scanf("%hhd", &n);
    }

   // printf("M: %d, N: %d", m, n);


    if (id == READ_CELL)
    { 

        rpc.payloadLength = sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n);

        rpc.payload = (uint8_t*) malloc(rpc.payloadLength); // makesure this works

        memcpy(rpc.payload, &matrixID, sizeof(matrixID));
        memcpy(rpc.payload + 2, &m, sizeof(m));
        memcpy(rpc.payload + 3, &n, sizeof(n));
        success = true;
    }

    else if (id == WRITE_CELL)
    {   
        float value;

        printf("Enter value:\n");
        scanf("%f", &value);

        rpc.payloadLength = sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n) + sizeof(float);

        rpc.payload = (uint8_t*) malloc(rpc.payloadLength); // makesure this works

        memcpy( rpc.payload, &matrixID, sizeof(matrixID));
        memcpy( rpc.payload + 2, &m, sizeof(m));
        memcpy( rpc.payload + 3, &n, sizeof(n));
        memcpy( rpc.payload + 4, &value, sizeof(float));
        success = true;
    }

    else if(id == WRITE_MATRIX){

        uint8_t memFlag;

        printf("Enter 1 for volatile memory and 2 for permanent memory:\n"); // should make a define for this

        scanf("%hhd", &memFlag);

        uint32_t matrixLength = m * n; 

        float* values = (float*)(malloc(matrixLength * sizeof(float)));  // double check this works

        printf("Enter %d floats:\n", matrixLength);
        
        for (int i = 0; i < matrixLength; ++i) {
            scanf("%f", &(values[i]));  
        }

        printf("Formatting pack:\n");


        rpc.payloadLength = sizeof(memFlag) + sizeof(Matrix::matrixID) +sizeof(Matrix::m) + sizeof(Matrix::n) +  matrixLength *sizeof(float);
        rpc.payload = (uint8_t*) malloc(rpc.payloadLength);
        rpc.payload[0] = memFlag;
        memcpy( rpc.payload + 1, &matrixID, sizeof(matrixID));
        memcpy( rpc.payload + 3, &m, sizeof(m));
        memcpy( rpc.payload + 4, &n, sizeof(n));
        memcpy( rpc.payload + 5, values, matrixLength * sizeof(float));

        printf("After memcpy\n");

        success = true;
    }

    printf("PayloadLength: %hd\n", rpc.payloadLength);

    rpc.payloadChecksum = fletcher16(rpc.payload, rpc.payloadLength);
    rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));

    printf("Checksums P: %d H: %d\n", rpc.payloadChecksum, rpc.headerChecksum);

    printf("Sending command \n");

    sendResponse(&rpc);

    return success;
}

void Client::sendResponse(Packet* packet){

    //test byprinting this shit out
    //also not response
    size_t meta_size = sizeof(Packet::header) + sizeof(Packet::headerChecksum);
    
    for (int b = 0; b < meta_size; b++) {
        printf("%X ", ((uint8_t*)(packet))[b]);
    }
    printf("\n");
    for (int b = 0; b < packet->payloadLength; b++) {
        printf("%X ", ((uint8_t*)(packet->payload))[b]);
    }
    printf("\n");
    
    int a = serial.writeBytes(packet->header, sizeof(Packet::header));
    int b = serial.writeBytes(&(packet->headerChecksum), sizeof(Packet::headerChecksum));
    int c = serial.writeBytes(packet->header, packet->payloadLength);

    printf("WS: %d %d %d\n", a,b,c);
}

void Client::waitResponse()
{

    uint8_t buf[MAX_SERIAL_BUFFER]; // find good size
    int16_t size = 0;

    bool responseFound = false;
    Packet responsePacket;

    printf("Waiting for response ...\n");

    while(!responseFound) { // add time out too


        if (serial.available())
        {
            size = serial.readBytes(buf, MAX_SERIAL_BUFFER);
        }

        if (size < 0)
        {
            size = 0;
        }

        for (size_t offset = 0; offset < (uint16_t)size; offset++)
        {
            printf("%X ", buf[offset]);

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
            uint8_t m;
            memcpy(&m, responsePacket.payload + 3, sizeof(Matrix::m));
            uint8_t n;
            memcpy(&n, responsePacket.payload + 4, sizeof(Matrix::n));
    
            uint32_t index = 0;
    
            printf("Matrix [%d]: ", matrixId);
    
            for(int i = 0; i < m; i++){
    
                for(int j = 0; j < n; j ++){
    
                    printf("%f ", *reinterpret_cast<float*>(responsePacket.payload + 5 + index));
                    index++;
    
                }
    
                printf("/n");
            }

        }
    
    } else if (responsePacket.payload[0] == FAIL_FLAG){
        printf("Command failure \n");

    }
}
