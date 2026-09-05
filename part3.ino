// Part 3: control two LED strips using the downloaded JSON data.
// Upload lightdata_raw.json to the Pico's LittleFS before running this sketch.

#include <ArduinoJson.h>
#include <FastLED.h>
#include <LittleFS.h>

#define LEDS_PER_STRIP 3
#define STRIP_1_PIN 2
#define STRIP_2_PIN 3
#define MAX_FRAMES 10

// Choose which two fields from the JSON control the strips.
const char* STRIP_1_DATA = "chestR";
const char* STRIP_2_DATA = "face";

CRGB strip1[LEDS_PER_STRIP];
CRGB strip2[LEDS_PER_STRIP];

struct Frame {
  uint32_t timeTicks;
  uint32_t strip1Data;
  uint32_t strip2Data;
};

Frame frames[MAX_FRAMES];
int frameCount = 0;
int frameIndex = 0;
unsigned long playbackStart = 0;

bool loadFrames() {
  File file = LittleFS.open("/lightdata_raw.json", "r");
  if (!file) return false;

  JsonDocument document;
  DeserializationError error = deserializeJson(document, file);
  file.close();
  if (error) return false;

  JsonArray playerData = document["player_data"].as<JsonArray>();
  if (playerData.isNull()) return false;

  frameCount = min((int)playerData.size(), MAX_FRAMES);

  for (int i = 0; i < frameCount; i++) {
    frames[i].timeTicks = playerData[i]["time"] | 0U;
    frames[i].strip1Data = playerData[i][STRIP_1_DATA] | 0U;
    frames[i].strip2Data = playerData[i][STRIP_2_DATA] | 0U;
  }

  return frameCount > 0;
}

void printLoadedFrames() {
  Serial.println("Loaded /lightdata_raw.json");

  for (int i = 0; i < frameCount; i++) {
    Serial.print("time: ");
    Serial.print(frames[i].timeTicks);
    Serial.print(" ticks (");
    Serial.print(frames[i].timeTicks * 50UL);
    Serial.println(" ms)");

    const char* bodyPartNames[] = {STRIP_1_DATA, STRIP_2_DATA};
    const uint32_t bodyPartData[] = {
      frames[i].strip1Data,
      frames[i].strip2Data
    };

    for (int bodyPart = 0; bodyPart < 2; bodyPart++) {
      uint32_t rgb = bodyPartData[bodyPart] >> 8;

      Serial.print("  ");
      Serial.print(bodyPartNames[bodyPart]);
      Serial.print(": rgb(");
      Serial.print((rgb >> 16) & 0xFF);
      Serial.print(", ");
      Serial.print((rgb >> 8) & 0xFF);
      Serial.print(", ");
      Serial.print(rgb & 0xFF);
      Serial.println(")");
    }
  }
}

void setStrip(CRGB leds[], uint32_t packedData) {
  // Bits 31..8 contain the RGB color.
  uint32_t rgb = packedData >> 8;

  // Bits 7..4 contain brightness from 0 to 15.
  uint8_t brightnessLevel = (packedData >> 4) & 0x0F;
  uint8_t brightness = uint8_t(
    powf(brightnessLevel / 15.0f, 2.2f) * 255
  );

  CRGB color(
    (rgb >> 16) & 0xFF,
    (rgb >> 8) & 0xFF,
    rgb & 0xFF
  );
  color.nscale8_video(brightness);

  fill_solid(leds, LEDS_PER_STRIP, color);
}

void showFrame(int index) {
  setStrip(strip1, frames[index].strip1Data);
  setStrip(strip2, frames[index].strip2Data);
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    delay(10);
  }

  FastLED.addLeds<NEOPIXEL, STRIP_1_PIN>(strip1, LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, STRIP_2_PIN>(strip2, LEDS_PER_STRIP);
  FastLED.setBrightness(100);
  FastLED.clear(true);

  if (!LittleFS.begin() || !loadFrames()) {
    Serial.println("Could not load /lightdata_raw.json");
    return;
  }
  Serial.println("Loaded /lightdata_raw.json");
  printLoadedFrames();

  playbackStart = millis();
  showFrame(0);
}

void loop() {
  if (frameCount == 0 || frameIndex >= frameCount - 1) return;

  unsigned long elapsed = millis() - playbackStart;
  unsigned long nextFrameTime = frames[frameIndex + 1].timeTicks * 50UL;

  if (elapsed >= nextFrameTime) {
    frameIndex++;
    showFrame(frameIndex);
  }
}

// Exercise:
// 1. Change STRIP_1_DATA and STRIP_2_DATA to two other JSON field names.
// 2. Change FastLED.setBrightness(100) to a lower or higher value.
