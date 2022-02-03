// Modified Sketch for "The Kube" sensor, incorporating the ESP8266 with OLED screen, DHT22 sensor, OTA and the ability to incorporate other external sensors.
// ------------------------------------------------------------------------
// Created by:
// Jacques Laroche
// https://github.com/jlar0che
// ------------------------------------------------------------------------
// Current Version:
// 0.5.6
// Last Modified: 2/3/2022
// ------------------------------------------------------------------------
// Inspiration and base project:
//  bkpsu:
//  https://www.thingiverse.com/thing:2539897
//
//  Borya:
//  https://www.instructables.com/id/ESP8266-GMail-Sender/
// ------------------------------------------------------------------------
// NodeMCU Installation in Arduino IDE:
// ------------------------------------
// Start Arduino and open Preferences window.
// Enter https://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
// Open Boards Manager from Tools > Board menu and find esp8266 platform.
// Select the version you need from a drop-down box.
// Click install button.
// Don’t forget to select your ESP8266 board from Tools > Board menu after installation.
// ------------------------------------------------------------------------
// Necessary Libraries: Install the following
// ------------------------------------------
// Adafruit Unified Sensor by Adafruit (v 1.0.2 or higher)
// ArduinoOTA by Juraj Andrassy (v 1.0.0 or higher)
// DHT Sensor Library by Adafruit (v 1.3.0 or higher)
// ESP8266 and ESP32 Oled Driver for SSD1306 Display by ThingPulse, Fabrice Weinberg (v 4.0.0 or higher)
// ESP8266_mDNS by Mr. Dunk (v 1.17 or higher)
// WiFiManager by tzapu and maintained by tablatronix (v 0.12.0 or higher)
// ------------------------------------------------------------------------
// Important Resources:
// --------------------
// https://www.base64encode.org/  - For encoding your username (email) and email password into Base64
// http://tomeko.net/online_tools/cpp_text_escape.php?lang=en - For converting Text to C or C++ strings


/************ HARDWARE CONFIG (CHANGE THESE FOR YOUR SENSOR SETUP) ******************/
#define REMOTE //Uncomment to enable remote sensor functionality via Webserver (Wifi)
//#define OLED_SPI //Uncomment if using SPI OLED screen (assuming I2C otherwise)
#define CELSIUS //Uncomment if you want temperature displayed in Celsius, Comment out if you want Fahrenheit
//#define DEEP_SLEEP //Uncomment if you want sensor to sleep after every update (Does NOT work with MOTION_ON, LED_ON or OTA_UPDATE which require constant uptime)
//#define FLIP_SCREEN //Uncomment if mounting to wall with USB connector on top
//#define MOTION_ON //Uncomment if using motion sensor
//#define OLED_MOTION //Uncomment if you want screen to turn on only if motion is detected
//#define LED_ON //Uncomment if using as LED controller
//#define PRESS_ON //Uncomment if using as Pressure monitor
#define ANTI_BURNIN //Uncomment if you want the OLED to invert colors every time the sensor is updated (to prevent burnin)

/************ WIFI and OTA INFORMATION (CHANGE THESE FOR YOUR SETUP) ******************/
//#define wifi_ssid "your_wifi_ssid_here_(keep_the_quotes)" //enter your WIFI SSID
//#define wifi_password "your_wifi_password_here_(keep_the_quotes)" //enter your WIFI Password

#define device_name "Server Room" //device name displayed on LCD
/********************************************************************************************/

/****************************** DHT 22 Calibration settings *************/
float temp_offset = -7.0;
//float hum_offset = 14.9;
/************************************************************************/

/****************************** Declarations for Web Server *************/
float Temperature;
float TemperatureF;
float Humidity;
/************************************************************************/

/****************************** Declarations for Progress Bar ***********/
int counter = 1;
/************************************************************************/

#define DHTPIN 12      // (D6) what digital pin we're connected to (NodeMCU Breakout v1.2)
//#define MOTIONPIN 5   // (D1) what digital pin the motion sensor is connected to
#define PRESSPIN A0    // (A0) what analog pin the pressure sensor is connected to

#define DHTTYPE DHT22   // DHT 22/11 (AM2302), AM2321

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h> //testing Wifimanager
#include <ESP8266WebServer.h> // For Web Server
ESP8266WebServer server(80);
#ifdef OLED_SPI
  #include "SSD1306Spi.h" //OLED Lib for SPI version
#else
  #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
  #include "SSD1306.h" //OLED Lib for I2C version
#endif

#ifdef REMOTE
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
#endif

DHT dht(DHTPIN, DHTTYPE);

unsigned long currentMillis = 60001; //Set so temperature is read on first scan of program
unsigned long previousMillis = 0;
unsigned long interval = 10000;

float h,t,f,p,h2,t2,f2,p2;//Added %2 for error correction
int motionState;

WiFiClient espClient;
//PubSubClient client(espClient);

#ifdef OLED_SPI
  SSD1306Spi  display(D10, D9, D8); //RES, DC, CS (SPI Initialization)
#else
  SSD1306     display(0x3c, 0 /*D3*/, 2 /*D4*/); //Recommended Setup: to be able to use the USB Serial Monitor, use the configuration on this line
#endif


void setup() {
  Serial.begin(9600);
  dht.begin();

  #ifdef MOTION_ON
    pinMode(MOTIONPIN, INPUT);
  #endif
  #ifdef PRESS_ON
    //pinMode(PRESSPIN, INPUT);
  #endif

  display.init(); // Initialize the Display
  #ifdef FLIP_SCREEN
    display.flipScreenVertically();
  #endif
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  #ifdef REMOTE
    setup_wifi();
  #else
    WiFi.mode(WIFI_OFF);
  #endif
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("Connecting to SSID: ");
  Serial.println(wifi_ssid);

  display.clear(); // clear the display
  display.drawString(17, 10, "Connecting to SSID:"); // display.drawString(Horizontal, Vertical, "String");
  
  
  display.drawString(0, 45, String(wifi_ssid));
  display.display(); // write the buffer to the display


  // Attempt to connect to Wifi, attempt connection for 10 seconds and show progress bar during this time
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  while (counter < 11) {
    display.drawString(0, 20, "");
    int progress = counter * 10;
    display.drawProgressBar(0, 32, 120, 10, progress);
    counter++;
    display.display();
    delay(1000);
  }
  

  // Test Connection
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_OFF);

    display.clear(); // clear the display
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(5, 45, "[ WiFi NOT CONNECTED ]");
    display.display(); // write the buffer to the display
    delay(10000); // delay 10 seconds
    display.clear(); // clear the display
  }

  else if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Create String of IP Address
    String currentIP = WiFi.localIP().toString();
    display.clear(); // clear the display
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 5, "WiFi Connected");
    display.drawString(0, 20, "IP: " + currentIP);
    display.drawString(14, 45, "[ SYSTEM BOOTING ]");
    display.display(); // write the buffer to the display
    delay(5500); // delay 5.5 seconds
    display.clear(); // clear the display

    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP");

    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started");
  }
}


void drawDHT(float h, float t, float f, float p)
{
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(18,0, String(device_name));

  // if statement logic for IP address
  if (WiFi.status() == WL_CONNECTED) {
    String currentIP = WiFi.localIP().toString();
    display.drawString(22,9, "IP: " + String(currentIP));
  }
  else {
    display.drawString(22,9, "IP: n/a");
      }

  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  #ifdef CELSIUS
    display.drawString(60,24, String(t,1) + " *C");
  #else
    display.drawString(60,24, String(f,1) + " *F");
  #endif
  display.setFont(ArialMT_Plain_10);
  #ifdef PRESS_ON
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0,44,"Prs: ");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(120,44, String(p) + "i");
  #endif
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0,53,"Humidity:");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(124,53, String(h,1) + " %");
}


// Web Server Code --------------[ BEGIN ]-------------------

void handle_OnConnect() {

  Temperature = dht.readTemperature() + temp_offset; // Gets the values of the Temperature in Celcius
  TemperatureF = (Temperature * 1.8) + 32; // Converts the Temperature from Celcius to Fahrenheit
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, TemperatureF, Humidity));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperature, float TemperatureF, float Humidity) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<title>ESP8266: FMK Server Room</title>\n";
  ptr += "<meta charset=\"UTF-8\">\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
  ptr += "<META HTTP-EQUIV=\"refresh\" CONTENT=\"15\">\n";
  ptr += "<link rel=\"stylesheet\" href=\"https://www.w3schools.com/w3css/4/w3.css\">\n";
  ptr += "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css?family=Montserrat\">\n";
  ptr += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">\n";
  ptr += "<style>\n";
  ptr += "body, h1,h2,h3,h4,h5,h6 {font-family: \"Montserrat\", sans-serif}\n";
  ptr += ".w3-row-padding img {margin-bottom: 12px}\n";
  ptr += ".bgimg {\n";
  ptr += "  background-position: center;\n";
  ptr += "  background-repeat: no-repeat;\n";  
  ptr += "  background-size: cover;\n";
  ptr += "  background-image: url(\"https://fmk.singidunum.ac.rs/wp-content/uploads/2020/01/FMK-Server_Room-medium.jpg\");\n";
  ptr += "  min-height: 100%;\n";
  ptr += "}\n";
  ptr += "</style>\n";
  ptr += "<body>\n";
  ptr += "\n";
  ptr += "<!-- Hidden Sidebar (reveals when clicked on menu icon)-->\n";
  ptr += "<nav class=\"w3-sidebar w3-black w3-animate-right w3-xxlarge\" style=\"display:none;padding-top:150px;right:0;z-index:2\" id=\"mySidebar\">\n";
  ptr += "  <a href=\"javascript:void(0)\" onclick=\"closeNav()\" class=\"w3-button w3-black w3-xxxlarge w3-display-topright\" style=\"padding:0 12px;\">\n";
  ptr += "    <i class=\"fa fa-remove\"></i>\n";
  ptr += "  </a>\n";
  ptr += "  <div class=\"w3-bar-block w3-center\">\n";
  ptr += "  This site was created by Jacques Laroche\n";
  ptr += "    <a href=\"https://www.linkedin.com/in/jacques-laroche-07032b174/\" target=\"_blank\" class=\"w3-bar-item w3-button w3-text-grey w3-hover-red\" >Linkedin</a>\n";
  ptr += "  </div>\n";
  ptr += "</nav>\n";
  ptr += "\n";
  ptr += "<!-- Page Content -->\n";
  ptr += "<div class=\"w3-main w3-padding-large\">\n";
  ptr += "\n";
  ptr += "  <!-- Menu icon to open sidebar -->\n";
  ptr += "  <span class=\"w3-button w3-top w3-white w3-xxlarge w3-text-grey w3-hover-text-black\" style=\"width:auto;right:0;\" onclick=\"openNav()\"><i class=\"fa fa-bars\"></i></span>\n";
  ptr += "\n";
  ptr += "  <!-- Header -->\n";
  ptr += "  <header class=\"w3-container w3-center\" style=\"padding:35px 16px\" id=\"home\">\n";
  ptr += "    <h1 class=\"w3-jumbo\"><img alt=\"\" src=\"data:image/jpeg;base64,/9j/4QAYRXhpZgAASUkqAAgAAAAAAAAAAAAAAP/sABFEdWNreQABAAQAAAAyAAD/4QNzaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLwA8P3hwYWNrZXQgYmVnaW49Iu+7vyIgaWQ9Ilc1TTBNcENlaGlIenJlU3pOVGN6a2M5ZCI/PiA8eDp4bXBtZXRhIHhtbG5zOng9ImFkb2JlOm5zOm1ldGEvIiB4OnhtcHRrPSJBZG9iZSBYTVAgQ29yZSA1LjYtYzE0NSA3OS4xNjM0OTksIDIwMTgvMDgvMTMtMTY6NDA6MjIgICAgICAgICI+IDxyZGY6UkRGIHhtbG5zOnJkZj0iaHR0cDovL3d3dy53My5vcmcvMTk5OS8wMi8yMi1yZGYtc3ludGF4LW5zIyI+IDxyZGY6RGVzY3JpcHRpb24gcmRmOmFib3V0PSIiIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIiB4bWxuczpzdFJlZj0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlUmVmIyIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bXBNTTpPcmlnaW5hbERvY3VtZW50SUQ9InhtcC5kaWQ6ODUxMjlDRjVEQTVGRTcxMUIzQTQ5NTU3RkRDRDM0QzIiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6RjcwNTc1MDQzQUM4MTFFQUEzRDhBQTQyQkZDRDU2MUQiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6RjcwNTc1MDMzQUM4MTFFQUEzRDhBQTQyQkZDRDU2MUQiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoV2luZG93cykiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDo5YTU3ZDlmMi04ZWZmLTcwNGQtODNlYS1hZjVjNzgxMDI4MTMiIHN0UmVmOmRvY3VtZW50SUQ9InhtcC5kaWQ6ODUxMjlDRjVEQTVGRTcxMUIzQTQ5NTU3RkRDRDM0QzIiLz4gPC9yZGY6RGVzY3JpcHRpb24+IDwvcmRmOlJERj4gPC94OnhtcG1ldGE+IDw/eHBhY2tldCBlbmQ9InIiPz7/7gAOQWRvYmUAZMAAAAAB/9sAhAAIBgYGBgYIBgYIDAgHCAwOCggICg4QDQ0ODQ0QEQwODQ0ODBEPEhMUExIPGBgaGhgYIyIiIiMnJycnJycnJycnAQkICAkKCQsJCQsOCw0LDhEODg4OERMNDQ4NDRMYEQ8PDw8RGBYXFBQUFxYaGhgYGhohISAhIScnJycnJycnJyf/wAARCADIAMgDASIAAhEBAxEB/8QAvQABAAIDAQEBAAAAAAAAAAAAAAYHBAUIAwECAQACAwEBAQAAAAAAAAAAAAAABAECAwUGBxAAAQMCAwQEBwsHCwQDAAAAAQIDBAAFERIGITETB0FRYSJxgZEyFBU1sUJSYiMzc7N0VReh0XKyJDSUgpKi4kNTY5MWNgjB0qN1g9NkEQACAQIDBAYFDAIDAQEAAAABAgARAzESBCFBcQVRYYGRMhOhsSJCFPDB0eFSYnKSsiM1BoJTMyQVohb/2gAMAwEAAhEDEQA/AL/pSlEIpVXao5tOWy6v2yyw2pKYiy0/JfUoJU4nYpLaUdCTsxJrSfjPf/u6H5Xfz1mbyVpt7p1LfJNfcRbgtgBhUVYA0PVLspVJ/jPqD7uh+V389Pxn1B93Q/K7+eo85Ovul/8AwOYfYX86y7KVSf4z6g+7ofld/PT8Z9Qfd0Pyu/no85Ovuh/4HMPsL+dZdlKpP8Z7/wDd0Pyu/nqxdE6yj6wgvOhn0aZFUlEqPmzAZhilaFYDFKsD4Kst1WNBjF9TyrWaa35t1BkBoSGDUrhWkk9KUq8QilKUQilarUeoIOmbU7dZ2JQjBDTSfPccV5jaMek/kG2qfk84NVOvKXGZiR2T5jRQtwgdq8ycT4qo1xVNDj1R/R8r1WrUvZUBQaZmNBXoEvWlUL+LmseuH/kq/wDsqzdB60Gr4L/HaSxcIakpktoJKFBYJQ43m24HA7DuqFuqxptHGX1fKNXpbRvXApUEAlTWlcKyW0pStJzYpSlEIpSlEIpSlEIpSlEJytdva9y+1yPrVViVmXb2vcvtcj61VYDnzavAaR3T6Vb8CfhHqknseg9T6hiCfb4qUxFbGnn1hsOYHAlsYEkdu6tn+E2sv7qL/n/1KvG0Nts2qC00kIbRHaShI3ABAAFZlMCyKCpM8nd/sWr8xsi21UEgAgk06zWUF+E2s/7qL/n/ANSn4Tay/uov+f8A1Kv2lT5C9Jmf/wCi13Ra/KfplBfhNrP+6i/5/wDUqzdAaLVpGFIVKdS9cJpSqQW8eGhKAQhtBVtOGYkmphSpW0qmu08Yvq+cavVWjZuFQpIJCilabRXGKUpWk5sUpQkAYnYBvNEJTXOe7ca4W6yNq7sZCpT4Hw3Pk2wfAlKj46rDsra6mupvmobldM2ZD76gx9E38k1/RTWZLtnoeiLdcFpwcudwdcSojbwWWy0gfzsxpNjmYn5UwnvtGi6TS6aww9pqLT77Au3dtkeq0uSf77e/o43uu1VtWlyS/fb39HG912pt+NePzSnOv47UcF/WsuKlK/C3mmsOK4lGO7MQMfLTc8Jwn7pX4bdadx4S0rw35SDh5K/dEOMUpSiEUpSiEUpSiE5Wu3te5fa5H1qqwXPm1eA1nXb2vcvtcj61VYSwVIUBvIIFI7p9KteBPwj1Tqy2ezYf0DX6grKrUaYusK8WKDMguBaCyhDiQe8haUhK21joKSK29Og1AI6J84uqy3HVgQQxBBxBrFKUqZSKUpRCKUpRCKjWvrubLpS4ykHB91Ho0fr4j/yYI/RBKvFUlqn+dF2zyLbYm1bGwqbIGPScWmgf6Zqlw0Q9ezvj3K9P5+ts2yKqGzN+FNp78JVjLDj7jUSOMzrykssjpK1kIT+U1avNa3tWnTenLYz83EWWQR05GcpV4ztqM8sbT601dFcWnMxbkqmOdWZPca/pqx8VTLnZ7Os/2lz6ulwv7bNwHpnpdXqM3NtFpwdiZnb8TKadwHplO1aXJL99vf0cb3Xaq2rB5YXmLp+NqW7zMeDGZjEIG9a1F0IbT2qVsqEIDgndX1Rrm6M+gvIgqzZAAN5NxaCTXmTrh3TjDdrtK0i6yklanCArgM7uJlOzMo7E49pqjZUiROeVImvOSX1nFTry1LUSe1Rr3udyl3i4SLpPVnkyllbh6EjclCfioT3RWfpXTcrVV4atjBLbIHFmSB/Zsg4Ej4yvNT20Mxdq9wldFpLPL9LmfKGVc125v6xXoG4SX8oNPzXrqvUIUtiBGStgAEgSHFDKU4dKW9+PwvAauusaBAiWuExb4LYZjRkBtptPQB7pO8nprJpm2mVab8TPIcw1h1mpa8RQeFRvCjCvXFKUq8TilKUQilKUQnK929r3L7XI+tVWHWXdva9y+1yPrVVhkgAk7htNIz6Vb8CfhHqnsxJkxSoxX3WCvzyy4pvNh8LIRjVncpdS3mRdnrFLfdmQ1MKfbU8ouKZUhSU7FqxOVWbcendWTpjlJb5VqYnX99/0qUhLqY7Cw2lpKhmSlRwJUrDfU907pSyaXacbtLBSt7DjPuKK3V4bgpZ6B1DZWtu21Q2Ax6557mvNdDcs3rCqbr+ENl9kMPeDY7JquYesHtJ21j0JCV3CctTccuDFCAgZluEe+wxGAqpTzK1wST61G3/87P8A2VL+dp22JPxpJ/ot1U1RcZs5FTsm/JdDpW0Nu69lHa4WJLqHwYqKVwwk1t/NbWEN5Lkp9qezj3mHWkt4j4q2gkg+Wrk0vqe36qtouEHFC0nhyYyyM7TmGOVWG8HeD0iuZu2t9o/U7+lLyiejFcR3BqewPftY+cB8NG9Pk6aEuFTtJI3y/MuTWL1lm09tbd1RUBRlV/ukDZwM6WpXlFkx5sZqXFcS7HfQlxl1O0KSoYgivWmp40ggkEUIxEVzLq67evNTXK4pVmaW8Wo56OEz8kgjw5cfHV9a2u/qTS9ynJOD3CLMfr4r3ySMPAVY1zUhtasrLIKnFENtgbypRyp8pNL322gdG2em/rWn2XtS3VbU/wD03zS6+Tdp9Gssu8LT37g9kaP+ExigYeFZVWPzs9nWf7S59XVhWK2Is1mg2pvDCIyhokdKgO+rxqxNV7zs9nWf7S59XVmXLZpw76xPSaj4jna3tzXGy/hCkL6JTtfcygkthRCVEKKMdhKcQFEdJGOyvlTLQum2tUwNRW85UykNxnYL6htQ8ku4DH4K/NV2VgASaCes1F9LFo3rnhUrXqDMFr2VrIbVn8lZUZu5XaG4QJMhplxkHepDRWlYHgKwarN1p6O65HkILT7KlNvNK85C0nKpJ8Br2t9wl2qdHuUBfDlRVhxpXRj0pV1pUO6R1UK1CGxpM9bp/itLdsK1PMX2TuqDmXs2TqulanTd/ialtEe7RNgcGV5onFTTqdi21eA+Uba21OAgio3z586Nbdrbgqykgg7iIpSlTKxSlKIRSlKITla7e17l9rkfWqrBc+bV4DWfdva9y+1yPrVVgOfNr8BpHdPpVrwJ+ETq22+zof0Df6grKrFtns2H9A1+oKyqdGAnzd/G3Eyoedp+WsQ+0n8jdVYxtkMjrdbHXvWmrQ52n9qsafiST9VVZRBmmRkjpeaH/kTSt3xt8t09vybZyyzwf9bSX8yNIf6buvpsJvC03BRLIG5l7zls9gPnI8Y6KhVdSXyzQ7/a5FqnJxZkJwCh5yFjahxHxknaK5ovFpm2K5SLTPThIjKylQHdWk7UOo+KsbfyVNxMp2YHD6JlyTmPxNnybh/dtDfi6bm4jAywOVOsfQZCdL3JzCLJUTbXFbm3VbVMforO1PxtnTVz1yXiRtBKSNoUk4EEbQQesV0By61gNTWv0WasetoISiTjsLqNyH0j425XUrwir2X9w9n0Tnc/5blJ1tkbGP7oG5vt9u/rkY503b2ZYm1byqbIT2J+Sa/KVVEOXFp9b6ugpWnMxCzTXtmI+S+bx/8AkUmsTW129daquU1KszKXfRo56OGx8mCOxSgTVi8mLTwbdPvbie9LdEdg/wCGx5xHYVqPkqg9u51V9Ajr/wDQ5Llwdkp1572PaoPoloVVvOz2dZ/tLn1dWlVW87PZ1n+0ufV1td8B7PXOByb+R0/Fv0mU7Vpckv329/RxvddqratLkn++3v6ON7rtYW/GvH5p6rnX8dqOC/rWfrm5pPhODVUFv5NeVq5pSNyvNbkePzFeKqrrq6XFjzoz0OW2HY8hCmnm1blJUMqga5q1Tp6Rpi9P2p7FTSflIbx/tGFHuK8KfNV2irXUoajA+uJcg5h5tr4W4fbtD2Cfet9H+Pqm15e6tOl7wG5S8LVOKW5mO5tW5Egfo7lfF8FdCghQCknEHaCNxFcmb9h3dVXRyo1eZ8X/AE1cHMZcNGMFxR2uxx/Z9qmt36PgNTZehynA4cZl/YOXZl+NtDaopdA3rufs3yzKUpTE8tFKUohFKUohOVrt7XuX2uR9aqsFz5tfgNZ93BF4uQIwImSAQfpVVgO/Nr8BpHdPpVrwJ+ETq22ezYf0DX6grKrGtwKbfESoYEMtgg7wcgrJp0YCfN38bcTKd52fv1kH+HJ91qq2twxuMIdclgf+RNWRzsSr06yKIOQtSAFdGOLZwqu7OhTt5tjaElS1zI6UpG0k8VNK3PG3y3T2/KTTlVo9CP8AqadT1BeZmjzqG2esoDea629JLaU73mfOWz2n3yO3Z01OqU0yhgQZ4zTai5p7yXrZoyGvURvB6jOSgQRiNxrNtd0n2Wai4210syWwpAVvBSsZVJUOkf8AXbU15paQ9TTzfoDeFtnL/aEpHdYkK3nsQ7v/AEvCKr6kyCDQ4ie+09+1q9OtxQGS4KMp20+0rT5grDKgFSzsSN5KicB5TXUGmbSmx2C32oDBUZlKXe1w95w+NZNULoCzqvWq7eyUFUeKv0yScMQEs95AV+k5lFdH1tYGLdk8/wD2XUVazpgcAbjcTsX54qredns6z/aXPq6tKqu51pV6stC8DlElYKugEtnAePCr3fAez1zmcm/kdPxb9JlOVaXJL99vf0cb3Xaq2rT5JJV6XfF4HJkjJzdGOLpwrC3414/NPVc6/jtRwX9ay4aiHMPSg1PZSqMgG5wcz0I9K9nfY8DgGztwqX0pplDAg754mxeexdS9bNGQ1H0HqM5L29IIO4g7CCN4PbXvCmSrdMYuEJzhSoyw6y51KT19YO4jpFT3mrpP1Vcf9Qwm8IE9X7UEjutST749SXf1vDVd0mQQSDiJ7/Tai3q9Ot1aFXFCp20PvKZ03pbUUXU9mYukfBC1dySxjiWnk+eg+6OsYVua500HqxWlbyFvqPquZlanI6E7cEPgdaMdvxa6KSpK0hSSFJUMUqG0EHpFM23zDbiMfpnjOa6A6PUEL/x3KtbPVvXis+0pStJzopSlEJUevNM6devrj/CurEp9IekrgQzJjuKVszY+9X3e9hWmtNh0vb5zUybGvtwQyoLRGXbVttqUk4p4mXEqAPRU75tatu2jNKpu9m4XpSpTUfF5OdIQtK1HBOI29ytrMm6nuGimLjpxEf1/LiRn2UP4hgLdCFu4Y/FKsuPjrI2QTXZ0zqW+b31sixmcqFy+IA045ajvmL+Idv8Aui8fwDtPxDt/3ReP4B2qx1brTnVoqCzcL4LYiO+7wGy0hLhzlJXtAV1JNSeyyOekibb356bV6sdcaXKKcM3AUQpeGU45sm7Dpq9G+16In5mn/wBJ/P8AVNlqDUGmtTQfQLrZLytCVZ2nUQXUuNr3Zm1dGzxVpNOMaP03NFxatV+lzG8eA5JhOEN4jAlCUJSM2HSat+vKRIaiR3ZT6sjLCFOurPQlAzKPiAqpt1NSRXhN01+S01hBcW22Ki4abcd2+RX8Q7f90Xj+Adp+Idv+6Lx/AO1ALfrTmrzGelTdDMxLRY4zhZakTAlS3FAZtpUl3vYEEhKcBjvNZtg5h6zsGromjeZMZjPcSlMC5x8EpUpZyoxyYJUlSxl3JIO8Vajfa9Ew8zT/AOk/n+qS6RruzymHI0qyXZ5h1JS405bnFJUk7woHYagc6w6GlPF2NbNRQUE4llmI4pvb8EOpWU+WrG5jX+4aX0bc75a8npkVLfCLqcyRndQ2SU4jHYqq6st7583+1RbzbkWtcOYjiMlQQlRTiRtSVbN1Va2WxIPZN7Gv+HJNhXt1xpc2HiKSUafvmltMRVRbTYrw3xCC88uC6t1wjcVrO/DoG4Vt/wAQ7d90Xj+AdrD0S7zPVcZCdcJgJgcH5D0Up4nGzDDzCe7lxxx7K0PMrXGsrLrCzaX0p6NxLoykpElGbF1bi2xiokADBNSFIFAQBwmVy/auOXuW3Zm2ktcJJ9ElX4hW/wC6Lx/AO1g3jVWnr9Adttzsd3ejO4Ej0B4KSobUrQobUqHQajUiT/yFjMrkejWt8NArLLQQVrCduVIzpxJ6galPLHmCNfWh9+TGEO5wHAzOYTjkxUCUOIzbQFYEYHaCKCrEUJHdIW9ZVgy2mVlNQRcIII7JADpnR+YkMalCcdg9CJOHVjw6mtg1FprTUAW+1WO8ttFRW4tcF1TjizvW4rpNYeite32dr2/6J1OGUvQyty2qZQW8zaFbtpObM2tKx46lHMHVJ0dpO4XxsJVKaSG4aF7UqfdIQjEDDEDHMewVAt0NQR3Ri/zN76hL3mOoNaG5sr+WY/4h2/7ovH8A7T8Q7f8AdF4/gHa9NFXe/wBy0REvt/DXrGQwuUEtoKE8M5ls5k47yjAnCtHyd1vetcWW4Tb2GePFlcFssIKBkKErwIKlbiatRvteiLeZp/8ASfz/AFTZyddWeYw5FlWS6vMOpKXGnLc4pKknoUDUDm2DQ8l4uxbdqKEhRJ4LUNxSB+jxUrUPLW/13zD1DG1NH0HoaE3Jvr6A6/JfwLbKVAuYBJIGIQMxKtgGGwmtDeNQc7tCxhfr/wCgXi0tKT6Y2wlKS2lZCdqm22lp2nDNgoDpqrWy2JB7JvY1/wAPXyFe3XEC5sPZSZdh5faUvkpbCDemUspDjgmsCOhaccMiVlvf4KuFlluOy2wynK00kIbSOhKRlA8la/Tt9h6lskG+wMfR5zQdSlXnJPmrbVh0pUCk1s6sqBcMZlq9be1LA3HYhfCGNaVx6IpSlWisUpSiEqj/AJDf7Cb/APYMfqO1YGlf9sWT7BF+pRUK562q53fRCI1qhvTX0zWHFNR0FxYQEuJKsqcThioVOdNsvRtPWiPIbLT7MKM260rYpK0tJSpKu0EUQlV/8kP9qWr/ANgPqXatu0+yoP2dr9RNVpz7sl4vmmrcxZoL091ucFuNx0FxSUlpxOYpTtwxOFWbbW1s26G06nK42w2laTvCggAiiEyq8JjDEqJIiyv3d9tbb2Jw7i0lKtvRsNe9R3WD1skWWfZJrhHrCO4woNnvIS4kp4h6gnHGoJAFSaS9u29xgiKWJ3CVTb9Fc1eXin06CnRrzYpCy+3GeKMTjszFLhQM2UAEtr73VWRH5ntjUNut3NTSjVtnsrCoFzUjOllajhxAHQshObDvIWcKiFm1DrnSNq/007bJNxtMZxwxJ0FK1gJWrMpIUlKgQTtAVgRjXu3pnU/My522OLNKtVmiO8SXcbgVBxQUU5+HnSjE4DBKUDfvNVDEnYKr9qs3bT20tk3bpW6DTychzf5MaAbOMsLm5qqxXDQV5gQ5XGkLDISlKF4Eh9snvFOG4VAtHcwOZMCw2y0Wa2252I03w4i5BKXFpzHar5dA/IKsrmjpqGnQN1YtMHiTXAylhDYKnVEPNkhA3k5QTsqqbJq3U1otMSxxdHvuT4bYjCRlcJJxUonhBo9/b0mqlrgHulq4dXfNktaJ7hot8W8niNCc9cTlU+zSWppzV+qg3Od1/Ah25LZb9AdiupAcxx4gUS67iBswIqteY+rePzE01e7LH9OkQGm+FH72DriXlqCQcATjj0Ctzpxu/wB5lSl3rS82KthnjCdNK1JKsQnJlcQnDfiAjqrTzYsx/mNpR2bCQmIw6229JbxDCk8VSsVFRwRlB69tUDuHo9FqD8qxl9HozpDc0zXL7KygkbKD3qIdpm7uPOPmOiC+41pD0QpQo+krDrobAHnlGCccu/bsrz5PX3S1isMl+XOkKuN0fD06TwFFptaAcrWZGYkjOVE4Df2Vc69NWRYUlUNGVQIOBUDgRgdx7ap/lRoq8R4t+tdzgvW9TMtLkYzGcWHkkKbKUqO04ZR3k9Yq7eZlNKViln4DzlD+b5ZBBJIVlbcQQG9Uw+YN2t9n1vpnmPZ5bUlkrTEuqmFYnBHdOdHnArjrUNo97W15vvnV2qtK8vYLmduU4mbNU2cQG1YhKtnwWkuK8Yrw1zoZmVapsQwzEueQuw+GczTzjfeCEE/DGI7KjHLeZeLDqE6r1db5LmEdFujPOoKHG0ISlvOhtQGfI22EnpwPTULdotblBQ0M0v8ALyb629HmvLcUstQK7KlhUbDhuxnR0plqPaX47CA2y1HW22hO5KUoKUpHgAqov+Nv+2rx9uH1KKtj06PdbU49BWl9ElhZjrQcUuBSSBlPh2YdFVryAsl3smnro1eIL8Fx2bmbRIQptSgltKCQleBwxG+tQQdonOIIJBFCNhBnprzl3fJ+qm9a6FujcTUEdCEyYrqgAoBPDSoHBQ7zfdKVjA1o7jrzmhp2G61r7STFzs6hw5b7IBQWzsPELan2sD8ZIrL1ZYtYaJ17I5haTgqvMG4t5Lnb0Zi4O6kKGVOKiCWwtKkg4HYRhv8AO8c1NUamtcqw2DRU9M6e0uMtySlRbbS6koWdraBuPviBUyJZmiLvYL3pqFO0yymLbCkobiJQG+CpJ77SkJ2AhXVv31IahnK7SErRWkY9pnrSuc44uVKSg4oQtzAcNJ6cqUjE9eNTOiEUpSiEUpSiEUBBpWPNmRrfGcly3A2y2MVKP5AOsmgmm0yVUsQqgkk0AGJMyKxJVzgQgTKkNt4fCUBUM9Nvmq5S0w1qiW9JwTh3dnWsjapR6twryXamtPzDKujRmsqT3HVDPgvqIPX11gbxO1V2faOE6SctQHJeu/uUr5SbX4ZjsrN1cNawEsqbtivSJSu62cDkTj75R7KhbqhNfxnyCiMXB6TIUdq1no8Ar4whciQ48hGD0hzJHaHQpZ2fzRU3XoyA5BajrJ4yE7XB0qPnHCsvbu1OIG7dOl/1OX5V9oG5i3icdJ7JtbP6rbiIatSkFhI2FBBxPWSK2Axqq59quumZIfiOFoY91xHzauxaN1Sm2asRcLVK444E9ho4t47yrupUjrGJrVLw8LDKR3Tnarlr0GosXPPtudre+Cx96anUF79JfkyUKwYhksxSD7891Tg7Sd3YK3OjbF6DETcpYJlyU5kJV/ZoVt6ffK3qNRFmKmde7ZZyPkOJxXx8LIMysfEnCrYGwYDdVbK5mNxt2HGMcxuDT6e3pbXsi4MzdPljYo/yNSYqv9a6aYwMptH7M+cHkAbEKPvh4asCsS6Rky7dJYUMczasPCBiPy1rdQOpB7JztDqn02oS4pIFQG4fVI1oO9SJUd6yXFZXOtoSEOq3ux1fNr7SPNNTCqiss5UTVVpdGzjFUN34yXB3cfAoCrdqunfMlDipp9EY5xpxa1IdQFW8vmUGAapDU7RXtmPNhRrhHVGloC21eUHoUk9BFVpfbW/bX1QpR48dwYtrV75OOxXYU9NWnWk1XEjyLNIkPEIVDQqQ24royDFSfAobKm9bzLUYj5UlOXa06e6FY/tsdv3TuYSrbTqeHoa6IhXKWBZ54LikDFa47u7i8NGKhidixh3tihVp227wrxDauVslImQ3cQiQ2TgSk4FJB2pUDvBrmfV7bCrsiQqKptyTFbW8rPscXtCljDvJKfNwxpZ9aao09GMOxT/QoxXxSzw0OJKyAnMrihR2hIxot28qgVJ+W6U1+p+JvtdyKhwNMWp7zfe6aTqxp7HBKt/XXtUb0lexqXTsC9tgBx5GEppPvH0dx1KR1ZgcOypC24F7NyhvBrQRMz90pSpkRSlKIRSlKIRVe3mW9qa9It8c/sUZZS3huWsbFunwbk1ML9MVCtUh1GxwoKUHqKtmNRrQ8QF6RJUMeGA2nx7TWF05mW3uO0zp6ACzZva0irIMtvqZsT6ZLYMJmBGRHZSEpSMK02sVD1YhkDvPOpTj2DaakdR3VjZUzEPQHTj4xV7gpbIHRF9GxbV23c1OYsSekbZGtOGOm+NqfwAbCkx0npXhtNWNVS3JS4EmK8jYUOcQHx7fyVakRwPR23BuUkHyiqWDivRt745zi2a2b4NQ6leGUz5Nhszo6476QpCxhtqq5ERVqvaIT25K8G19aV7qtyq+5isBt2HLRsdOKQe1PeFRqFGXPvEOS3m886Yn2bqkcGpjPCKEw9X2iQrYl9LjJ/SUgpFWRVWSVOXW1R7jCP7UwtLzPWl5s95B8Yqw7Ld497tzU+OcCoZXmj5zbidi21DoINTYI2jp2iTza05W1dI/4wbL/dZWLLXjU902FeE1wMw5DqjgENrUSexJr3qK6zvLUWCuEhYzubXzj5qBty+FVauwVSTOdpbLXr6W1FasK8N8rqAlbmo7Q0NqzLaPkVmP5Ku+qn5f29y7X9d3Wn9mgBWVR3F5wZUpH6KdtWxWOlUhCek7OydX+w3AdRasjG1bAbqZjWndSK0Wskxl6YuQluFpkNZipO8lKgpKf5SgBW9qE81WJTukJLrBVwo6kuyUI3qbHdP80kGtzhOIviHGUvqaOufaEXJSOG9DXxCgd48B3urGI35TlVUPwwOI6KsxPAU0zFdGLUwKaIG7hlBzK29Sf+lVu7GchSHoL/zsVxTC+3KcAoeEYGoXCaXBtr2GW3yMv3Blz9NPr7kgemwkk+/SAh9I8KcqvEau0oBOO5Q3GuR7Dd3rDeYN6ZJC4TqXFpBwzN45XkfykEiuto77UphqSwoLZeQlxtY3FKxmSfGDVpmZ6UpSiRFKUohFKUohNJqlsuWteHQoE+DGsXSKUttSEDfnCvKK302OJUZxg+/BFRO2yVWi4huR3W3Pk1k7gR5p8dYsKXAxwwnR05NzR3LC+IHMB0/KkmdYF4ielwloSMVoIcR4U1nAhQBBxB6a1OoLwxabe68tQz4ENp6So7hWjkZTXCkT063DeQWxVswoOuQXUDILCXFDzDh/OqxLQkpt0cK3hCfcqu7RdE6hQq1yE4TspIUkYpKcd/ZhW+Z1HdrG2mJeLa66hsZUS2BmCgN2YdBrC0ygl9xFO6drX6e89tNMAPMtMWy5gCytgUrjJnVc8xZja3WGEnMpoE5BtJUrYBWdJ1vLmJLFntkhbytgUpBAFfNP6QmOzhe9RELfBzsxccwSr4az1joFTcbzBkTbXE7hMdFY+Cf4vWEIUByW6g3HY9QwHXNvpywtxdPRoUtHyyklx3rCl94jxVq52nbzapTl2008ESF/vMZQxakAblKT71wdfTU2pWvlrQDCmBGMSXX3hduXDRhdJLowzI2Y1II6JWMzXmpEgxnLYWJG45Qvf2bCa10PS2p9VSEvXIKhQicVLcGVRB+A2dpPaqrfpWZsZj7blh0RtebrZU/CaW3Yc4vUuR+EN9cw7ZbIdohNQIKMjLQ8aj0rUekmsylK3AAFBOUzM7F3JZmNSTtJJisedEbnwpEJ0YofbU2r+UMMfFWRSiVnOkhLrFykqexzWtz0Rxo7CXSkKOA6UrKgPAK1+p9PXCbITd4baVMhpLcnMoIUHU7gn4RKTt6qnWurMYesDOaZzx7kwl5wjZkkRzkbV25wsp8lYdwaImRLPjihoBx7DZ8pv2/y/coRak13CS70HEyrWLZJetpubYJHfIjFJDh4SihzYekYHZ4q6X5dxZkPRloZmPcZXAStlRQpCksud9ltaV7cyUKANU1ebc1bbvNhyHODHdyXaE7iUjFzBqW2o7dgcAXh21d2i7um86chygQXG0+jvYfDa7mPjGBogZv6UpRIilKUQilKUQisG4WqJcUFLye8RhmG+s6lQQDsMsjsjBkJUjeJG1QNQ29rhW99ElobEJexCgOrMN9RmdpjVN7fBmqQ2noViSEj4qRVlUrNrKtsJNOisds8yu2WLpbtZz75X2vomg03paFp5o8P5SS586+refzDsrfkA7DtFKVoqhRQCgil69cvXDdusWZsSZ8CUp80AeCvtKVMzilKUQilKUQilKUQilKUQmlv1k9av26QAFKgvFwoOzMkjDDHsUAfFVfOw5BvdwmSgEr4xaaR05G9iFK6sdqsO2rbqCaxbjWyd62mu8C3uN5pLwSVFJbIScqEd5SlAjADaTU13DfIIBNTuka1I9Jjqsca05GtR3Nx1MWapKXFQ7cgZpj4QsKT3xsGI8FWhZYkeHBQIqUpZePGSUpyKVnAOdzditW9R66rPQ7b2r9ZXrVjzD8SDFSxbbM26lTTiYqMXFqyrG50p2j4xFW5QeiAilKVEmKUpRCKUpRCKUpRCKUpRCKUpRCKUpRCKUpRCKUpRCKUpRCKUpRCK/DjTTuXioSvKcycwBwPWMaUohP3SlKIRSlKIRSlKIT/2Q==\" /><b>Server Room Temp Reading</b></h1>\n";
  ptr += "  </header>\n";
  ptr += "\n";
  ptr += "<center><h1>\n";
  ptr += "<p>Temperature: ";
  ptr += (int)Temperature;
  ptr += " C | ";
  ptr += (int)TemperatureF;
  ptr += " F</p>";
  ptr += "<p>Humidity: ";
  ptr += (float)Humidity;
  ptr += "%</p>";
  ptr += "</center></h1>\n";
  ptr += "\n";
  ptr += "  <!-- About Section -->\n";
  ptr += "  <div class=\"w3-content w3-justify w3-text-grey w3-padding-32\" id=\"about\">\n";
  ptr += "    <h2>About</h2>\n";
  ptr += "    <hr class=\"w3-opacity\">\n";
  ptr += "    <p>Temperature and Humidy readings from a <a href=\"https://www.adafruit.com/product/385\">DHT22 sensor</a> running on a <a href=\"https://www.nodemcu.com/index_en.html\">ESP8266 NodeMCU</a> microcontroller. The details of this project can be found on my Github page.\n";
  ptr += "    <br><br><b>NOTE:</b> <u>This page automatically refreshes every 15 seconds.</u>\n";
  ptr += "    </p>\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "    <button onclick=\"window.open('https://github.com/jlar0che/NodeMCU-Interface','_blank');\" class=\"w3-button w3-light-grey w3-padding-large w3-section\">\n";
  ptr += "      <i class=\"fa fa-github\"></i> Follow this Project on Github\n";
  ptr += "    </button>\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "  <!-- End About Section -->\n";
  ptr += "  </div>\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "\n";
  ptr += "  <!-- Footer -->\n";
  ptr += "  <footer class=\"w3-container w3-padding-64 w3-light-grey w3-center w3-opacity w3-xlarge\" style=\"margin:-24px\">\n";
  ptr += "    Follow me:\n";
  ptr += "    <a href=\"https://github.com/jlar0che\" target=\"_blank\"><i class=\"fa fa-github w3-hover-opacity\"></i></a>\n";
  ptr += "    <a href=\"https://www.linkedin.com/in/jacques-laroche-07032b174/\" target=\"_blank\"><i class=\"fa fa-linkedin w3-hover-opacity\"></i></a>\n";
  ptr += "    <p class=\"w3-medium\">Powered by <a href=\"https://www.w3schools.com/w3css/default.asp\" target=\"_blank\" class=\"w3-hover-text-green\">w3.css</a></p>\n";
  ptr += "  <!-- End footer -->\n";
  ptr += "  </footer>\n";
  ptr += "\n";
  ptr += "<!-- END PAGE CONTENT -->\n";
  ptr += "</div>\n";
  ptr += "\n";
  ptr += "<script>\n";
  ptr += "// Open and close sidebar\n";
  ptr += "function openNav() {\n";
  ptr += "  document.getElementById(\"mySidebar\").style.width = \"100%\";\n";
  ptr += "  document.getElementById(\"mySidebar\").style.display = \"block\";\n";
  ptr += "}\n";
  ptr += "\n";
  ptr += "function closeNav() {\n";
  ptr += "  document.getElementById(\"mySidebar\").style.display = \"none\";\n";
  ptr += "}\n";
  ptr += "</script>\n";
  ptr += "\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

// Web Server Code --------------[ END ]---------------------


void loop() {

  char strCh[10];
  String str;
  //#ifdef REMOTE
  //  if (!client.loop()) {
  //    reconnect();
  //  }
  //#endif

  if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;

      h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      //f = dht.readTemperature(true);
      f = ((t + temp_offset) * 1.8) + 32;

      // Read pressure (if external pressure sensor connected)
      p = analogRead(PRESSPIN);
      p = ((p/1023)*16.6246)-8.3123; //Convert analog value from MPXV7002 differential pressure sensor to psi range (-8...8 in/h2O) via formula ((ain/max_analog)*full range)-offset

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        //h=t=f=-1;
        t=t2;//Writes previous read values if the read attempt failed
        f=f2;
        h=h2;

      }
      else { //add offsets, if any
        //t = t + ((5 / 9) * temp_offset);
        t = t + temp_offset;
        //f = f + temp_offset;
        //h = h + hum_offset;
        h2=h;//Store values encase next read fails
        t2=t;
        f2=f;
      }

      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);


      Serial.print("Humidity: ");
      Serial.print(h,1);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t,1);
      Serial.print(" *C ");
      Serial.print(f,1);
      Serial.print(" *F\t");
      Serial.print("Heat index: ");
      Serial.print(hic,1);
      Serial.print(" *C ");
      Serial.print(hif,1);
      Serial.println(" *F");
      Serial.print(p,1);
      Serial.println(" in/H2O");
      Serial.println(""); // space out Serial info so it is easier to read
#ifdef ANTI_BURNIN
      display.invertDisplay();
      display.display();
#endif
      display.clear();
      display.normalDisplay();
#ifdef OLED_MOTION //Clears display on each loop if OLED Motion is active, otherwise prints display as usual
      display.clear();
#else
      drawDHT(h,t,f,p);
#endif
      display.display();
  }
  currentMillis = millis();

#ifdef MOTION_ON
  motionState = digitalRead(MOTIONPIN);
//   print out the state of the button:
  if (motionState == 1)
{
    Serial.println("Motion Event:");

    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(120,0, "M");
    #ifdef OLED_MOTION//Writes the whole display if motion is sensed, otherwise just the "M"
    drawDHT(h,t,f,p);
    #endif
    display.display();
    delay (10000);
    display.clear();
    drawDHT(h,t,f,p);
    display.display();
  }
#else
  #ifdef DEEP_SLEEP
    #ifndef OTA
      ESP.deepSleep(60000000,WAKE_RF_DEFAULT);   //If a motion sensor is not being used, we can put ESP into deep-sleep to save energy/wifi channel
    #endif
  #endif
#endif

if (WiFi.status() == WL_CONNECTED) {
  server.handleClient();
  }

}
