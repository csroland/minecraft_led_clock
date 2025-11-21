#include <Arduino.h>
#include <FastLED.h>
#include <clockfaces.h>
#include <WiFi.h>
#include <time.h>
#include "clock_webserver.h"

#define LED_PIN     7
#define NUM_LEDS    50
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB


//change these to your own WiFi's credentials:
const char* ssid = "Your SSID";
const char* password = "Your PW";

//variables for the current time
//adjust gmtOffset_sec and daylightOffset_sec to your location
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

//variables, for updating the screen and data
int last_p_frame_ms = millis();
int last_p_frame_nr = 0;

int last_ntp_update = 0;
int last_c_frame = 0;

float pos = 0.0f;
float vel = 0.0f;
int target_c_frame = 0;

float stiffness = 0.03f;
float damping = 0.80f;

int lastRandom = 0;
int RANDOM_INTERVAL = 300;

unsigned long last_time_check = 0;
const int time_check_interval = 30000;

CRGB leds[NUM_LEDS];

void moveClockTo(int frame) {
    //checks of the given frame is not out of bounds, and sets the target within bound
    if (frame < 0) frame = 0;
    if (frame >= 64) frame = 64 - 1;

    target_c_frame = frame;
}

void dispPortal() {
    //display the portal animation on the matrix
    if (millis() - last_p_frame_ms > 50) {
        last_p_frame_nr++;
        if (last_p_frame_nr >= 32) {
            last_p_frame_nr = 0;
        }
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB(
                nether[last_p_frame_nr][i][0],
                nether[last_p_frame_nr][i][1],
                nether[last_p_frame_nr][i][2]
            );
        };
        FastLED.show();
        last_p_frame_ms = millis();
    };
}

void updateClockFrame() {
    //update the clock frame with 20 FPS, using spring mechanics for smoothness
    unsigned long now = millis();
    if (now - last_p_frame_ms < 50) return;
    last_p_frame_ms = now;

    float x = pos - target_c_frame;

    float force = -stiffness * x;

    vel = vel * damping + force;

    pos += vel;

    int frame = (int)pos;
    if (frame < 0) frame = 0;
    if (frame > 63) frame = 63;

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(
            clockface[frame][i][0],
            clockface[frame][i][1],
            clockface[frame][i][2]
        );
    }
    FastLED.show();
}


int getCurrentClockFrame() {
    //get the current clock frame from the current time
    //The 0th frame is midnight, and the 32nd is midnight
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    float totalDaySeconds = hour*3600 + minute*60 + second;
    float fraction = totalDaySeconds / 86400.0f;  // 24h = 86400s

    int frame = (int)(fraction * 64.0f);
    frame = (frame + 32) % 64;
    if (frame < 0) frame = 0;
    if (frame > 63) frame = 63;

    return frame;
}

int getCurrentClockFrameMinecraft() {
    //get the current clock frame from the current time, adjust for 20 minute days
    //essentially the same as the previous one
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    float totalDaySeconds = hour*3600 + minute*60 + second;
    float fraction = totalDaySeconds / 1200.0f;  // 20 minutes = 1200 sec

    int frame = (int)(fraction * 64.0f);
    frame = (frame + 32) % 64;
    if (frame < 0) frame = 0;
    if (frame > 63) frame = 63;

    return frame;
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(50);
    while (WiFi.status() != WL_CONNECTED) {
        dispPortal();
    };
    Serial.println("Connected");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    unsigned long lastRandom = 0;

    setupWebServer();

    while (!getLocalTime(&timeinfo)) {
        unsigned long now = millis();
        if (now - lastRandom > 300) {
            lastRandom = now;
            moveClockTo(random(0, 64));
        }
        updateClockFrame();
        Serial.println("Getting time");
    };
}

void loop() {
    updateClockFrame();
    server.handleClient();
    unsigned long now = millis();
    if (now - last_time_check > time_check_interval) {
        last_time_check = now;
        int frame;
        if (mode == 0) {
            frame = getCurrentClockFrame();
        } else if (mode == 1) {
            frame = getCurrentClockFrameMinecraft();
        }
        moveClockTo(frame);
    }
}
