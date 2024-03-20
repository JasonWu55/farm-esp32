#include <ModbusMaster.h>
#include <vector>
//#include <Arduino.h>
#include "function.hpp"

//ModbusMaster node;
uint8_t r;
uint8_t r1;
std::vector<float> data_485(7);

float rs485::round485(float a) {
    return ((int)((a * 100) + 0.5) / 100.0);
}

std::vector<float> rs485::read(ModbusMaster &node, int highh, int loww) {
    r = node.readInputRegisters(0, 7);
    if (r == node.ku8MBSuccess) {
        for (int i=0; i<7; i++) {
            float a=1.0;
            if (i==0 || i==1 || i==3) a=10.0;
            Serial.println(node.getResponseBuffer(i)/a);
            data_485[i] = (float)node.getResponseBuffer(i)/a;
        }
    }
    node.readInputRegisters(0x07D0, 1);
    Serial.println(node.getResponseBuffer(0));
    return data_485;
}

