#include "mbed.h"
#include "server.h"
#include "matrixTransfer.h"

Server::Server(PinName Tx, PinName Rx, uint32_t baud) : serial(Tx, Rx, baud)
{
    init();
    string msg = "INIT\n";
    serial.write(msg.c_str(), msg.size());
}

void Server::init()
{
    parser = Parser();
}

void Server::waitForCommands()
{

    uint8_t buf[MAX_SERIAL_BUFFER]; // find good size
    int size = 0;
   
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
                    Packet packet = parser.completedPacket;

                    Packet response = Packet();
                    response.id = packet.payloadChecksum;
                    uint8_t errorFlag = 1;

                    float value;
                    Matrix * matrix;

                    switch (packet.id)
                    {

                    case WRITE_MATRIX:

                        if(!writeMatrix(packet.payload)){
                            errorFlag = 2;
                        }
                        break;

                    case READ_MATRIX:

                        if(!readMatrix(packet.payload, matrix)){
                            errorFlag = 2;
                        }

                        
                        break;
                    case DELETE_MATRIX:
                        if(!deleteMatrix(packet.payload)){
                            errorFlag = 2;
                        }
                        break;

                    case WRITE_CELL:
                        if(!writeCell(packet.payload)){
                            errorFlag = 2;
                        }
                        break;

                    case READ_CELL:
                        if(!readCell(packet.payload, value)){
                            errorFlag = 2;
                        }

                        break;
                    }

                    if(packet.id == READ_MATRIX && errorFlag == 1){
                        response.payloadLength = sizeof(errorFlag) + sizeof(Matrix::matrixID) +sizeof(Matrix::m) + sizeof(Matrix::n) + sizeof(matrix->data);
                        response.payload = (uint8_t*) malloc(response.payloadLength);

                        memcpy(response.payload + 1, &(matrix->matrixID), sizeof(matrix->matrixID));
                        memcpy(response.payload + 3, &(matrix->m), sizeof(matrix->m));
                        memcpy(response.payload + 4, &(matrix->n), sizeof(matrix->n));
                        memcpy(response.payload + 5, matrix->data, matrix->m*matrix->n*sizeof(decltype(*matrix->data)));
                    }
                    else if (packet.id == READ_CELL&& errorFlag == 1){
                        response.payloadLength = sizeof(errorFlag) + sizeof(float);
                        response.payload = (uint8_t*) malloc(response.payloadLength);

                        memcpy(response.payload + 1, &value, sizeof(value));
                    }
                    else{
                        response.payloadLength = sizeof(errorFlag);
                        response.payload = (uint8_t*) malloc(response.payloadLength);

                    }

                    response.payload[0] = errorFlag;

                    response.payloadChecksum = fletcher16(response.payload, response.payloadLength);
                    response.headerChecksum = fletcher16(response.header, sizeof(Packet::header));

                    sendResponse(&response);
                }
            }
        }
    
}

void Server::sendResponse(Packet *packet)
{
    serial.write(packet->header, sizeof(Packet::header));
    serial.write(&(packet->headerChecksum), sizeof(Packet::headerChecksum)); // double check this works
    serial.write(packet->payload, packet->payloadLength);
}

bool Server::writeMatrix(uint8_t* data){
    uint16_t matrixId;
    memcpy(&matrixId, data + 1, sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data + 3, sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data + 4, sizeof(Matrix::n));

    uint32_t matrixLength = m*n;

    float* values = (float*)malloc(matrixLength * sizeof(float));
   
    memcpy(&values, data, sizeof(values));

    // future error checking

    if(data[0] == VOLATILE_MEMORY_FLAG){
        matrices[matrixId] = new Matrix {matrixId, m, n, values};
    } // add code for permanenet memory

    return true;
}


bool Server::deleteMatrix(uint8_t* data){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(uint16_t));

    return matrices.erase(matrixId);
}


bool Server::writeCell(uint8_t* data){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data + 2, sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data + 3, sizeof(Matrix::n));
    float value;
    memcpy(&value, data +6, sizeof(float));

    if (matrices.find(matrixId) != matrices.end()) {
        matrices[matrixId]->writeCell(m, n, value);  
        return true;
    
    }else{
        return false;
    }

}


bool Server::readCell(uint8_t* data, float & value){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data + 2, sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data + 3, sizeof(Matrix::n));

    if (matrices.find(matrixId) != matrices.end()) {
        value = *(matrices[matrixId]->readCell(m, n));
        return true;
    }else{
        // code for searching through permanent memory
        return false;
    }

}

bool Server::readMatrix(uint8_t* data, Matrix *& matrix){

    uint16_t matrixId;

    memcpy(&matrixId, data, sizeof(matrixId));

    if (matrices.find(matrixId) != matrices.end()) {
        matrix = matrices[matrixId];
        return true;
    }

    return false;
}