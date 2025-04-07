#include "matrixTransfer.h"
#include "mbed.h"
#include <unordered_set>


struct Receive{

    Receive();
    void init();
    


    void receiveCommands();
    void sendResponse(RPCPacket packet);

    void writeMatrix(bool memory, Matrix * matrix );
    void writeCell(uint16_t matrixID, uint16_t m, uint16_t n, float value);    // diff for if permanement or not -- does it need memory or not
    uint16_t readMatrix(uint16_t matrixID, Matrix * matrix, uint8_t * args);
    uint16_t readCell(uint16_t matrixID, uint16_t m, uint16_t n, uint8_t * args);
    void deleteMatrix(uint16_t matrixID);



    Parser parser;
    BufferedSerial serial;
    std::unordered_set<Matrix, MatrixHash> matrices;

    // could need a variable for permanent memory management;
    // need a data structure to store matrixes


};