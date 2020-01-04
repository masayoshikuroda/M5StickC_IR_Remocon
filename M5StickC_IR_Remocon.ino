#include <M5StickC.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <BLEDevice.h>
#include <BLEServer.h>

class ServerCallbacks: public BLEServerCallbacks {
 public:
    bool* _pConnected;

    ServerCallbacks(bool* connected) {
      _pConnected = connected;
    }
    void onConnect(BLEServer* pServer) {
      *_pConnected = true;
      M5.Lcd.println("ServerCallbacks onConnect");
      Serial.println("ServerCallbacks onConnect");
    }
    void onDisconnect(BLEServer* pServer) {
      *_pConnected = false;
      M5.Lcd.println("ServerCallbacks onDisconnect");
      Serial.println("ServerCallbacks onDisconnect");
    }
};

BLEServer* createServer(char* name, bool* pConnected) {
  BLEDevice::init(name);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks(pConnected));  
};

uint16_t hton(uint16_t h) {
  uint8_t n[2];
  n[0] = *((uint8_t*)&h + 1);
  n[1] = *((uint8_t*)&h + 0);
  return *(uint16_t*)n;
};

uint32_t htonl(uint32_t h) {
  uint8_t n[4];
  n[0] = *((uint8_t*)&h + 3);
  n[1] = *((uint8_t*)&h + 2);
  n[2] = *((uint8_t*)&h + 1);
  n[3] = *((uint8_t*)&h + 0);
  return *(uint32_t*)n;
};

uint64_t htonll(uint64_t h) {
  uint8_t n[8];
  n[0] = *((uint8_t*)&h + 7);
  n[1] = *((uint8_t*)&h + 6);
  n[2] = *((uint8_t*)&h + 5);
  n[3] = *((uint8_t*)&h + 4);
  n[4] = *((uint8_t*)&h + 3);
  n[5] = *((uint8_t*)&h + 2);
  n[6] = *((uint8_t*)&h + 1);
  n[7] = *((uint8_t*)&h + 0);
  return *(uint64_t*)n;
};

void SerialPrint(uint8_t v) {
  if (v<0x10) {
    Serial.print("0");
  }
  Serial.print(v, HEX); 
};

void SerialPrint(uint16_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=1; i>=0; i--) {
    SerialPrint(*(p + i));
  }
};

void SerialPrint(uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=3; i>=0; i--) {
    SerialPrint(*(p + i));
  }
};

void SerialPrint(uint64_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=7; i>=0; i--) {
    SerialPrint(*(p + i));
  }
};

void SerialPrint(char* msg, uint64_t data, uint16_t nbits, uint16_t repeat) {
  Serial.print(msg);    Serial.print(", ");
  SerialPrint(data);    Serial.print(", ");
  Serial.print(nbits);  Serial.print(", ");
  Serial.print(repeat); Serial.println("");
};

void SerialPrint(char* msg, uint32_t address, uint64_t data, uint16_t nbits, uint16_t repeat) {
  Serial.print(msg);    Serial.print(", ");
  SerialPrint(address); Serial.print(", ");
  SerialPrint(data);    Serial.print(", ");
  Serial.print(nbits);  Serial.print(", ");
  Serial.print(repeat); Serial.println("");
};

void M5LcdPrint(uint8_t v) {
  if (v<0x10) {
    M5.Lcd.print("0");
  }
  M5.Lcd.print(v, HEX);   
};

void M5LcdPrint(uint16_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=1; i>=0; i--) {
    M5LcdPrint(*(p + i));
  }
};

void M5LcdPrint(uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=3; i>=0; i--) {
    M5LcdPrint(*(p + i));
  }
};

void M5LcdPrint(uint64_t v) {
  uint8_t* p = (uint8_t*)&v;
  for (int i=7; i>=0; i--) {
    M5LcdPrint(*(p + i));
  }
};

void M5LcdPrint(char* msg, uint64_t data, uint16_t nbits, uint16_t repeat) {
  M5.Lcd.print(msg);    M5.Lcd.print(", ");
  M5LcdPrint(data);     M5.Lcd.print(", ");
  M5.Lcd.print(nbits);  M5.Lcd.print(", ");
  M5.Lcd.print(repeat); M5.Lcd.println("");
  
};

void M5LcdPrint(char* msg, uint32_t address, uint32_t data, uint16_t nbits, uint16_t repeat) {
  M5.Lcd.print(msg);    M5.Lcd.print(", ");
  M5LcdPrint(address);  M5.Lcd.print(", ");
  M5LcdPrint(data);     M5.Lcd.print(", ");
  M5.Lcd.print(nbits);  M5.Lcd.print(", ");
  M5.Lcd.print(repeat); M5.Lcd.println("");
  
};

#define ir_sensor "f000aa00-0451-4000-b000-000000000000"
#define ir_nec    "f000aa01-0451-4000-b000-000000000000"
#define ir_sony   "f000aa02-0451-4000-b000-000000000000"
#define ir_pana   "f000aa07-0451-4000-b000-000000000000"

const uint16_t kIrLed = 9;
IRsend irsend(kIrLed);

class NecIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint64_t data = htonll(*(uint64_t*)(pValue +  0));
    uint16_t nbits  = hton(*(uint16_t*)(pValue +  8));
    uint16_t repeat = hton(*(uint16_t*)(pValue + 10));
    irsend.sendNEC(data, nbits, repeat);
    M5LcdPrint( "SEND NEC: ", data, nbits, repeat);
    SerialPrint("SEND NEC: ", data, nbits, repeat);
  }
};

class SonyIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint64_t data = htonll(*(uint64_t*)(pValue +  0));
    uint16_t nbits  = hton(*(uint16_t*)(pValue +  8));
    uint16_t repeat = hton(*(uint16_t*)(pValue + 10));
    irsend.sendSony(data, nbits, repeat);
    M5LcdPrint( "SEND SONY: ", data, nbits, repeat);
    SerialPrint("SEND SONY: ", data, nbits, repeat);
  }  
};

class PanasonicIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint16_t address  = hton(*(uint16_t*)(pValue + 0));
    uint32_t data   =  htonl(*(uint32_t*)(pValue + 2));
    uint16_t nbits    = hton(*(uint16_t*)(pValue + 6));
    uint16_t repeat   = hton(*(uint16_t*)(pValue + 8));    
    irsend.sendPanasonic(address, data, nbits, repeat);
    M5LcdPrint( "SEND PANASONIC: ", address, data, nbits, repeat);
    SerialPrint("SEND PANASONIC: ", address, data, nbits, repeat);
  }  
};

class IRCharacteristic : public BLECharacteristic {
public:
  IRCharacteristic(const char* uuid, BLECharacteristicCallbacks* pCallbacks, std::string value) : BLECharacteristic(BLEUUID(uuid)) {
    this->setCallbacks(pCallbacks);
    this->setReadProperty(true);
    this->setWriteProperty(true);
    this->setValue(value);   
  }
};

BLEService* createIRService(BLEServer* pServer) {
  BLEService* pService = pServer->createService(ir_sensor);
  char n[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x62, 0x9D, 0x00, 0x20, 0x00, 0x00};
  char s[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x2B, 0x4F, 0x00, 0x14, 0x00, 0x03};
  char p[] = {0x40, 0x04, 0x40, 0x04, 0x01, 0x64, 0x0A, 0x6F, 0x00, 0x30, 0x00, 0x03};
  pService->addCharacteristic(new IRCharacteristic(ir_nec,  new NecIRCallbacks(),       std::string(n, std::end(n))));
  pService->addCharacteristic(new IRCharacteristic(ir_sony, new SonyIRCallbacks(),      std::string(s, std::end(s))));
  pService->addCharacteristic(new IRCharacteristic(ir_pana, new PanasonicIRCallbacks(), std::string(p, std::end(p))));
  return pService;
};

bool connected;

void setup() {
  M5.begin();

  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 0);
  
  M5.Lcd.println("enter setup");
  Serial.println("enter setup");

  BLEServer *pServer = createServer("M5StickC IR Remocon", &connected);

  irsend.begin();
  createIRService(pServer)->start();
  M5.Lcd.print("IR Remocon Send Pin: "); M5.Lcd.println(kIrLed); 
  Serial.print("IR Remocon Send Pin: "); Serial.println(kIrLed);
  M5.Lcd.println("IR Remocon Service start");
  Serial.println("IR Remocon Service start");
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  M5.Lcd.println("Advertising start");
  Serial.println("Advertising start");
    
  M5.Lcd.println("exit setup");
  Serial.println("exit setup");
}

void loop() {
  if (M5.BtnA.wasReleased()) {
    uint64_t data = 0x0000000000FD629D;
    uint16_t nbits = 0x0020;
    uint16_t repeat = 0x0000;
    irsend.sendNEC(data, nbits, repeat);
    //irsend.sendNEC(0xFD629D, 32);
    M5LcdPrint( "SEND NEC : ", data, nbits, repeat);
    SerialPrint("SEND NEC : ", data, nbits, repeat); 
  }
  if (M5.BtnB.wasReleased()) {
    uint64_t data = 0x0000000000082B4F;
    uint16_t nbits = 0x0014;
    uint16_t repeat = 0x0003;    
    irsend.sendSony(data, nbits, repeat);
    M5LcdPrint( "SEND SONY: ", data, nbits, repeat);
    SerialPrint("SEND SONY: ", data, nbits, repeat); 
  }
  delay(300);
  M5.update();
}
