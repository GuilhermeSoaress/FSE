#include"sMQTTBroker.h"
#include <WiFi.h>

const char* ssid     = "YAN";
const char* password = "04102002";
const char* host     = "example.com";
const char* url      = "/index.html";

//IPAddress local_IP(192, 168, 31, 115);
//IPAddress gateway(192, 168, 31, 1);
//IPAddress subnet(255, 255, 0, 0);
sMQTTBroker broker;

void setup()
{
    pinMode(4,INPUT_PULLUP);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(1000);
    }
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    
    const unsigned short mqttPort=1883;
    broker.init(mqttPort);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4,1);
    // all done
}
void loop()
{
    while(digitalRead(4)==1){
      broker.update();

    }
    Serial.print("Going to sleep now");
    esp_deep_sleep_start();
}
