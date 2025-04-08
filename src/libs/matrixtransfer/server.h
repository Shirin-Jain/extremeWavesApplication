#include "matrixTransfer.h"
#include "mbed.h"
#include <unordered_set>


struct Server{

    Server();
    void init();


    std::unordered_set<Matrix, MatrixHash> matrices;

    
    void waitForCommands();
    void sendResponse(Packet *packet);



    bool writeMatrix(uint8_t* data );
    bool writeCell(uint8_t* data );    // diff for if permanement or not -- does it need memory or not
    bool readMatrix(uint8_t* data, const Matrix * matrix );
    bool readCell(uint8_t* data,  float * value);
    bool deleteMatrix(uint8_t* data );



    Parser parser;
    BufferedSerial serial;
    // could need a variable for permanent memory management;
    // need a data structure to store matrixes


};