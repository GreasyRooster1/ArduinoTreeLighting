#include <WiFiNINA.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#define SECRET_SSID "wilsonhub"
#define SECRET_PASSWORD "tinyearth490"

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  WiFi.begin("wilsonhub", "tinyearth490");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println(client.remoteIP());
  }
}
