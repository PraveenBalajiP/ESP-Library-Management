#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

/* WIFI */
const char* ssid="YOUR_WIFI_NAME";
const char* password="YOUR_WIFI_PASSWORD";

/* SERVER */
String addinfoURL="http://192.168.X.X:5000/api/addinfo";
String checkoutURL="http://192.168.X.X:5000/api/checkout";

/* RFID PINS */
#define SS_ENTRY 5
#define SS_EXIT 4
#define RST_PIN 22

/* BUZZER */
#define BUZZER 26

MFRC522 entryReader(SS_ENTRY,RST_PIN);
MFRC522 exitReader(SS_EXIT,RST_PIN);
String currentStudent="";
String currentBook="";

/* READ UID */
String readUID(MFRC522 &reader)
{
    if(!reader.PICC_IsNewCardPresent())
        return "";
    if(!reader.PICC_ReadCardSerial())
        return "";
    String uid="";
    for(byte i=0;i<reader.uid.size;i++){
        if(reader.uid.uidByte[i]<0x10) uid+="0";
        uid+=String(reader.uid.uidByte[i],HEX);
    }
    uid.toUpperCase();
    reader.PICC_HaltA();
    return uid;
}

/* STUDENT TAGS */
String getStudent(String uid)
{
    if(uid=="61DA6406")
        return "PES2UG24CS001";
    if(uid=="C57E4E06")
        return "PES2UG24CS002";
    return "";
}

/* BOOK CARDS */
String getBook(String uid)
{
    if(uid=="6AFDEF06")
        return "Microprocessor and Computer Architecture";
    if(uid=="B95EFF06")
        return "Data Structures and Algorithms";
    if(uid=="5BB72472")
        return "Computer Networks";
    if(uid=="8E75D8C4")
        return "Operating Systems";
    if(uid=="5E4E91C4")
        return "Python for All";
    if(uid=="5EEC4DC5")
        return "All About C and C++";
    return "";
}

/* SEND ISSUE DATA */
void sendIssue()
{
    if(WiFi.status()==WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(addinfoURL);
        http.addHeader("Content-Type","application/json");
        String json ="{\"id\":\""+currentStudent+"\",\"bookName\":\""+currentBook+"\"}";
        Serial.println("Sending issue request:");
        Serial.println(json);
        http.POST(json);
        http.end();
    }
}

/* SEND EXIT CHECK */
void sendCheckout(String book)
{
    if(WiFi.status()==WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(checkoutURL);
        http.addHeader("Content-Type","application/json");
        String json="{\"bookName\":\""+book+"\"}";
        Serial.println("Exit check request:");
        Serial.println(json);
        http.POST(json);
        String response=http.getString();
        Serial.println(response);
        if(response.indexOf("false")>=0)
        {
            Serial.println("THEFT DETECTED");
            digitalWrite(BUZZER,HIGH);
            delay(2000);
            digitalWrite(BUZZER,LOW);
        }
        http.end();
    }
}

/* SETUP */
void setup()
{
    Serial.begin(115200);
    SPI.begin();
    entryReader.PCD_Init();
    exitReader.PCD_Init();
    pinMode(BUZZER,OUTPUT);
    WiFi.begin(ssid,password);
    Serial.print("Connecting WiFi");
    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");
}

/* LOOP */
void loop()
{
    /* ENTRY SCANNER */
    String uidEntry=readUID(entryReader);
    if(uidEntry!="")
    {
        Serial.print("ENTRY UID: ");
        Serial.println(uidEntry);
        String student=getStudent(uidEntry);
        String book=getBook(uidEntry);
        if(student!="")
        {
            currentStudent=student;
            Serial.print("Student scanned: ");
            Serial.println(student);
        }
        if(book!="")
        {
            currentBook=book;
            Serial.print("Book scanned: ");
            Serial.println(book);
        }
        if(currentStudent!="" && currentBook!="")
        {
            sendIssue();
            currentStudent="";
            currentBook="";
        }
        delay(1500);
    }
    /* EXIT SCANNER */
    String uidExit=readUID(exitReader);
    if(uidExit!="")
    {
        Serial.print("EXIT UID: ");
        Serial.println(uidExit);
        String book = getBook(uidExit);
        if(book!="")
        {
            sendCheckout(book);
        }
        delay(1500);
    }
}