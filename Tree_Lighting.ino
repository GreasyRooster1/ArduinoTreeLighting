#include <memory>
#include <WiFiNINA.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <FastLED.h>

#define SECRET_SSID "wilsonhub"
#define SECRET_PASS "tinyearth490"

#define LED_PIN 5
#define NUM_LEDS 1000 
#define DEFAULT_BRIGHTNESS 20 
#define PROGRAM_COUNT 2

#define MAP_SPIRAL 5
#define MAP_STAR 6
#define MAP_UNDERGLOW 7
#define MAP_LENGTH 8

CRGB leds[NUM_LEDS];
int frameCount = 0;

int mappings[8][2] = {
  {100,199}, //strand 1
  {200,299},  //strand 2
  {300,399},  //strand 3
  {400,499},  //strand 4
  {500,599},  //strand 5

  {600,699},  //spiral
  {700,799},  //star
  {800,899},  //underglow
};

class Program{
  public:
  Program() = default;
  virtual String getId() {};
  virtual void run() {}
  virtual void setup() {}
  virtual void disconnect() {}
};

class Firefly : public Program{
  public:
  Firefly() = default;

  String getId() {
    return "FIREFLY";
  };

  void setup(){}

  void run(){
    randomSeed(13);
    for (int i = 0; i < NUM_LEDS; i++) {
      float x = random()*1000 + frameCount;
      float m = 0.95;
      float b = 2;
      float bright = max(sin(b*x)-m,0)*(1/(1-m));
      leds[i].setRGB(255*bright, 198*bright, 145*bright);
    }
  }

  void disconnect(){
    randomSeed(analogRead(A0));
  }
};

class MappingTest : public Program{
  public:
  MappingTest() = default;
  CRGB mapColors[8] = {
    CRGB::Red,
    CRGB::Yellow,
    CRGB::Green,
    CRGB::Blue,
    CRGB::White,
    CRGB::Orange,
    CRGB::Violet,
    CRGB::Pink,
  };

  String getId() {
    return "MAP_TEST";
  };

  void run(){
    for (int i = 0; i < MAP_LENGTH; i++) {
      for(int j=mappings[i][0];j<mappings[i][1];j++){
        leds[j] = mapColors[i];
      }
    }
  }
};

Program* currentProgram;
std::unique_ptr<Program> programs[PROGRAM_COUNT];

char ssid[] = SECRET_SSID;    
char pass[] = SECRET_PASS;    
int keyIndex = 0;            

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  while (!Serial);
  Serial.println("asdasd");
  
  //begin setup
  fillColor(CRGB::Red);

  setupPrograms();
  setupServer();
  setupLighting();
  
  pinMode(LED_BUILTIN, OUTPUT);

  //setup concluded
  fillColor(CRGB::Green);
  delay(500);
  //switchProgram(programs[0]);
}

void loop() {
  runServer();
  //runLighting();
}

void setupPrograms(){
  programs[0] = std::unique_ptr<Firefly>(new Firefly());
  programs[1] = std::unique_ptr<MappingTest>(new MappingTest());
}

/*LIGHTING*/
void setupLighting() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void runLighting() {
  currentProgram->run();

  FastLED.show(); 

  frameCount++;
}

void switchProgram(Program* program) {
  if(program==nullptr){return;}
  if(currentProgram!=nullptr){
    currentProgram->disconnect();
  }
  currentProgram = program;
  Serial.println("Connecting new program");
  currentProgram->setup();
}

void fillColor(CRGB color){
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}


/* SERVER */
void setupServer(){
  Serial.begin(9600);      // initialize serial communication
  pinMode(LED_BUILTIN, OUTPUT);      // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();
}

void runServer(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("Current Program: ");
            client.print(currentProgram->getId());
            client.print("<br/>");

            // the content of the HTTP response follows the header:
            for(int i=0;i<PROGRAM_COUNT;i++){
              client.print("<a href='/");
              client.print(programs[i]->getId());
              client.print("'>");
              client.print(programs[i]->getId());
              client.print("</a><br>");
            }

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }


        // Check to see if the client request was a key
        for(int i=0;i<PROGRAM_COUNT;i++){
          if (currentLine.endsWith("GET /"+programs[i]->getId())) {
            Serial.print("/* SWITCHING PROGRAMS: ");
            Serial.println(programs[i]->getId());
            switchProgram(programs[i].get());
          }
        }
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

/* ERROR */
void fatalError(){
  while(true){
    delay(1000);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN,LOW);
  }
}

void loudError(CRGB color = CRGB::Red){
  while(true){
    delay(1000);
    digitalWrite(LED_BUILTIN,HIGH);
    fillColor(CRGB::Red);
    delay(1000);
    digitalWrite(LED_BUILTIN,LOW);
    fillColor(CRGB::Black);
  }
}




