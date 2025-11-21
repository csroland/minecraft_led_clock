#include "clock_webserver.h"

WebServer server(80);
int mode = 0; // 0 = IRL, 1 = Minecraft

void setupWebServer() {
    server.on("/", HTTP_GET, []() {
        String html = "<html><body>";
        html += "<h2>Minecraft Clock</h2>";
        html += "<p>Current time: " + String(hour()) + ":" + String(minute()) + ":" + String(second()) + "</p>";
        html += "<form action='/set_brightness' method='get'>";
        html += "Brightness: <input type='range' min='1' max='255' name='b' value='" + String(FastLED.getBrightness()) + "'>";
        html += "<input type='submit' value='Set'>";
        html += "</form>";
        html += "<form action='/set_mode' method='get'>";
        html += "<input type='radio' name='m' value='0' " + String(mode==0?"checked":"") + "> IRL ";
        html += "<input type='radio' name='m' value='1' " + String(mode==1?"checked":"") + "> Minecraft ";
        html += "<input type='submit' value='Set Mode'>";
        html += "</form>";
        html += "<script>setTimeout(()=>location.reload(),5000);</script>";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/set_brightness", HTTP_GET, []() {
        if (server.hasArg("b")) {
            int b = server.arg("b").toInt();
            FastLED.setBrightness(b);
        }
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/set_mode", HTTP_GET, []() {
        if (server.hasArg("m")) {
            mode = server.arg("m").toInt();
        }
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.begin();
}

int hour() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_hour;
}
int minute() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_min;
}
int second() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_sec;
}