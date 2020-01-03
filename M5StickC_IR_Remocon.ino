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

#define ir_sensor "f000aa00-0451-4000-b000-000000000000"
#define ir_nec    "f000aa01-0451-4000-b000-000000000000"
#define ir_sony   "f000aa02-0451-4000-b000-000000000000"
#define ir_pana   "f000aa07-0451-4000-b000-000000000000"

const uint16_t kIrLed = 9;
IRsend irsend(kIrLed);

class NecIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint64_t data = htonll(*(uint64_t*)pValue);
    uint16_t nbits = hton(*(uint16_t*)(pValue + 4));
    uint16_t repeat = *(uint16_t*)(pValue + 5);
    irsend.sendNEC(data, nbits, repeat);
    M5LcdPrint( "SEND NEC: ", data, nbits, repeat);
    SerialPrint("SEND NEC: ", data, nbits, repeat);
  }
};

class SonyIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint64_t data = htonll(*(uint64_t*)pValue);
    uint16_t nbits = hton(*(uint16_t*)(pValue + 4));
    uint16_t repeat = *(uint16_t*)(pValue + 5);
    irsend.sendSony(data, nbits, repeat);
    M5LcdPrint( "SEND SONY: ", data, nbits, repeat);
    SerialPrint("SEND SONY: ", data, nbits, repeat);
  }  
};

class PanasonicIRCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    const char* pValue = pCharacteristic->getValue().c_str();
    uint16_t address = hton(*(uint16_t*)pValue);
    uint32_t data   =  htonl(*(uint32_t*)(pValue + 2));
    uint16_t nbits = hton(*(uint16_t*)(pValue + 6));
    uint16_t repeat = *(uint16_t*)(pValue + 8);    
    irsend.sendPanasonic(address, data, nbits, repeat);
    M5.Lcd.print("SEND PANASONIC: "); M5.Lcd.print(address, HEX); M5.Lcd.print(", "); M5.Lcd.println(data, HEX); M5.Lcd.print(nbits); M5.Lcd.print(", "); M5.Lcd.println(repeat);
    Serial.print("SEND PANASONIC: "); Serial.print(address, HEX); Serial.print(", "); Serial.println(data, HEX); Serial.print(nbits); Serial.print(", "); Serial.println(repeat);
  }  
};

class UInt32IRCharacteristic : public BLECharacteristic {
public:
  UInt32IRCharacteristic(const char* uuid, BLECharacteristicCallbacks* pCallbacks) : BLECharacteristic(BLEUUID(uuid)) {
    this->setCallbacks(pCallbacks);
    this->setReadProperty(true);
    this->setWriteProperty(true);
    uint32_t value = 0x00000000;
    this->setValue(value);   
  }
};

BLEService* createIRService(BLEServer* pServer) {
  BLEService* pService = pServer->createService(ir_sensor);
  pService->addCharacteristic(new UInt32IRCharacteristic(ir_nec,  new NecIRCallbacks()));
  pService->addCharacteristic(new UInt32IRCharacteristic(ir_sony, new SonyIRCallbacks()));
  pService->addCharacteristic(new UInt32IRCharacteristic(ir_pana, new PanasonicIRCallbacks()));
  
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
    uint64_t data = 0x082B4F;
    uint16_t nbits = 20;
    uint16_t rep = 3;
    irsend.sendSony(data, nbits, rep);
    M5LcdPrint( "SEND SONY: ", data, nbits, rep);
    SerialPrint("SEND SONY: ", data, nbits, rep);
  }
  if (M5.BtnB.wasReleased()) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0, 0);
  }
  delay(300);
  M5.update();
}
