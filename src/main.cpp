
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <MFRC522.h>

// TFT pins
#define TFT_CS   8
#define TFT_DC   9
#define TFT_RST  10

// Shared SPI
#define SPI_SCK  12
#define SPI_MISO 13
#define SPI_MOSI 11

// RFID
#define RFID_SS  5
#define RFID_RST 4

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
MFRC522 rfid(RFID_SS, RFID_RST);

void showHeader() {
    tft.fillScreen(ST77XX_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);

    tft.setCursor(10, 10);
    tft.println("RFID ANALYZER");

    tft.drawFastHLine(10, 40, 300, ST77XX_WHITE);
}

void showWaiting() {
    showHeader();

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 75);
    tft.println("RC522 READY");

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 125);
    tft.println("Waiting for");

    tft.setCursor(10, 155);
    tft.println("card...");
}

void showCard() {
    showHeader();

    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 60);
    tft.println("CARD DETECTED");

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 105);
    tft.println("UID:");

    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 135);

    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) {
            tft.print("0");
        }

        tft.print(rfid.uid.uidByte[i], HEX);
        tft.print(" ");
    }

    MFRC522::PICC_Type type =
        rfid.PICC_GetType(rfid.uid.sak);

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 190);
    tft.println("CARD TYPE:");

    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 220);
    tft.println(rfid.PICC_GetTypeName(type));
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    // Keep both SPI devices inactive
    pinMode(TFT_CS, OUTPUT);
    pinMode(RFID_SS, OUTPUT);

    digitalWrite(TFT_CS, HIGH);
    digitalWrite(RFID_SS, HIGH);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, RFID_SS);

    // Initialize TFT
    tft.init(240, 320);
    tft.setRotation(1);

    showWaiting();

    // Initialize RC522
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(RFID_SS, HIGH);

    rfid.PCD_Init();
    delay(100);

    Serial.println("RFID ANALYZER READY");

    rfid.PCD_DumpVersionToSerial();
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent()) {
        return;
    }

    if (!rfid.PICC_ReadCardSerial()) {
        return;
    }

    Serial.println("\n========== CARD DETECTED ==========");

    Serial.print("UID: ");

    for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) {
            Serial.print("0");
        }

        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
    }

    Serial.println();

    MFRC522::PICC_Type type =
        rfid.PICC_GetType(rfid.uid.sak);

    Serial.print("CARD TYPE: ");
    Serial.println(rfid.PICC_GetTypeName(type));

    showCard();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    delay(2000);

    showWaiting();
}
