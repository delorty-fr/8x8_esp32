#include <WiFi.h>
#include <WiFiProv.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <string>

#define WORD_LEDS_PIN 17
#define MATRIX_WIDTH  8
#define MATRIX_HEIGHT 8
#define NUM_LEDS      MATRIX_WIDTH*MATRIX_HEIGHT

CRGB _leds[NUM_LEDS];

const char* ssid            = "";
const char* password        = "";
const String LAMBDA_API_KEY = "";
const String GET_ICON_URL   = "";

void setAllLEDs(CRGB c, CRGB* strip, uint16_t numLeds) {
  for (uint16_t i = 0; i < numLeds; ++i) {
      strip[i] = c;
  }
  FastLED.show();
}

// RAII wrapper for HTTPClient
class HTTPClientWrapper {
  private:
    HTTPClient& _http;
  public:
    HTTPClientWrapper(HTTPClient& http) : _http(http) {};
    ~HTTPClientWrapper() {
      _http.end();  
    };
};

int charToHex(char *value) {
  char *ptr;
  return strtoul(value, &ptr ,16);
}

void displayIcon(CRGB* pixelValues) {

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Error in WiFi connection");
    setAllLEDs(CRGB::Red, _leds, 1);
    return;
  }

  HTTPClient http;
  HTTPClientWrapper wrapper(http);
  
  http.begin(GET_ICON_URL);
  http.addHeader("X-Api-Key", LAMBDA_API_KEY);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();
  String content = http.getString();

  if (httpCode != 200) {
    setAllLEDs(CRGB::Red, _leds, 1);
    Serial.print("displayIcon - error code: ");  Serial.println(httpCode);
    Serial.print("displayIcon - content: ");  Serial.println(content);
    return;
  }

  Serial.println(content);
  
  // String -> char *
  char char_content[content.length() + 1];
  content.toCharArray(char_content, content.length() + 1);
  
  char *result = char_content+1; 
  char *value;
  const char *delimiter =" ";
  uint8_t i = 0;
  value = strtok(result, delimiter);
  while (value != NULL && i < NUM_LEDS) {
    pixelValues[i] = CRGB(charToHex(value));
    value=strtok(NULL, delimiter);
    i++;
  }
  FastLED.show();

}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  FastLED.addLeds<WS2812B, WORD_LEDS_PIN, GRB>(_leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 200);

  setAllLEDs(CRGB::Red, _leds, 1);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("CONNECTED");

  setAllLEDs(CRGB::Green, _leds, 1);
  delay(1000);

  setAllLEDs(CRGB::Black, _leds, NUM_LEDS);
}

void loop() {
  displayIcon(_leds);
  delay(30000);
}
