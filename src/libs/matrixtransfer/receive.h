#include "matrixTransfer.h"

struct receiveRPC{

    receiveRPC();


    void receiveCommands();

    void writeMatrix(Matrix matrix, bool memory);
    void writeCell(uint16_t matrixID, uint16_t m, uint16_t n, float value);    // diff for if permanement or not -- does it need memory or not

    Matrix readMatrix(uint16_t matrixID);
    float readCell(uint16_t matrixID, uint16_t m, uint16_t n);

    bool deleteMatrix(uint16_t matrixID);



    RPCParser parser;
    // could need a variable for permanent memory management;
    // need a data structure to store matrixes


};