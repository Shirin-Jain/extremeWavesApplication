#include <cstdint>
#include <functional>


#define SOH                             0x4E4C444D

#define SEARCHING_FOR_SOH 0
#define HEADER 1
#define HEADER_CHECKSUM 2
#define PAYLOAD 3

#define WRITE_MATRIX 1  // mem flag, matrix struct
#define READ_MATRIX 2 // matrix id
#define DELETE_MATRIX 4 // matrix id
#define WRITE_CELL 8 // matrix id, m, n, value
#define READ_CELL 16 // matrix id, m, n,

#define PEMRANENT_MEMORY_FLAG 1
#define VOLATILE_MEMORY_FLAG 2

#define SUCCESS_FLAG 1
#define FAIL_FLAG 2

#define MAX_SERIAL_BUFFER 1024
#define BAUD_RATE 115200

uint16_t fletcher16(const uint8_t* data, uint64_t length);



//matrix max dimenion is 127 * 127


struct Matrix {

    uint16_t matrixID;
    uint16_t m;
    uint16_t n;
    float* data;

    Matrix(uint16_t _matrixID, uint16_t _m, uint16_t _n, float * _data){
        matrixID = _matrixID;
        m = _m;
        n = _n;
        data = _data;

    };

    float * readCell (uint16_t _m, uint16_t _n) const{

        if(_m < m && _n < n){
            return data + (_m *(n-1) + _n);
        }

        return nullptr;

    };


    void writeCell(uint16_t _m, uint16_t _n, float value){

        if(_m < m && _n < n){
            data[ _m *(n-1) + _n] = value;  // double chek this
        }
        
        return;

    };


    bool operator==(const Matrix& other) const {
        return matrixID == other.matrixID;
    }
    
};


struct MatrixHash {
    std::size_t operator()(const Matrix& m) const {
        return std::hash<int>{}(m.matrixID);  
    }
};

struct Packet{
    
    union {
        struct {
            uint32_t startOfHeader = SOH;
            uint16_t payloadLength;
            uint16_t payloadChecksum;
            uint16_t id;      // either procedur or procedure checksum  
        };

        struct {
            uint8_t header[10];
        };
    };

    uint16_t headerChecksum;
    uint8_t* payload; // byte array of args 

    Packet(){
        payloadLength = 0;
        payloadChecksum = 0;
        id = 0;
        
        headerChecksum = 0;
    };

    Packet(uint8_t* _data){
        memcpy(header, _data, sizeof(header));
        payload = _data +11;
    }; // create a packet from formated pointer

    Packet(uint16_t _id, uint32_t _payloadLength, uint8_t* _payload); // create a packet from formatted args
    bool isValid();


};


struct Parser {
    Parser();
    void init();
    bool processByte(uint8_t);

    int state;  
    uint32_t soh;
    Packet inputPacket;
    Packet completedPacket;
    size_t position;
};




