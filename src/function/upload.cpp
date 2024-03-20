#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SimpleDHT.h> 
#include <vector>
#include "function.hpp"

char url[] = "";
int pinDHT11 = 33; //DHT11 GPIO
SimpleDHT11 dht11;

bool upload::update(std::vector<float> datas, int id) {
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    err = dht11.read(pinDHT11, &temperature, &humidity, NULL);
    if (err != SimpleDHTErrSuccess) {
        Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
    }
    HTTPClient http;
    StaticJsonDocument<1000> data;
    char body[150];
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    //set data
    data["temp"] = (int)temperature;
    data["humid"] = (int)humidity;
    data["soil_temp"] = rs485::round485((float)datas[1]);
    data["soil_humid"] = rs485::round485((float)datas[0]);
    data["soil_ph"] = rs485::round485((float)datas[3]);
    data["soil_N"] = datas[4];
    data["soil_P"] = datas[5];
    data["soil_K"] = datas[6];
    data["id"] = id;
    serializeJson(data, body);
    Serial.println(body);
    Serial.println("---------1--------");
    int httpCode = http.POST(body);
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        if (!payload.compareTo("ok")) return true;
        else return false;
    }
    return false;
    http.end();
}
    
    
    