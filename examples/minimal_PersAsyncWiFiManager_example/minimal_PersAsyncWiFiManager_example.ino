#define DEBUG_SERIAL //uncomment for Serial debugging statements

#ifdef DEBUG_SERIAL
#define DEBUG_BEGIN Serial.begin(115200)
#define DEBUG_PRINT(x) Serial.println(x)
#else
#define DEBUG_PRINT(x) 
#define DEBUG_BEGIN
#endif

//includes
#include <PersAsyncWiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266SSDP.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <FS.h>

#define DEVICE_NAME "ESP8266 DEVICE"

//const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"1; url=/\" /></head><body><a href=\"/\">redirecting...</a></body>";
const char *metaRefreshStr = "<script>window.location='/'</script><a href='/'>redirecting...</a>";

//server objects
AsyncWebServer server(80);
DNSServer dnsServer;
PersWiFiManager persWM(server, dnsServer);

////// Sample program data

void setup() {
  DEBUG_BEGIN; //for terminal debugging
  DEBUG_PRINT();

  //allows serving of files from SPIFFS
  SPIFFS.begin();
  persWM.begin();

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });

  //SSDP makes device visible on windows network
//  server.on("/description.xml", HTTP_GET, []() {
//    SSDP.schema(server.client());
//    Serial.printf("SSDP Description Requested");
//  });
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(DEVICE_NAME);
  SSDP.setURL("/");
  SSDP.begin();
  SSDP.setDeviceType("upnp:rootdevice");

  server.begin();
  DEBUG_PRINT("setup complete.");
} //void setup

void loop() {
  dnsServer.processNextRequest();

  // do stuff 

} //void loop
