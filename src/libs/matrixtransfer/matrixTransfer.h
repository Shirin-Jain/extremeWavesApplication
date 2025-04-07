#include <cstdint>
#define RPC_SOH                             0x4E4C444D
#define RESPONCE_SOH                        0x4E4C444D //change
#define MAX_ARGS_SIZE            0xFFFF  // need to change to be reasonable

//128 by 128 or 254 by 254


uint16_t fletcher16(const uint8_t* data, uint64_t length);



struct Matrix {

    uint16_t matrixID;
    uint16_t m;
    uint16_t n;
    float* data;
    

};
/*
 procID = 0, 1, 2, 3, 4   (could be 1 hot coded)
1  , 2, 4, 8, 16 

 1 = write Matrix
    args - matrix struct + size (?)
 2 = read Matrix
    args - matrix ID
 4 = write to specific cell
    args - m, n, matrix ID, float data
 8 = read speicfic cell
    arg - m, n, matrixID
 16 = delete matrix
    args - matrix ID


*/

struct RPCPacket {
    public:
        union {
            struct {
                uint32_t startOfHeader = RPC_SOH;
                uint32_t procArgsLength;
                uint16_t procArgsChecksum;
                uint8_t procId;
                
            };

            struct {
                uint8_t header[11];
            };
        };

        uint16_t headerChecksum;
        uint8_t args[MAX_ARGS_SIZE]; // byte array of args  // ask madeline will this not send over the whole array each time? or can i use my sending command to cut off after (that makes sense)
        //matrix struct

        RPCPacket(){
            procArgsLength = 0;
            procArgsChecksum = 0;
            procId = 0;

            headerChecksum = 0;
        };

        RPCPacket(uint8_t* data); // create a packet from formated pointer

        RPCPacket(uint8_t id, uint32_t argsLength, uint8_t* args); // create a packet from formatted args

        RPCPacket(int8_t id, uint16_t matrix_id,  uint16_t m, uint16_t n, bool memory, Matrix * matrix); // create packet from base args

        bool isValid();    // checks procId and checksums
};


struct ResponsePacket {
    public:
        union {
            struct {
                uint32_t startOfHeader = RPC_SOH;
                uint16_t responseId;
                uint16_t payloadSize;
                uint16_t payloadChecksum;
                                
            };

            struct {
                uint8_t header[10];
            };

        };

        uint16_t headerChecksum;
        uint8_t *payload;

        ResponsePacket(){
            payloadSize = 0;
            payloadChecksum = 0;
            headerChecksum = 0;
            responseId = 0;
            payload = nullptr;
        };

        ResponsePacket(uint8_t* data);

        ResponsePacket(uint16_t _responseId,  uint16_t _payloadSize,  uint8_t* _payload);

        uint16_t serialize(uint8_t* output);

        bool isValid();
};


#define SEARCHING_FOR_SOH 0
#define HEADER 1
#define HEADER_CHECKSUM 2
#define PAYLOAD 3



struct RPCParser {
    RPCParser();
    void init();
    bool processByte(uint8_t);

    int state;  
    uint32_t soh;
    RPCPacket inputPacket;
    RPCPacket completedPacket;
    size_t position;
};


struct ResponseParser {
    ResponseParser();
    void init();
    bool processByte(uint8_t);

    int state;  
    uint32_t soh;
    ResponsePacket inputPacket;
    ResponsePacket completedPacket;
    size_t position;
};

