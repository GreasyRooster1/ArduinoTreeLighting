#include <memory>
#include <FastLED.h>
#define LED_PIN 5
#define BUTTON_PIN 4
#define NUM_LEDS 600 
#define DEFAULT_BRIGHTNESS 20 
#define PROGRAM_COUNT 3

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

class BitTest : public Program{
  public:
  BitTest() = default;
  String getId() {
    return "MAP_TEST";
  };

  void run(){
    int frame = int(floor(frameCount/60.0))%11;
    for (int i = 0; i < NUM_LEDS; i++) {
      if(bitRead(i,frame)){
        leds[i] = CRGB::Blue;
      }else{
        leds[i] = CRGB::Black;
      }
    }
  }
};

int currentProgIndex = 0;
Program* currentProgram;
std::unique_ptr<Program> programs[PROGRAM_COUNT];

int keyIndex = 0;            

void setup() {
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  while (!Serial);
  Serial.println("asdasd");
  
  //begin setup
  fillColor(CRGB::Red);

  setupPrograms();
  setupLighting();
  
  pinMode(LED_BUILTIN, OUTPUT);

  //setup concluded
  fillColor(CRGB::Green);
  delay(500);
  //switchProgram(programs[0]);
}

void loop() {
  runLighting();
  if(digitalRead(BUTTON_PIN)==HIGH){
    currentProgIndex++;
    currentProgIndex=currentProgIndex%PROGRAM_COUNT;
    switchProgram(programs[currentProgIndex].get());
  }
}

void setupPrograms(){
  programs[0] = std::unique_ptr<Firefly>(new Firefly());
  programs[1] = std::unique_ptr<MappingTest>(new MappingTest());
  programs[2] = std::unique_ptr<BitTest>(new BitTest());
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




