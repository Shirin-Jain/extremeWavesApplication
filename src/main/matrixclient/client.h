#include "matrixTransfer.h"
#include "serialib.h"
#include <string>
 
 
 // send command
 // get from terminal - easy structure
 // convert scanf
 // wait fro response
 // display response

 struct Client{

    Client(std::string port, uint32_t baud);
    void init();

    
    uint8_t commandId;
    uint16_t commandChecksum;

    //ResponseParser parser; 

    bool askForCommand(); // ask for command and send response

    void sendResponse(Packet *packet);

    void waitResponse(); // wait for response and print it, if time out say timed out;

    // main is just ask for command then wait repsonse;

    serialib serial;
    Parser parser;
 };