#include <cstdint>
#include <functional>
#include <cstring>
#include <cassert>

#pragma once

#define SOH 0x4E4C444D
#define MAX_MATRIX_DIMENSION 127

#define SEARCHING_FOR_SOH 0
#define HEADER 1
#define HEADER_CHECKSUM 2
#define PAYLOAD 3

#define WRITE_MATRIX 1  // mem flag, matrix struct
#define READ_MATRIX 2   // matrix id
#define DELETE_MATRIX 4 // matrix id
#define WRITE_CELL 8    // matrix id, m, n, value
#define READ_CELL 16    // matrix id, m, n,

#define VOLATILE_MEMORY_FLAG 1
#define PEMRANENT_MEMORY_FLAG 2

#define SUCCESS_FLAG 1
#define FAIL_FLAG 2

#define MAX_SERIAL_BUFFER 1024
#define MT_BAUD_RATE 115200

/*
RPC:

WRITE_MATRIX : id = 1,      payload = memFlag , matrixID, m, n, float[m*n] data
READ_MATRIX : id =  2       payload = matrixID
DELETE_MATRIX : id =  4     payload = matrixID
WRITE_CELL : id =  8        payload = matrixID, m, n, float value
READ_CELL : id = 16         payload = matrixID, m, n

memFlag, m, n - 1byte
matrixID - 2bytes

float - 4bytes

*/

/*
RESPONSE:

WRITE_MATRIX : id = rpcChecksum     payload = successFlag
READ_MATRIX : id =  rpcChecksum     payload = successFlag, matrixID,, m, n, float[m*n] data
DELETE_MATRIX : id =  rpcChecksum   payload = successFlag
WRITE_CELL : id =  rpcChecksum      payload = successFlag
READ_CELL : id = rpcChecksum        payload = successFlag, float value

successFlag, 1byte

*/

uint16_t fletcher16(const uint8_t *data, uint64_t length);

#pragma pack(push, 1)
struct Matrix
{
    uint16_t matrixID;
    uint8_t m;
    uint8_t n;
    float *data;

    Matrix(uint16_t _matrixID, uint16_t _m, uint16_t _n, float *_data = nullptr)
    {
        matrixID = _matrixID;
        m = _m;
        n = _n;
        data = _data;
    };

    float *readCell(uint16_t _m, uint16_t _n) const
    {
        assert(_m < m && _n < n);

        return data + (_m * (n) + _n);
    };

    void writeCell(uint16_t _m, uint16_t _n, float value)
    {
        assert(_m < m && _n < n);

        data[_m * (n) + _n] = value;
    };

    bool operator==(const Matrix &other) const
    {
        return matrixID == other.matrixID;
    }

    ~Matrix()
    {
        if (data != nullptr)
        {
            free(data);
        }
    }
};

struct MatrixHash
{
    std::size_t operator()(const Matrix &m) const
    {
        return std::hash<int>{}(m.matrixID);
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Packet
{

    union
    {
        struct
        {
            uint32_t startOfHeader = SOH;
            uint16_t payloadLength;
            uint16_t payloadChecksum;
            uint16_t id; // either rpc or rpc checksum
        };

        struct
        {
            uint8_t header[10];
        };
    };

    uint16_t headerChecksum;
    uint8_t *payload;

    Packet()
    {
        startOfHeader = SOH;
        payloadLength = 0;
        payloadChecksum = 0;
        id = 0;
        headerChecksum = 0;
    };

    bool isValid();
};
#pragma pack(pop)

struct Parser
{
    Parser();
    void init();
    bool processByte(uint8_t);

    int state;
    uint32_t soh;
    Packet inputPacket;
    Packet completedPacket;
    size_t position;
};
