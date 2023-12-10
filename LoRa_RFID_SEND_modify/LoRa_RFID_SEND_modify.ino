#include <SoftwareSerial.h>
#include "SNIPE.h"
#include <SPI.h>
#include <MFRC522.h>
#include <stdlib.h>
#include "Linked_List.h"

#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

#define ATSerial Serial

SNIPE SNIPE(ATSerial);

//16byte hex key
String lora_app_key = "10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00";  

int status = 0;



void setup() {
  ATSerial.begin(115200); //LoRa 통신을 위해 115200 설정

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  while(ATSerial.read()>= 0) {}
  while(!ATSerial);
  listinit();
}

void loop() {
  String ver;

// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  // 리스트에 UID가 있는 경우
  // 승객이 하차했음을 알리는 OUT 신호를 전송 
  if (search_list(rfid.uid.uidByte)) {
    deleteNode(rfid.uid.uidByte); 
    Serial.println(F("Card read previously."));
      if(SNIPE.lora_send("OUT")){
      Serial.println("send success");
      ver = SNIPE.lora_recv();
      if(ver == "Ack") Serial.println("command [OUT] received");
    }
    else{
      Serial.println("send fail");
      delay(500);
    }
  }

  // 리스트에 UID가 없는 경우
  // 승객이 탑승했음을 알리는 IN 신호를 전송 
  else{
    Serial.println(F("A new card has been detected."));

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

    insert(rfid.uid.uidByte);

    if(SNIPE.lora_send("IN")){
      Serial.println("send success");
      ver = SNIPE.lora_recv();
      if(ver == "Ack") Serial.println("command [IN] received");
    }
    else{
      Serial.println("send fail");
      delay(500);
    }
  }
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
       
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
