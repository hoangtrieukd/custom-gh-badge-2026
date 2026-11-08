
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS   5
#define RFID_RST  4
#define RFID_SCK  12
#define RFID_MISO 13
#define RFID_MOSI 11

MFRC522 rfid(RFID_SS, RFID_RST);

void printHex(byte value) {
    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);
}

void printCardType(MFRC522::PICC_Type type) {
    Serial.print("Card type: ");
    Serial.println(rfid.PICC_GetTypeName(type));
}

void analyzeCard() {
    Serial.println("\n========== RFID CARD ANALYZER ==========");

    // UID
    Serial.println("\n[1] CARD IDENTIFICATION");

    Serial.print("UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
        printHex(rfid.uid.uidByte[i]);
        Serial.print(" ");
    }
    Serial.println();

    Serial.printf("UID length: %d bytes\n", rfid.uid.size);

    // SAK
    Serial.print("SAK: 0x");
    printHex(rfid.uid.sak);
    Serial.println();

    // ATQA
    byte atqa[2];
    byte atqaSize = sizeof(atqa);

    MFRC522::StatusCode status =
        rfid.PICC_WakeupA(atqa, &atqaSize);

    if (status == MFRC522::STATUS_OK ||
        status == MFRC522::STATUS_COLLISION) {
        Serial.print("ATQA: ");
        for (byte i = 0; i < atqaSize; i++) {
            printHex(atqa[i]);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("ATQA: unavailable");
    }

    // Card type
    Serial.println("\n[2] CARD CHARACTERISTICS");

    MFRC522::PICC_Type type =
        rfid.PICC_GetType(rfid.uid.sak);

    printCardType(type);

    if (type == MFRC522::PICC_TYPE_MIFARE_1K) {
        Serial.println("Reported capacity: 1024 bytes");
        Serial.println("Reported sectors: 16");
        Serial.println("Reported blocks: 64");
    } else if (type == MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println("Reported capacity: 4096 bytes");
        Serial.println("Reported sectors: 40");
        Serial.println("Reported blocks: 256");
    } else {
        Serial.println("Memory layout: not determined");
    }

    // Security assessment
    Serial.println("\n[3] SECURITY CHARACTERISTICS");

    if (type == MFRC522::PICC_TYPE_MIFARE_1K ||
        type == MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println("Authentication: supported");
        Serial.println("Sector access: may be protected");
        Serial.println("Encryption: card-family dependent");
        Serial.println("UID changeability: not determined");
    } else {
        Serial.println("Security details: not determined");
    }

    // Conclusion
    Serial.println("\n[4] COMPATIBILITY REPORT");

    Serial.println("Card detected: YES");
    Serial.println("Protocol compatibility: ISO14443A reader");

    Serial.println(
        "Writable memory: NOT TESTED"
    );

    Serial.println(
        "Complete card duplication: NOT DETERMINED"
    );

    Serial.println("========================================");

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nRFID CARD ANALYZER v1.0");

    SPI.begin(
        RFID_SCK,
        RFID_MISO,
        RFID_MOSI,
        RFID_SS
    );

    rfid.PCD_Init();
    delay(50);

    byte version = rfid.PCD_ReadRegister(
        MFRC522::VersionReg
    );

    Serial.print("RC522 version: 0x");
    printHex(version);
    Serial.println();

    if (version == 0x00 || version == 0xFF) {
        Serial.println("ERROR: Reader communication failed.");
        while (true) delay(1000);
    }

    Serial.println("Reader ready.");
    Serial.println("Place a card near the reader.");
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent()) {
        return;
    }

    if (!rfid.PICC_ReadCardSerial()) {
        return;
    }

    analyzeCard();

    delay(1000);
}
