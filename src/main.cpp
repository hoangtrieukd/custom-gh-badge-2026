
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "github_logo.h"
#include "secrets.h"

#define TFT_CS 8
#define TFT_RST 10
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO 13

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

const char *githubUsername = "hoangtrieukd";

const char *githubApiUrl =
    "https://api.github.com/users/hoangtrieukd";

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void drawBadge()
{
    tft.fillScreen(ST77XX_BLACK);

    int logoX = (SCREEN_WIDTH - GITHUB_LOGO_WIDTH) / 2;
    int logoY = 35;

    tft.drawBitmap(
        logoX,
        logoY,
        github_logo_bitmap,
        GITHUB_LOGO_WIDTH,
        GITHUB_LOGO_HEIGHT,
        ST77XX_WHITE);

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(false);

    int16_t textX, textY;
    uint16_t textW, textH;

    tft.getTextBounds(
        githubUsername, 0, 0,
        &textX, &textY, &textW, &textH);

    tft.setCursor((SCREEN_WIDTH - textW) / 2, 180);
    tft.print(githubUsername);
}

void connectWiFi()
{
    Serial.println("\nConnecting to Wi-Fi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < 15000)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Wi-Fi connection failed.");
    }
}

void fetchGitHubProfile()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("No Wi-Fi connection. Cannot fetch profile.");
        return;
    }

    Serial.println("\nFetching GitHub profile...");

    WiFiClientSecure client;

    // Temporary prototype setting.
    // We'll configure certificate validation later.
    client.setInsecure();

    HTTPClient http;

    http.setTimeout(10000);

    if (!http.begin(client, githubApiUrl))
    {
        Serial.println("Could not initialize HTTP connection.");
        return;
    }

    http.addHeader("User-Agent", "ESP32-GitHub-Badge");
    http.addHeader("Accept", "application/vnd.github+json");

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (error)
        {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        Serial.println("\n========== GITHUB PROFILE ==========");

        Serial.print("Username: ");
        Serial.println(doc["login"] | "N/A");

        Serial.print("Name: ");
        Serial.println(doc["name"] | "N/A");

        Serial.print("Public repositories: ");
        Serial.println(doc["public_repos"] | 0);

        Serial.print("Followers: ");
        Serial.println(doc["followers"] | 0);

        Serial.print("Following: ");
        Serial.println(doc["following"] | 0);

        Serial.println("====================================");
    }
    else
    {
        Serial.print("GitHub API request failed. HTTP code: ");
        Serial.println(httpCode);

        if (httpCode > 0)
        {
            Serial.println(http.getString());
        }
    }

    http.end();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nGitHub Badge 2026");

    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

    tft.init(240, 320);
    tft.setRotation(1);

    drawBadge();

    Serial.println("TFT initialized.");

    connectWiFi();

    fetchGitHubProfile();

    Serial.println("\nBadge initialization complete.");
}

void loop()
{
    // Nothing to update yet.
}
