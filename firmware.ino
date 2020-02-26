#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Ethernet.h>

#define DHTPIN 2
#define DHTTYPE 22
#define DISPLAY_COL 20
#define DISPLAY_ROW 4
#define LISTENPORT 9871

DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, DISPLAY_COL, DISPLAY_ROW);
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x32, 0x2C }; // MAC ADDRESS
IPAddress ip(192, 168, 0, 230); // IP Address in DEC format
byte gw[] = { 192, 168, 0, 1 }; // Gateway address
byte mask[] = {255, 255, 255, 0}; // Netmask
EthernetServer server(LISTENPORT);
float temp = 0.0f;
bool linked = true;
sensors_event_t event;

void setup() {
  lcd.init();
  lcd.setBacklight(1);
  //Serial.begin(57600);
  Ethernet.begin(mac, ip , gw, mask);
  server.begin();

  lcd.setCursor(0, 0);
  lcd.print("IP: ");
  lcd.print(Ethernet.localIP());

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delay(1000);
  //Serial.println("Starting...");
}

void loop() {
  dht.temperature().getEvent(&event);
  temp = event.temperature;
  lcd.setCursor(0, 1);
  lcd.print("T:  ");
  lcd.print(temp);
  lcd.setCursor(0, 2);
  lcd.print("Server listening...");
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");

    boolean currentLineIsBlank = true;

    dht.temperature().getEvent(&event);
    temp = event.temperature;
    lcd.setCursor(0, 1);
    lcd.print("T:  ");
    lcd.print(temp);
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<meta http-equiv=\"refresh\" content=\"5\">");

          client.print("{ 'temperature': '");
          client.print(temp);
          client.print("' }\n");
          
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    //Serial.println("client disonnected");
  }
}
