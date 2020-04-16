#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include <NTPClient.h>

// #include <DS3231.h>
#include <Wire.h>
#include <RtcDS3231.h>

// #include <FastLED.h>
#include <NeoPixelBus.h>

#include "web_Server_ota.h"



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


void toggle_led();
void loop();

void tick_drawer_shower(RgbColor col);
void tick_drawer(RgbColor col);

//void first_clock_circle();
void update_clock_circle();
void nigth_minimal_mode();

void time_to_serial(int type_t, uint8_t hh, uint8_t mm, uint8_t ss);


void serial_sim();



// ************** WiFi related   *******************


const char* host = "esp32";
const char* ssid = "****";
const char* password = "****";

WebServer server(80);

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 101);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional



// ************** RTC related  *******************

const int SDA_PIN = 16;
const int SCL_PIN = 17;

RtcDS3231<TwoWire> Rtc(Wire);

//DS3231 Clock;
//RTClib RTC;

bool Century=false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;


int para_gmt_correction = 3;

// ************** Clock related  *******************

//led location four hours
// int hourDigits[13] = {0,4,9,14,19,24,29,34,39,44,49,54,59};

//int hourDigits[13] = {0,0,5,10,15,20,25,30,35,40, 45,50,55};

const int cf =29;
int hourDigits[13] = {0, cf + 0, cf + 5, cf + 10, cf + 15, cf + 20, cf + 25, cf + 30, cf + 35 - 60, cf + 40 - 60, cf + 45 -60 ,cf + 50 - 60, cf + 55 - 60};



// DateTime now;
RtcDateTime now;
RtcDateTime old_time;

uint8_t  old_hour, old_minute, old_second;
unsigned long int lastTime = 0;

// TODO SİL int display_clock();



// ************** LED ring related  *******************
// Which pin on the Arduino is connected to the NeoPixels?

// Fastled
//
//#define NUM_LEDS 60
//#define DATA_PIN 25
//CRGBArray<NUM_LEDS> leds;
//CRGB old_second_color =  CRGB::Black;


// NeoPixelBus
//
const uint16_t PixelCount = 60; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 25;  // make sure to set this to the correct pin, ignored for Esp8266 


// NeoPixelBus<NeoRgbFeature, NeoWs2812Method> strip(PixelCount, PixelPin);
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

int colorSaturation = 128;
RgbColor red(colorSaturation, 32, 32);
RgbColor green(32, colorSaturation, 32);
RgbColor blue(0, 0, colorSaturation);

RgbColor semi_blue(0, 32, colorSaturation/2);

RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, (uint8_t) colorSaturation * 0.65, 0);

RgbColor white(colorSaturation);
RgbColor semi_white(colorSaturation / 2 );
RgbColor quarter_white(colorSaturation / 4 );
RgbColor black(0);



uint8_t clock_mod = 1;
bool first_run= true;

// Clock mod related


// Mod 1
RgbColor para_second_color = red;
RgbColor para_minute_color = green;
RgbColor para_hour_color =  blue; //RgbColor(57, 100, 179); //semi_white;

//Mod2 
RgbColor para_minimal_color_1 = semi_white;
RgbColor para_minimal_color_2 = quarter_white;


// RgbColor para_tick_colors[13] = {0, semi_white, semi_blue, semi_blue, semi_white, semi_blue, semi_blue, semi_white, semi_blue, semi_blue, semi_white, semi_blue, semi_blue};

RgbColor para_tick_colors[13] = {0, quarter_white, quarter_white, quarter_white, quarter_white, quarter_white, quarter_white, 
                                quarter_white, quarter_white, quarter_white, quarter_white, quarter_white, quarter_white};


RgbColor mycolor(17, 34 , 51);
RgbColor mycolor2(170, 170 , 170);

RgbColor old_second_color = black;
RgbColor old_minute_color = black;
RgbColor old_hour_color = black;

// Mod 2 
// little blink using ledstate




// ************** Blink LED related  *******************
//variabls to blink without delay:
const int LED_BUILTIN_ESP = 22;
unsigned long previousMillis = 0;        // will store last time LED was updated
int ledState = LOW;


const long interval = 1000 ;           // interval at which to blink (milliseconds)
const int sub_interval = 10;
int loop_counter = 0;

// *********************************

// ************** Setup  *******************

// *********************************
/*
 * setup function
 */
void setup(void) {

  
  Serial.begin(115200);

  // General setup

  pinMode(LED_BUILTIN_ESP, OUTPUT);
  
  pinMode(19, OUTPUT);
  pinMode(25, OUTPUT);


  // RTC setup
 
  Wire.begin(SDA_PIN, SCL_PIN);
  Rtc.Begin();
  
  //RtcDateTime real_time = RtcDateTime(__DATE__, __TIME__);


  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }


  //***********************  led setup  *********************** 
  


  strip.Begin();
  
 

  // Draw ticks for power-on indicator time
  tick_drawer_shower(red);
  //for (int i = 1; i<=12; i++) strip.SetPixelColor(hourDigits[i], red);
  //strip.Show();

  //***********************  wifi setup  *********************** 
  
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  Serial.println("STA Success to configure");

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  int wifi_conn_counter = 0;

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    tick_drawer_shower(red);
    delay(250);
    Serial.print(".");
    tick_drawer_shower(black);
    delay(250);

    wifi_conn_counter++;

    if (wifi_conn_counter == 20) break;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Draw ticks for wifi-on indicator time
    tick_drawer_shower(green);
    delay(500);


    /*use mdns for host name resolution*/
    if (!MDNS.begin(host)) { //http://esp32.local
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", loginIndex);
    });
    server.on("/serverIndex", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    /*handling uploading firmware file */
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });

    // Start OTA server
    server.begin();


    //***********************  NTP setup and read***************** 
    timeClient.begin();
    timeClient.update();
    timeClient.end();
  

    RtcDateTime ntp_real_time = RtcDateTime(__DATE__, __TIME__);
    


    ntp_real_time = RtcDateTime( timeClient.getEpochTime() - 946684800 + (para_gmt_correction * 3600) );
    Rtc.SetDateTime(ntp_real_time); // update RTC with NTP time



    Serial.print("NTP Time: ");
    Serial.println(timeClient.getFormattedTime());



  } else {
    // No net connection
    Serial.println("No net connection");
  }
  


  now = Rtc.GetDateTime();

  time_to_serial(0, now.Hour(), now.Minute(), now.Second());
  

  old_time = now;

  // if (clock_mod == 1) first_clock_circle();

  tick_drawer(para_tick_colors[1]);


}



void loop(void) {

  
    
  server.handleClient();
  
  // delay(1);


  //loop to blink without delay
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {


    previousMillis = currentMillis;

   toggle_led();
     
    
    now = Rtc.GetDateTime();

    
    
    switch (clock_mod) {

      case 1:
        update_clock_circle();
        break;
      
      case 2:
        nigth_minimal_mode();
      
      default:
        break;
      }
    
    


    //Serial.write(0);
    //Serial.write(1);
    //Serial.write(0);
    //Serial.write(1);
    //Serial.write(0);
    //Serial.write(1);
    //Serial.write(0);
    //Serial.write(1);

    //Serial.write(strip.Pixels(), 180);

    
  }

  if (strip.IsDirty()) {
    strip.Show();
  }

  
}



void update_clock_circle() {


  uint8_t current_hour = now.Hour();
  uint8_t current_minute = now.Minute();
  uint8_t current_second =  (now.Second() - 1) % 60;


  uint8_t old_hour = old_time.Hour();
  uint8_t old_minute = old_time.Minute();
  uint8_t old_second =  (old_time.Second() - 1) % 60;

  uint8_t current_second_led = (current_second + cf ) % 60;
  uint8_t current_minute_led = (current_minute + cf) % 60;
  uint8_t current_hour_led = hourDigits[current_hour % 12 ] + (int) (current_minute /12) ;


  uint8_t old_second_led = (old_second + cf ) % 60;
  uint8_t old_minute_led = (old_minute + cf) % 60;


  // update second new
  strip.SetPixelColor(old_second_led, old_second_color);
  old_second_color = strip.GetPixelColor(current_second_led);
  strip.SetPixelColor(current_second_led, para_second_color);


  if ( (old_minute != current_minute) || (first_run == true)  ) {

    // update minute 
    strip.SetPixelColor(old_minute_led, old_minute_color);
    old_minute_color = strip.GetPixelColor(current_minute_led);
    strip.SetPixelColor(current_minute_led, para_minute_color);

  }
 
 if ( (old_hour != current_hour) || (first_run == true) ) {
  // update  hour 
  strip.SetPixelColor(old_hour, old_hour_color);
  old_hour_color = strip.GetPixelColor(current_hour_led);
  strip.SetPixelColor(current_hour_led, para_hour_color);

 }

 


  old_time = now;
  first_run = false;

}


// void first_clock_circle() {


//   uint8_t current_hour = now.Hour();
//   uint8_t current_minute = now.Minute();
//   uint8_t current_second =  (now.Second() - 1) % 60;


//   uint8_t old_hour = old_time.Hour();
//   uint8_t old_minute = old_time.Minute();
//   uint8_t old_second =  (old_time.Second() - 1) % 60;

//   uint8_t current_second_led = (current_second + cf ) % 60;
//   uint8_t current_minute_led = (current_minute + cf) % 60;
//   uint8_t current_hour_led = hourDigits[current_hour % 12 ] + (int) (current_minute /12) ;


//   uint8_t old_second_led = (old_second + cf ) % 60;
//   uint8_t old_minute_led = (old_minute + cf) % 60;

//   tick_drawer(para_tick_colors[1]);


//   // update second new
//   strip.SetPixelColor(old_second_led, old_second_color);
//   old_second_color = strip.GetPixelColor(current_second_led);
//   strip.SetPixelColor(current_second_led, para_second_color);


//   // update minute 
//   strip.SetPixelColor(old_minute_led, old_minute_color);
//   old_minute_color = strip.GetPixelColor(current_minute_led);
//   strip.SetPixelColor(current_minute_led, para_minute_color);

//   // update  hour 
//   strip.SetPixelColor(old_hour, old_hour_color);
//   old_hour_color = strip.GetPixelColor(current_hour_led);
//   strip.SetPixelColor(current_hour_led, para_hour_color);


//   old_time = now;

// }

void nigth_minimal_mode() {

  strip.ClearTo(black);

  if (ledState) {
    strip.SetPixelColor(0 + cf, para_minimal_color_1);
  } else 

  {
    strip.SetPixelColor(0 + cf, para_minimal_color_2);
  }

}


void tick_drawer(RgbColor col){

  for (int i = 1; i<=12; i++) strip.SetPixelColor(hourDigits[i], col);
  
}


void tick_drawer_shower(RgbColor col){

  for (int i = 1; i<=12; i++) strip.SetPixelColor(hourDigits[i], col);
  strip.Show();

}

void toggle_led() {
  
  // if the LED is off turn it on and vice-versa:
  ledState = not(ledState);
  // set the LED with the ledState of the variable:    
  digitalWrite(LED_BUILTIN_ESP, ledState);




  if (ledState) {
    //strip.ClearTo(mycolor, 0, 59);
    //Serial.write(strip.Pixels(), 180);
  } else {
    //delayMicroseconds(100);

    //strip.ClearTo(mycolor2, 0, 59);
    //Serial.write(strip.Pixels(), 180);
  }
    
}


void serial_sim() {

  static long blinkCopy = 0;

  Serial.print("b1~");
  Serial.print(blinkCopy);
  Serial.print(",");
  Serial.print("b2~");
  Serial.print(blinkCopy*2);
  Serial.println();
  Serial.println();

  blinkCopy++;

}


void time_to_serial(int type_t, uint8_t hh, uint8_t mm, uint8_t ss) {
  
    
    Serial.print(type_t);
    Serial.print('-');

    Serial.print(now.Year(), DEC);
    Serial.print('/');
    Serial.print(now.Month(), DEC);
    Serial.print('/');
    Serial.print(now.Day(), DEC);
    Serial.print(' ');
    Serial.print(hh, DEC);
    Serial.print(':');
    Serial.print(mm, DEC);
    Serial.print(':');
    Serial.print(ss, DEC);
    
}













