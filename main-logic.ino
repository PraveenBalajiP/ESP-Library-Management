#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

/* WIFI */
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

/* SERVER - BACKEND IP CONFIGURED */
String addinfoURL = "https://esp-library-management-rju9.vercel.app/api/addinfo";
String exitVerifyURL = "https://esp-library-management-rju9.vercel.app/api/exit-verify";
String peopleCountURL = "https://esp-library-management-rju9.vercel.app/api/people-count";

/* RFID PINS */
#define SS_ENTRY 5
#define SS_EXIT 27
#define RST_PIN 22

/* ESP32 SPI PINS (adjust if wired differently) */
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

/* BUZZER */
#define BUZZER 26

/* STATUS LEDS */
#define GREEN_LED 25
#define RED_LED 33

/* IR SENSORS (people counter) */
#define IR_ENTRY_PIN 32
#define IR_EXIT_PIN 34
#define IR_ACTIVE_STATE LOW

MFRC522 entryReader(SS_ENTRY, RST_PIN);
MFRC522 exitReader(SS_EXIT, RST_PIN);

String currentStudent = "";
String currentBook = "";
String lastExitStudent = "";  // Track student at exit gate
unsigned long lastStatusPrint = 0;
int peopleInLibrary = 0;
int lastEntryIRState = HIGH;
int lastExitIRState = HIGH;
unsigned long lastEntryIRTriggerMs = 0;
unsigned long lastExitIRTriggerMs = 0;
const unsigned long IR_DEBOUNCE_MS = 700;

/* WIFI CONNECT (merged from wifi-connection.ino) */
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

/* RFID INIT + DEBUG */
void initRFIDReaders() {
  pinMode(SS_ENTRY, OUTPUT);
  pinMode(SS_EXIT, OUTPUT);
  digitalWrite(SS_ENTRY, HIGH);
  digitalWrite(SS_EXIT, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  entryReader.PCD_Init();
  delay(50);
  exitReader.PCD_Init();
  delay(50);

  byte entryVersion = entryReader.PCD_ReadRegister(MFRC522::VersionReg);
  byte exitVersion = exitReader.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print("Entry reader version: 0x");
  Serial.println(entryVersion, HEX);
  Serial.print("Exit reader version: 0x");
  Serial.println(exitVersion, HEX);

  if (entryVersion == 0x00 || entryVersion == 0xFF) {
    Serial.println("⚠️ Entry reader not detected. Check wiring for SS=5, RST=22, SCK=18, MISO=19, MOSI=23, 3.3V, GND");
  }

  if (exitVersion == 0x00 || exitVersion == 0xFF) {
    Serial.println("⚠️ Exit reader not detected. Check wiring for SS=27, RST=22, SCK=18, MISO=19, MOSI=23, 3.3V, GND");
  }

  if (entryVersion != 0x00 && entryVersion != 0xFF && exitVersion != 0x00 && exitVersion != 0xFF) {
    Serial.println("✅ Both RFID readers initialized");
  }
}

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

/* SEND PEOPLE COUNT */
void sendPeopleCount() {
  ensureWiFi();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, peopleCountURL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"count\":" + String(peopleInLibrary) + "}";

  Serial.println("Sending PEOPLE COUNT:");
  Serial.println(json);

  int code = http.POST(json);
  Serial.print("People Count Response Code: ");
  Serial.println(code);

  String response = http.getString();
  Serial.println("People Count Response: " + response);

  http.end();
}

/* IR PEOPLE COUNTER */
void handleIRPeopleCount() {
  int entryState = digitalRead(IR_ENTRY_PIN);
  int exitState = digitalRead(IR_EXIT_PIN);
  unsigned long now = millis();

  if (entryState == IR_ACTIVE_STATE && lastEntryIRState != IR_ACTIVE_STATE && (now - lastEntryIRTriggerMs) > IR_DEBOUNCE_MS) {
    peopleInLibrary++;
    lastEntryIRTriggerMs = now;
    Serial.println("✅ Person entered library");
    Serial.print("People in Library: ");
    Serial.println(peopleInLibrary);
    sendPeopleCount();
  }

  if (exitState == IR_ACTIVE_STATE && lastExitIRState != IR_ACTIVE_STATE && (now - lastExitIRTriggerMs) > IR_DEBOUNCE_MS) {
    if (peopleInLibrary > 0) {
      peopleInLibrary--;
    }
    lastExitIRTriggerMs = now;
    Serial.println("↘ Person exited library");
    Serial.print("People in Library: ");
    Serial.println(peopleInLibrary);
    sendPeopleCount();
  }

  lastEntryIRState = entryState;
  lastExitIRState = exitState;
}

void blinkLED(int pin, int times, int onMs, int offMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(onMs);
    digitalWrite(pin, LOW);
    if (i < times - 1) {
      delay(offMs);
    }
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
  if (uid == "5EEC4DC5") return "C";
  return "";
}

/* SEND ISSUE */
void sendIssue() {
  ensureWiFi();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, addinfoURL);
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

/* SEND EXIT (theft check + valid withdraw) */
void sendCheckout(String student, String book) {
  ensureWiFi();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, exitVerifyURL);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"id\":\"" + student + "\",\"bookName\":\"" + book + "\"}";

  Serial.println("Sending EXIT VERIFICATION:");
  Serial.println(json);

  int code = http.POST(json);
  Serial.print("Response Code: ");
  Serial.println(code);

  String response = http.getString();
  Serial.println("Response: " + response);

  if (code == 200 && (response.indexOf("verified") >= 0 || response.indexOf("valid") >= 0 || response.indexOf("issued") >= 0)) {
    Serial.println("✅ Exit allowed: no theft detected");
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    blinkLED(GREEN_LED, 3, 180, 140);
  } 
  else if (code == 400 || response.indexOf("does not exist") >= 0) {
    Serial.println("🚨 THEFT DETECTED 🚨");
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    delay(1200);
    digitalWrite(BUZZER, LOW);
    delay(300);
    digitalWrite(BUZZER, HIGH);
    delay(1200);
    digitalWrite(BUZZER, LOW);
    blinkLED(RED_LED, 3, 220, 140);
  }
  else if (code != 200) {
    Serial.println("⚠️ Exit verification error: " + String(code));
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    blinkLED(RED_LED, 3, 140, 120);
  }

  http.end();
}

/* SETUP */
void setup() {
  Serial.begin(115200);

  connectWiFi();

  initRFIDReaders();

  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(IR_ENTRY_PIN, INPUT);
  pinMode(IR_EXIT_PIN, INPUT);
  digitalWrite(BUZZER, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  lastEntryIRState = digitalRead(IR_ENTRY_PIN);
  lastExitIRState = digitalRead(IR_EXIT_PIN);
  sendPeopleCount();
  Serial.println("Place card near ENTRY(SS=5) or EXIT(SS=27) reader...");
}

/* LOOP */
void loop() {

  handleIRPeopleCount();

  /* ENTRY */
  String uidEntry = readUID(entryReader);
  if (millis() - lastStatusPrint > 3000) {
    Serial.println("Waiting for RFID card...");
    lastStatusPrint = millis();
  }
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

  /* EXIT - Scan student card first, then book */
  String uidExit = readUID(exitReader);
  if (uidExit != "") {
    Serial.println("\n--- EXIT SCAN ---");
    Serial.println(uidExit);

    String student = getStudent(uidExit);
    if (student != "") {
      lastExitStudent = student;
      Serial.println("Exit Student: " + student);
      delay(1500);
      return;
    }

    String book = getBook(uidExit);
    if (book != "" && lastExitStudent != "") {
      sendCheckout(lastExitStudent, book);
      lastExitStudent = "";
    } 
    else if (book != "") {
      Serial.println("⚠️ Scan student card first");
    }

    delay(1500);
  }
}