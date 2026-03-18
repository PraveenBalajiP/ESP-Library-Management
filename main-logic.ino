#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

/* WIFI */
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

/* SERVER */
String addinfoURL = "http://192.168.X.X:5000/api/addinfo";
String checkoutURL = "http://192.168.X.X:5000/api/checkout";

/* RFID PINS */
#define SS_ENTRY 5
#define SS_EXIT 27
#define RST_PIN 22

/* BUZZER */
#define BUZZER 26

MFRC522 entryReader(SS_ENTRY, RST_PIN);
MFRC522 exitReader(SS_EXIT, RST_PIN);

String currentStudent = "";
String currentBook = "";

/* WIFI CHECK */
void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Lost! Reconnecting...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nReconnected!");
  }
}

/* READ UID */
String readUID(MFRC522 &reader) {
  if (!reader.PICC_IsNewCardPresent()) return "";
  if (!reader.PICC_ReadCardSerial()) return "";

  String uid = "";
  for (byte i = 0; i < reader.uid.size; i++) {
    if (reader.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(reader.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  reader.PICC_HaltA();
  return uid;
}

/* STUDENT TAGS */
String getStudent(String uid) {
  if (uid == "61DA6406") return "PES2UG24CS001";
  if (uid == "C57E4E06") return "PES2UG24CS002";
  return "";
}

/* BOOK TAGS */
String getBook(String uid) {
  if (uid == "6AFDEF06") return "MPCA";
  if (uid == "B95EFF06") return "DSA";
  if (uid == "5BB72472") return "CN";
  if (uid == "8E75D8C4") return "OS";
  if (uid == "5E4E91C4") return "Python";
  if (uid == "5EEC4DC5") return "C_CPP";
  return "";
}

/* SEND ISSUE */
void sendIssue() {
  ensureWiFi();

  HTTPClient http;
  http.begin(addinfoURL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"id\":\"" + currentStudent + "\",\"bookName\":\"" + currentBook + "\"}";

  Serial.println("Sending ENTRY:");
  Serial.println(json);

  int code = http.POST(json);
  Serial.print("Response Code: ");
  Serial.println(code);

  String response = http.getString();
  Serial.println(response);

  http.end();
}

/* SEND EXIT */
void sendCheckout(String book) {
  ensureWiFi();

  HTTPClient http;
  http.begin(checkoutURL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"bookName\":\"" + book + "\"}";

  Serial.println("Sending EXIT:");
  Serial.println(json);

  int code = http.POST(json);
  Serial.print("Response Code: ");
  Serial.println(code);

  String response = http.getString();
  Serial.println(response);

  if (response.indexOf("false") >= 0) {
    Serial.println("🚨 THEFT DETECTED 🚨");

    digitalWrite(BUZZER, HIGH);
    delay(2000);
    digitalWrite(BUZZER, LOW);
  }

  http.end();
}

/* SETUP */
void setup() {
  Serial.begin(115200);

  SPI.begin();
  entryReader.PCD_Init();
  exitReader.PCD_Init();

  pinMode(BUZZER, OUTPUT);

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
  Serial.println(WiFi.localIP());
}

/* LOOP */
void loop() {

  /* ENTRY */
  String uidEntry = readUID(entryReader);
  if (uidEntry != "") {
    Serial.println("\n--- ENTRY SCAN ---");
    Serial.println(uidEntry);

    String student = getStudent(uidEntry);
    String book = getBook(uidEntry);

    if (student != "") {
      currentStudent = student;
      Serial.println("Student: " + student);
    }

    if (book != "") {
      currentBook = book;
      Serial.println("Book: " + book);
    }

    if (currentStudent != "" && currentBook != "") {
      sendIssue();
      currentStudent = "";
      currentBook = "";
    }

    delay(1500);
  }

  /* EXIT */
  String uidExit = readUID(exitReader);
  if (uidExit != "") {
    Serial.println("\n--- EXIT SCAN ---");
    Serial.println(uidExit);

    String book = getBook(uidExit);
    if (book != "") {
      sendCheckout(book);
    }

    delay(1500);
  }
}