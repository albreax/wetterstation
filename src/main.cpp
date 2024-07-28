#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "config.h" 

// NTP-Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);

// Definieren Sie hier Ihre e-Paper Display Variante
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

void setup() {
  // Initialisieren Sie die serielle Kommunikation (optional)
  Serial.begin(115200);
  
  // Setzen Sie die SPI-Pins für den ESP32
  SPI.begin(/*SCK=*/ 13, /*MISO=*/ -1, /*MOSI=*/ 11, /*SS=*/ 10);

  // Initialisieren Sie das Display
  display.init(115200);

  // Verbinden Sie sich mit dem WLAN
  display.nextPage();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");

  }
  Serial.println("Connected to WiFi");
  display.println("Connected to WiFi");
  display.display();
  // NTP-Client initialisieren
  timeClient.begin();
}

// Funktion zur Erkennung der Sommerzeit in Europa
bool isSummerTimeEurope(struct tm &time) {
  // Letzter Sonntag im März, 1:00 UTC
  if (time.tm_mon > 2 && time.tm_mon < 9) return true; // April bis September ist Sommerzeit
  if (time.tm_mon < 2 || time.tm_mon > 9) return false; // Januar, Februar, November, Dezember ist keine Sommerzeit
  
  int lastSunday = time.tm_mday - (time.tm_wday ? time.tm_wday : 7);
  if (time.tm_mon == 2) return (time.tm_hour >= 1) && (time.tm_mday > lastSunday); // März
  return (time.tm_hour < 1) && (time.tm_mday < lastSunday); // Oktober
}

void loop() {
  // Aktualisieren Sie die Zeit vom NTP-Server
  timeClient.update();

  // Holen Sie sich das aktuelle Datum und die Uhrzeit
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime); 

  // Sommerzeit berücksichtigen
  if (isSummerTimeEurope(*ptm)) {
    epochTime += 3600; // Eine Stunde hinzufügen
    ptm = gmtime((time_t *)&epochTime);
  }

  int day = ptm->tm_mday;
  int month = ptm->tm_mon + 1;
  int year = ptm->tm_year + 1900;
  int hour = ptm->tm_hour;
  int minute = ptm->tm_min;

  // Datum und Uhrzeit formatieren
  char dateBuffer[16];
  snprintf(dateBuffer, sizeof(dateBuffer), "%02d.%02d.%04d", day, month, year);

  char timeBuffer[16];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", hour, minute);

  // Aktualisieren Sie das Display
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.println(String(dateBuffer) + "              " + String(timeBuffer) + " Uhr");
  } while (display.nextPage());

  // Warten Sie eine Sekunde vor der nächsten Aktualisierung
  delay(1000);
}

