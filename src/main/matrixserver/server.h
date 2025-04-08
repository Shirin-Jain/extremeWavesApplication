#include "matrixTransfer.h"
#include "mbed.h"
#include <unordered_map>

struct Server
{

    Server(PinName Tx, PinName Rx, uint32_t baud);
    void init();

    std::unordered_map<uint16_t, Matrix *> matrices;

    void waitForCommands();
    void sendResponse(Packet *packet);

    bool writeMatrix(uint8_t *data);
    bool writeCell(uint8_t *data); // diff for if permanement or not -- does it need memory or not
    bool readMatrix(uint8_t *data, Matrix *&matrix);
    bool readCell(uint8_t *data, float &value);
    bool deleteMatrix(uint8_t *data);

    Parser parser;
    BufferedSerial serial;
    // could need a variable for permanent memory management;
    // need a data structure to store matrixes
};