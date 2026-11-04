
#include <Arduino.h>

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" GITHUB BADGE 2026");
  Serial.println(" ESP32-S3 Hardware Diagnostic");
  Serial.println("==============================");

  Serial.printf("Chip model: %s\n", ESP.getChipModel());
  Serial.printf("CPU frequency: %u MHz\n",
                ESP.getCpuFreqMHz());

  Serial.printf("Flash size: %u MB\n",
                ESP.getFlashChipSize() / (1024 * 1024));

  Serial.printf("PSRAM detected: %s\n",
                psramFound() ? "YES" : "NO");

  Serial.printf("PSRAM size: %u MB\n",
                ESP.getPsramSize() / (1024 * 1024));

  Serial.println("==============================");
  Serial.println(" Diagnostic complete!");
}

void loop()
{
  Serial.println("GitHub Badge 2026 is running...");
  delay(3000);
}
