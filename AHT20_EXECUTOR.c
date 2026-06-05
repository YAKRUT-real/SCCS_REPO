#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <SPI.h>
#include <AHT20.h>
AHT20 aht20;
#define TOOL D5
int id = 1; //0 - cooler, 1 - heater, 2 - humidifier, 3 - dryer
//A0 20 A6 11 C7 33
//8C AA B5 53 20 5A the usb slave
//Intrusion adress: 40 F5 20 32 CB ED
//0x40, 0xF5,0x20,0x32,0xCB,0xED
//0xB4, 0xE6, 0x2D, 0x15, 0x03, 0xD7
uint8_t broadcastAddress[] = {0x40, 0xF5,0x20,0x32,0xCB,0xED};
struct struct_message {
  double ts;
  double press;
  int executor_id;
} myData;

struct updata {
  double ts;
  double hum;
  int adolf;
};
struct MAc{
  uint8_t mac[6];
} mastermac, borderlinemac;
updata receivedValue = {30,60};
updata receivedData = {30,60};

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0) {
    Serial.println("Успешно отправлено!");
  } else {
    Serial.println("Ошибка отправки!");
  }
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(receivedData)) {
    memcpy(&receivedValue, incomingData, len);
    receivedData = receivedValue;
    myData.executor_id = receivedValue.adolf;
    Serial.print("теперь мой адрес ");
    Serial.println(myData.executor_id);
    
  }//8C:AA:B5:53:20:5A

  else
  {
    memcpy(&borderlinemac, incomingData, len);
    mastermac = borderlinemac;
    esp_now_del_peer(broadcastAddress);
    for(int i = 0;i<6;++i){broadcastAddress[i]=mastermac.mac[i];}
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    for(int i = 0; i<6; i++){Serial.println(broadcastAddress[i]);}
  }
}

void setup() {
  myData.executor_id=7;
  Wire.begin();
  aht20.begin();
  pinMode(TOOL, OUTPUT);
  digitalWrite(TOOL,HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println(WiFi.macAddress());
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0) != 0) {
    return;
  }
  delay(1000);

}

void loop() {
  for(int i = 0; i<6; i++){Serial.println(broadcastAddress[i]);}
  Serial.println(myData.executor_id);
  float tem = aht20.getTemperature();
  float hum = aht20.getHumidity();
  myData.press = hum;
  myData.ts = tem;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  delay(5000);
  if(tem+0.5<receivedData.ts && id == 0)
    digitalWrite(TOOL,HIGH);
  else if(tem>receivedData.ts-1.0 && id == 0)
    digitalWrite(TOOL,LOW);
  else if(tem+1.0<receivedData.ts && id == 1)
    digitalWrite(TOOL,LOW);
  else if(tem>receivedData.ts-0.5 && id == 1)
    digitalWrite(TOOL,HIGH);

}
