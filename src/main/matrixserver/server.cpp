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

    uint8_t buf[MAX_SERIAL_BUFFER];
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
                Matrix *matrix;

                switch (packet.id)
                {

                case WRITE_MATRIX:

                    if (!writeMatrix(packet.payload))
                    {
                        errorFlag = 2;
                    }
                    break;

                case READ_MATRIX:

                    if (!readMatrix(packet.payload, matrix))
                    {
                        errorFlag = 2;
                    }

                    break;
                case DELETE_MATRIX:
                    if (!deleteMatrix(packet.payload))
                    {
                        errorFlag = 2;
                    }
                    break;

                case WRITE_CELL:
                    if (!writeCell(packet.payload))
                    {
                        errorFlag = 2;
                    }
                    break;

                case READ_CELL:
                    if (!readCell(packet.payload, value))
                    {
                        errorFlag = 2;
                    }

                    break;
                }

                if (packet.id == READ_MATRIX && errorFlag == 1)
                {
                    size_t matrixDataLength = matrix->m * matrix->n * sizeof(decltype(*matrix->data));
                    response.payloadLength = sizeof(errorFlag) + sizeof(Matrix::matrixID) + sizeof(Matrix::m) + sizeof(Matrix::n) + matrixDataLength;
                    response.payload = (uint8_t *)malloc(response.payloadLength);

                    memcpy(response.payload + 1, &(matrix->matrixID), sizeof(matrix->matrixID));
                    memcpy(response.payload + 3, &(matrix->m), sizeof(matrix->m));
                    memcpy(response.payload + 4, &(matrix->n), sizeof(matrix->n));
                    memcpy(response.payload + 5, matrix->data, matrixDataLength);
                }
                else if (packet.id == READ_CELL && errorFlag == 1)
                {
                    response.payloadLength = sizeof(errorFlag) + sizeof(float);
                    response.payload = (uint8_t *)malloc(response.payloadLength);

                    memcpy(response.payload + 1, &value, sizeof(value));
                }
                else
                {
                    response.payloadLength = sizeof(errorFlag);
                    response.payload = (uint8_t *)malloc(response.payloadLength);
                }

                response.payload[0] = errorFlag;

                response.payloadChecksum = fletcher16(response.payload, response.payloadLength);
                response.headerChecksum = fletcher16(response.header, sizeof(Packet::header));

                sendResponse(&response);
                free(response.payload);
            }
        }
    }
}

void Server::sendResponse(Packet *packet)
{
    serial.write(packet->header, sizeof(Packet::header));
    serial.write(&(packet->headerChecksum), sizeof(Packet::headerChecksum));
    serial.write(packet->payload, packet->payloadLength);
}

bool Server::writeMatrix(uint8_t *data)
{
    uint8_t memFlag = data[0];
    uint16_t matrixId;
    memcpy(&matrixId, data += sizeof(memFlag), sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data += sizeof(Matrix::matrixID), sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data += sizeof(Matrix::m), sizeof(Matrix::n));

    uint32_t matrixElements = m * n;
    uint32_t matrixMemorySize = matrixElements * sizeof(float);

    float *values = (float *)malloc(matrixMemorySize);

    memcpy(values, data += sizeof(Matrix::n), matrixMemorySize);

    // future error checking

    if (memFlag == VOLATILE_MEMORY_FLAG)
    {
        if (matrices.find(matrixId) != matrices.end())
        {
            delete matrices[matrixId];
            return matrices.erase(matrixId);
        }
        matrices[matrixId] = new Matrix{matrixId, m, n, values};
    } // add code for permanenet memory

    return true;
}

bool Server::deleteMatrix(uint8_t *data)
{

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(uint16_t));

    if (matrices.find(matrixId) != matrices.end())
    {
        delete matrices[matrixId];
        return matrices.erase(matrixId);
    }
    else
    {
        return false;
    }
}

bool Server::writeCell(uint8_t *data)
{

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data += sizeof(Matrix::matrixID), sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data += sizeof(Matrix::m), sizeof(Matrix::n));
    float value;
    memcpy(&value, data += sizeof(Matrix::n), sizeof(float));

    if (matrices.find(matrixId) != matrices.end())
    {

        if (m < matrices[matrixId]->m && n < matrices[matrixId]->n)
        {
            matrices[matrixId]->writeCell(m, n, value);
            return true;
        }
    }

    return false;
}

bool Server::readCell(uint8_t *data, float &value)
{

    uint16_t matrixId;
    memcpy(&matrixId, data, sizeof(Matrix::matrixID));
    uint8_t m;
    memcpy(&m, data += sizeof(Matrix::matrixID), sizeof(Matrix::m));
    uint8_t n;
    memcpy(&n, data += sizeof(Matrix::m), sizeof(Matrix::n));

    if (matrices.find(matrixId) != matrices.end())
    {

        if (m < matrices[matrixId]->m && n < matrices[matrixId]->n)
        {
            value = *(matrices[matrixId]->readCell(m, n));
            return true;
        }
    }

    return false;
}

bool Server::readMatrix(uint8_t *data, Matrix *&matrix)
{

    uint16_t matrixId;

    memcpy(&matrixId, data, sizeof(matrixId));

    if (matrices.find(matrixId) != matrices.end())
    {
        matrix = matrices[matrixId];
        return true;
    }

    return false;
}