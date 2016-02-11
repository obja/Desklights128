#define WEBDUINO_FAIL_MESSAGE "NOT ok\n"
#define WEBDUINO_COMMANDS_COUNT 20
#define WEBDUINO_SERIAL_DEBUGGING 1
#define PIN 2

#include "SPI.h"
#include "avr/pgmspace.h"
#include "Ethernet.h"
#include "EthernetBonjour.h"
#include "WebServer.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

unsigned long lastCheck = 0;

/*** This is what you will almost certainly have to change ***/

// WEB stuff
static uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0xF9, 0x04, 0xF9 }; // update this to match your arduino/shield
static uint8_t ip[] = {   192, 168, 1, 220 }; // update this to match your network
String theIP = (String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3]; //create the IP as a string

//UDP stuff
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
EthernetUDP Udp;
unsigned int localPort = 8888;

//LED Grid Stuff
uint16_t max_x = 30;
uint16_t max_y = 8;

#define STRIPLEN 128
int defaultPattern = 6;
Adafruit_NeoMatrix theMatrix = Adafruit_NeoMatrix(30, 8, PIN,
                               NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
                               NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                               NEO_GRB            + NEO_KHZ800);


//Matrix Scrolling
unsigned long prevFrameTime = 0L;             // For animation timing
#define FPS 20                                // Scrolling speed
uint8_t       msgLen        = 0;              // Empty message
int           msgX          = 16; // Start off right edge
String writeCharStr = "";

int aax, aay, aayE, aaxE;
uint32_t aac;

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
  
  server.println(F("<!DOCTYPE HTML PUBLIC -//W3C//DTD HTML 4.00 TRANSITIONAL//EN>")); //opening html
  server.print(F("<html>\r\n\  <head>\r\n    <title>"));
  server.println(F("DeskLights128</title>")); //title
  server.println(F("    <meta name = 'viewport' content = 'width = device-width'>\r\n    <meta name = 'viewport' content = 'initial-scale = 1.0'>\r\n  </head>"));
  server.println(F("  <script src=\"color.js\"></script>"));
  server.println(F("<script> var colorAlertValue = 'r=255&g=0&b=0'; var colorTableValue = 'r=255&g=0&b=0'; var colorPixelValue = 'r=255&g=0&b=0'; var colorWipeValue = 'r=255&g=0&b=0'; var colorWriteValue = 'r=255&g=0&b=0'; var colorsss = 'r=255&g=0&b=0';</script>"));
  server.println(F("<script> function writefunc() { var url='write?t=' + document.getElementById('url').value + '&' + colorWriteValue; location.href=url; return false; } </script>")); //script for form submitting
  server.println(F("<script> function alertfunc() { var urlalert='alert?' + colorAlertValue + '&d=1000'; location.href=urlalert; return false; } </script>")); //script for alert submitting
  server.println(F("<script> function defaultfunc() { var urlalert='default?id=' + document.getElementById('default').value; location.href=urlalert; return false; } </script>")); //script for default submitting
  server.println(F("<script> function colortable() { var url='color?' + colorTableValue; location.href=url; return false; } </script>"));
  server.println(F("<script> function colorpixel() { var url='pixel?x=' + document.getElementById('pixx').value + '&y=' + document.getElementById('pixy').value + '&' + colorPixelValue; location.href=url; return false; } </script>"));
  server.println(F("<script> function colorwipe() { var url='wipe?' + colorWipeValue + '&d=' + document.getElementById('wipedel').value; location.href=url; return false; } </script>"));
  server.println(F("<script> function updateVar(picker) { return 'r=' + Math.round(picker.rgb[0]) + '&g=' + Math.round(picker.rgb[1]) + '&b=' + Math.round(picker.rgb[2]); } </script>"));
  server.println(F("<body>")); //links below here
  server.println(F("<form onSubmit='return alertfunc();'>Send Alert: <button class=\"jscolor {valueElement:'chosen-value', onFineChange:'colorAlertValue = updateVar(this)'}\"> Color </button>")); //form select for Alerts
  server.println(F("<input type='submit' value='go'> </form>")); //end alert
  server.println(F("<form onSubmit='return colortable();'>Color Table: <button class=\"jscolor {valueElement:'chosen-value', onFineChange:'colorTableValue = updateVar(this)'}\"> Color </button>"));
  server.println(F("<input type='submit' value='go'> </form>")); //end color table
  server.println(F("<form onSubmit='return colorpixel();'>Color Pixel: x=<input type='text' value='1' maxlength='3' size='3' name='pixx' id='pixx' type='number'> y=<input type='text' value='1' maxlength='3' size='3' name='pixy' id='pixy' type='number'>"));
  server.println(F("<button class=\"jscolor {valueElement:'chosen-value', onFineChange:'colorPixelValue = updateVar(this)'}\"> Color </button> <input type='submit' value='go'> </form>")); //end colorpixel
  server.println(F("<form onSubmit='return colorwipe();'>Color Wipe: delay=<input type='text' value='100' maxlength='6' size='6' name='wipedel' id='wipedel' type='number'> <button class=\"jscolor {valueElement:'chosen-value', onFineChange:'colorWipeValue = updateVar(this)'}\"> Color </button>"));
  server.println(F("<input type='submit' value='go'> </form>")); //end colorwipe
  server.println(F("<form onSubmit='return defaultfunc();'>Run Command <select id='default'>")); //form select for Defaults
  server.println(F("<option value='1'>Rainbow</option>")); //rainbow
  server.println(F("<option value='2'>Random</option>")); //random
  server.println(F("<option value='3'>K.I.T.T. (Blue)</option>")); //KITT (blue)
  server.println(F("<option value='5'>K.I.T.T. (Red)</option>")); //KITT (red)
  server.println(F("<option value='8'>K.I.T.T. (Multi)</option>")); //KITT (rainbow run through)
  server.println(F("<input type='submit' value='go'> </select> </form>")); //end select
  server.println(F("<form onSubmit='return writefunc();'> Write Character: <input type='text' name='url' id='url'> <button class=\"jscolor {valueElement:'chosen-value', onFineChange:'colorWriteValue = updateVar(this)'}\"> Color </button>"));
  server.println(F("<input type='submit' value='go'> </form>")); //this writes a single character to the board
  server.print(F("<a href='default?id=4'>All Off</a><p></p>"));
  server.println(F("</body></html>"));
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
void cmd_js(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  printJS(server);
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
  Serial.print(F("antidelay, arduinoTime: ")); Serial.print(arduinoTime); Serial.print(F(", nowTime: ")); Serial.print(nowTime); Serial.print(F(", delayTime: ")); Serial.println(delayTime);
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
uint16_t g2p(uint16_t x, uint16_t y) { //this is for a row-major set, bottom left starting position
  if (y % 2) { // if odd
    return (((y - 1) * max_x) + x - 1);
  }
  else { //else true, so
    return (((y - 1) * max_x) + x - 1) + ((max_x - 1) * -1) + 2 * (max_x - x);
  }
}
// flash color "c" for "wait" ms
void alert(uint32_t c, int wait) {
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
  Serial.print(F("device IP is: "));
  Serial.println(Ethernet.localIP());

  Serial.print(F("gateway IP is: "));
  Serial.println(Ethernet.gatewayIP());

  Serial.print(F("subnet mask is: "));
  Serial.println(Ethernet.subnetMask());

  Serial.print(F("DNS is: "));
  Serial.println(Ethernet.dnsServerIP());
  EthernetBonjour.begin("AlecTable");
  EthernetBonjour.addServiceRecord("DeskLights._http", 80, MDNSServiceTCP);
  EthernetBonjour.addServiceRecord("DeskLights._DeskLights", 80, MDNSServiceTCP);
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
  webserver.addCommand("color.js", &cmd_js);
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
    Serial.println("Alive!");
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

void printJS(WebServer &server) {
  server.println(F("HTTP/1.1 200 OK"));
  server.println(F("Content-Type: x-javascript"));
  server.println(F("Cache-Control: max-age=2592000"));
  server.println(F("Connection: close"));
  server.print(F("\r\n"));
  
server.print(F(" \"use strict\";window.jscolor||(window.jscolor=function(){var e={register:function(){e.attachDOMReadyEvent(e.init),e.attachEvent(document,\"mousedown\",e.onDocumentMouseDown),e.attachEvent(document,"));
server.print(F("\"touchstart\",e.onDocumentTouchStart),e.attachEvent(window,\"resize\",e.onWindowResize)},init:function(){e.jscolor.lookupClass&&e.jscolor.installByClassName(e.jscolor.lookupClass)},tryInstallOnElement"));
server.print(F("s:function(t,n){var r=new RegExp(\"(^|\\\\s)(\"+n+\")(\\\\s*(\\\\{[^}]*\\\\})|\\\\s|$)\",\"i\");for(var i=0;i<t.length;i+=1){if(t[i].type!==undefined&&t[i].type.toLowerCase()==\"color\"&&e.isColorAttrSupported)"));
server.print(F("continue;var s;if(!t[i].jscolor&&t[i].className&&(s=t[i].className.match(r))){var o=t[i],u=null,a=e.getDataAttr(o,\"jscolor\");a!==null?u=a:s[4]&&(u=s[4]);var f={};if(u)try{f=(new Function(\"return ("));
server.print(F("\"+u+\")\"))()}catch(l){e.warn(\"Error parsing jscolor options: \"+l+\":\ "));
server.print(F("n\"+u)}o.jscolor=new e.jscolor(o,f)}}},isColorAttrSupported:function(){var e=document.createElement(\"input\");if(e.setAttribute"));
server.print(F("){e.setAttribute(\"type\",\"color\");if(e.type.toLowerCase()==\"color\")return!0}return!1}(),isCanvasSupported:function(){var e=document.createElement(\"canvas\");return!!e.getContext&&!!e.getContext("));
server.print(F("\"2d\")}(),fetchElement:function(e){return typeof e==\"string\"?document.getElementById(e):e},isElementType:function(e,t){return e.nodeName.toLowerCase()===t.toLowerCase()},getDataAttr:function(e,t){v"));
server.print(F("ar n=\"data-\"+t,r=e.getAttribute(n);return r!==null?r:null},attachEvent:function(e,t,n){e.addEventListener?e.addEventListener(t,n,!1):e.attachEvent&&e.attachEvent(\"on\"+t,n)},detachEvent:function(e,"));
server.print(F("t,n){e.removeEventListener?e.removeEventListener(t,n,!1):e.detachEvent&&e.detachEvent(\"on\"+t,n)},_attachedGroupEvents:{},attachGroupEvent:function(t,n,r,i){e._attachedGroupEvents.hasOwnProperty(t)||"));
server.print(F("(e._attachedGroupEvents[t]=[]),e._attachedGroupEvents[t].push([n,r,i]),e.attachEvent(n,r,i)},detachGroupEvents:function(t){if(e._attachedGroupEvents.hasOwnProperty(t)){for(var n=0;n<e._attachedGroupEv"));
server.print(F("ents[t].length;n+=1){var r=e._attachedGroupEvents[t][n];e.detachEvent(r[0],r[1],r[2])}delete e._attachedGroupEvents[t]}},attachDOMReadyEvent:function(e){var t=!1,n=function(){t||(t=!0,e())};if(documen"));
server.print(F("t.readyState===\"complete\"){setTimeout(n,1);return}if(document.addEventListener)document.addEventListener(\"DOMContentLoaded\",n,!1),window.addEventListener(\"load\",n,!1);else if(document.attachEven"));
server.print(F("t){document.attachEvent(\"onreadystatechange\",function(){document.readyState===\"complete\"&&(document.detachEvent(\"onreadystatechange\",arguments.callee),n())}),window.attachEvent(\"onload\",n);if("));
server.print(F("document.documentElement.doScroll&&window==window.top){var r=function(){if(!document.body)return;try{document.documentElement.doScroll(\"left\"),n()}catch(e){setTimeout(r,1)}};r()}}},warn:function(e){"));
server.print(F("window.console&&window.console.warn&&window.console.warn(e)},preventDefault:function(e){e.preventDefault&&e.preventDefault(),e.returnValue=!1},captureTarget:function(t){t.setCapture&&(e._capturedTarge"));
server.print(F("t=t,e._capturedTarget.setCapture())},releaseTarget:function(){e._capturedTarget&&(e._capturedTarget.releaseCapture(),e._capturedTarget=null)},fireEvent:function(e,t){if(!e)return;if(document.createEve"));
server.print(F("nt){var n=document.createEvent(\"HTMLEvents\");n.initEvent(t,!0,!0),e.dispatchEvent(n)}else if(document.createEventObject){var n=document.createEventObject();e.fireEvent(\"on\"+t,n)}else e[\"on\"+t]&&"));
server.print(F("e[\"on\"+t]()},classNameToList:function(e){return e.replace(/^\\\s+|\\\s+$/g,\"\").split(/\\\s+/)},hasClass:function(e,t){return t?-1!=(\" \"+e.className.replace(/\\\s+/g,\" \")+\" \").indexOf(\" \"+t+\" \"):"));
server.print(F("!1},setClass:function(t,n){var r=e.classNameToList(n);for(var i=0;i<r.length;i+=1)e.hasClass(t,r[i])||(t.className+=(t.className?\" \":\"\")+r[i])},unsetClass:function(t,n){var r=e.classNameToList(n);"));
server.print(F("for(var i=0;i<r.length;i+=1){var s=new RegExp(\"^\\\\s*\"+r[i]+\"\\\\s*|\"+\"\\\\s*\"+r[i]+\"\\\\s*$|\"+\"\\\\s+\"+r[i]+\"(\\\\s+)\",\"g\");t.className=t.className.replace(s,\"$1\")}},getStyle:function(e){return "));
server.print(F("window.getComputedStyle?window.getComputedStyle(e):e.currentStyle},setStyle:function(){var e=document.createElement(\"div\"),t=function(t){for(var n=0;n<t.length;n+=1)if(t[n]in e.style)return t[n]},n="));
server.print(F("{borderRadius:t([\"borderRadius\",\"MozBorderRadius\",\"webkitBorderRadius\"]),boxShadow:t([\"boxShadow\",\"MozBoxShadow\",\"webkitBoxShadow\"])};return function(e,t,r){switch(t.toLowerCase()){case\"o"));
server.print(F("pacity\":var i=Math.round(parseFloat(r)*100);e.style.opacity=r,e.style.filter=\"alpha(opacity=\"+i+\")\";break;default:e.style[n[t]]=r}}}(),setBorderRadius:function(t,n){e.setStyle(t,\"borderRadius\","));
server.print(F("n||\"0\")},setBoxShadow:function(t,n){e.setStyle(t,\"boxShadow\",n||\"none\")},getElementPos:function(t,n){var r=0,i=0,s=t.getBoundingClientRect();r=s.left,i=s.top;if(!n){var o=e.getViewPos();r+=o[0],"));
server.print(F("i+=o[1]}return[r,i]},getElementSize:function(e){return[e.offsetWidth,e.offsetHeight]},getAbsPointerPos:function(e){e||(e=window.event);var t=0,n=0;return typeof e.changedTouches!=\"undefined\"&&e.chan"));
server.print(F("gedTouches.length?(t=e.changedTouches[0].clientX,n=e.changedTouches[0].clientY):typeof e.clientX==\"number\"&&(t=e.clientX,n=e.clientY),{x:t,y:n}},getRelPointerPos:function(e){e||(e=window.event);var "));
server.print(F("t=e.target||e.srcElement,n=t.getBoundingClientRect(),r=0,i=0,s=0,o=0;return typeof e.changedTouches!=\"undefined\"&&e.changedTouches.length?(s=e.changedTouches[0].clientX,o=e.changedTouches[0].clientY"));
server.print(F("):typeof e.clientX==\"number\"&&(s=e.clientX,o=e.clientY),r=s-n.left,i=o-n.top,{x:r,y:i}},getViewPos:function(){var e=document.documentElement;return[(window.pageXOffset||e.scrollLeft)-(e.clientLeft||"));
server.print(F("0),(window.pageYOffset||e.scrollTop)-(e.clientTop||0)]},getViewSize:function(){var e=document.documentElement;return[window.innerWidth||e.clientWidth,window.innerHeight||e.clientHeight]},redrawPositio"));
server.print(F("n:function(){if(e.picker&&e.picker.owner){var t=e.picker.owner,n,r;t.fixed?(n=e.getElementPos(t.targetElement,!0),r=[0,0]):(n=e.getElementPos(t.targetElement),r=e.getViewPos());var i=e.getElementSize("));
server.print(F("t.targetElement),s=e.getViewSize(),o=e.getPickerOuterDims(t),u,a,f;switch(t.position.toLowerCase()){case\"left\":u=1,a=0,f=-1;break;case\"right\":u=1,a=0,f=1;break;case\"top\":u=0,a=1,f=-1;break;defau"));
server.print(F("lt:u=0,a=1,f=1}var l=(i[a]+o[a])/2;if(!t.smartPosition)var c=[n[u],n[a]+i[a]-l+l*f];else var c=[-r[u]+n[u]+o[u]>s[u]?-r[u]+n[u]+i[u]/2>s[u]/2&&n[u]+i[u]-o[u]>=0?n[u]+i[u]-o[u]:n[u]:n[u],-r[a]+n[a]+i[a"));
server.print(F("]+o[a]-l+l*f>s[a]?-r[a]+n[a]+i[a]/2>s[a]/2&&n[a]+i[a]-l-l*f>=0?n[a]+i[a]-l-l*f:n[a]+i[a]-l+l*f:n[a]+i[a]-l+l*f>=0?n[a]+i[a]-l+l*f:n[a]+i[a]-l-l*f];var h=c[u],p=c[a],d=t.fixed?\"fixed\":\"absolute\",v="));
server.print(F("(c[0]+o[0]>n[0]||c[0]<n[0]+i[0])&&c[1]+o[1]<n[1]+i[1];e._drawPosition(t,h,p,d,v)}},_drawPosition:function(t,n,r,i,s){var o=s?0:t.shadowBlur;e.picker.wrap.style.position=i,e.picker.wrap.style.left=n+\""));
server.print(F("px\",e.picker.wrap.style.top=r+\"px\",e.setBoxShadow(e.picker.boxS,t.shadow?new e.BoxShadow(0,o,t.shadowBlur,0,t.shadowColor):null)},getPickerDims:function(t){var n=!!e.getSliderComponent(t),r=[2*t.in"));
server.print(F("setWidth+2*t.padding+t.width+(n?2*t.insetWidth+e.getPadToSliderPadding(t)+t.sliderSize:0),2*t.insetWidth+2*t.padding+t.height+(t.closable?2*t.insetWidth+t.padding+t.buttonHeight:0)];return r},getPicke"));
server.print(F("rOuterDims:function(t){var n=e.getPickerDims(t);return[n[0]+2*t.borderWidth,n[1]+2*t.borderWidth]},getPadToSliderPadding:function(e){return Math.max(e.padding,1.5*(2*e.pointerBorderWidth+e.pointerThic"));
server.print(F("kness))},getPadYComponent:function(e){switch(e.mode.charAt(1).toLowerCase()){case\"v\":return\"v\"}return\"s\"},getSliderComponent:function(e){if(e.mode.length>2)switch(e.mode.charAt(2).toLowerCase())"));
server.print(F("{case\"s\":return\"s\";case\"v\":return\"v\"}return null},onDocumentMouseDown:function(t){t||(t=window.event);var n=t.target||t.srcElement;n._jscLinkedInstance?n._jscLinkedInstance.showOnClick&&n._jsc"));
server.print(F("LinkedInstance.show():n._jscControlName?e.onControlPointerStart(t,n,n._jscControlName,\"mouse\"):e.picker&&e.picker.owner&&e.picker.owner.hide()},onDocumentTouchStart:function(t){t||(t=window.event);v"));
server.print(F("ar n=t.target||t.srcElement;n._jscLinkedInstance?n._jscLinkedInstance.showOnClick&&n._jscLinkedInstance.show():n._jscControlName?e.onControlPointerStart(t,n,n._jscControlName,\"touch\"):e.picker&&e.pi"));
server.print(F("cker.owner&&e.picker.owner.hide()},onWindowResize:function(t){e.redrawPosition()},onParentScroll:function(t){e.picker&&e.picker.owner&&e.picker.owner.hide()},_pointerMoveEvent:{mouse:\"mousemove\",tou"));
server.print(F("ch:\"touchmove\"},_pointerEndEvent:{mouse:\"mouseup\",touch:\"touchend\"},_pointerOrigin:null,_capturedTarget:null,onControlPointerStart:function(t,n,r,i){var s=n._jscInstance;e.preventDefault(t),e.ca"));
server.print(F("ptureTarget(n);var o=function(s,o){e.attachGroupEvent(\"drag\",s,e._pointerMoveEvent[i],e.onDocumentPointerMove(t,n,r,i,o)),e.attachGroupEvent(\"drag\",s,e._pointerEndEvent[i],e.onDocumentPointerEnd(t"));
server.print(F(",n,r,i))};o(document,[0,0]);if(window.parent&&window.frameElement){var u=window.frameElement.getBoundingClientRect(),a=[-u.left,-u.top];o(window.parent.window.document,a)}var f=e.getAbsPointerPos(t),l"));
server.print(F("=e.getRelPointerPos(t);e._pointerOrigin={x:f.x-l.x,y:f.y-l.y};switch(r){case\"pad\":switch(e.getSliderComponent(s)){case\"s\":s.hsv[1]===0&&s.fromHSV(null,100,null);break;case\"v\":s.hsv[2]===0&&s.fro"));
server.print(F("mHSV(null,null,100)}e.setPad(s,t,0,0);break;case\"sld\":e.setSld(s,t,0)}e.dispatchFineChange(s)},onDocumentPointerMove:function(t,n,r,i,s){return function(t){var i=n._jscInstance;switch(r){case\"pad\""));
server.print(F(":t||(t=window.event),e.setPad(i,t,s[0],s[1]),e.dispatchFineChange(i);break;case\"sld\":t||(t=window.event),e.setSld(i,t,s[1]),e.dispatchFineChange(i)}}},onDocumentPointerEnd:function(t,n,r,i){return f"));
server.print(F("unction(t){var r=n._jscInstance;e.detachGroupEvents(\"drag\"),e.releaseTarget(),e.dispatchChange(r)}},dispatchChange:function(t){t.valueElement&&e.isElementType(t.valueElement,\"input\")&&e.fireEvent("));
server.print(F("t.valueElement,\"change\")},dispatchFineChange:function(e){if(e.onFineChange){var t;typeof e.onFineChange==\"string\"?t=new Function(e.onFineChange):t=e.onFineChange,t.call(e)}},setPad:function(t,n,r,"));
server.print(F("i){var s=e.getAbsPointerPos(n),o=r+s.x-e._pointerOrigin.x-t.padding-t.insetWidth,u=i+s.y-e._pointerOrigin.y-t.padding-t.insetWidth,a=o*(360/(t.width-1)),f=100-u*(100/(t.height-1));switch(e.getPadYComp"));
server.print(F("onent(t)){case\"s\":t.fromHSV(a,f,null,e.leaveSld);break;case\"v\":t.fromHSV(a,null,f,e.leaveSld)}},setSld:function(t,n,r){var i=e.getAbsPointerPos(n),s=r+i.y-e._pointerOrigin.y-t.padding-t.insetWidth"));
server.print(F(",o=100-s*(100/(t.height-1));switch(e.getSliderComponent(t)){case\"s\":t.fromHSV(null,o,null,e.leavePad);break;case\"v\":t.fromHSV(null,null,o,e.leavePad)}},_vmlNS:\"jsc_vml_\",_vmlCSS:\"jsc_vml_css_\""));
server.print(F(",_vmlReady:!1,initVML:function(){if(!e._vmlReady){var t=document;t.namespaces[e._vmlNS]||t.namespaces.add(e._vmlNS,\"urn:schemas-microsoft-com:vml\");if(!t.styleSheets[e._vmlCSS]){var n=[\"shape\",\"s"));
server.print(F("hapetype\",\"group\",\"background\",\"path\",\"formulas\",\"handles\",\"fill\",\"stroke\",\"shadow\",\"textbox\",\"textpath\",\"imagedata\",\"line\",\"polyline\",\"curve\",\"rect\",\"roundrect\",\"ova"));
server.print(F("l\",\"arc\",\"image\"],r=t.createStyleSheet();r.owningElement.id=e._vmlCSS;for(var i=0;i<n.length;i+=1)r.addRule(e._vmlNS+\"\\\\:\"+n[i],\"behavior:url(#default#VML);\")}e._vmlReady=!0}},createPalette:f"));
server.print(F("unction(){var t={elm:null,draw:null};if(e.isCanvasSupported){var n=document.createElement(\"canvas\"),r=n.getContext(\"2d\"),i=function(e,t,i){n.width=e,n.height=t,r.clearRect(0,0,n.width,n.height);va"));
server.print(F("r s=r.createLinearGradient(0,0,n.width,0);s.addColorStop(0,\"#F00\"),s.addColorStop(1/6,\"#FF0\"),s.addColorStop(2/6,\"#0F0\"),s.addColorStop(.5,\"#0FF\"),s.addColorStop(4/6,\"#00F\"),s.addColorStop(5"));
server.print(F("/6,\"#F0F\"),s.addColorStop(1,\"#F00\"),r.fillStyle=s,r.fillRect(0,0,n.width,n.height);var o=r.createLinearGradient(0,0,0,n.height);switch(i.toLowerCase()){case\"s\":o.addColorStop(0,\"rgba(255,255,25"));
server.print(F("5,0)\"),o.addColorStop(1,\"rgba(255,255,255,1)\");break;case\"v\":o.addColorStop(0,\"rgba(0,0,0,0)\"),o.addColorStop(1,\"rgba(0,0,0,1)\")}r.fillStyle=o,r.fillRect(0,0,n.width,n.height)};t.elm=n,t.draw"));
server.print(F("=i}else{e.initVML();var s=document.createElement(\"div\");s.style.position=\"relative\",s.style.overflow=\"hidden\";var o=document.createElement(e._vmlNS+\":fill\");o.type=\"gradient\",o.method=\"line"));
server.print(F("ar\",o.angle=\"90\",o.colors=\"16.67% #F0F, 33.33% #00F, 50% #0FF, 66.67% #0F0, 83.33% #FF0\";var u=document.createElement(e._vmlNS+\":rect\");u.style.position=\"absolute\",u.style.left=\"-1px\",u.sty"));
server.print(F("le.top=\"-1px\",u.stroked=!1,u.appendChild(o),s.appendChild(u);var a=document.createElement(e._vmlNS+\":fill\");a.type=\"gradient\",a.method=\"linear\",a.angle=\"180\",a.opacity=\"0\";var f=document.c"));
server.print(F("reateElement(e._vmlNS+\":rect\");f.style.position=\"absolute\",f.style.left=\"-1px\",f.style.top=\"-1px\",f.stroked=!1,f.appendChild(a),s.appendChild(f);var i=function(e,t,n){s.style.width=e+\"px\",s."));
server.print(F("style.height=t+\"px\",u.style.width=f.style.width=e+1+\"px\",u.style.height=f.style.height=t+1+\"px\",o.color=\"#F00\",o.color2=\"#F00\";switch(n.toLowerCase()){case\"s\":a.color=a.color2=\"#FFF\";bre"));
server.print(F("ak;case\"v\":a.color=a.color2=\"#000\"}};t.elm=s,t.draw=i}return t},createSliderGradient:function(){var t={elm:null,draw:null};if(e.isCanvasSupported){var n=document.createElement(\"canvas\"),r=n.getC"));
server.print(F("ontext(\"2d\"),i=function(e,t,i,s){n.width=e,n.height=t,r.clearRect(0,0,n.width,n.height);var o=r.createLinearGradient(0,0,0,n.height);o.addColorStop(0,i),o.addColorStop(1,s),r.fillStyle=o,r.fillRect("));
server.print(F("0,0,n.width,n.height)};t.elm=n,t.draw=i}else{e.initVML();var s=document.createElement(\"div\");s.style.position=\"relative\",s.style.overflow=\"hidden\";var o=document.createElement(e._vmlNS+\":fill\""));
server.print(F(");o.type=\"gradient\",o.method=\"linear\",o.angle=\"180\";var u=document.createElement(e._vmlNS+\":rect\");u.style.position=\"absolute\",u.style.left=\"-1px\",u.style.top=\"-1px\",u.stroked=!1,u.appen"));
server.print(F("dChild(o),s.appendChild(u);var i=function(e,t,n,r){s.style.width=e+\"px\",s.style.height=t+\"px\",u.style.width=e+1+\"px\",u.style.height=t+1+\"px\",o.color=n,o.color2=r};t.elm=s,t.draw=i}return t},le"));
server.print(F("aveValue:1,leaveStyle:2,leavePad:4,leaveSld:8,BoxShadow:function(){var e=function(e,t,n,r,i,s){this.hShadow=e,this.vShadow=t,this.blur=n,this.spread=r,this.color=i,this.inset=!!s};return e.prototype.t"));
server.print(F("oString=function(){var e=[Math.round(this.hShadow)+\"px\",Math.round(this.vShadow)+\"px\",Math.round(this.blur)+\"px\",Math.round(this.spread)+\"px\",this.color];return this.inset&&e.push(\"inset\"),e"));
server.print(F(".join(\" \")},e}(),jscolor:function(t,n){function i(e,t,n){e/=255,t/=255,n/=255;var r=Math.min(Math.min(e,t),n),i=Math.max(Math.max(e,t),n),s=i-r;if(s===0)return[null,0,100*i];var o=e===r?3+(n-t)/s:t="));
server.print(F("==r?5+(e-n)/s:1+(t-e)/s;return[60*(o===6?0:o),100*(s/i),100*i]}function s(e,t,n){var r=255*(n/100);if(e===null)return[r,r,r];e/=60,t/=100;var i=Math.floor(e),s=i%2?e-i:1-(e-i),o=r*(1-t),u=r*(1-t*s);sw"));
server.print(F("itch(i){case 6:case 0:return[r,u,o];case 1:return[u,r,o];case 2:return[o,r,u];case 3:return[o,u,r];case 4:return[u,o,r];case 5:return[r,o,u]}}function o(){e.unsetClass(d.targetElement,d.activeClass),e"));
server.print(F(".picker.wrap.parentNode.removeChild(e.picker.wrap),delete e.picker.owner}function u(){function l(){var e=d.insetColor.split(/\\\s+/),n=e.length<2?e[0]:e[1]+\" \"+e[0]+\" \"+e[0]+\" \"+e[1];t.btn.style.b"));
server.print(F("orderColor=n}d._processParentElementsInDOM(),e.picker||(e.picker={owner:null,wrap:document.createElement(\"div\"),box:document.createElement(\"div\"),boxS:document.createElement(\"div\"),boxB:document"));
server.print(F(".createElement(\"div\"),pad:document.createElement(\"div\"),padB:document.createElement(\"div\"),padM:document.createElement(\"div\"),padPal:e.createPalette(),cross:document.createElement(\"div\"),cro"));
server.print(F("ssBY:document.createElement(\"div\"),crossBX:document.createElement(\"div\"),crossLY:document.createElement(\"div\"),crossLX:document.createElement(\"div\"),sld:document.createElement(\"div\"),sldB:do"));
server.print(F("cument.createElement(\"div\"),sldM:document.createElement(\"div\"),sldGrad:e.createSliderGradient(),sldPtrS:document.createElement(\"div\"),sldPtrIB:document.createElement(\"div\"),sldPtrMB:document.c"));
server.print(F("reateElement(\"div\"),sldPtrOB:document.createElement(\"div\"),btn:document.createElement(\"div\"),btnT:document.createElement(\"span\")},e.picker.pad.appendChild(e.picker.padPal.elm),e.picker.padB.ap"));
server.print(F("pendChild(e.picker.pad),e.picker.cross.appendChild(e.picker.crossBY),e.picker.cross.appendChild(e.picker.crossBX),e.picker.cross.appendChild(e.picker.crossLY),e.picker.cross.appendChild(e.picker.cross"));
server.print(F("LX),e.picker.padB.appendChild(e.picker.cross),e.picker.box.appendChild(e.picker.padB),e.picker.box.appendChild(e.picker.padM),e.picker.sld.appendChild(e.picker.sldGrad.elm),e.picker.sldB.appendChild(e"));
server.print(F(".picker.sld),e.picker.sldB.appendChild(e.picker.sldPtrOB),e.picker.sldPtrOB.appendChild(e.picker.sldPtrMB),e.picker.sldPtrMB.appendChild(e.picker.sldPtrIB),e.picker.sldPtrIB.appendChild(e.picker.sldPt"));
server.print(F("rS),e.picker.box.appendChild(e.picker.sldB),e.picker.box.appendChild(e.picker.sldM),e.picker.btn.appendChild(e.picker.btnT),e.picker.box.appendChild(e.picker.btn),e.picker.boxB.appendChild(e.picker.bo"));
server.print(F("x),e.picker.wrap.appendChild(e.picker.boxS),e.picker.wrap.appendChild(e.picker.boxB));var t=e.picker,n=!!e.getSliderComponent(d),r=e.getPickerDims(d),i=2*d.pointerBorderWidth+d.pointerThickness+2*d.cr"));
server.print(F("ossSize,s=e.getPadToSliderPadding(d),o=Math.min(d.borderRadius,Math.round(d.padding*Math.PI)),u=\"crosshair\";t.wrap.style.clear=\"both\",t.wrap.style.width=r[0]+2*d.borderWidth+\"px\",t.wrap.style.he"));
server.print(F("ight=r[1]+2*d.borderWidth+\"px\",t.wrap.style.zIndex=d.zIndex,t.box.style.width=r[0]+\"px\",t.box.style.height=r[1]+\"px\",t.boxS.style.position=\"absolute\",t.boxS.style.left=\"0\",t.boxS.style.top="));
server.print(F("\"0\",t.boxS.style.width=\"100%\",t.boxS.style.height=\"100%\",e.setBorderRadius(t.boxS,o+\"px\"),t.boxB.style.position=\"relative\",t.boxB.style.border=d.borderWidth+\"px solid\",t.boxB.style.borderCo"));
server.print(F("lor=d.borderColor,t.boxB.style.background=d.backgroundColor,e.setBorderRadius(t.boxB,o+\"px\"),t.padM.style.background=t.sldM.style.background=\"#FFF\",e.setStyle(t.padM,\"opacity\",\"0\"),e.setStyle("));
server.print(F("t.sldM,\"opacity\",\"0\"),t.pad.style.position=\"relative\",t.pad.style.width=d.width+\"px\",t.pad.style.height=d.height+\"px\",t.padPal.draw(d.width,d.height,e.getPadYComponent(d)),t.padB.style.posit"));
server.print(F("ion=\"absolute\",t.padB.style.left=d.padding+\"px\",t.padB.style.top=d.padding+\"px\",t.padB.style.border=d.insetWidth+\"px solid\",t.padB.style.borderColor=d.insetColor,t.padM._jscInstance=d,t.padM._"));
server.print(F("jscControlName=\"pad\",t.padM.style.position=\"absolute\",t.padM.style.left=\"0\",t.padM.style.top=\"0\",t.padM.style.width=d.padding+2*d.insetWidth+d.width+s/2+\"px\",t.padM.style.height=r[1]+\"px\","));
server.print(F("t.padM.style.cursor=u,t.cross.style.position=\"absolute\",t.cross.style.left=t.cross.style.top=\"0\",t.cross.style.width=t.cross.style.height=i+\"px\",t.crossBY.style.position=t.crossBX.style.position"));
server.print(F("=\"absolute\",t.crossBY.style.background=t.crossBX.style.background=d.pointerBorderColor,t.crossBY.style.width=t.crossBX.style.height=2*d.pointerBorderWidth+d.pointerThickness+\"px\",t.crossBY.style.h"));
server.print(F("eight=t.crossBX.style.width=i+\"px\",t.crossBY.style.left=t.crossBX.style.top=Math.floor(i/2)-Math.floor(d.pointerThickness/2)-d.pointerBorderWidth+\"px\",t.crossBY.style.top=t.crossBX.style.left=\"0"));
server.print(F("\",t.crossLY.style.position=t.crossLX.style.position=\"absolute\",t.crossLY.style.background=t.crossLX.style.background=d.pointerColor,t.crossLY.style.height=t.crossLX.style.width=i-2*d.pointerBorderWi"));
server.print(F("dth+\"px\",t.crossLY.style.width=t.crossLX.style.height=d.pointerThickness+\"px\",t.crossLY.style.left=t.crossLX.style.top=Math.floor(i/2)-Math.floor(d.pointerThickness/2)+\"px\",t.crossLY.style.top=t"));
server.print(F(".crossLX.style.left=d.pointerBorderWidth+\"px\",t.sld.style.overflow=\"hidden\",t.sld.style.width=d.sliderSize+\"px\",t.sld.style.height=d.height+\"px\",t.sldGrad.draw(d.sliderSize,d.height,\"#000\","));
server.print(F("\"#000\"),t.sldB.style.display=n?\"block\":\"none\",t.sldB.style.position=\"absolute\",t.sldB.style.right=d.padding+\"px\",t.sldB.style.top=d.padding+\"px\",t.sldB.style.border=d.insetWidth+\"px solid"));
server.print(F("\",t.sldB.style.borderColor=d.insetColor,t.sldM._jscInstance=d,t.sldM._jscControlName=\"sld\",t.sldM.style.display=n?\"block\":\"none\",t.sldM.style.position=\"absolute\",t.sldM.style.right=\"0\",t.sld"));
server.print(F("M.style.top=\"0\",t.sldM.style.width=d.sliderSize+s/2+d.padding+2*d.insetWidth+\"px\",t.sldM.style.height=r[1]+\"px\",t.sldM.style.cursor=\"default\",t.sldPtrIB.style.border=t.sldPtrOB.style.border=d."));
server.print(F("pointerBorderWidth+\"px solid \"+d.pointerBorderColor,t.sldPtrOB.style.position=\"absolute\",t.sldPtrOB.style.left=-(2*d.pointerBorderWidth+d.pointerThickness)+\"px\",t.sldPtrOB.style.top=\"0\",t.sldP"));
server.print(F("trMB.style.border=d.pointerThickness+\"px solid \"+d.pointerColor,t.sldPtrS.style.width=d.sliderSize+\"px\",t.sldPtrS.style.height=m+\"px\",t.btn.style.display=d.closable?\"block\":\"none\",t.btn.styl"));
server.print(F("e.position=\"absolute\",t.btn.style.left=d.padding+\"px\",t.btn.style.bottom=d.padding+\"px\",t.btn.style.padding=\"0 15px\",t.btn.style.height=d.buttonHeight+\"px\",t.btn.style.border=d.insetWidth+\""));
server.print(F("px solid\",l(),t.btn.style.color=d.buttonColor,t.btn.style.font=\"12px sans-serif\",t.btn.style.textAlign=\"center\";try{t.btn.style.cursor=\"pointer\"}catch(c){t.btn.style.cursor=\"hand\"}t.btn.onmou"));
server.print(F("sedown=function(){d.hide()},t.btnT.style.lineHeight=d.buttonHeight+\"px\",t.btnT.innerHTML=\"\",t.btnT.appendChild(document.createTextNode(d.closeText)),a(),f(),e.picker.owner&&e.picker.owner!==d&&e.u"));
server.print(F("nsetClass(e.picker.owner.targetElement,d.activeClass),e.picker.owner=d,e.isElementType(v,\"body\")?e.redrawPosition():e._drawPosition(d,0,0,\"relative\",!1),t.wrap.parentNode!=v&&v.appendChild(t.wrap)"));
server.print(F(",e.setClass(d.targetElement,d.activeClass)}function a(){switch(e.getPadYComponent(d)){case\"s\":var t=1;break;case\"v\":var t=2}var n=Math.round(d.hsv[0]/360*(d.width-1)),r=Math.round((1-d.hsv[t]/100)"));
server.print(F("*(d.height-1)),i=2*d.pointerBorderWidth+d.pointerThickness+2*d.crossSize,o=-Math.floor(i/2);e.picker.cross.style.left=n+o+\"px\",e.picker.cross.style.top=r+o+\"px\";switch(e.getSliderComponent(d)){cas"));
server.print(F("e\"s\":var u=s(d.hsv[0],100,d.hsv[2]),a=s(d.hsv[0],0,d.hsv[2]),f=\"rgb(\"+Math.round(u[0])+\",\"+Math.round(u[1])+\",\"+Math.round(u[2])+\")\",l=\"rgb(\"+Math.round(a[0])+\",\"+Math.round(a[1])+\",\"+"));
server.print(F("Math.round(a[2])+\")\";e.picker.sldGrad.draw(d.sliderSize,d.height,f,l);break;case\"v\":var c=s(d.hsv[0],d.hsv[1],100),f=\"rgb(\"+Math.round(c[0])+\",\"+Math.round(c[1])+\",\"+Math.round(c[2])+\")\",l"));
server.print(F("=\"#000\";e.picker.sldGrad.draw(d.sliderSize,d.height,f,l)}}function f(){var t=e.getSliderComponent(d);if(t){switch(t){case\"s\":var n=1;break;case\"v\":var n=2}var r=Math.round((1-d.hsv[n]/100)*(d.he"));
server.print(F("ight-1));e.picker.sldPtrOB.style.top=r-(2*d.pointerBorderWidth+d.pointerThickness)-Math.floor(m/2)+\"px\"}}function l(){return e.picker&&e.picker.owner===d}function c(){d.importColor()}this.value=null"));
server.print(F(",this.valueElement=t,this.styleElement=t,this.required=!0,this.refine=!0,this.hash=!1,this.uppercase=!0,this.onFineChange=null,this.activeClass=\"jscolor-active\",this.minS=0,this.maxS=100,this.minV=0"));
server.print(F(",this.maxV=100,this.hsv=[0,0,100],this.rgb=[255,255,255],this.width=181,this.height=101,this.showOnClick=!0,this.mode=\"HSV\",this.position=\"bottom\",this.smartPosition=!0,this.sliderSize=16,this.cro"));
server.print(F("ssSize=8,this.closable=!1,this.closeText=\"Close\",this.buttonColor=\"#000000\",this.buttonHeight=18,this.padding=12,this.backgroundColor=\"#FFFFFF\",this.borderWidth=1,this.borderColor=\"#BBBBBB\",th"));
server.print(F("is.borderRadius=8,this.insetWidth=1,this.insetColor=\"#BBBBBB\",this.shadow=!0,this.shadowBlur=15,this.shadowColor=\"rgba(0,0,0,0.2)\",this.pointerColor=\"#4C4C4C\",this.pointerBorderColor=\"#FFFFFF\""));
server.print(F(",this.pointerBorderWidth=1,this.pointerThickness=2,this.zIndex=1e3,this.container=null;for(var r in n)n.hasOwnProperty(r)&&(this[r]=n[r]);this.hide=function(){l()&&o()},this.show=function(){u()},this."));
server.print(F("redraw=function(){l()&&u()},this.importColor=function(){this.valueElement?e.isElementType(this.valueElement,\"input\")?this.refine?!this.required&&/^\\\s*$/.test(this.valueElement.value)?(this.valueElem"));
server.print(F("ent.value=\"\",this.styleElement&&(this.styleElement.style.backgroundImage=this.styleElement._jscOrigStyle.backgroundImage,this.styleElement.style.backgroundColor=this.styleElement._jscOrigStyle.backg"));
server.print(F("roundColor,this.styleElement.style.color=this.styleElement._jscOrigStyle.color),this.exportColor(e.leaveValue|e.leaveStyle)):this.fromString(this.valueElement.value)||this.exportColor():this.fromStrin"));
server.print(F("g(this.valueElement.value,e.leaveValue)||(this.styleElement&&(this.styleElement.style.backgroundImage=this.styleElement._jscOrigStyle.backgroundImage,this.styleElement.style.backgroundColor=this.style"));
server.print(F("Element._jscOrigStyle.backgroundColor,this.styleElement.style.color=this.styleElement._jscOrigStyle.color),this.exportColor(e.leaveValue|e.leaveStyle)):this.exportColor():this.exportColor()},this.expo"));
server.print(F("rtColor=function(t){if(!(t&e.leaveValue)&&this.valueElement){var n=this.toString();this.uppercase&&(n=n.toUpperCase()),this.hash&&(n=\"#\"+n),e.isElementType(this.valueElement,\"input\")?this.valueEle"));
server.print(F("ment.value=n:this.valueElement.innerHTML=n}t&e.leaveStyle||this.styleElement&&(this.styleElement.style.backgroundImage=\"none\",this.styleElement.style.backgroundColor=\"#\"+this.toString(),this.style"));
server.print(F("Element.style.color=this.isLight()?\"#000\":\"#FFF\"),!(t&e.leavePad)&&l()&&a(),!(t&e.leaveSld)&&l()&&f()},this.fromHSV=function(e,t,n,r){if(e!==null){if(isNaN(e))return!1;e=Math.max(0,Math.min(360,e)"));
server.print(F(")}if(t!==null){if(isNaN(t))return!1;t=Math.max(0,Math.min(100,this.maxS,t),this.minS)}if(n!==null){if(isNaN(n))return!1;n=Math.max(0,Math.min(100,this.maxV,n),this.minV)}this.rgb=s(e===null?this.hsv[0"));
server.print(F("]:this.hsv[0]=e,t===null?this.hsv[1]:this.hsv[1]=t,n===null?this.hsv[2]:this.hsv[2]=n),this.exportColor(r)},this.fromRGB=function(e,t,n,r){if(e!==null){if(isNaN(e))return!1;e=Math.max(0,Math.min(255,e"));
server.print(F("))}if(t!==null){if(isNaN(t))return!1;t=Math.max(0,Math.min(255,t))}if(n!==null){if(isNaN(n))return!1;n=Math.max(0,Math.min(255,n))}var o=i(e===null?this.rgb[0]:e,t===null?this.rgb[1]:t,n===null?this.r"));
server.print(F("gb[2]:n);o[0]!==null&&(this.hsv[0]=Math.max(0,Math.min(360,o[0]))),o[2]!==0&&(this.hsv[1]=o[1]===null?null:Math.max(0,this.minS,Math.min(100,this.maxS,o[1]))),this.hsv[2]=o[2]===null?null:Math.max(0,t"));
server.print(F("his.minV,Math.min(100,this.maxV,o[2]));var u=s(this.hsv[0],this.hsv[1],this.hsv[2]);this.rgb[0]=u[0],this.rgb[1]=u[1],this.rgb[2]=u[2],this.exportColor(r)},this.fromString=function(e,t){var n;if(n=e.m"));
server.print(F("atch(/^\\\W*([0-9A-F]{3}([0-9A-F]{3})?)\\\W*$/i))return n[1].length===6?this.fromRGB(parseInt(n[1].substr(0,2),16),parseInt(n[1].substr(2,2),16),parseInt(n[1].substr(4,2),16),t):this.fromRGB(parseInt(n[1]"));
server.print(F(".charAt(0)+n[1].charAt(0),16),parseInt(n[1].charAt(1)+n[1].charAt(1),16),parseInt(n[1].charAt(2)+n[1].charAt(2),16),t),!0;if(n=e.match(/^\\\W*rgba?\\\(([^)]*)\\\)\\\W*$/i)){var r=n[1].split(\",\"),i=/^\\\s*(\\\d*"));
server.print(F(")(\\\.\\\d+)?\\\s*$/,s,o,u;if(r.length>=3&&(s=r[0].match(i))&&(o=r[1].match(i))&&(u=r[2].match(i))){var a=parseFloat((s[1]||\"0\")+(s[2]||\"\")),f=parseFloat((o[1]||\"0\")+(o[2]||\"\")),l=parseFloat((u[1]||"));
server.print(F("\"0\")+(u[2]||\"\"));return this.fromRGB(a,f,l,t),!0}}return!1},this.toString=function(){return(256|Math.round(this.rgb[0])).toString(16).substr(1)+(256|Math.round(this.rgb[1])).toString(16).substr(1)"));
server.print(F("+(256|Math.round(this.rgb[2])).toString(16).substr(1)},this.toHEXString=function(){return\"#\"+this.toString().toUpperCase()},this.toRGBString=function(){return\"rgb(\"+Math.round(this.rgb[0])+\",\"+M"));
server.print(F("ath.round(this.rgb[1])+\",\"+Math.round(this.rgb[2])+\")\"},this.isLight=function(){return.213*this.rgb[0]+.715*this.rgb[1]+.072*this.rgb[2]>127.5},this._processParentElementsInDOM=function(){if(this."));
server.print(F("_linkedElementsProcessed)return;this._linkedElementsProcessed=!0;var t=this.targetElement;do{var n=e.getStyle(t);n&&n.position.toLowerCase()===\"fixed\"&&(this.fixed=!0),t!==this.targetElement&&(t._js"));
server.print(F("cEventsAttached||(e.attachEvent(t,\"scroll\",e.onParentScroll),t._jscEventsAttached=!0))}while((t=t.parentNode)&&!e.isElementType(t,\"body\"))};if(typeof t==\"string\"){var h=t,p=document.getElementBy"));
server.print(F("Id(h);p?this.targetElement=p:e.warn(\"Could not find target element with ID '\"+h+\"'\")}else t?this.targetElement=t:e.warn(\"Invalid target element: '\"+t+\"'\");if(this.targetElement._jscLinkedInsta"));
server.print(F("nce){e.warn(\"Cannot link jscolor twice to the same element. Skipping.\");return}this.targetElement._jscLinkedInstance=this,this.valueElement=e.fetchElement(this.valueElement),this.styleElement=e.fetc"));
server.print(F("hElement(this.styleElement);var d=this,v=this.container?e.fetchElement(this.container):document.getElementsByTagName(\"body\")[0],m=3;if(e.isElementType(this.targetElement,\"button\"))if(this.targetEl"));
server.print(F("ement.onclick){var g=this.targetElement.onclick;this.targetElement.onclick=function(e){return g.call(this,e),!1}}else this.targetElement.onclick=function(){return!1};if(this.valueElement&&e.isElementT"));
server.print(F("ype(this.valueElement,\"input\")){var y=function(){d.fromString(d.valueElement.value,e.leaveValue),e.dispatchFineChange(d)};e.attachEvent(this.valueElement,\"keyup\",y),e.attachEvent(this.valueElement"));
server.print(F(",\"input\",y),e.attachEvent(this.valueElement,\"blur\",c),this.valueElement.setAttribute(\"autocomplete\",\"off\")}this.styleElement&&(this.styleElement._jscOrigStyle={backgroundImage:this.styleElemen"));
server.print(F("t.style.backgroundImage,backgroundColor:this.styleElement.style.backgroundColor,color:this.styleElement.style.color}),this.value?this.fromString(this.value)||this.exportColor():this.importColor()}};re"));
server.print(F("turn e.jscolor.lookupClass=\"jscolor\",e.jscolor.installByClassName=function(t){var n=document.getElementsByTagName(\"input\"),r=document.getElementsByTagName(\"button\");e.tryInstallOnElements(n,t),e"));
server.print(F(".tryInstallOnElements(r,t)},e.register(),e.jscolor}());"));
}
