#define WEBDUINO_FAIL_MESSAGE "NOT ok\n"
#define WEBDUINO_COMMANDS_COUNT 20
#define WEBDUINO_SERIAL_DEBUGGING 1

#include "SPI.h"
#include "avr/pgmspace.h"
#include "Ethernet.h"
#include "EthernetUdp.h"
#include "EthernetBonjour.h"
#include "WebServer.h"
#include "Adafruit_GFX.h"
#include "glcdfont.c"
#include <Adafruit_2801Matrix.h>
#include <Adafruit_WS2801.h>

unsigned long lastCheck = 0;

/*** This is what you will almost certainly have to change ***/

// WEB stuff
static uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0xF9, 0x04, 0xF9 }; // update this to match your arduino/shield
static uint8_t ip[] = {   192,168,0,220 }; // update this to match your network
String theIP = (String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3]; //create the IP as a string
//UDP stuff
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
EthernetUDP Udp;
unsigned int localPort = 8888;
// LED Stuff
uint8_t dataPin = 2; // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3; // Green wire on Adafruit Pixels


//LED Grid Stuff
uint16_t max_x = 16;
uint16_t max_y = 8;

#define STRIPLEN 128
int defaultPattern = 6;
Adafruit_2801Matrix theMatrix = Adafruit_2801Matrix(max_x, max_y, dataPin, clockPin,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  WS2801_RGB);


//Matrix Scrolling
unsigned long prevFrameTime = 0L;             // For animation timing
#define FPS 20                                // Scrolling speed
uint8_t       msgLen        = 0;              // Empty message
int           msgX          = 16; // Start off right edge
String writeCharStr = "";

uint32_t aac;
int aayE, aaxE, aay, aax;

//ada gfx vars
int16_t cursor_x_orig = 1;
int16_t cursor_y_orig = 1;
int16_t cursor_x = cursor_x_orig;
int16_t cursor_y = cursor_y_orig;
uint8_t textsize = 1;
uint32_t textcolor = Color(255,255,255);
uint32_t textbgcolor = Color(0,0,0);
boolean wrap = false;
int16_t _width = max_x;
int16_t _height = max_y;

//cylon colors

uint32_t red[6] = {  Color(255, 0, 0),  Color(200, 0, 0),  Color(150, 0, 0),  Color(100, 0, 0),  Color(50, 0, 0),  Color(0, 0, 0) };
uint32_t orange[6] = {  Color(255, 128, 0),  Color(200, 100, 0),  Color(150, 75, 0),  Color(100, 50, 0),  Color(50, 25, 0),  Color(0, 0, 0) };
uint32_t yellow[6] = {  Color(255, 255, 0),  Color(200, 200, 0),  Color(150, 150, 0),  Color(100, 100, 0),  Color(50, 50, 0),  Color(0, 0, 0) };
uint32_t green[6] = {  Color(0, 255, 0),  Color(0, 200, 0),  Color(0, 150, 0),  Color(0, 100, 0),  Color(0, 50, 0),  Color(0, 0, 0) };
uint32_t blue[6] = {  Color(0, 0, 255),  Color(0, 0, 200),  Color(0, 0, 150),  Color(0, 0, 100),  Color(0, 0, 50),  Color(0, 0, 0) };
uint32_t purple[6] = {  Color(127, 0, 255),  Color(100, 0, 200),  Color(75, 0, 150),  Color(50, 0, 100),  Color(25, 0, 50),  Color(0, 0, 0) };

//end cylon colors

/*** Things you might want to change ***/

WebServer webserver("", 80); // port to listen on

// ROM-based messages for webduino lib, maybe overkill here

void printNothing(WebServer &server) { //for snakemove, should respond faster not sending the entire page.
  server.println("good");
}

void printOk(WebServer &server) {
  server.println(F("HTTP/1.1 200 OK"));
  server.println(F("Content-Type: text/html"));
  server.println(F("Connection: close"));
  server.print(F("\r\n"));
  server.println(F("<!DOCTYPE HTML PUBLIC -//W3C//DTD HTML 4.00 TRANSITIONAL//EN><html><head><title>")); //opening html
  server.println(F("DeskLights128</title>")); //title
  server.println(F("<meta name = 'viewport' content = 'width = device-width'> <meta name = 'viewport' content = 'initial-scale = 1.0'> </head>"));
  server.print(F("<script> function process() { var url='write?t=' + document.getElementById('url').value + '&h=' + document.getElementById('colorwrite').value; location.href=url; return false; } </script>")); //script for form submitting
  server.print(F("<script> function process2() { var urlalert='alert?h=' + document.getElementById('alert').value + '&d=1000'; location.href=urlalert; return false; } </script>")); //script for alert submitting
  server.print(F("<script> function process3() { var urlalert='default?id=' + document.getElementById('default').value; location.href=urlalert; return false; } </script>")); //script for default submitting
  server.print(F("<script> function colortable() { var url='color?h=' + document.getElementById('colortab').value; location.href=url; return false; } </script><script> function colorpixel() { var url='pixel?x=' + document.getElementById('pixx').value + '&y=' + document.getElementById('pixy').value + '&h=' + document.getElementById('pixcolor').value; location.href=url; return false; } </script><script> function colorwipe() { var url='wipe?h=' + document.getElementById('wipecolor').value + '&d=' + document.getElementById('wipedel').value; location.href=url; return false; } </script>"));
  server.println(F("<body>")); //links below here
  server.println(F("<form onSubmit='return process2();'>Send Alert <select id='alert'>")); //form select for Alerts
  server.println(F("<option value='FF0000'>Red</option>")); //red
  server.println(F("<option value='FF6600'>Orange</option>")); //orange
  server.println(F("<option value='FFFF00'>Yellow</option>")); //yellow
  server.println(F("<option value='336600'>Green</option>")); //green
  server.println(F("<option value='003333'>Blue</option>")); //blue
  server.println(F("<option value='660033'>Purple</option>")); //purple
  server.println(F("<input type='submit' value='go'> </select> </form>")); //end select
  server.println(F("<form onSubmit='return colortable();'>Color Table: <select id='colortab'> <option value='FF0000'>Red</option><option value='FF6600'>Orange</option> <option value='FFFF00'>Yellow</option><option value='336600'>Green</option> <option value='003333'>Blue</option><option value='660033'>Purple</option><input type='submit' value='go'> </select> </form>"));
  server.println(F("<form onSubmit='return colorpixel();'>Color Pixel: x=<input type='text' value='1' maxlength='3' size='3' name='pixx' id='pixx' type='number'> y=<input type='text' value='1' maxlength='3' size='3' name='pixy' id='pixy' type='number'><select id='pixcolor'> <option value='FF0000'>Red</option><option value='FF6600'>Orange</option> <option value='FFFF00'>Yellow</option><option value='336600'>Green</option> <option value='003333'>Blue</option><option value='660033'>Purple</option><input type='submit' value='go'> </select> </form>"));
  server.println(F("<form onSubmit='return colorwipe();'>Color Wipe: delay=<input type='text' value='100' maxlength='6' size='6' name='wipedel' id='wipedel' type='number'><select id='wipecolor'> <option value='FF0000'>Red</option><option value='FF6600'>Orange</option> <option value='FFFF00'>Yellow</option><option value='336600'>Green</option> <option value='003333'>Blue</option><option value='660033'>Purple</option><input type='submit' value='go'> </select> </form>"));  server.println(F("<form onSubmit='return process3();'>Run Command <select id='default'>")); //form select for Defaults
  server.println(F("<option value='1'>Rainbow</option>")); //red
  server.println(F("<option value='2'>Random</option>")); //orange
  server.println(F("<option value='3'>K.I.T.T.</option>")); //yellow
  server.println(F("<option value='8'>K.I.T.T. Multi</option>")); //yellow
  server.println(F("<input type='submit' value='go'> </select> </form>")); //end select
  server.println(F("<form onSubmit='return process();'> Write Character: <input type='text' name='url' id='url'> <select id='colorwrite'> <option value='FFFFFF'>White</option><option value='FF0000'>Red</option><option value='FF6600'>Orange</option> <option value='FFFF00'>Yellow</option><option value='336600'>Green</option> <option value='003333'>Blue</option><option value='660033'>Purple</option> </select> <input type='submit' value='go'> </form>")); //this writes a single character to the board
  server.print(F("<a href='default?id=4'>All Off</a><p></p>"));
  server.println(F("</body></html>")); //end html
}
P(noauth) = "User Denied\n";

// max length of param names and values
#define NAMELEN 2
#define VALUELEN 32

/*** Below here shouldn't need to change ***/
void fetchData(int *inData, WebServer &server, char *webData) {
  int x, y, r, g, b, xE, yE, use_hex = 0;
  int s, reply = 1;
  int delayTime = 100;
  int id = -1;
  uint32_t c = Color(0, 0, 0);

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(webData)) {
    rc = server.nextURLparam(&webData, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch (name[0]) {
        case'i':
          id = atoi(value);
          break;
        case 'x':
          x = atoi(value);
          break;
        case 'y':
          y = atoi(value);
          break;
        case 'h':
          c = hexColor(value);
          use_hex = 1;
          break;
        case 'r':
          r = atoi(value);
          break;
        case 'g':
          g = atoi(value);
          break;
        case 'b':
          b = atoi(value);
          break;
        case 's':
          s = atoi(value);
          break;
        case 'c':
          xE = atoi(value);
          break;
        case 'u':
          yE = atoi(value);
          break;
        case 'z':
          reply = atoi(value);
          break;
        case 'd':
          delayTime = atoi(value);
          break;
        case 't':
          writeCharStr = value;
          break;
        case 'v':
          vu(String(value));
          theMatrix.show();
          break;
      }
    }
  }

  if (use_hex == 0) {
    c = Color(r, g, b);
  }

  inData[0] = g2p(x, y);
  inData[1] = c;
  inData[2] = x;
  inData[3] = y;
  inData[4] = xE;
  inData[5] = yE;
  inData[6] = delayTime;
  inData[7] = id;
  inData[8] = reply;
  inData[9] = s;
}
void cmd_index(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  printOk(server);
}
void my_failCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  server.httpFail();
}
void cmd_off(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  colorAll(Color(0, 0, 0));
  theMatrix.setCursor(1, 1);
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_color(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);

  colorAll(data[1]);
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_wipe(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);

  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
  colorWipe(data[1], data[6]);
}
void cmd_default(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  
  defaultPattern = data[7];
  colorAllDef(Color(0, 0, 0));
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_alert(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  
  alert((uint32_t)data[1], data[6]);
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_alertArea(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);

  aax = data[2];
  aay = data[3];
  aaxE = data[4];
  aayE = data[5];
  aac = data[1];

  defaultPattern = 10;
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_test(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);

  switch (data[7]) {
    case 0:
      lightTest(data[6]);
      break;
    case 1:
      gridTest(data[6]);
      break;
  }

  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_writechar(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  theMatrix.setTextColor(convertTo16(data[1]));
  defaultPattern = 7;
  printOk(server);
}
void cmd_show(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  defaultPattern = 0;
  theMatrix.show();
  printOk(server);
}
void cmd_vu(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  defaultPattern = 0;
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}
void cmd_pixel(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int data[10];
  fetchData(data, server, url_tail);
  theMatrix.setPixelColor(data[0], data[1]);

  if (data[9]) {
    theMatrix.show();
  }

  defaultPattern = 0;
  if (data[8]) {
    printOk(server);
  }
  else {
    printNothing(server);
  }
}

// --------------------------------------- END CMDS ------------------------------------------ //


// --------------------------------------- START FUNCTIONS ----------------------------------- //
int antiDelay(unsigned long nowTime, int delayTime) {
  unsigned long arduinoTime = millis();
  Serial.print("antidelay, arduinoTime: "); Serial.print(arduinoTime); Serial.print(", nowTime: "); Serial.print(nowTime); Serial.print(", delayTime: "); Serial.println(delayTime);
  while (arduinoTime - nowTime < delayTime) {
    arduinoTime = millis();
    if (webserver.available()) {
      //check for new web requests
      char buff[64];
      int len = 64;
      webserver.processConnection(buff, &len);
      return 0;
    }
  }
  return 1;
}

uint16_t convertTo16(uint32_t input) {
  int red = (input >> 16) & 0xFF;
  int green = (input >> 8) & 0xFF;
  int blue = input & 0xFF;
  return ((red / 8) << 11) | ((green / 4) << 5) | (blue / 8);
}

uint32_t Color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

// create a "Color" value from a hex string (no prefix)
// for example: ffffff
uint32_t hexColor(char * in) {
  return strtol(in, NULL, 16);
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
    WheelPos -= 170;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// set all pixels to a "Color" value
void colorAll(uint32_t c) {
  Serial.println("Matrix");
  for (int i = 0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
  }
  defaultPattern = 0;
  theMatrix.show();
}
//colorAllDef is just colorAll without the defaultPattern set
void colorAllDef(uint32_t c) {
  for (int i = 0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
  }
  theMatrix.show();
}
// set all pixels to a "Color" value, one at a time, with a delay
int colorWipe(uint32_t c, uint8_t wait) {
  for (int i = 0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
    defaultPattern = 0;
    theMatrix.show();
    if (antiDelay(millis(), wait) == 0) return 0;
  }
  return 1;
}
// show the grid to verify
void gridTest(int wait) {
  int x;
  int y;
  uint32_t on = Color(255, 255, 255);
  uint32_t off = Color(0, 0, 0);

  if (!wait) {
    wait = 250;
  }

  for ( x = 0; x <= max_x; x++) {
    for ( y = 0; y <= max_y; y++) {
      defaultPattern = 0;
      theMatrix.setPixelColor(g2p(x, y), on);
      theMatrix.show();
      antiDelay(millis(), wait);
      theMatrix.setPixelColor(g2p(x, y), off);
      theMatrix.show();
    }
  }
}
// random pixel, random color
// short pattern, very responsive
void p_random (int wait) {
  theMatrix.setPixelColor(
    random(0, theMatrix.numPixels()),
    Color(random(0, 255), random(0, 255), random(0, 255)));
  theMatrix.show();
  antiDelay(millis(), wait);
}

// If you were at maker faire, you know this pattern
// it takes about a second to run, so new requests will wait
void p_rainbow() {
  int i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < theMatrix.numPixels(); i++) {
      theMatrix.setPixelColor(i, Wheel( ((i * 256 / theMatrix.numPixels()) + j) & 255) );
      if (webserver.available()) {
        char buff[64];
        int len = 64;
        webserver.processConnection(buff, &len);
        return;
      }
    }
    theMatrix.show();
  }
}

// cylon or K.I.T.T. whichever
int p_cylon(uint32_t c[6]) {
  int x;
  int wait = 75;

  for (x = 0; x <= max_x; x++) {
    int mod = 0;
    while ((mod < 6) && (x - mod >= 0)) {
      int y = 0;
      while (y <= max_y) {
        theMatrix.setPixelColor(g2p(x - mod, y++), c[mod]);
      }
      mod++;
    }
    theMatrix.show();
    if (antiDelay(millis(), wait) == 0) return 0;
  }

  for (x = max_x; x >= 0; x--) {
    int mod = 0;
    while ((mod < 6) && (x + mod <= max_x)) {
      int y = 0;
      while (y <= max_y) {
        theMatrix.setPixelColor(g2p(x + mod, y++), c[mod]);
      }
      mod++;
    }
    theMatrix.show();
    if (antiDelay(millis(), wait) == 0) return 0;
  }
  return 1;
}

//visualizer, takes string of 16 numbers which are Y heights
void vu(String input) {
  uint32_t color = Color(255, 0, 0);
  for (int i = 0; i < input.length(); i++) {
    int y = input.charAt(i) - '0';
    if (y > max_y) {
      y = max_y;
    }
    if (y < 1) {
      for (y = 1; y < max_y + 1; y++) {
        theMatrix.setPixelColor(g2p(i + 1, y), Color(0, 0, 0));
      }
    }
    else {
      int y_orig = y;
      for (y; y > 0; y--) {
        if (y > 6) color = Color(255, 0, 0);
        else if (y > 3) color = Color(255, 128, 0);
        else color = Color(255, 255, 0);
        theMatrix.setPixelColor(g2p(i + 1, y), color);
      }
      y = y_orig + 1;
      for (y; y < max_y + 1; y++) {
        theMatrix.setPixelColor(g2p(i + 1, y), Color(0, 0, 0));
      }
    }
  }
}

// fade from one color to another: UNFINISHED
void fade(uint32_t c1, uint32_t c2, int wait) {
  if (c1 < c2) {
    while (c1 < c2) {
      colorAll(c1++);
      antiDelay(millis(), wait);
    }
  }
  else {
    while (c1 > c2) {
      colorAll(c1--);
      antiDelay(millis(), wait);
    }
  }
}

uint16_t g2p(uint16_t x, uint16_t y) {
  if(x%2) { // if odd
  return (max_y * x) + y-1-max_y;
  }
  else { //else true, so
  return (max_y * x) + y -1 -max_y + ((max_y - 1)*-1) + 2 * (max_y - y);
  }
}

// flash color "c" for "wait" ms
void alert(uint32_t c, int wait) {
  Serial.print("alert wait: "); Serial.println(wait);
  colorAll(c);
  antiDelay(millis(), wait);
  colorAll(Color(0, 0, 0));
}
// flash color "c" on x/y/c/u as a rectangle
void alertArea(uint32_t c, int x, int y, int xE, int yE) {
  int w = xE - x + 1;
  int h = yE - y + 1;
  for (int16_t i = x; i < x + w; i++) {
    theMatrix.drawFastVLine(i, y, h, c);
  }
  theMatrix.show();
}
// wipe the major colors through all pixels
void lightTest(int wait) {
  if (colorWipe(Color(255, 0, 0), wait) == 0) return;
  if (colorWipe(Color(0, 255, 0), wait) == 0) return;
  if (colorWipe(Color(0, 0, 255), wait) == 0) return;
  if (colorWipe(Color(255, 255, 255), wait) == 0) return;
  if (colorWipe(Color(0, 0, 0), wait) == 0) return;
}

// begin standard arduino setup and loop pattern

void setup() {
  delay(1000);
  Serial.begin(9600);
  //send first message after serial port is connected
  Serial.println(F("Initializing... "));

  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);
  delay(1000); //resetting should fix our issues with not connecting intiially

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
    Serial.println(F("Fixed IP initialization complete"));
  }
  else
  {
    Serial.println(F("Configured Ethernet using DHCP"));
  }
  Serial.print(("device IP is: "));
  Serial.println(Ethernet.localIP());

  Serial.print(("gateway IP is: "));
  Serial.println(Ethernet.gatewayIP());

  Serial.print(("subnet mask is: "));
  Serial.println(Ethernet.subnetMask());

  Serial.print(("DNS is: "));
  Serial.println(Ethernet.dnsServerIP());

  String tableName = "DeskLights." + String(ip[3]);
  String tableNameDL = "DeskLights." + String(ip[3]) + "._desklights";
  String tableNameHTTP = "DeskLights." + String(ip[3]) + "._http";
  char tableNameAr[30];
  char tableNameDLAr[30];
  char tableNameHTTPAr[30];
  tableName.toCharArray(tableNameAr, 30);
  EthernetBonjour.begin(tableNameDLAr);
  tableNameDL.toCharArray(tableNameDLAr, 30);
  tableNameHTTP.toCharArray(tableNameHTTPAr, 30);
  EthernetBonjour.addServiceRecord(tableNameDLAr, 80, MDNSServiceTCP);
  EthernetBonjour.addServiceRecord(tableNameHTTPAr, 80, MDNSServiceTCP);
  webserver.setFailureCommand(&my_failCmd);
  webserver.setDefaultCommand(&cmd_index);
  webserver.addCommand("off", &cmd_off);
  webserver.addCommand("show", &cmd_show);
  webserver.addCommand("wipe", &cmd_wipe);
  webserver.addCommand("color", &cmd_color);
  webserver.addCommand("alert", &cmd_alert);
  webserver.addCommand("pixel", &cmd_pixel);
  webserver.addCommand("default", &cmd_default);
  webserver.addCommand("write", &cmd_writechar);
  webserver.addCommand("test", &cmd_test);
  webserver.addCommand("vu", &cmd_vu);
  webserver.addCommand("alertArea", &cmd_alertArea);
  webserver.begin();
  Udp.begin(localPort);

  theMatrix.begin();
  theMatrix.setCursor(1, 1);
  theMatrix.setTextWrap(false);

  //theMatrix.setRemapFunction(g2p); //this is causing issues with matrix.write() ? tested it at tinker night and was scrolling fine commented out.

  // light blip of light to signal we are ready to listen
  colorAll(Color(0, 0, 11));
  delay(500);
  colorAll(Color(0, 0, 0));
}

void loop()
{
  unsigned long t = millis(); // Current elapsed time, milliseconds.
  if (t - lastCheck > 1000) {
    lastCheck = t;
    //Serial.println("Alive!");
  }
  EthernetBonjour.run();
  // listen for connections
  if (webserver.available()) {
    char buff[64];
    int len = 64;
    webserver.processConnection(buff, &len);
  }
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    vu(packetBuffer);
    theMatrix.show();
  }

  switch (defaultPattern) {
    case 1:
      p_rainbow();
      break;
    case 2:
      p_random(50);
      break;
    case 3:
      p_cylon(blue);
      break;
    case 4:
      colorAll(Color(0, 0, 0));
      theMatrix.setCursor(1, 1);
      break;
    case 5:
      p_cylon(red);
      break;
    case 6:
      if ((t - prevFrameTime) >= (1000L / FPS)) { // Handle scrolling
        theMatrix.fillScreen(0);
        theMatrix.setCursor(msgX, 0);
        theMatrix.print(Ethernet.localIP());
        msgLen = String(Ethernet.localIP()).length();
        if (--msgX < (msgLen * -6)) msgX = 15; // We must repeat!
        theMatrix.show();
        prevFrameTime = t;
      }
      break;
    case 7:
      if ((t - prevFrameTime) >= (1000L / FPS)) { // Handle scrolling
        theMatrix.fillScreen(0);
        theMatrix.setCursor(msgX, 0);
        theMatrix.print(writeCharStr);
        msgLen = String(writeCharStr).length();
        if (--msgX < (msgLen * -6)) msgX = 15; // We must repeat!
        theMatrix.show();
        prevFrameTime = t;
      }
      break;
    case 8:
      if (p_cylon(red) == 0) break;
      if (p_cylon(orange) == 0) break;
      if (p_cylon(yellow) == 0) break;
      if (p_cylon(green) == 0) break;
      if (p_cylon(blue) == 0) break;
      if (p_cylon(purple) == 0) break;
      break;
    case 10:
      for (int p = 0; p < 200; p++) {
        alertArea(aac, aax, aay, aaxE, aayE);
        aac--;
        antiDelay(millis(), 5);
      }
      for (int p = 0; p < 200; p++) {
        alertArea(aac, aax, aay, aaxE, aayE);
        aac++;
        antiDelay(millis(), 5);
      }
      break;
  }
}

