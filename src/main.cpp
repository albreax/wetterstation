#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

// Definieren Sie hier Ihre e-Paper Display Variante
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=*/ 10, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));

// Standard Adafruit `GFX` initialisieren (falls erforderlich)
Adafruit_GFX* displaygfx = &display;

void setup() {
  // Initialisieren Sie die serielle Kommunikation (optional)
  Serial.begin(115200);
  while (!Serial);

  Serial.println();
  Serial.println("setup");

  // Setzen Sie die SPI-Pins für den ESP32
  SPI.begin(/*SCK=*/ 13, /*MISO=*/ -1, /*MOSI=*/ 11, /*SS=*/ 5);

  // Initialisieren Sie das Display
  display.init(115200); // Die Baudrate kann bei Bedarf angepasst werden

  // Beispielanzeige - Ein einfaches "Hello World"
  display.setRotation(1); // Optional: Anzeigerotation einstellen
  display.clearScreen(); 
  display.refresh();
  display.fillScreen(GxEPD_WHITE);
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(0, 0);
  display.print("Hello World!");
  display.display();

  Serial.println("setup done");
}

int i = 0;

void loop() {
 // Teilerneuerung verwenden
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(GxEPD_BLACK);
  display.print("Hello World: " + String(i));
  i=i+1;
  display.display(true); // true aktiviert die Teilerneuerung
  delay(1000);

}
