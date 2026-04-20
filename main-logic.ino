#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

/* ─── WIFI ──────────────────────────────────────────────────────────────────── */
const char* ssid     = "Moto";
const char* password = "toek5869";

/* ─── SERVER ─────────────────────────────────────────────────────────────────── */
const char* addinfoURL    = "https://esp-library-management-rju9.vercel.app/api/addinfo";
const char* exitVerifyURL = "https://esp-library-management-rju9.vercel.app/api/exit-verify";
const char* peopleCountURL= "https://esp-library-management-rju9.vercel.app/api/people-count";

/* ─── RFID PINS ──────────────────────────────────────────────────────────────── */
#define SS_ENTRY  5
#define SS_EXIT   27
#define RST_PIN   22

/* ─── SPI PINS ───────────────────────────────────────────────────────────────── */
#define SPI_SCK   18
#define SPI_MISO  19
#define SPI_MOSI  23

/* ─── OUTPUT PINS ────────────────────────────────────────────────────────────── */
#define BUZZER    26
#define GREEN_LED 25
#define RED_LED   33

/* ─── IR SENSOR PINS ─────────────────────────────────────────────────────────── */
#define IR_ENTRY_PIN    32
#define IR_EXIT_PIN     34
#define IR_ACTIVE_STATE LOW

/* ─── TIMEOUTS ────────────────────────────────────────────────────────────────── */
// How long (ms) to wait for the second card after the first at entry/exit
#define PAIR_TIMEOUT_MS   8000
// Debounce for IR sensors
#define IR_DEBOUNCE_MS    700

/* ─── RFID READERS ───────────────────────────────────────────────────────────── */
MFRC522 entryReader(SS_ENTRY, RST_PIN);
MFRC522 exitReader (SS_EXIT,  RST_PIN);

/* ─── ENTRY STATE MACHINE ────────────────────────────────────────────────────── */
//  Sequence enforced: student card FIRST, then book card
//  Both must be scanned at the ENTRY reader
enum EntryState { ENTRY_IDLE, ENTRY_WAIT_BOOK };
EntryState entryState          = ENTRY_IDLE;
String     entryStudentID      = "";
unsigned long entryFirstScanMs = 0;

/* ─── EXIT STATE MACHINE ─────────────────────────────────────────────────────── */
//  Sequence enforced: student card FIRST, then book card
//  Both must be scanned at the EXIT reader
enum ExitState { EXIT_IDLE, EXIT_WAIT_BOOK };
ExitState  exitState           = EXIT_IDLE;
String     exitStudentID       = "";
unsigned long exitFirstScanMs  = 0;

/* ─── IR PEOPLE COUNTER ──────────────────────────────────────────────────────── */
int           peopleInLibrary      = 0;
int           lastEntryIRState     = HIGH;
int           lastExitIRState      = HIGH;
unsigned long lastEntryIRTriggerMs = 0;
unsigned long lastExitIRTriggerMs  = 0;

/* ─── MISC ────────────────────────────────────────────────────────────────────── */
unsigned long lastStatusPrintMs    = 0;

/* ══════════════════════════════════════════════════════════════════════════════
   WIFI
══════════════════════════════════════════════════════════════════════════════ */
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.println("WiFi lost — reconnecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nReconnected!");
}

/* ══════════════════════════════════════════════════════════════════════════════
   RFID INIT
══════════════════════════════════════════════════════════════════════════════ */
void initRFIDReaders() {
  pinMode(SS_ENTRY, OUTPUT); digitalWrite(SS_ENTRY, HIGH);
  pinMode(SS_EXIT,  OUTPUT); digitalWrite(SS_EXIT,  HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  entryReader.PCD_Init(); delay(50);
  exitReader.PCD_Init();  delay(50);

  byte ev = entryReader.PCD_ReadRegister(MFRC522::VersionReg);
  byte xv = exitReader.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("Entry reader version: 0x%02X\n", ev);
  Serial.printf("Exit  reader version: 0x%02X\n", xv);

  if (ev == 0x00 || ev == 0xFF)
    Serial.println("WARNING: Entry reader not detected — check SS=5, RST=22, SPI wiring, 3.3V");
  if (xv == 0x00 || xv == 0xFF)
    Serial.println("WARNING: Exit reader not detected — check SS=27, RST=22, SPI wiring, 3.3V");
  if (ev != 0x00 && ev != 0xFF && xv != 0x00 && xv != 0xFF)
    Serial.println("Both RFID readers OK");
}

/* ══════════════════════════════════════════════════════════════════════════════
   LED + BUZZER HELPERS
══════════════════════════════════════════════════════════════════════════════ */
void allOff() {
  digitalWrite(BUZZER,    LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED,   LOW);
}

// Single short beep + green blink  →  valid action confirmed
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

// Prompt beep (waiting for second card)
void signalPrompt() {
  digitalWrite(BUZZER, HIGH);
  delay(80);
  digitalWrite(BUZZER, LOW);
}

// Continuous buzzer + red LED for theft — duration ms
void signalTheft() {
  allOff();
  digitalWrite(RED_LED, HIGH);
  // Three long blasts separated by short gaps
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(900);
    digitalWrite(BUZZER, LOW);
    delay(250);
  }
  // Hold red LED on for 2 more seconds so it is visible
  delay(2000);
  digitalWrite(RED_LED, LOW);
}

// Short warning (timeout / scan-order violation)
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

/* ══════════════════════════════════════════════════════════════════════════════
   READ UID
══════════════════════════════════════════════════════════════════════════════ */
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

/* ══════════════════════════════════════════════════════════════════════════════
   TAG LOOKUP TABLES
══════════════════════════════════════════════════════════════════════════════ */
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

/* ══════════════════════════════════════════════════════════════════════════════
   HTTP CALLS
══════════════════════════════════════════════════════════════════════════════ */
void sendIssue(const String &student, const String &book) {
  ensureWiFi();
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, addinfoURL);
  http.addHeader("Content-Type", "application/json");
  String json = "{\"id\":\"" + student + "\",\"bookName\":\"" + book + "\"}";
  Serial.println("POST addinfo: " + json);
  int code = http.POST(json);
  Serial.println("Response " + String(code) + ": " + http.getString());
  http.end();

  if (code == 200) {
    signalSuccess();
    Serial.println("Entry recorded — GREEN LED + double beep");
  } else {
    signalWarning();
    Serial.println("Entry error (code " + String(code) + ") — RED blink");
  }
}

void sendCheckout(const String &student, const String &book) {
  ensureWiFi();
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;
  http.begin(client, exitVerifyURL);
  http.addHeader("Content-Type", "application/json");
  String json = "{\"id\":\"" + student + "\",\"bookName\":\"" + book + "\"}";
  Serial.println("POST exit-verify: " + json);
  int code = http.POST(json);
  String resp = http.getString();
  Serial.println("Response " + String(code) + ": " + resp);
  http.end();

  bool verified = (code == 200) &&
                  (resp.indexOf("verified") >= 0 ||
                   resp.indexOf("valid")    >= 0 ||
                   resp.indexOf("issued")   >= 0 ||
                   resp.indexOf("success")  >= 0);

  if (verified) {
    Serial.println("Exit ALLOWED — GREEN LED + single beep");
    signalSuccess();
  } else {
    // 400 / record not found / any unexpected code → THEFT
    Serial.println("THEFT DETECTED — RED LED + continuous buzzer");
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
  Serial.println("POST people-count: " + json);
  int code = http.POST(json);
  Serial.println("People-count response " + String(code) + ": " + http.getString());
  http.end();
}

/* ══════════════════════════════════════════════════════════════════════════════
   IR PEOPLE COUNTER
══════════════════════════════════════════════════════════════════════════════ */
void handleIR() {
  unsigned long now = millis();
  int es = digitalRead(IR_ENTRY_PIN);
  int xs = digitalRead(IR_EXIT_PIN);

  if (es == IR_ACTIVE_STATE && lastEntryIRState != IR_ACTIVE_STATE &&
      (now - lastEntryIRTriggerMs) > IR_DEBOUNCE_MS) {
    peopleInLibrary++;
    lastEntryIRTriggerMs = now;
    Serial.println("IR: Person entered. Count = " + String(peopleInLibrary));
    sendPeopleCount();
  }
  if (xs == IR_ACTIVE_STATE && lastExitIRState != IR_ACTIVE_STATE &&
      (now - lastExitIRTriggerMs) > IR_DEBOUNCE_MS) {
    if (peopleInLibrary > 0) peopleInLibrary--;
    lastExitIRTriggerMs = now;
    Serial.println("IR: Person exited. Count = " + String(peopleInLibrary));
    sendPeopleCount();
  }
  lastEntryIRState = es;
  lastExitIRState  = xs;
}

/* ══════════════════════════════════════════════════════════════════════════════
   ENTRY SCANNER STATE MACHINE
   ─────────────────────────────────────────────────────────────────────────────
   Step 1: Scan STUDENT card  →  store ID, beep once, wait for book
   Step 2: Scan BOOK   card   →  send issue, reset
   If BOOK card is presented first  →  warn and ignore
   If timeout (PAIR_TIMEOUT_MS) between step 1 and 2  →  reset, warn user
══════════════════════════════════════════════════════════════════════════════ */
void handleEntry() {
  // Timeout check — if student scanned but no book within PAIR_TIMEOUT_MS
  if (entryState == ENTRY_WAIT_BOOK &&
      (millis() - entryFirstScanMs) > PAIR_TIMEOUT_MS) {
    Serial.println("Entry timeout — no book scanned. Reset.");
    signalWarning();
    entryState     = ENTRY_IDLE;
    entryStudentID = "";
  }

  String uid = readUID(entryReader);
  if (uid == "") return;

  Serial.println("\n--- ENTRY SCAN: " + uid + " ---");

  String student = getStudent(uid);
  String book    = getBook(uid);

  switch (entryState) {

    /* ── Waiting for the FIRST card ────────────────────────────────────────── */
    case ENTRY_IDLE:
      if (student != "") {
        // Correct: student card first
        entryStudentID   = student;
        entryFirstScanMs = millis();
        entryState       = ENTRY_WAIT_BOOK;
        signalPrompt();
        Serial.println("Student: " + student + " — now scan the BOOK card");
      } else if (book != "") {
        // WRONG ORDER: book scanned before student
        Serial.println("WRONG ORDER: Scan student card FIRST, then book.");
        signalWarning();
        // State stays IDLE — do not record anything
      } else {
        Serial.println("Unknown tag at entry: " + uid);
        signalWarning();
      }
      break;

    /* ── Have student, waiting for BOOK ────────────────────────────────────── */
    case ENTRY_WAIT_BOOK:
      if (book != "") {
        // Correct: book card second
        Serial.println("Book: " + book + " — sending issue for " + entryStudentID);
        sendIssue(entryStudentID, book);
        entryStudentID = "";
        entryState     = ENTRY_IDLE;
      } else if (student != "") {
        // Another student card — replace (edge case: person hands wrong card first)
        Serial.println("New student scanned mid-sequence: " + student + " — resetting");
        entryStudentID   = student;
        entryFirstScanMs = millis();
        signalPrompt();
        Serial.println("Now scan the BOOK card for " + student);
      } else {
        Serial.println("Unknown tag at entry: " + uid);
        signalWarning();
      }
      break;
  }

  delay(800); // prevent double-read
}

/* ══════════════════════════════════════════════════════════════════════════════
   EXIT SCANNER STATE MACHINE
   ─────────────────────────────────────────────────────────────────────────────
   Step 1: Scan STUDENT card  →  store ID, beep once, wait for book
   Step 2: Scan BOOK   card   →  verify against DB:
              • Found  (200) → GREEN LED + short beep
              • Not found    → RED LED + CONTINUOUS BUZZER (THEFT)
   If BOOK card is presented first (no student stored)  →  THEFT signal
   Timeout between step 1 and 2  →  reset, warn user
══════════════════════════════════════════════════════════════════════════════ */
void handleExit() {
  // Timeout check
  if (exitState == EXIT_WAIT_BOOK &&
      (millis() - exitFirstScanMs) > PAIR_TIMEOUT_MS) {
    Serial.println("Exit timeout — no book scanned. Reset.");
    signalWarning();
    exitState     = EXIT_IDLE;
    exitStudentID = "";
  }

  String uid = readUID(exitReader);
  if (uid == "") return;

  Serial.println("\n--- EXIT SCAN: " + uid + " ---");

  String student = getStudent(uid);
  String book    = getBook(uid);

  switch (exitState) {

    /* ── Waiting for the FIRST card ────────────────────────────────────────── */
    case EXIT_IDLE:
      if (student != "") {
        exitStudentID   = student;
        exitFirstScanMs = millis();
        exitState       = EXIT_WAIT_BOOK;
        signalPrompt();
        Serial.println("Exit student: " + student + " — now scan the BOOK card");
      } else if (book != "") {
        // Book scanned with NO student → treat as attempted theft
        Serial.println("THEFT ATTEMPT: Book scanned at exit with no student ID!");
        signalTheft();
        // State stays IDLE
      } else {
        Serial.println("Unknown tag at exit: " + uid);
        signalWarning();
      }
      break;

    /* ── Have student, waiting for BOOK ────────────────────────────────────── */
    case EXIT_WAIT_BOOK:
      if (book != "") {
        Serial.println("Exit book: " + book + " — verifying for " + exitStudentID);
        sendCheckout(exitStudentID, book);
        exitStudentID = "";
        exitState     = EXIT_IDLE;
      } else if (student != "") {
        // Another student card replaces the first
        Serial.println("New student at exit: " + student + " — resetting");
        exitStudentID   = student;
        exitFirstScanMs = millis();
        signalPrompt();
        Serial.println("Now scan the BOOK card for " + student);
      } else {
        Serial.println("Unknown tag at exit: " + uid);
        signalWarning();
      }
      break;
  }

  delay(800);
}

/* ══════════════════════════════════════════════════════════════════════════════
   SETUP
══════════════════════════════════════════════════════════════════════════════ */
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

  Serial.println("\nReady — scan STUDENT card at ENTRY reader (SS=5)");
  Serial.println("         scan STUDENT card at EXIT  reader (SS=27)");
}

/* ══════════════════════════════════════════════════════════════════════════════
   LOOP
══════════════════════════════════════════════════════════════════════════════ */
void loop() {
  handleIR();
  handleEntry();
  handleExit();

  // Periodic heartbeat on Serial
  if (millis() - lastStatusPrintMs > 4000) {
    lastStatusPrintMs = millis();
    String entryStatus = (entryState == ENTRY_IDLE) ? "IDLE" : "WAITING FOR BOOK (student=" + entryStudentID + ")";
    String exitStatus  = (exitState  == EXIT_IDLE)  ? "IDLE" : "WAITING FOR BOOK (student=" + exitStudentID  + ")";
    Serial.println("[STATUS] Entry: " + entryStatus + " | Exit: " + exitStatus +
                   " | People: " + String(peopleInLibrary));
  }
}
