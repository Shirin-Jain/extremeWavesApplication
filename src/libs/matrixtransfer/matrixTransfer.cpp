#include <cstring>

#include "matrixTransfer.h"

#define RPC_SOH                             0x4E4C444D
#define RESPONCE_SOH                        0x4E4C444D //change
#define MAX_ARGS_SIZE                       0xFFFF  // need to change to be reasonable


uint16_t fletcher16(const uint8_t* data, uint64_t length){
    uint32_t c0, c1;

    /*  Found by solving for c1 overflow: */
    /* n > 0 and n * (n+1) / 2 * (2^8-1) < (2^32-1). */
    for (c0 = c1 = 0; length > 0; ) {
        uint64_t blocklen = length;
        if (blocklen > 5802) {
            blocklen = 5802;
        }
        length -= blocklen;
        do {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        } while (--blocklen);
        c0 = c0 % 255;
        c1 = c1 % 255;
    }
    return (c1 << 8 | c0);
}


uint32_t copyMatrix(uint8_t* destination, Matrix* matrix){ // could be done during send command
    destination[0] = matrix -> matrixID;
    destination[1] = matrix -> m;
    destination[2] = matrix -> n;

    uint32_t matrixSize = matrix->m * matrix->n;
    memcpy(destination, matrix->data, matrixSize *4); // caust its a float


    return matrixSize + 6; // size of whole thing
}



/*
format
    header:
        uint32_t startOfHeader = RPC_SOH;
        uint32_t procArgsLength;
        uint16_t procArgsChecksum;
        uint8_t procId;
    payload:
        uint16_t headerChecksum;
        uint8_t args[MAX_ARGS_SIZE];

*/

/*
 procID (could be 1 hot coded)
1  , 2, 4, 8, 16 

0 =  nothing

 1 = write Matrix
    args - matrix struct + size (?)
 2 = read Matrix
    args - matrix ID
4 = delete matrix
    args - matrix ID
 8 = write to specific cell
    args -matrix ID, m, n,  float data - in data*
 16 = read speicfic cell
    arg - matrix ID, m, n,
            8 byts, 8 bytes, 16, byt


*/

//ask madeline about memory issues - current plan is code that calls will allocate the memory;


RPCPacket::RPCPacket(uint8_t* _data){ //size of args
    memcpy(header, _data, sizeof(header));
    uint32_t argsLength = _data[4]; // does this work
    memcpy(args, _data + 11, argsLength);

}

RPCPacket::RPCPacket(uint8_t id, uint32_t argsLength, uint8_t* _args){
    procArgsLength = argsLength;
    procArgsChecksum = fletcher16(_args, argsLength);
    procId = id;

    headerChecksum = fletcher16(header, 11);
    memcpy(args, _args, argsLength);
    
}

RPCPacket::RPCPacket(int8_t id, uint16_t matrix_id,  uint16_t m, uint16_t n, bool memory, Matrix * matrix){

    uint8_t* args;
    uint32_t argsLength = 0; // maybe add error code if procID is worng id

    if(id == 1){
        args[0] = 1;   
        argsLength = copyMatrix(args +1, matrix) + 1;
    }

    args[0] = matrix_id;
    argsLength = 2;

    if( id == 8 || id == 16 ){
        args[2] = matrix_id;
        args[4] = matrix_id;
        argsLength = 6;
    }

    RPCPacket(id, argsLength, args);


}



bool RPCPacket::isValid(){
    return (procId > 0 && procId <= 16)  &&  (fletcher16(header, 11) == headerChecksum)  && (fletcher16(args, procArgsLength) == procArgsChecksum);
}



ResponsePacket::ResponsePacket(uint16_t _responseId,  uint16_t _payloadSize,  uint8_t* _payload){
    responseId = _responseId;
    payloadSize = _payloadSize;
    payloadChecksum = fletcher16(payload, payloadSize); //check for null ptrs


    headerChecksum = fletcher16(header, 10);
    payload = _payload; // during send command will this be copied over
}


//response packet
/*
    should it return a confirmation (not every rpc has a response) - if so what (nonce/ checksum/ repeat of command idk/ procID)

    write/ delete - no necessary response - could have a confirmation

    read Whole matrix - responds with whole amtrix

    read one value - responds witha  float

    should probably repsond with procID so it knows whats up
    confirmation maybe - procID + args checksum


*/

/*


        create parsers for both packets
*/

RPCParser::RPCParser(){
    init();
}

void RPCParser::init(){
    completedPacket = inputPacket;
    state = SEARCHING_FOR_SOH;
    soh = 0;
    inputPacket = RPCPacket();
    position = 0;
}


bool RPCParser::processByte(uint8_t byte){
    if(state == SEARCHING_FOR_SOH){
        soh = soh >> 8;
        soh |= (byte << 24);

        if(soh == RPC_SOH){
            state = HEADER;
            memcpy(inputPacket.header, &soh, sizeof(soh));
            position = sizeof(soh);
        }
    }

    else if (state == HEADER){
        inputPacket.header[position] = byte;
        position++;

        if(position >= sizeof(RPCPacket::header)){
            state = HEADER_CHECKSUM;
            position = 0;
        }
    }

    else if (state == HEADER_CHECKSUM){
        ((uint8_t*)(&inputPacket.headerChecksum))[position] = byte;
        position++;
        if(position == sizeof(inputPacket.headerChecksum)){
            uint16_t redo_checksum = fletcher16(inputPacket.header, sizeof(RPCPacket::header));

            if(redo_checksum == inputPacket.headerChecksum){
                if (inputPacket.procArgsLength == 0) {
                    init();
                    return true;
                }

                state = PAYLOAD;
                position = 0;
            }
            else {
                init();
            }
        }
    }

    else if (state == PAYLOAD) {
        inputPacket.args[position] = byte;

        position++;

        if(position >= inputPacket.procArgsLength){
            init();
            return true;
        }

        
    }
    return false;
}



ResponseParser::ResponseParser(){
    init();
}

void ResponseParser::init(){
    completedPacket = inputPacket;
    state = SEARCHING_FOR_SOH;
    soh = 0;
    inputPacket = ResponsePacket();
    position = 0;
}



bool ResponseParser::processByte(uint8_t byte){
    if(state == SEARCHING_FOR_SOH){
        soh = soh >> 8;
        soh |= (byte << 24);

        if(soh == RPC_SOH){
            state = HEADER;
            memcpy(inputPacket.header, &soh, sizeof(soh));
            position = sizeof(soh);
        }
    }

    else if (state == HEADER){
        inputPacket.header[position] = byte;
        position++;

        if(position >= sizeof(ResponsePacket::header)){
            state = HEADER_CHECKSUM;
            position = 0;
        }
    }

    else if (state == HEADER_CHECKSUM){
        ((uint8_t*)(&inputPacket.headerChecksum))[position] = byte;
        position++;
        if(position == sizeof(inputPacket.headerChecksum)){
            uint16_t redo_checksum = fletcher16(inputPacket.header, sizeof(ResponsePacket::header));

            if(redo_checksum == inputPacket.headerChecksum){
                if (inputPacket.payloadSize == 0) {
                    init();
                    return true;
                }

                state = PAYLOAD;
                position = 0;
            }
            else {
                init();
            }
        }
    }

    else if (state == PAYLOAD) {
        inputPacket.payload[position] = byte;

        if(position >= inputPacket.payloadSize){
            ///send packet or something or return true
            init();
            return true;
        }

        position++;
    }
    return false;
}








  


//next steps

/*
basic rexeive structure from stm32 

needs to receive it, figure out where to store (might have to be part of command), send response back

*/

/*

send structure from laptop, just sned  a couple commands, if time turn into nice UI


*/



      



