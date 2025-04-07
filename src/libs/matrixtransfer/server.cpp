#include "matrixTransfer.h"
#include "server.h"
#include "mbed.h"
#include <unordered_set>


Receive::Receive()
{
    parser = Parser();
    serial = Serial(); //find pins
}

void Receive::init()
{
    parser.init();
}

// BufferedSerial xtend = BufferedSerial(SENS_PIN_XTEND_TX, SENS_PIN_XTEND_RX, XTEND_BAUD_RATE);
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

 32 - response -


*/

void Receive::receiveCommands()
{
    uint8_t buf[MAX_SERIALBUFFER_SIZE]; // find good size
    int16_t size = 0;
    if (serial.readable())
    {
        size = serial.read(buf, MAX_SERIALBUFFER_SIZE);
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

                RPCPacket inputPacket = parser.completedPacket; 

                uint8_t * responseArgs; //(uint8_t*) inputPacket.headerChecksum; // does this work allocate before hand
                uint16_t responseLength = 2;
                RPCPacket responsePacket;

                switch (inputPacket.procId)
                {
                case 1:
                    float * data;
                    data = (float*) malloc(2 *sizeof(float));
                    Matrix * matrix = new Matrix(0, 0, 0, data);

                    uint8_t * responseArgs = ( uint8_t *) malloc(responseLength);
                    //code for chekcsum response
                    writeMatrix ( inputPacket.args[0], matrix);
                    break;
                case 2:

                    Matrix * matrix;
                    readMatrix((uint16_t)inputPacket.args[0], matrix, responseArgs +2);
                case 4:
                    deleteMatrix((uint16_t)inputPacket.args[0]);
                case 8:
                    writeCell( (uint16_t) inputPacket.args[0], (uint16_t) inputPacket.args[2], (uint16_t) inputPacket.args[4], (float) inputPacket.args[6] );
                case 16:
                    responseLength = readCell( (uint16_t) inputPacket.args[0], (uint16_t) inputPacket.args[2], (uint16_t) inputPacket.args[4], responseArgs +2);
                }

                responsePacket = RPCPacket(inputPacket.procId, responseLength, responseArgs);

                sendResponse(responsePacket);

            }
        }
    }
}

void Receive::sendResponse(RPCPacket packet){

    serial.write(packet.header, 8);
    serial.write((uint8_t*) packet.headerChecksum, 2); // double check this works
    serial.write((uint8_t*) packet.headerChecksum, 1);
    serial.write(packet.args, packet.procArgsLength);


}



void Receive:: writeMatrix(bool memory, Matrix * matrix){
    size_t size = 0;

    if(!memory){
        matrices.emplace(matrix);  // will this segfault
    }else{
        printf("No permament memory yet");
    }

};

void Receive::deleteMatrix(uint16_t matrixID){
    matrices.erase(Matrix(matrixID, 0, 0, nullptr)); // check this shit cause chat wrote it

}



void Receive::writeCell(uint16_t matrixID, uint16_t m, uint16_t n, float value){

    //look through permanent memory - not implemented 

    auto it = matrices.find(Matrix(matrixID,0,0, nullptr));  // Find the element with matching id

    if (it != matrices.end()) {
        Matrix matrix = *it;
        
        matrix.writeCell(m, n, value);  

        matrices.erase(it);

        matrices.emplace(matrix);

    
    }
    // error handling if bounds are off


}    // diff for if permanement or not -- does it need memory or not

uint16_t Receive::readCell(uint16_t matrixID, uint16_t m, uint16_t n, uint8_t * args){

    auto it = matrices.find(Matrix(matrixID,0,0, nullptr));  // Find the element with matching id

    if (it != matrices.end()) {
        float value = it->readCell(m, n);  
        args[0] = value; // figure out how to set that

        // add confirm flag

        return sizeof(float);
    }else{
        args[0] = 53; // not found FLag
        return 3; //size of flag
    }

     //


};



uint16_t Receive::readMatrix(uint16_t matrixID, Matrix * matrix, uint8_t * args){


    auto it = matrices.find(Matrix(matrixID,0,0, nullptr));  // Find the element with matching id

    float value;

    if (it != matrices.end()) {
        args = (uint8_t*) it; // figure conversion out

    }else{
        // look trhough permanent memory
        args[0] = 53; // not found FLag
        return 3; //size of flag
    }

    args[0] = value; // figure out how to set that

    return sizeof(float);




}
