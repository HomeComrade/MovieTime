#include <Time.h>
#include <Adafruit_NeoPixel.h>

#define LED_COUNT 3
#define LED_OUT_PIN 6

#define TIME_HEADER "T"
#define TIME_REQUEST 7

#define STATUS_UNKNOWN 0
#define STATUS_GOOD 1
#define STATUS_START_DOWNLOAD 2
#define STATUS_SHOULD_BE_DOWNLOADING 3
#define STATUS_START_WATCHING 4
#define STATUS_TOO_LATE 5

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_OUT_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t RED = strip.Color(255, 0, 0);//red
const uint32_t PINK = strip.Color(255, 0, 255);//pink
const uint32_t BLUE = strip.Color(0, 0, 255);//blue
const uint32_t TEAL = strip.Color(0, 255, 255);//teal
const uint32_t GREEN = strip.Color(0, 255, 0);//green
const uint32_t YELLOW = strip.Color(255, 255, 0);//yellow
const uint32_t ORANGE = strip.Color(255, 128, 0);//orange
const uint32_t WHITE = strip.Color(255, 255, 255);//white
const uint32_t OFF = strip.Color(0, 0, 0);//off

void setup() {
  Serial.begin(9600);
  
  setSyncProvider(requestSync);
  
  strip.begin();
  strip.setBrightness(20);
  strip.show();
  
  Serial.println("Waiting for sync message");
}

void loop() {
  if (Serial.available()) {
    processSyncMessage();
  }
  
  if (timeStatus() == timeNotSet) {
    notSynced();
  }
  else {
    synced();
    /*
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.print(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(month());
    Serial.print(" ");
    Serial.print(year());
    Serial.println();
    */
  }
  
  delay(1000);
}

void notSynced() {
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, RED);
  }
  
  strip.show();
  delay(1000);
  
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, OFF);
  }
  
  strip.show();
}

void synced() {
  int currentStatus = getStatus();
  
  switch (currentStatus) {
    case STATUS_GOOD:
      for (uint16_t i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, GREEN);
      }
      break;
    case STATUS_START_DOWNLOAD:
      strip.setPixelColor(0, GREEN);
      strip.setPixelColor(1, GREEN);
      strip.setPixelColor(2, YELLOW);
      break;
    case STATUS_SHOULD_BE_DOWNLOADING:
      strip.setPixelColor(0, GREEN);
      strip.setPixelColor(1, YELLOW);
      strip.setPixelColor(2, YELLOW);
      break;
    case STATUS_START_WATCHING:
      strip.setPixelColor(0, YELLOW);
      strip.setPixelColor(1, YELLOW);
      strip.setPixelColor(2, YELLOW);
      break;
    case STATUS_TOO_LATE:
      for (uint16_t i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, RED);
      }
      break;
  }
  
  strip.show();
}

int getStatus() {
  //hold our important times
  int iweekday = weekday();
  int ihour = hour();
  int iminute = minute();
  
  //the weekday is between sunday and thursday (inclusive)
  if (iweekday < 6) {
    //between 9AM and 21PM, movies are wonderful!
    if (ihour > 9 && ihour < 21) {
      return STATUS_GOOD;
    }
    
    //if it is 21PM, we transition into new rules
    if (ihour == 21) {
      //21:00 - 21:30 is start downloading status
      if (iminute < 30) {
        return STATUS_START_DOWNLOAD;
      }
      //21:30 - 21:59 is should be downloading status
      else {
        return STATUS_SHOULD_BE_DOWNLOADING;
      }
    }
    
    //if it is 22PM, transition into new rules
    if (ihour == 22 && iminute < 30) {
      return STATUS_START_WATCHING;
    }
    
    //any other time, it is too late
    return STATUS_TOO_LATE;
  }
  else {
    //from 8AM till midnight, all is fine
    if (ihour > 8) {
      return STATUS_GOOD;
    }
    
    //if it is 00, then we must start downloading
    if (ihour == 0) {
      if (iminute < 30) {
        return STATUS_START_DOWNLOAD;
      }
      else {
        return STATUS_SHOULD_BE_DOWNLOADING;
      }
    }
    
    if (ihour == 1 && iminute < 30) {
      return STATUS_START_WATCHING;
    }
    
    //if its past 1:30AM, then it is too late
    return STATUS_TOO_LATE;
  }
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  
  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    
    if (pctime >= DEFAULT_TIME) {
      setTime(pctime);
    }
  }
}

time_t requestSync() {
  Serial.write(TIME_REQUEST);
  return 0;
}
