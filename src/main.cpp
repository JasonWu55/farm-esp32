//#include <Arduino.h>
#include <string.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ModbusMaster.h>
#include <vector>
#include "function/function.hpp"
//#include <Husarnet.h>
#ifdef U8X8_HAVE_HW_SPI
//#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// For ESP32 hardware serial use these lines instead
#include <HardwareSerial.h>
HardwareSerial SerialAT(2);


// -- SEASTO GPIO ---------------------------------
#define GPIO_LED2     2
#define GPIO_LED3    23
#define GPIO_SSD_SDA 21   // I2C SDA
#define GPIO_SSD_SCL 22   // I2C SCL
#define GPIO_485_ROR  18   //ESP32_RX1
#define GPIO_485_DIR  19   //ESP32_TX1
//#define GPIO_GSM_TX  32   // ESP32 RX2
//#define GPIO_GSM_RX  33   // ESP32 TX2
//#define GPIO_FAN     13

// -- SSD1306 -------------------------------------
#define SSD_ADDRESS    0x3c
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH  128
#define DHTTYPE DHT11
std::vector<float> datas_1, datas_3;
ModbusMaster node1;
ModbusMaster node3;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
char ssid[] = "";
char password[] = "";
const char* PARAM_MESSAGE = "action";
//const char *hostName = "esp32-webserver";
//const char *husarnetJoinCode = "fc94:b01d:1803:8dd8:b293:5c7d:7639:932a/gonjGRzpKJ4NZVainC2rt5";
AsyncWebServer server(8080);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


unsigned long lastHr = 0;
unsigned long timerDelay = 1;
String param;

void run();

void setup() {
  	Serial.begin(115200);
  	Serial1.begin(9600, SERIAL_8N1, GPIO_485_ROR, GPIO_485_DIR);

  	node1.begin(1, Serial1);
  	node3.begin(3, Serial1);
  
  	pinMode(GPIO_LED2, OUTPUT);
	digitalWrite(GPIO_LED2, LOW);
	pinMode(GPIO_LED3, OUTPUT);
	digitalWrite(GPIO_LED3, LOW);

  //顯示中文初始化
	u8g2.begin();
	u8g2.enableUTF8Print();
	u8g2.setFont(u8g2_font_unifont_t_chinese1);  // use chinese2 for all the glyphs
	u8g2.firstPage();
	do {
		u8g2.setCursor(0, 15);
		u8g2.print("Connecting to WiFi");
		u8g2.setCursor(0, 35);
		u8g2.print(ssid);
	} while (u8g2.nextPage());
	delay(1000);

	WiFi.mode(WIFI_STA); //設為無線網卡
	//2.啟動WiFi連線
	WiFi.disconnect();
	delay(1000);
	while (WiFi.status() != WL_CONNECTED) {
		WiFi.begin(ssid, password);
		delay(1000);
	}

	u8g2.firstPage();
	do {
		u8g2.setCursor(0, 15);
		u8g2.print("Connected to WiFi");
		u8g2.setCursor(0, 35);
		u8g2.print(WiFi.localIP());
	} while (u8g2.nextPage());
	digitalWrite(GPIO_LED3, HIGH);
	delay(1000);
	// Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT -1 = -3600
	// GMT 0 = 0
	timeClient.setTimeOffset(28800);
	digitalWrite(GPIO_LED2, HIGH);
		//time update.
	while (!timeClient.update()) {
		timeClient.forceUpdate();
	}
	digitalWrite(GPIO_LED2, LOW);
	delay(3000);


	//web server
	//Husarnet.join(husarnetJoinCode, hostName);
	//Husarnet.start();

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", "Hello world!!!");
	});
	server.on("/data", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message="";
		param = "";
        if (request->hasParam(PARAM_MESSAGE)) {
            param = request->getParam(PARAM_MESSAGE)->value();
			if (param == "get") {
				Serial.println("---------1--------");
				datas_1 = rs485::read(node1, 0, 7);
				Serial.println("---------1--------");
				delay(2000);
				message+="---------1--------\n";
				for (float i: datas_1) {
					String d = String(std::to_string(i).c_str());
					message+=d+"\n";
				}
				message+="---------1--------\n";
				Serial.println("---------3--------");
				datas_3 = rs485::read(node3, 0, 7);
				delay(2000);
				message+="---------3--------\n";
				for (float i: datas_3) {
					String d = String(std::to_string(i).c_str());
					message+=d+"\n";
				}
				message+="---------3--------\n";
				Serial.println("---------3--------");
				request->send(200, "text/plain", message);
			} else if (param == "set") {
				message = "ok";
				request->send(200, "text/plain", message);
			} else if (param == "reset") {
				message = "ok";
				request->send(200, "text/plain", message);
				ESP.restart();
			} else {
				message = "Illegal action";
				request->send(200, "text/plain", message);
			}
        } else {
            message = "No action";
			request->send(200, "text/plain", message);
        }
    });
	server.begin();
	run();
	lastHr = millis()/3600000;
}

void loop() {
	
	if (param == "set") {
		delay(5000);
		run();
		param="";
	}
	
	
	if ((millis()/3600000 - lastHr) >= timerDelay) {
		run();
		lastHr = millis()/3600000;
	}
}

void run() {
	Serial.println("---------1--------");
	datas_1 = rs485::read(node1, 0, 7);
	Serial.println("---------1--------");
	delay(5000);
	Serial.println("---------3--------");
	datas_3 = rs485::read(node3, 0, 7);
	Serial.println("---------3--------");

	if (upload::update(datas_1, 1) && upload::update(datas_3, 3)) {
		u8g2.firstPage();
		do {
			u8g2.setCursor(0, 15);
			u8g2.print("ok");
			u8g2.setCursor(0, 35);
			u8g2.print(WiFi.localIP());
		} while (u8g2.nextPage());
	}
}


