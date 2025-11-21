#pragma once
#include <WebServer.h>
#include <time.h>
#include <FastLED.h>

extern WebServer server;
extern int mode;

void setupWebServer();
int hour();
int minute();
int second();