#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid     = "YOUR WIFI NAME HERE";
const char* password = "YOUR WIFI PASSWORD HERE";

const char* addinfoURL    = "URL"; //https://esp-library-management.vercel.app/api/addinfo
const char* exitVerifyURL = "URL"; //https://esp-library-management.vercel.app/api/exit-verify
const char* peopleCountURL= "URL"; //https://esp-library-management.vercel.app/api/people-count

#define SS_ENTRY  5
#define SS_EXIT   27
#define RST_PIN   22
#define SPI_SCK   18
#define SPI_MISO  19
#define SPI_MOSI  23
#define BUZZER    26
#define GREEN_LED 25
#define RED_LED   33
#define IR_ENTRY_PIN    32
#define IR_EXIT_PIN     34
#define IR_ACTIVE_STATE LOW

#define PAIR_TIMEOUT_MS   8000
#define IR_DEBOUNCE_MS    700

MFRC522 entryReader(SS_ENTRY, RST_PIN);
MFRC522 exitReader (SS_EXIT,  RST_PIN);

enum EntryState { ENTRY_IDLE, ENTRY_WAIT_BOOK };
EntryState entryState          = ENTRY_IDLE;
String     entryStudentID      = "";
unsigned long entryFirstScanMs = 0;

enum ExitState { EXIT_IDLE, EXIT_WAIT_BOOK };
ExitState  exitState           = EXIT_IDLE;
String     exitStudentID       = "";
unsigned long exitFirstScanMs  = 0;

int           peopleInLibrary      = 0;
int           lastEntryIRState     = HIGH;
int           lastExitIRState      = HIGH;
unsigned long lastEntryIRTriggerMs = 0;
unsigned long lastExitIRTriggerMs  = 0;

unsigned long lastStatusPrintMs    = 0;

//Wifi Connection
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("[ ] WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[+] WiFi connected  IP: " + WiFi.localIP().toString());
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.println("[!] WiFi lost  reconnecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[+] WiFi back online");
}

//Ensuring RFID Readers work
void initRFIDReaders() {
  pinMode(SS_ENTRY, OUTPUT); digitalWrite(SS_ENTRY, HIGH);
  pinMode(SS_EXIT,  OUTPUT); digitalWrite(SS_EXIT,  HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  entryReader.PCD_Init(); delay(50);
  exitReader.PCD_Init();  delay(50);

  byte ev = entryReader.PCD_ReadRegister(MFRC522::VersionReg);
  byte xv = exitReader.PCD_ReadRegister(MFRC522::VersionReg);

  if (ev == 0x00 || ev == 0xFF)
    Serial.println("[!] ENTRY reader not found  check SS=5, RST=22, 3.3V wiring");
  else
    Serial.printf("[+] ENTRY reader OK  (v0x%02X)\n", ev);

  if (xv == 0x00 || xv == 0xFF)
    Serial.println("[!] EXIT reader not found  check SS=27, RST=22, 3.3V wiring");
  else
    Serial.printf("[+] EXIT  reader OK  (v0x%02X)\n", xv);
}

void allOff() {
  digitalWrite(BUZZER,    LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED,   LOW);
}

void signalSuccess() {
  allOff();
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
  delay(150);
  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(GREEN_LED, LOW);
}

void signalPrompt() {
  digitalWrite(BUZZER, HIGH);
  delay(80);
  digitalWrite(BUZZER, LOW);
}


void signalTheft() {
  allOff();
  digitalWrite(RED_LED, HIGH);
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(900);
    digitalWrite(BUZZER, LOW);
    delay(250);
  }
  delay(2000);
  digitalWrite(RED_LED, LOW);
}

void signalWarning() {
  allOff();
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER,  HIGH);
    delay(120);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER,  LOW);
    delay(120);
  }
}

# UID Reader Function
String readUID(MFRC522 &reader) {
  if (!reader.PICC_IsNewCardPresent()) return "";
  if (!reader.PICC_ReadCardSerial())   return "";
  String uid = "";
  for (byte i = 0; i < reader.uid.size; i++) {
    if (reader.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(reader.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  reader.PICC_HaltA();
  reader.PCD_StopCrypto1();
  return uid;
}

String getStudent(const String &uid) {
  if (uid == "61DA6406") return "PES2UG24CS001";
  if (uid == "C57E4E06") return "PES2UG24CS002";
  return "";
}

String getBook(const String &uid) {
  if (uid == "6AFDEF06") return "MPCA";
  if (uid == "B95EFF06") return "DSA";
  if (uid == "5BB72472") return "CN";
  if (uid == "8E75D8C4") return "OS";
  if (uid == "5E4E91C4") return "Python";
  if (uid == "5EEC4DC5") return "C";
  return "";
}

# Sending JSON back to live server
void sendIssue(const String &student, const String &book) {
  ensureWiFi();
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, addinfoURL);
  http.addHeader("Content-Type", "application/json");
  String json = "{\"id\":\"" + student + "\",\"bookName\":\"" + book + "\"}";
  int code = http.POST(json);
  String resp = http.getString();
  bool duplicate = (resp.indexOf("already") >= 0 || resp.indexOf("exists") >= 0 || code == 400);
  http.end();
  if (code == 200) {
    Serial.println("[+] ENTRY recorded  | " + student + " | " + book + " | duplicate: false");
    signalSuccess();
  } else if (duplicate) {
    Serial.println("[!] ENTRY duplicate  | " + student + " | " + book + " | duplicate: true");
    signalWarning();
  } else {
    Serial.println("[!] ENTRY failed  | code: " + String(code));
    signalWarning();
  }
}

void sendCheckout(const String &student, const String &book) {
  ensureWiFi();
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, exitVerifyURL);
  http.addHeader("Content-Type", "application/json");
  String json = "{\"id\":\"" + student + "\",\"bookName\":\"" + book + "\"}";
  int code = http.POST(json);
  String resp = http.getString();
  http.end();

  bool verified = (code == 200) &&
                  (resp.indexOf("verified") >= 0 ||
                   resp.indexOf("valid")    >= 0 ||
                   resp.indexOf("issued")   >= 0 ||
                   resp.indexOf("success")  >= 0);

  if (verified) {
    Serial.println("[+] EXIT OK  | " + student + " | " + book + " | cleared");
    signalSuccess();
  } else {
    Serial.println("[X] THEFT DETECTED  | " + student + " | " + book + " | not in records");
    signalTheft();
  }
}

void sendPeopleCount() {
  ensureWiFi();
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, peopleCountURL);
  http.addHeader("Content-Type", "application/json");
  String json = "{\"count\":" + String(peopleInLibrary) + "}";
  int code = http.POST(json);
  Serial.println("[ ] People count sent  | in library: " + String(peopleInLibrary) + " | code: " + String(code));
  http.end();
}

void handleIR() {
  unsigned long now = millis();
  int es = digitalRead(IR_ENTRY_PIN);
  int xs = digitalRead(IR_EXIT_PIN);
  if (es == IR_ACTIVE_STATE && lastEntryIRState != IR_ACTIVE_STATE &&
      (now - lastEntryIRTriggerMs) > IR_DEBOUNCE_MS) {
    peopleInLibrary++;
    lastEntryIRTriggerMs = now;
    Serial.println("[+] IR: Person entered  | in library: " + String(peopleInLibrary));
    sendPeopleCount();
  }
  if (xs == IR_ACTIVE_STATE && lastExitIRState != IR_ACTIVE_STATE &&
      (now - lastExitIRTriggerMs) > IR_DEBOUNCE_MS) {
    if (peopleInLibrary > 0) peopleInLibrary--;
    lastExitIRTriggerMs = now;
    Serial.println("[-] IR: Person exited   | in library: " + String(peopleInLibrary));
    sendPeopleCount();
  }
  lastEntryIRState = es;
  lastExitIRState  = xs;
}

void handleEntry() {
  // Timeout check — if student scanned but no book within PAIR_TIMEOUT_MS
  if (entryState == ENTRY_WAIT_BOOK &&
      (millis() - entryFirstScanMs) > PAIR_TIMEOUT_MS) {
    Serial.println("[!] ENTRY timeout  no book scanned in 8s  reset");
    signalWarning();
    entryState     = ENTRY_IDLE;
    entryStudentID = "";
  }

  String uid = readUID(entryReader);
  if (uid == "") return;
  Serial.println("\n--- ENTRY ---  UID: " + uid);
  String student = getStudent(uid);
  String book    = getBook(uid);
  switch (entryState) {
    case ENTRY_IDLE:
      if (student != "") {
        // Correct: student card first
        entryStudentID   = student;
        entryFirstScanMs = millis();
        entryState       = ENTRY_WAIT_BOOK;
        signalPrompt();
        Serial.println("[ ] ENTRY student scanned  | " + student + "  -->  now scan book");
      } else if (book != "") {
        // WRONG ORDER: book scanned before student
        Serial.println("[!] Wrong order at ENTRY  scan student tag FIRST then book");
        signalWarning();
        // State stays IDLE — do not record anything
      } else {
        Serial.println("[?] Unknown tag at ENTRY  | UID: " + uid);
    case ENTRY_WAIT_BOOK:
      if (book != "") {
        // Correct: book card second
        Serial.println("[ ] ENTRY book scanned  | " + book + "  sending to server...");
        sendIssue(entryStudentID, book);
        entryStudentID = "";
        entryState     = ENTRY_IDLE;
      } else if (student != "") {
        // Another student card — replace (edge case: person hands wrong card first)
        Serial.println("[ ] ENTRY new student mid-sequence  | " + student + "  -->  now scan book");
        entryStudentID   = student;
        entryFirstScanMs = millis();
        signalPrompt();
        // no extra print needed
      } else {
        Serial.println("[?] Unknown tag at ENTRY (waiting for book)  | UID: " + uid);
}

void handleExit() {
  // Timeout check
  if (exitState == EXIT_WAIT_BOOK &&
      (millis() - exitFirstScanMs) > PAIR_TIMEOUT_MS) {
    Serial.println("[!] EXIT timeout  no book scanned in 8s  reset");
    signalWarning();
    exitState     = EXIT_IDLE;
    exitStudentID = "";
  }
  String uid = readUID(exitReader);
  if (uid == "") return;
  Serial.println("\n--- EXIT ---  UID: " + uid);
  String student = getStudent(uid);
  String book    = getBook(uid);
  switch (exitState) {
    case EXIT_IDLE:
      if (student != "") {
        exitStudentID   = student;
        exitFirstScanMs = millis();
        exitState       = EXIT_WAIT_BOOK;
        signalPrompt();
        Serial.println("[ ] EXIT student scanned  | " + student + "  -->  now scan book");
      } else if (book != "") {
        // Book scanned with NO student → treat as attempted theft
        Serial.println("[X] THEFT  book scanned at exit with no student ID!");
        signalTheft();
        // State stays IDLE
      } else {
        Serial.println("[?] Unknown tag at EXIT  | UID: " + uid);
      if (book != "") {
        Serial.println("[ ] EXIT book scanned  | " + book + "  verifying...");
        sendCheckout(exitStudentID, book);
        exitStudentID = "";
        exitState     = EXIT_IDLE;
      } else if (student != "") {
        // Another student card replaces the first
        Serial.println("[ ] EXIT new student  | " + student + "  -->  now scan book");
        exitStudentID   = student;
        exitFirstScanMs = millis();
        signalPrompt();
      } else {
        Serial.println("[?] Unknown tag at EXIT (waiting for book)  | UID: " + uid);
        signalWarning();
      }
      break;
  }
  delay(800);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER,       OUTPUT); digitalWrite(BUZZER,       LOW);
  pinMode(GREEN_LED,    OUTPUT); digitalWrite(GREEN_LED,    LOW);
  pinMode(RED_LED,      OUTPUT); digitalWrite(RED_LED,      LOW);
  pinMode(IR_ENTRY_PIN, INPUT);
  pinMode(IR_EXIT_PIN,  INPUT);
  connectWiFi();
  initRFIDReaders();
  lastEntryIRState = digitalRead(IR_ENTRY_PIN);
  lastExitIRState  = digitalRead(IR_EXIT_PIN);
  sendPeopleCount();  // push 0 on boot
  // Startup success blink
  for (int i = 0; i < 2; i++) {
    digitalWrite(GREEN_LED, HIGH); delay(200);
    digitalWrite(GREEN_LED, LOW);  delay(150);
  }
  Serial.println("\n[+] System ready  |  scan STUDENT tag at ENTRY (SS=5) or EXIT (SS=27)");
}

void loop() {
  handleIR();
  handleEntry();
  handleExit();
  if (millis() - lastStatusPrintMs > 4000) {
    lastStatusPrintMs = millis();
    String entryStatus = (entryState == ENTRY_IDLE) ? "ENTRY: idle" : "ENTRY: waiting book  student=" + entryStudentID;
    String exitStatus  = (exitState  == EXIT_IDLE)  ? "EXIT: idle"  : "EXIT: waiting book  student=" + exitStudentID;
    Serial.println("[ ] " + entryStatus + "  |  " + exitStatus + "  |  people: " + String(peopleInLibrary));
  }
}
