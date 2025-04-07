#include <cstring>

#include "matrixTransfer.h"

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




Packet::Packet(uint16_t _id, uint32_t _payloadLength, uint8_t* _payload){
    id = _id;

    payloadLength = _payloadLength;
    payloadChecksum = fletcher16(_payload, _payloadLength);

    headerChecksum = fletcher16(header, 11);
    payload = _payload;    
}


bool Packet::isValid(){
    return (fletcher16(header, sizeof(header)) == headerChecksum)  && (fletcher16(payload, payloadLength) == payloadChecksum);
}




Parser::Parser(){
    init();
}

void Parser::init(){
    completedPacket = inputPacket;
    state = SEARCHING_FOR_SOH;
    soh = 0;
    inputPacket = Packet();
    position = 0;
}


bool Parser::processByte(uint8_t byte){
    if(state == SEARCHING_FOR_SOH){
        soh = soh >> 8;
        soh |= (byte << 24);

        if(soh == SOH){
            state = HEADER;
            memcpy(inputPacket.header, &soh, sizeof(soh));
            position = sizeof(soh);
        }
    }

    else if (state == HEADER){
        inputPacket.header[position] = byte;
        position++;

        if(position >= sizeof(Packet::header)){
            state = HEADER_CHECKSUM;
            position = 0;
        }
    }

    else if (state == HEADER_CHECKSUM){
        ((uint8_t*)(&inputPacket.headerChecksum))[position] = byte;
        position++;
        if(position == sizeof(inputPacket.headerChecksum)){
            uint16_t redo_checksum = fletcher16(inputPacket.header, sizeof(Packet::header));

            if(redo_checksum == inputPacket.headerChecksum){
                if (inputPacket.payloadLength == 0) {
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

        position++;

        if(position >= inputPacket.payloadLength){
            init();
            return true;
        }

        
    }
    return false;
}




