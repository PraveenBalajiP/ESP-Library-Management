#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN  5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() 
{
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    delay(50);
    Serial.println("Scan a card...");
}

void loop() 
{
  if(!rfid.PICC_IsNewCardPresent()) 
    return;
  if(!rfid.PICC_ReadCardSerial()) 
    return;
  String uid="";
  for (byte i=0;i<rfid.uid.size;i++)
  {
    if (rfid.uid.uidByte[i]<0x10) 
        uid+="0";
    uid+=String(rfid.uid.uidByte[i],HEX);
  }
  uid.toUpperCase();
  Serial.println("UID: "+uid);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}