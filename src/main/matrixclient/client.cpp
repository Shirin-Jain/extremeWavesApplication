#include "client.h"
#include "serialib.h"
#include "matrixTransfer.h"
#include <string>

Client::Client(std::string port, uint32_t baud)
{

    int status = serial.openDevice(port.c_str(), baud);
    if (status != 1)
    {
        printf("Serial error: %d\n", status);
    }
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

    rpc.id = id;
    commandId = id;

    uint8_t m;
    uint8_t n;

    uint16_t matrixID;
    printf("Enter matrixID:\n");
    scanf("%hd", &matrixID);

    if (id == READ_MATRIX or id == DELETE_MATRIX)
    {
        rpc.payloadLength = sizeof(Matrix::matrixID);
        rpc.payload = (uint8_t *)malloc(rpc.payloadLength);
        memcpy(rpc.payload, &matrixID, sizeof(matrixID));
        success = true;
    }
    else
    {
        printf("Enter m:\n");
        scanf("%hhd", &m);

        printf("Enter n:\n");
        scanf("%hhd", &n);
    }

    // printf("M: %d, N: %d", m, n);

    if (id == READ_CELL)
    {

        rpc.payloadLength = sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n);

        rpc.payload = (uint8_t *)malloc(rpc.payloadLength);

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

        rpc.payload = (uint8_t *)malloc(rpc.payloadLength);

        memcpy(rpc.payload, &matrixID, sizeof(matrixID));
        memcpy(rpc.payload + 2, &m, sizeof(m));
        memcpy(rpc.payload + 3, &n, sizeof(n));
        memcpy(rpc.payload + 4, &value, sizeof(float));
        success = true;
    }

    else if (id == WRITE_MATRIX)
    {

        uint8_t memFlag;

        printf("Enter 1 for volatile memory and 2 for permanent memory:\n");

        scanf("%hhd", &memFlag);

        uint32_t matrixLength = m * n;

        float *values = (float *)(malloc(matrixLength * sizeof(float)));
        printf("Enter %d floats:\n", matrixLength);

        for (int i = 0; i < matrixLength; ++i)
        {
            scanf("%f", &(values[i]));
        }

        rpc.payloadLength = sizeof(memFlag) + sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n) + matrixLength * sizeof(float);
        rpc.payload = (uint8_t *)malloc(rpc.payloadLength);
        rpc.payload[0] = memFlag;
        memcpy(rpc.payload + 1, &matrixID, sizeof(matrixID));
        memcpy(rpc.payload + 3, &m, sizeof(m));
        memcpy(rpc.payload + 4, &n, sizeof(n));
        memcpy(rpc.payload + 5, values, matrixLength * sizeof(float));

        success = true;
    }

    printf("PayloadLength: %hd\n", rpc.payloadLength);

    rpc.payloadChecksum = fletcher16(rpc.payload, rpc.payloadLength);
    rpc.headerChecksum = fletcher16(rpc.header, sizeof(Packet::header));
    commandChecksum = rpc.payloadChecksum;

    printf("Sending command \n");

    sendRPC(&rpc);

    return success;
}

void Client::sendRPC(Packet *packet)
{

    serial.writeBytes(packet->header, sizeof(Packet::header));
    serial.writeBytes(&(packet->headerChecksum), sizeof(Packet::headerChecksum));
    serial.writeBytes(packet->payload, packet->payloadLength);
}

void Client::waitResponse()
{

    uint8_t b;
    int16_t size = 0;

    bool responseFound = false;
    Packet responsePacket;

    printf("Waiting for response ...\n");

    while (!responseFound)
    { // add time out too
        size = serial.readChar((char *)&b);

        if (size < 0)
        {
            size = 0;
        }

        bool packetedCompleted = parser.processByte(b);

        if (packetedCompleted)
        {
            if (parser.completedPacket.isValid())
            {
                if (parser.completedPacket.id == commandChecksum)
                {
                    responsePacket = parser.completedPacket;
                    responseFound = true;
                }
            }
        }
    }

    if (responsePacket.payload[0] == SUCCESS_FLAG)
    {
        printf("Success\n");

        if (commandId == READ_CELL)
        {
            float value;
            memcpy(&value, responsePacket.payload + 1, sizeof(float));

            printf("Cell: %f \n", value);
        }

        if (commandId == READ_MATRIX)
        { // repsose format - error flag( 1byte), 2 byte mID, 2 byte m, 2 byte n, etc

            uint16_t matrixId;
            memcpy(&matrixId, responsePacket.payload + 1, sizeof(uint16_t));
            uint8_t m;
            memcpy(&m, responsePacket.payload + 3, sizeof(Matrix::m));
            uint8_t n;
            memcpy(&n, responsePacket.payload + 4, sizeof(Matrix::n));

            uint32_t index = 0;

            printf("Matrix #%d :\n", matrixId);

            for (int i = 0; i < m; i++)
            {

                for (int j = 0; j < n; j++)
                {

                    printf("%f ", (reinterpret_cast<float *>(responsePacket.payload + 5))[index]);
                    index++;
                }
                printf("\n");
            }
        }
    }
    else if (responsePacket.payload[0] == FAIL_FLAG)
    {
        printf("Command failure \n");
    }
}
