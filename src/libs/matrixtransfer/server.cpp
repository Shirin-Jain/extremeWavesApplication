#include "mbed.h"
#include "server.h"
#include "matrixTransfer.h"

Server::Server()
{
    init();
}

void Server::init()
{
    serial = BufferedSerial(CONSOLE_TX, CONSOLE_RX, 115200); // look this up
    parser = Parser();
}

void Server::waitForCommands()
{

    uint8_t buf[MAX_SERIAL_BUFFER]; // find good size
    int16_t size = 0;


   
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

                    float * value;
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
                        memcpy(response.payload + 5, &(matrix->n), sizeof(matrix->n));
                        memcpy(response.payload + 7, matrix->data, sizeof(matrix->data));



                    }else if (packet.id == READ_CELL&& errorFlag == 1){
                        response.payloadLength = sizeof(errorFlag) + sizeof(float);
                        response.payload = (uint8_t*) malloc(response.payloadLength);

                        memcpy(response.payload + 1, value, sizeof(float));

                    }else{
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
    memcpy(&matrixId, data + 1, sizeof(uint16_t));
    uint16_t m;
    memcpy(&m, data + 3, sizeof(uint16_t));
    uint16_t n;
    memcpy(&n, data, sizeof(uint16_t));

    uint32_t matrixLength = m*n;

    float* values = (float*)malloc(matrixLength * sizeof(float));
   
    memcpy(&values, data, sizeof(values));

    // future error checking


    if(data[0] == VOLATILE_MEMORY_FLAG){

        matrices.emplace(matrixId, m, n, values);

    } // add code for permanenet memory


    return true;

}


bool Server::deleteMatrix(uint8_t* data){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(uint16_t));

    return matrices.erase(Matrix(matrixId, 0, 0, nullptr));
}


bool Server::writeCell(uint8_t* data){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(uint16_t));
    uint16_t m;
    memcpy(&m, data + 2, sizeof(uint16_t));
    uint16_t n;
    memcpy(&n, data +4, sizeof(uint16_t));
    float value;
    memcpy(&value, data +6, sizeof(float));


    auto it = matrices.find(Matrix(matrixId,0,0, nullptr));  // Find the element with matching id

    if (it != matrices.end()) {
        Matrix matrix = *it;
        
        matrix.writeCell(m, n, value);  

        matrices.erase(it);

        matrices.emplace(matrix);

    
    }else{
        return false;
    }

}


bool Server::readCell(uint8_t* data, float * value){

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(uint16_t));
    uint16_t m;
    memcpy(&m, data + 2, sizeof(uint16_t));
    uint16_t n;
    memcpy(&n, data +4, sizeof(uint16_t));

    auto it = matrices.find(Matrix(matrixId,0,0, nullptr));  // Find the element with matching id

    if (it != matrices.end()) {
        value = it->readCell(m, n);
        return true;
    }else{
        // code for searching through permanent memory
        return false;
    }

}


bool Server::readMatrix(uint8_t* data, const Matrix  * matrix){

    uint16_t matrixId;


    auto it = matrices.find(Matrix(matrixId,0,0, nullptr));  // Find the element with matching id

    if (it != matrices.end()) {
        matrix = &(*it);
        return true;
    }else{
        // code for searching through permanent memory
        return false;
    }

}