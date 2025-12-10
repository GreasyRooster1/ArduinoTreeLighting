#include "arduino_secrets.h"

#include <WiFiNINA.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

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
