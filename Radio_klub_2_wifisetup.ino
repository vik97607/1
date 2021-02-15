#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

//Wifisetup related
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//Variables
int i = 0;
int statusCode;
const char* ssid = "Default_SSID";
const char* passphrase = "Default_Password";
String st;
String content;


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);









// To run, set your ESP8266 build to 160MHz, update the SSID info, and upload.
// Enter your WiFi setup here:
//const char *SSID = "Cafe_au_lait";
//const char *PASSWORD = "R4puszedli";
// Uncomment one link (I have added 6 radio streaming link, you can check each)
//flawlessly working radio streaming link
const char *URL="http://hu-stream02.klubradio.hu:8080/bpstream"; 
//const char *URL="http://ndr-edge-10ad-fra-dtag-cdn.cast.addradio.de/ndr/ndr1niedersachsen/hannover/mp3/128/stream.mp3";
//const char *URL="http://jazz.streamr.ru/jazz-64.mp3";
// It will work but buffer alot
//const char *URL="http://stream.ca.morow.com:8003/morow_med.mp3";
//const char *URL= "http://ndr-ndr1radiomv-schwerin.sslcast.addradio.de/ndr/ndr1radiomv/schwerin/mp3/128/stream.mp3";
//const char *URL="http://mms.hoerradar.de:8000/rst128k";//Radio RST(German)
AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;
// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
const char *ptr = reinterpret_cast<const char *>(cbData);
(void) isUnicode; // Punt this ball for now
// Note that the type and string may be in PROGMEM, so copy them to RAM for printf
char s1[32], s2[64];
strncpy_P(s1, type, sizeof(s1));
s1[sizeof(s1)-1]=0;
strncpy_P(s2, string, sizeof(s2));
s2[sizeof(s2)-1]=0;
Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
Serial.flush();
}
// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
const char *ptr = reinterpret_cast<const char *>(cbData);
// Note that the string may be in PROGMEM, so copy it to RAM for printf
char s1[64];
strncpy_P(s1, string, sizeof(s1));
s1[sizeof(s1)-1]=0;
Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
Serial.flush();
}
void setup()
{
Serial.begin(115200);
delay(1000);

/* Old wifi setup 
Serial.println("Connecting to WiFi");
WiFi.disconnect();
WiFi.softAPdisconnect(true);
WiFi.mode(WIFI_STA);
WiFi.begin(SSID, PASSWORD);
// Try forever
while (WiFi.status() != WL_CONNECTED) {
Serial.println("...Connecting to WiFi");
delay(1000);
}

*/
// New WIFIsetup
Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);


  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    //return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }





Serial.println("Connected");
audioLogger = &Serial;
file = new AudioFileSourceICYStream(URL);
file->RegisterMetadataCB(MDCallback, (void*)"ICY");
buff = new AudioFileSourceBuffer(file, 8192);
buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
out = new AudioOutputI2SNoDAC();
mp3 = new AudioGeneratorMP3();
mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
mp3->begin(buff, out);
}


void loop()
{
static int lastms = 0;
if (mp3->isRunning()) {
if (millis()-lastms > 1000) {
lastms = millis();
//Serial.printf("Running for %d ms...\n", lastms);
//Serial.flush();
}
if (!mp3->loop()) mp3->stop();
} else {
Serial.printf("MP3 done\n");
delay(1000);
}
}

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("Klubradio", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}
