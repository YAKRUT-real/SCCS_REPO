//A0 20 A6 11 C7 33 - slave, DHT_22. weak.
//4apr. DD.
//22apr - DEADLINE.
//E8:68:E7:C7:DC:D0 - microusb slave
//0x40, 0xF5,0x20,0x32,0xCB,0xED - wtf, is it me??
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#define EEPROM_SIZE 512
#define MACS_START 0
#define TOOLS_ADDR 100 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define UP D5
#define SELECT D6
#define DOWN D7
unsigned char tools;
typedef struct received_struct {
  double ts;
  double press;
  int executor_id;
} received_struct;

struct MAc{
  uint8_t mac[6];
} mymac;

typedef struct updata {
  double ts;
  double hum;
  int adolf;
} send_struct;

received_struct receivedValue;
received_struct receivedData;
updata myMessage;
int ind = 0;
unsigned char joint = 0;
unsigned char chosen = 0;
short int screen = 0;
short int temperature = 24;
short int humidity = 80;
short int min_temp = -99;
short int max_temp = 99;
unsigned char capt = 3;
unsigned long lastActionTime = 0;
const unsigned long timeout = 9000;
bool flag = false;
bool send = false;
bool motivation = false;
uint8_t broadcastAddress1[] = {0x40, 0xF5, 0x20, 0x32, 0xCB, 0xED};
uint8_t broadcastAddress2[] = {0x8C, 0xAA, 0xB5, 0x53, 0x20, 0x5A};
uint8_t broadcastAddress3[] = {0xE8,0x68,0xE7,0xC7,0xDC,0xD0};
uint8_t broadcastAddress4[] = {0xA0, 0x20, 0xA6, 0x11, 0xC7, 0x33};
uint8_t broadcastAddress5[] = {0x40, 0xF5, 0x20, 0x32, 0xCB, 0xED};
uint8_t broadcastAddress6[] = {0xA0, 0x20, 0xA6, 0x11, 0xC7, 0x33};
uint8_t* broadcastAddress[] = {broadcastAddress1, broadcastAddress2,
                              broadcastAddress3, broadcastAddress4,
                              broadcastAddress5, broadcastAddress6};


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void saveMACs() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(TOOLS_ADDR, tools);
    for (int i = 0; i < tools; i++) {
        int addr = MACS_START + i * 6;
        for (int j = 0; j < 6; j++) {
            EEPROM.write(addr + j, broadcastAddress[i][j]);
        }
    }
    EEPROM.commit();
    EEPROM.end();
}

void loadMACs() {
    EEPROM.begin(EEPROM_SIZE);
    tools = EEPROM.read(TOOLS_ADDR);
    if (tools == 0xFF || tools > 6) {
        tools = 3;
        EEPROM.end();
        return;
    }
    for (int i = 0; i < tools; i++) {
        int addr = MACS_START + i * 6;
        for (int j = 0; j < 6; j++) {
            broadcastAddress[i][j] = EEPROM.read(addr + j);
        }
    }
    EEPROM.end();
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(receivedData)) {
    received_struct temp;
    memcpy(&temp, incomingData, len);
    if (temp.executor_id == chosen) {
      receivedData = temp;
      flag = true;
    }
  }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Статус отправки: ");
  if (sendStatus == 0) {
    Serial.println("Успешно");
  } else {
    Serial.println("Ошибка");
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  switch(screen) {
    case 0: 
      myMessage.adolf = chosen;
      display.print("=== DATA === TOOL:");
      display.print(chosen);
      display.println(receivedData.executor_id);
      if(receivedData.executor_id==chosen && chosen>=1)
      {
      display.print("TEMP: ");
      display.print(receivedData.ts, 1);
      display.println(" C");
      display.print("HUM:  ");
      display.print(receivedData.press);
      display.print(" %");
      display.setCursor(0, 30);
      }
      else if(receivedData.executor_id==chosen && chosen==0)
      {
        display.print("TEMP: ");
        display.print(receivedData.ts, 1);
        display.println(" C");
        display.print("PRES: ");
        display.print(receivedData.press);
        display.println(" hPa");
        display.setCursor(0, 30);
      }
      display.println("UP - Update data");
      display.println("DOWN - Change tool");
      display.setCursor(0,50);
      display.println("SELECT - Menu");
      break;
      
    case 1:
      display.println("=== MENU ========");
      display.println("");
      if (capt == 0)
      {
        display.println("-> TEMP");
        display.println("   HUM");
        display.println("   ADD");
      }
      else if(capt == 1)
      {
        display.println("   TEMP");
        display.println("-> HUM");
        display.println("   ADD");
      }
      else if(capt == 2)
      {
        display.println("   TEMP");
        display.println("   HUM");
        display.println("-> ADD");
      }
      else if(capt>2)
      {
        switch(capt)
        {
          case 3:
          display.println("-> DELETE");
          display.println("   RETURN");
          display.println("   SCCS");
          break;
          case 4:
          display.println("   DELETE");
          display.println("-> RETURN");
          display.println("   SCCS");
          break;
          case 5:
          display.println("   DELETE");
          display.println("   RETURN");
          display.println("-> SCCS");
          break;
          case 255:
          display.println("   DELETE");
          display.println("   RETURN");
          display.println("-> SCCS");
          capt = 5;
          break;
        }
      }
      display.setCursor(0, 45);
      display.println("UP/DOWN: Choose");
      display.println("SELECT: Setup.");
      break;
      
    case 2: 
      display.println("Redacting");
      display.println("Temp:");
      display.setTextSize(2);
      display.print("  ");
      display.print(temperature);
      display.println(" C");
      display.setTextSize(1);
      display.setCursor(0, 45);
      display.println("UP: +1  DOWN: -1");
      display.println("SELECT: Back");
      break;
      
    case 3:
      display.println("Redacting");
      display.println("Hum:");
      display.setTextSize(2);
      display.print("  ");
      display.print(humidity);
      display.println(" %");
      display.setTextSize(1);
      display.setCursor(0, 45);
      display.println("UP: +1  DOWN: -1");
      display.println("SELECT: Back");
      break;
      
    case 4:
      display.println("Sending data");
      display.println("Wait...");
      display.print("Temp: ");
      display.print(myMessage.ts);
      display.println(" C");
      display.print("Hum: ");
      display.print(myMessage.hum);
      display.println(" %");
      display.display();
      delay(2000);
      screen = 0;
      updateDisplay();
      break;
  }
  display.display();
}

bool buttonPressed(int pin) {
  static unsigned long lastPressTime = 0;
  static bool lastState = HIGH;
  
  bool currentState = digitalRead(pin);
  bool pressed = false;
  
  if (currentState != lastState) {
    delay(50);
    currentState = digitalRead(pin);
    
    if (currentState == LOW && lastState == HIGH) {
      if (millis() - lastPressTime > 300) {
        pressed = true;
        lastPressTime = millis();
      }
    }
    
    lastState = currentState;
  }
  
  return pressed;
}

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(1);
  }
  randomSeed(millis() + analogRead(A0));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  receivedData.ts = 0;
  receivedData.press = 0;
  receivedData.executor_id = -1;
  myMessage.ts = temperature;
  myMessage.hum = humidity;
  pinMode(UP, INPUT_PULLUP);
  pinMode(SELECT, INPUT_PULLUP); 
  pinMode(DOWN, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(broadcastAddress[chosen], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  lastActionTime = millis();  
  updateDisplay();
  loadMACs();
  if (tools==3 && broadcastAddress[0][0] == 0 && broadcastAddress[0][1] == 0) {
        uint8_t defaultMACs[3][6] = {
            {0x40, 0xF5, 0x20, 0x32, 0xCB, 0xED},
            {0x8C, 0xAA, 0xB5, 0x53, 0x20, 0x5A},
            {0xE8, 0x68, 0xE7, 0xC7, 0xDC, 0xD0}
        };
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 6; j++) {
                broadcastAddress[i][j] = defaultMACs[i][j];
            }
        }
        saveMACs();
    }
    WiFi.macAddress(mymac.mac);
    for (int i = 0; i < tools; i++) {
            for (int j = 0; j < 6; j++) {
      esp_now_send(broadcastAddress[i], (uint8_t *) &mymac, sizeof(mymac));
    }}
}
void loop() {
  if (flag) {
    updateDisplay();
    flag = false;
  }
  if (screen > 0 && millis() - lastActionTime > timeout) {
    screen = 0;
    updateDisplay();
  }
  if (screen == 5 && tools<6) {
    static uint8_t byteIndex = 0;
    static uint8_t nibbleIndex = 0;
    static uint8_t tempMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static bool firstTime = true; 
    if (firstTime) {
      firstTime = false;
      byteIndex = 0;
      nibbleIndex = 0;
      if (tools > 0 && tools <= 6) {
        for (int j = 0; j < 6; j++) {
          tempMAC[j] = broadcastAddress[tools-1][j];
        }
      }
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Input MAC address:");
    for (int j = 0; j < 6; j++) {
      if (tempMAC[j] < 0x10) display.print("0");
      display.print(tempMAC[j], HEX);
      if (j < 5) display.print(" ");
    }
    display.setCursor(byteIndex * 18 + (nibbleIndex * 9), 30);
    display.print("^");
    display.setCursor(0, 45);
    display.println("UP/DOWN: +/-  SELECT: Next");
    display.display();
    bool localUp = buttonPressed(UP);
    bool localDown = buttonPressed(DOWN);
    bool localSelect = buttonPressed(SELECT);
    
    if (localUp || localDown || localSelect) {
      lastActionTime = millis();
    }
    
    if (localUp) {
      uint8_t current = tempMAC[byteIndex];
      if (nibbleIndex == 0) {
        uint8_t high = (current >> 4) & 0x0F;
        high = (high + 1) & 0x0F;
        tempMAC[byteIndex] = (high << 4) | (current & 0x0F);
      } else {
        uint8_t low = current & 0x0F;
        low = (low + 1) & 0x0F;
        tempMAC[byteIndex] = (current & 0xF0) | low;
      }
    }
    
    if (localDown) {
      uint8_t current = tempMAC[byteIndex];
      if (nibbleIndex == 0) {
        uint8_t high = (current >> 4) & 0x0F;
        high = (high - 1) & 0x0F;
        tempMAC[byteIndex] = (high << 4) | (current & 0x0F);
      } else {
        uint8_t low = current & 0x0F;
        low = (low - 1) & 0x0F; 
        tempMAC[byteIndex] = (current & 0xF0) | low;
      }
    }
    
    if (localSelect) {
      nibbleIndex++;
      if (nibbleIndex >= 2) {
        nibbleIndex = 0;
        byteIndex++;
        
        if (byteIndex >= 6) {
          if (tools < 6) {
            for (int j = 0; j < 6; j++) {
              broadcastAddress[tools][j] = tempMAC[j];
            }
            tools++;
            esp_now_add_peer(broadcastAddress[tools-1], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
            display.clearDisplay();
            display.setCursor(0, 5);
            saveMACs();
            display.println("Tool added:D\n... Check it");
            display.display();
            delay(1000);
          }
          
          firstTime = true;
          screen = 0;
          updateDisplay();
        }
      }
    }
  }
  else if(screen == 5 && tools==6)
  {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("You have too many of\n them!\nPlease, remove one.\nOf course, if you want...");
    display.display();
    delay(5000);
    screen = 0;
  } 
  else {
    bool upPressed = buttonPressed(UP);
    bool selectPressed = buttonPressed(SELECT);
    bool downPressed = buttonPressed(DOWN);
    
    if (upPressed || selectPressed || downPressed || motivation) {
      lastActionTime = millis();
      motivation = !motivation;
      switch(screen) {
        case 0:
          if (selectPressed) {
            screen = 1;
            capt = 0;
          } else if (upPressed) {
            screen = 4;
            esp_now_send(broadcastAddress[chosen], (uint8_t *) &myMessage, sizeof(myMessage));
          } else if (downPressed) {
            chosen = (chosen + 1) % tools;
          }
          break;
          
        case 1:
          if (upPressed) {
            capt = (capt - 1 + 6) % 6;
          } else if (downPressed) {
            capt = (capt + 1) % 6;
          } else if (selectPressed) {
            if (capt <= 1) {
              screen = 2 + capt;
            }
            else if (capt == 2) {
              screen = 3 + capt;
            }
            else if(capt>=3){
              screen = 3 + capt;
            }
          }
          break;
          
        case 2:
          
          if (downPressed && temperature < max_temp){
            temperature--;
            myMessage.ts = temperature;
          } else if (upPressed && temperature > min_temp) {
            temperature++;
            myMessage.ts = temperature;
          } else if (selectPressed) {
            screen = 1;
          }
          break;
          
        case 3:
          if (upPressed) {
            humidity = humidity < 100 ? humidity + 1 : 100;
            myMessage.hum = humidity;
          } else if (downPressed) {
            humidity = humidity > 0 ? humidity - 1 : 0;
            myMessage.hum = humidity;
          } else if (selectPressed) {
            screen = 1;
          }
          break;
        case 6:
          if(tools==1)
          {
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Please, do not delete\nits only child...");
            display.display();
            delay(5000);
            screen = 0;
            break;
          } 
          while(!selectPressed)
          {
            upPressed = buttonPressed(UP);
            selectPressed = buttonPressed(SELECT);
            downPressed = buttonPressed(DOWN);
            joint+=(upPressed-downPressed)%tools;
            joint = joint==255 ? tools: joint;
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Select Tool to delete...");
            display.setCursor(0,38);
            if(joint>=tools)
            {
              Serial.println(joint);
              display.println("go back ^-^");
              if(joint>tools)
                joint = 0;
            }
            else
            {
            display.print(joint);
            display.setCursor(0,38);
            }
            display.display();
            delay(50);
          }
          if(joint!=tools)
          {
          tools--;
          for(int i = joint;i<tools-1;i++)
          {
          for(int j = 0;j<6;j++)
          {
          broadcastAddress[i][j] = broadcastAddress[i+1][j];
          }
          }
          for(int i = 0;i<6;i++)
          {
          broadcastAddress[5][i] = 0x01;
          }
          }
          screen = 0;
          saveMACs();
          break;
        case 8: //kids control - max t min t
          int omt = max_temp;
          int omit = min_temp;
          max_temp = 0;
          min_temp = 0;
            while(!selectPressed)
            {
            upPressed = buttonPressed(UP);
            selectPressed = buttonPressed(SELECT);
            downPressed = buttonPressed(DOWN);
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("choose min & max temp");
            display.println("and answer a maths\ntask:)");
            display.println("");
            display.print(" ->");
            display.print(min_temp);
            display.print("   ");
            display.println(max_temp);
            display.display();
            delay(50);
            if(upPressed || downPressed)
              min_temp += upPressed - downPressed;
            }
            display.clearDisplay();
            selectPressed = buttonPressed(SELECT);
            while(!selectPressed){
            upPressed = buttonPressed(UP);
            selectPressed = buttonPressed(SELECT);
            downPressed = buttonPressed(DOWN);
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("choose min & max temp");
            display.println("and answer a maths\ntask:)");
            display.println("");
            display.print("   ");
            display.print(min_temp);
            display.print(" ->");
            display.println(max_temp);
            display.display();
            delay(50);
            if(upPressed || downPressed)
              max_temp += upPressed - downPressed;
            }
          display.clearDisplay();
          display.setCursor(0,0);
          int random_summifier=random(9);
          display.print("well then\n((7+");
          display.print(random_summifier);
          display.println(")/2-1)*(1-12+30) = ?");
          display.print("\nsel - ");
          display.print(((7+random_summifier)/2-1)*(1-12+30));
          display.println("\ndown - 143.11\nup - -122");
          display.display();
          while(1)
          {
            upPressed = buttonPressed(UP);
            selectPressed = buttonPressed(SELECT);
            downPressed = buttonPressed(DOWN);
            if(selectPressed)
            {
              if(min_temp>max_temp)
              {
                int temp_min_temp = min_temp;
                min_temp = max_temp;
                max_temp = temp_min_temp;
              }
              if(max_temp<temperature){
                temperature=max_temp;
              }
              if(min_temp>temperature){
                temperature=min_temp;
              }
              break;
            }
            else if(upPressed || downPressed)
            {
              max_temp = omt;
              min_temp = omit;
              break;
            }
            delay(50);
          }
          screen = 0;
          break;
      }
      if (screen != 5) {
        updateDisplay();
      }
    }
  }
  
  if (screen == 4) {
    delay(2000);
    screen = 0;
    updateDisplay();
  }
  delay(50);
}
