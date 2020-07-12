/**
 * HSL:n kaupunkipyöräasemien tilanäyttö: alustana Wemos D1 Mini (Pro)
 * ja WaveShare 4.2" e-paperinäyttö.
 *
 * https://github.com/mpolla/hsl-tellinkimonitori
 *
 * Matti Pöllä <mpo@iki.fi>
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Time.h>

// Ensimmäinen sarake: kaupunkipyöräaseman nimi
#define COL1_X 15
// Toinen sarake: pyörien lukumäärä
#define COL2_X 298
// 42 pisteen rivikorkeudella 4.2" näytölle mahduu neljän tellngin tiedot
#define ROW_HEIGHT 42

const char* ssid     = "WLAN-VERKKONI-NIMI";
const char* password = "WLAN-VERKKONI-SALANASA";
WiFiClient client;

const char * headerKeys[] = {"date"} ;
const size_t numberOfHeaders = 1;

/*
  Esimerkki digitransit.fi:n rajapintakutsusta curl-työkalulla

  $ curl -X POST -H "Content-Type: application/graphql" -d "{bikeRentalStation(id: \"224\") {stationId name bikesAvailable spacesAvailable allowDropoff}}"  "https://api.digitransit.fi/routing/v1/routers/hsl/index/graphql"
  {"data":{"bikeRentalStation":{"stationId":"224","name":"Vesakkotie","bikesAvailable":1,"spacesAvailable":11,"allowDropoff":true}}}

*/

// Kuinka paljon varataan tilaa rajapinnan palauttamalle vastaukselle
const size_t bufferSize = 10000;

// Tuki e-paperinäytöille https://github.com/ZinggJM/GxEPD
#include <GxEPD.h>
#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w

// Adafruitin fontit
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

// Kehitysalustan ja e-paperinäytön väliset kytkennät
//
// LOLIN D1 Mini
// BUSY -> D2 (GPIO4), RST -> D4 (GPIO2), DC -> D3 (GPIO0), CS -> ?, CLK -> D5 (GPIO14), DIN -> D7 (GPIO13), GND -> GND, 3.3V -> 3.3V
//
// LOLIN D1 Mini Pro
// BUSY -> GPIO4/D2, RST -> GPIO2/D4, DC -> GPIO0/D3, CS -> GPIO5/D1, CLK -> GPIO14/D5, DIN -> GPIO13/D7, GND -> GND, VCC -> 3V3

GxIO_Class io(SPI, /* CS <--> GPIO5/D1 */ 5, /* DC <--> GPIO0/D3 */ D3, /* RST <--> GPIO2/D4 */ D4); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
GxEPD_Class display(io, D4, D2); // default selection of D4(=2), D2(=4)

class Tellinki {
  public:
    int av;
    int sp;
    String nimi;
    String paivitetty;
    String getString(void) {
      return String("INFO: nimi=" + nimi + " av=" + av + " sp=" + sp + " paivitetty=" + paivitetty);
    }
};

void printtaa(Tellinki t, int rivi) {
  display.setCursor(COL1_X, rivi * ROW_HEIGHT+35);
  display.println(t.nimi);
  display.setCursor(COL2_X, rivi * ROW_HEIGHT+35);
  display.println(t.av);
  display.setCursor(COL2_X + 22, rivi * ROW_HEIGHT+35);
  display.println("/");
  display.setCursor(COL2_X + 43, rivi * ROW_HEIGHT+35);
  display.println(t.sp);

  display.setFont(&FreeMonoBold12pt7b);
  for (int p=1; p<=t.sp; p++) {
    int x = COL1_X+14*(p-1);
    int y = rivi * ROW_HEIGHT+42;
    int thickness = 2;
    display.fillRect(x,y,12,14, GxEPD_BLACK);
    if (t.av < p) {
      display.fillRect(x+thickness,y+thickness,12-thickness*2,14-thickness*2, GxEPD_WHITE);
    }
  }
  
}


void setup()
{
  Serial.begin(9600);
  display.update();
  display.init(); // enable diagnostic output on Serial
  
  display.setRotation(0);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(45, 150);
  display.println("Haen verkkoyhteytta...");
  display.update();
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
  }
  display.init(); 
  
  String tellingit[] = {
                        "225" /* Maunula */,
                        "224" /* Vesakkotie */,
                        "079" /* Uimastadion */,
                        "032" /* Eläinmuseo */
  };
  
  String paivitettyViimeksi = "?";
  display.fillRect(0, 0, 400, 40, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(52, 28);
  display.println("Tellinkimonitori v0.1");
  display.setTextColor(GxEPD_BLACK);
  
  Tellinki t;
  for (int i = 0; i < 4; i++) {
    t = haeTiedot(tellingit[i]);
    Serial.println(t.getString());
    paivitettyViimeksi = t.paivitetty;
    printtaa(t, i + 1);
  }
  display.setFont(&FreeMonoBold9pt7b);
  
  // Päivitysleima ja ulkolämpötila
  display.setCursor(5, 290);
  display.println("Paivitetty " + paivitettyViimeksi);
  display.setCursor(5, 270);
  float temperature = haeSaa();
  String saaString;
  char buffer[5];
  sprintf(buffer, "Lampotila %+02.01f °C", temperature);
  display.println(buffer);
  display.update();
  ESP.deepSleep(20 /* minutes */ * 60 * 1000000);
}

void loop() {}

Tellinki haeTiedot(String id) {
  Tellinki t;
  HTTPClient http;
  http.begin("http://api.digitransit.fi/routing/v1/routers/hsl/index/graphql");
  http.collectHeaders(headerKeys, numberOfHeaders);
  http.addHeader("Content-Type", "application/graphql");
  String postData = String("{bikeRentalStation(id: \"" + id + "\") {stationId name bikesAvailable spacesAvailable}}");
  int httpCode = http.POST(postData);
  
  // E.g. "Thu, 11 Jul 2019 09:27:19 GMT"
  String headerDate = http.header("date");
  String payload = http.getString();
  http.end();
  DynamicJsonDocument jsonBuffer(bufferSize);
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error) {
    Serial.println("JSON error:");
    Serial.println(error.c_str());
  }
  String nimi = jsonBuffer["data"]["bikeRentalStation"]["name"].as<char*>();
  nimi = mankeloiSkandit(nimi);
  int bikesAvailable = jsonBuffer["data"]["bikeRentalStation"]["bikesAvailable"].as<int>();
  int spacesAvailable = jsonBuffer["data"]["bikeRentalStation"]["spacesAvailable"].as<int>();
  // Tellinki t;
  t.av = bikesAvailable;
  t.sp = spacesAvailable + bikesAvailable;
  t.nimi = nimi;
  t.paivitetty = headerDate.substring(5);
  return t;
}

float haeSaa() {
  HTTPClient http;
  http.begin("http://api.openweathermap.org/data/2.5/weather?id=658225&units=metric&APPID=MINUN-APPID-KOODINI");
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  DynamicJsonDocument jsonBuffer(1500);
  DeserializationError error = deserializeJson(jsonBuffer, payload);
  if (error) {
    Serial.println("JSON error:");
    Serial.println(error.c_str());
  }
  float temperature = jsonBuffer["main"]["temp"];
  return temperature;
}

/** 
 * Adafruitin GFX-fontit eivät tunne ei-ASCII-merkkejä, joten
 * poistetaan umlautit.
 */
String mankeloiSkandit(String s) {
  String ret = s;
  ret.replace("Ä", "A");
  ret.replace("Ö", "O");
  ret.replace("ä", "a");
  ret.replace("ö", "o");
  return ret;
}
