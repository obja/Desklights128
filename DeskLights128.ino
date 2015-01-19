/*

 ### Parameter Explanation
 All exposed function calls take 0 or more single letter parameters:
 r: red value (0 - 255)
 g: green value (0 - 255)
 b: blue value (0 - 255)
 h: hex string representing color (000000 - ffffff)
 x: x coordinate
 y: y coordinate
 d: delay in milliseconds
 i: grid id of pattern/pixel (grid 0,0 = 0; grid 1,0 = 1, etc)
 n: number of pixel on strip, starts at 0
 s: call show() command automatically (0 or 1, defaults to 1)
 
 Note: All color parameters can be specified as either RGB or hex
 
 ### Public Functions
 off : Turn off all pixels
 show : show any set, but not yet shown pixels
 color: set all pixels to color; takes color
 wipe: like color, but down the strip; optional delay
 alert: flash all pixels; takes color and optional delay
 pixel: set pixel to color; takes id or number, color, and optional show
 default: set default pattern to loop, takes pattern id
 gridtest: show grid pixel by pixel, no params
 lighttest: show rgb on all pixels, no params
 
 ### Example
 
 Here's an example using alert...
 
 Flash bright white for default length of time 
 http://server/alert?r=255&g=255&b=255
 or
 http://server/alert?h=ffffff
 or with a 1 second duration
 http://server/alert?h=ffffff&d=1000
 
 */

#define WEBDUINO_FAIL_MESSAGE "NOT ok\n"
#define WEBDUINO_COMMANDS_COUNT 10

#define ISMATRIX

//commands count should fix the error where newly added commands don't work
#include "SPI.h"
#include "avr/pgmspace.h"
#include "Ethernet.h"
#include "EthernetUdp.h"
#include "EthernetBonjour.h"
#include "WebServer.h"
#include <Adafruit_WS2801.h>
#include "Adafruit_GFX.h"
#include "glcdfont.c"
#if defined (ISMATRIX)
#include <Adafruit_2801Matrix.h>
#endif

/*** This is what you will almost certainly have to change ***/

// WEB stuff
static uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0xF9, 0x04, 0xF9 }; // update this to match your arduino/shield
static uint8_t ip[] = {   192,168,0,220 }; // update this to match your network
String theIP = (String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3]; //create the IP as a string
//secondary tables web client
boolean tableTwo = true; //set to false to disable tableTwo
EthernetClient client;
IPAddress tableTwoIP(192,168,0,221); //this is the second table
String tableTwoIPStr = (String)tableTwoIP[0] +  "." + (String)tableTwoIP[1] + "." + (String)tableTwoIP[2] + "." + (String)tableTwoIP[3];
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
int defaultPattern = 0;
#if defined (ISMATRIX)
Adafruit_2801Matrix theMatrix = Adafruit_2801Matrix(max_x, max_y, dataPin, clockPin,
NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
WS2801_RGB);
#else
Adafruit_WS2801 strip = Adafruit_WS2801(max_x, max_y, dataPin, clockPin, WS2801_RGB); // setting max_x and max_y here lets us use draw functions
#endif

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

// 'graph' style x,y where 0,0 is bottom left

int grid[STRIPLEN] = {
7,8,23,24,39,40,55,56,71,72,87,88,103,104,119,120,
6,9,22,25,38,41,54,57,70,73,86,89,102,105,118,121,
5,10,21,26,37,42,53,58,69,74,85,90,101,106,117,122,
4,11,20,27,36,43,52,59,68,75,84,91,100,107,116,123,
3,12,19,28,35,44,51,60,67,76,83,92,99,108,115,124,
2,13,18,29,34,45,50,61,66,77,82,93,98,109,114,125,
1,14,17,30,33,46,49,62,65,78,81,94,97,110,113,126,
0,15,16,31,32,47,48,63,64,79,80,95,96,111,112,127
};

// 'screen' style x,y where 0,0 is top left
/* int grid[STRIPLEN] = {
 	0,9,10,19,20,29,30,39,
 	1,8,11,18,21,28,31,38,
 	2,7,12,17,22,27,32,37,
 	3,6,13,16,23,26,33,36,
 	4,5,14,15,24,25,34,35
 };
 */
    uint32_t red[6] = { //red
    Color(255,0,0),
    Color(200,0,0),
    Color(150,0,0),
    Color(100,0,0),
    Color(50,0,0),
    Color(0,0,0),
  };
  uint32_t blue[6] = { //blue
    Color(0,0,255),
    Color(0,0,200),
    Color(0,0,150),
    Color(0,0,100),
    Color(0,0,50),
    Color(0,0,0),
  };


/*** Things you might want to change ***/

WebServer webserver("", 80); // port to listen on

// ROM-based messages for webduino lib, maybe overkill here
void printOk(WebServer &server) {
  server.println(F("HTTP/1.1 200 OK"));
  server.println(F("Content-Type: text/html"));
  server.println(F("Connection: close"));
  server.print(F("\r\n"));
  server.println(F("<!DOCTYPE HTML PUBLIC -//W3C//DTD HTML 4.00 TRANSITIONAL//EN><html><head><title>")); //opening html
  server.println(F("DeskLights128</title>")); //title
  server.println(F("<meta name = 'viewport' content = 'width = device-width'> <meta name = 'viewport' content = 'initial-scale = 1.0'> </head>"));
  server.print(F("<script> function process() { var url='write?c=' + document.getElementById('url').value; location.href=url; return false; } </script>")); //script for form submitting
  server.print(F("<script> function process2() { var urlalert='alert?h=' + document.getElementById('alert').value + '&d=1000'; location.href=urlalert; return false; } </script>")); //script for alert submitting
  server.print(F("<script> function process3() { var urlalert='default?id=' + document.getElementById('default').value; location.href=urlalert; return false; } </script>")); //script for default submitting
  server.println(F("<body>")); //links below here
  server.println(F("<form onSubmit='return process2();'>Send Alert <select id='alert'>")); //form select for Alerts
  server.println(F("<option value='FF0000'>Red</option>")); //red
  server.println(F("<option value='FF6600'>Orange</option>")); //orange
  server.println(F("<option value='FFFF00'>Yellow</option>")); //yellow
  server.println(F("<option value='336600'>Green</option>")); //green
  server.println(F("<option value='003333'>Blue</option>")); //blue
  server.println(F("<option value='660033'>Purple</option>")); //purple
  server.println(F("<input type='submit' value='go'> </select> </form>")); //end select
  server.println(F("<form onSubmit='return process3();'>Run Command <select id='default'>")); //form select for Defaults
  server.println(F("<option value='1'>Rainbow</option>")); //red
  server.println(F("<option value='2'>Random</option>")); //orange
  server.println(F("<option value='3'>K.I.T.T.</option>")); //yellow
  server.println(F("<input type='submit' value='go'> </select> </form>")); //end select
  server.println(F("<form onSubmit='return process();'> Write Character: <input type='text' name='url' id='url'> <input type='submit' value='go'> </form>")); //this writes a single character to the board
  server.print(F("<a href='default?id=4'>All Off</a><p></p>"));
  server.println(F("</body></html>")); //end html
}
P(noauth) = "User Denied\n";

// max length of param names and values
#define NAMELEN 2
#define VALUELEN 32

/*** Below here shouldn't need to change ***/
// LED support functions

// create the "Color" value from rgb...This is right from Adafruit
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

#if defined (ISMATRIX)
// set all pixels to a "Color" value
void colorAll(uint32_t c) {
  for (int i=0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
  }
  defaultPattern = 0;
  theMatrix.show();
}
//colorAllDef is just colorAll without the defaultPattern set
void colorAllDef(uint32_t c) {
  for (int i=0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
  }
  theMatrix.show();
}
// set all pixels to a "Color" value, one at a time, with a delay
void colorWipe(uint32_t c, uint8_t wait) {
  for (int i=0; i < theMatrix.numPixels(); i++) {
    theMatrix.setPixelColor(i, c);
    defaultPattern = 0;
    theMatrix.show();
    delay(wait);
  }
}
// show the grid to verify
void gridTest(int wait) {
  int x;
  int y;
  uint32_t on = Color(255,255,255);
  uint32_t off = Color(0,0,0);

  if (!wait) {
    wait = 250;
  }

  for ( x = 0; x <= max_x; x++) {
    for ( y = 0; y <= max_y; y++) {
      defaultPattern = 0;
      theMatrix.setPixelColor(g2p(x,y), on);
      theMatrix.show();
      delay(wait);
      theMatrix.setPixelColor(g2p(x,y), off);
      theMatrix.show();
    }
  }
}
// random pixel, random color
// short pattern, very responsive
void p_random (int wait) {
  theMatrix.setPixelColor(
  random(0, theMatrix.numPixels()),
  Color(random(0,255), random(0,255), random(0,255))
    );
  theMatrix.show();
  delay(wait);
}

// If you were at maker faire, you know this pattern
// it takes about a second to run, so new requests will wait
void p_rainbow() {
  int i, j;
  for (j=0; j < 256; j++) {
    for (i=0; i < theMatrix.numPixels(); i++) {
      theMatrix.setPixelColor(i, Wheel( ((i * 256 / theMatrix.numPixels()) + j) % 256) );
    }
    theMatrix.show();
  }
}

// cylon or K.I.T.T. whichever 
void p_cylon(uint32_t c[6]) {
  int x;
  int wait=75;

  for (x=0; x <= max_x; x++) {
    int mod = 0;
    while ((mod < 6) && (x - mod >= 0)) {
      int y = 0;
      while (y <= max_y) {
        theMatrix.setPixelColor(g2p(x-mod,y++), c[mod]);
      }
      mod++;
    }
    theMatrix.show();
    delay(wait);
  }

  for (x=max_x; x >= 0; x--) {
    int mod = 0;
    while ((mod < 6) && (x + mod <= max_x)) {
      int y = 0;
      while (y <= max_y) {
        theMatrix.setPixelColor(g2p(x+mod,y++), c[mod]);
      }
      mod++;
    }
    theMatrix.show();
    delay(wait);
  }

}
//visualizer, takes string of 16 numbers which are Y heights
void vu(String input) {
  uint32_t color = Color(255,0,0);
  for(int i = 0; i<16; i++) {
    int y = input.charAt(i) - '0';
    if(y > max_y) {
      y = max_y;
    }
    if(y < 1) {
      for(y = 1; y<max_y+1; y++) {
        theMatrix.setPixelColor(g2p(i+1,y), Color(0,0,0));
      }
    }
    else {
      int y_orig = y;
      for(y; y>0; y--) {
        theMatrix.setPixelColor(g2p(i+1,y), color);
      }
      y = y_orig+1;
      for(y; y<max_y+1; y++) {
        theMatrix.setPixelColor(g2p(i+1,y), Color(0,0,0));
      }
    }
  }
}

void cmd_writechar(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  unsigned char theChar;
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while(strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if((rc != URLPARAM_EOS)) {
      switch(name[0]) {
        case 'c':
           theChar = value[0];
           break;
      }
    }
  }
  theMatrix.print(theChar);
  defaultPattern = 0;
  theMatrix.show();
  printOk(server);
}

void cmd_show(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  defaultPattern = 0;
  theMatrix.show();
  printOk(server);
}
void cmd_vu(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  String inputData;
  
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
        case 'v':
        for(int i = 0; i<16; i++) {
          inputData += String(value[i] - '0');
        }
        break;
      }
    }
  }
  vu(inputData);
  theMatrix.show();
  defaultPattern = 0;
  printOk(server);
}

void cmd_pixel(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int id;
  int gid;
  int x;
  int y;
  int r;
  int g;
  int b;
  int s = 1;
  uint32_t c;
  int use_hex = 0;
  int use_id = 0;
  int use_gid = 0;

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
      case 'i':
        gid = atoi(value);
        use_gid = 1;
        break;
      case 'n':
        id = atoi(value);
        use_id = 1;
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
      }
    }
  }

  if (use_id == 0) {
    if (use_gid != 0) {
      id = grid[gid];
    } 
    else {
      id = g2p(x,y);
    }
  }

  if (use_hex == 0) {
    c = Color(r,g,b);
  }

  theMatrix.setPixelColor(id, c);

  if (s) {
    theMatrix.show();
  }

  defaultPattern = 0;
  printOk(server);
}
#else
// set all pixels to a "Color" value
void colorAll(uint32_t c) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  defaultPattern = 0;
  strip.show();
}
//colorAllDef is just colorAll without the defaultPattern set
void colorAllDef(uint32_t c) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}
// set all pixels to a "Color" value, one at a time, with a delay
void colorWipe(uint32_t c, uint8_t wait) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    defaultPattern = 0;
    strip.show();
    delay(wait);
  }
}
// show the grid to verify
void gridTest(int wait) {
  int x;
  int y;
  uint32_t on = Color(255,255,255);
  uint32_t off = Color(0,0,0);

  if (!wait) {
    wait = 250;
  }

  for ( x = 0; x <= max_x; x++) {
    for ( y = 0; y <= max_y; y++) {
      defaultPattern = 0;
      strip.setPixelColor(g2p(x,y), on);
      strip.show();
      delay(wait);
      strip.setPixelColor(g2p(x,y), off);
      strip.show();
    }
  }
}
// random pixel, random color
// short pattern, very responsive
void p_random (int wait) {
  strip.setPixelColor(
  random(0, strip.numPixels()),
  Color(random(0,255), random(0,255), random(0,255))
    );
  strip.show();
  delay(wait);
}

// If you were at maker faire, you know this pattern
// it takes about a second to run, so new requests will wait
void p_rainbow() {
  int i, j;
  for (j=0; j < 256; j++) {
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }
    strip.show();
  }
}

// cylon or K.I.T.T. whichever 
void p_cylon(uint32_t c[6]) {
  int x;
  int wait=75;

  for (x=0; x <= max_x; x++) {
    int mod = 0;
    while ((mod < 6) && (x - mod >= 0)) {
      int y = 0;
      while (y <= max_y) {
        strip.setPixelColor(g2p(x-mod,y++), c[mod]);
      }
      mod++;
    }
    strip.show();
    delay(wait);
  }

  for (x=max_x; x >= 0; x--) {
    int mod = 0;
    while ((mod < 6) && (x + mod <= max_x)) {
      int y = 0;
      while (y <= max_y) {
        strip.setPixelColor(g2p(x+mod,y++), c[mod]);
      }
      mod++;
    }
    strip.show();
    delay(wait);
  }

}
//visualizer, takes string of 16 numbers which are Y heights
void vu(String input) {
  uint32_t color = Color(255,0,0);
  for(int i = 0; i<16; i++) {
    int y = input.charAt(i) - '0';
    if(y > max_y) {
      y = max_y;
    }
    if(y < 1) {
      for(y = 1; y<max_y+1; y++) {
        strip.setPixelColor(g2p(i+1,y), Color(0,0,0));
      }
    }
    else {
      int y_orig = y;
      for(y; y>0; y--) {
        strip.setPixelColor(g2p(i+1,y), color);
      }
      y = y_orig+1;
      for(y; y<max_y+1; y++) {
        strip.setPixelColor(g2p(i+1,y), Color(0,0,0));
      }
    }
  }
}

void cmd_show(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  defaultPattern = 0;
  strip.show();
  printOk(server);
}
void cmd_vu(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  String inputData;
  
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
        case 'v':
        for(int i = 0; i<16; i++) {
          inputData += String(value[i] - '0');
        }
        break;
      }
    }
  }
  vu(inputData);
  strip.show();
  defaultPattern = 0;
  printOk(server);
}

void cmd_pixel(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int id;
  int gid;
  int x;
  int y;
  int r;
  int g;
  int b;
  int s = 1;
  uint32_t c;
  int use_hex = 0;
  int use_id = 0;
  int use_gid = 0;

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
      case 'i':
        gid = atoi(value);
        use_gid = 1;
        break;
      case 'n':
        id = atoi(value);
        use_id = 1;
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
      }
    }
  }

  if (use_id == 0) {
    if (use_gid != 0) {
      id = grid[gid];
    } 
    else {
      id = g2p(x,y);
    }
  }

  if (use_hex == 0) {
    c = Color(r,g,b);
  }

  strip.setPixelColor(id, c);

  if (s) {
    strip.show();
  }

  defaultPattern = 0;
  printOk(server);
}
#endif


// fade from one color to another: UNFINISHED
void fade(uint32_t c1, uint32_t c2, int wait) {
  if (c1 < c2) {
    while (c1 < c2) {
      colorAll(c1++);
      delay(wait);
    }
  } 
  else {
    while (c1 > c2) {
      colorAll(c1--);
      delay(wait);
    }
  }
}

// this takes x/y coordinates and maps it to a pixel offset
// your grid will need to be updated to match your pixel count and layout
int g2p(int x, int y) {
  if(x%2) { // if odd
    return (max_y * x) + y-1-max_y;
  }
  else { //else true, so
  return (max_y * x) + y -1 -max_y + ((max_y - 1)*-1) + 2 * (max_y - y);
  }
}

// flash color "c" for "wait" ms
void alert(uint32_t c, int wait) {
  colorAll(c);
  delay(wait);
  colorAll(Color(0,0,0));
}



// wipe the major colors through all pixels
void lightTest(int wait) {
  colorWipe(Color(255, 0, 0), wait);
  colorWipe(Color(0, 255, 0), wait);
  colorWipe(Color(0, 0, 255), wait);
  colorWipe(Color(255, 255, 255), wait);
  colorWipe(Color(0, 0, 0), wait);
}

void cmd_index(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  printOk(server);
}

void my_failCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  server.httpFail();
}

void cmd_off(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  colorAll(Color(0,0,0));
  cursor_x = cursor_x_orig;
  cursor_y = cursor_y_orig;
  printOk(server);
}

void cmd_color(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int r;
  int g;
  int b;
  uint32_t c;
  int use_hex = 0;

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];

  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
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
      }
    }
  }

  if (!use_hex) {
    c = Color(r,g,b);
  }
  colorAll(c);
  printOk(server);
}

void cmd_wipe(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int r;
  int g;
  int b;
  int delay;
  uint32_t c;
  int use_hex = 0;

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];

  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
      case 'r':
        r = atoi(value);
        break;
      case 'g':
        g = atoi(value);
        break;
      case 'b':
        b = atoi(value);
        break;
      case 'h':
        c = hexColor(value);
        use_hex++;
        break;
      case 'd':
        delay = atoi(value);
        break;
      }
    }
  }

  if (!use_hex) {
    c = Color(r,g,b);
  }

  colorWipe(c, delay);
  printOk(server);
}

void cmd_default(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      if (name[0] == 'i') {
        defaultPattern = atoi(value);
        colorAllDef(Color(0,0,0));
      }
    }
  }

  printOk(server);
}

void cmd_alert(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int r;
  int g;
  int b;
  int d;
  int use_hex = 0;
  uint32_t c;

  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
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
      case 'd':
        d = atoi(value);
        break;
      }
    }
  }

  if (use_hex == 0) {
    c = Color(r,g,b);
  }

  if (!d) {
    d = 100;
  }

  alert(c, d);
  printOk(server);
}

void cmd_test(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  int id;
  int d;
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      switch(name[0]) {
      case 'i':
        id = atoi(value);
        break;
      case 'd':
        d = atoi(value);
        break;
      }
    }
  }

  switch(id) {
  case 0:
    lightTest(d);
    break;
  case 1:
    gridTest(d);
    break;
  }

  printOk(server);
}

// begin standard arduino setup and loop pattern

void setup() {
  Serial.begin(9600);

  //TODO: I think I've run out of memory, consolidate "tests"
  Ethernet.begin(mac,ip);
  Serial.println(Ethernet.localIP());
  EthernetBonjour.begin("DeskLights");
  EthernetBonjour.addServiceRecord("DeskLights128._http",80,MDNSServiceTCP);
  webserver.setFailureCommand(&my_failCmd);
  webserver.setDefaultCommand(&cmd_index);
  webserver.addCommand("off", &cmd_off);
  webserver.addCommand("show", &cmd_show);
  webserver.addCommand("wipe", &cmd_wipe);
  webserver.addCommand("color", &cmd_color);
  webserver.addCommand("alert", &cmd_alert);
  webserver.addCommand("pixel", &cmd_pixel);
  webserver.addCommand("default", &cmd_default);
  webserver.addCommand("test", &cmd_test);
  webserver.addCommand("write", &cmd_writechar);
  webserver.addCommand("vu", &cmd_vu);
  webserver.begin();
  Udp.begin(localPort);
  
  #if defined (ISMATRIX)
  theMatrix.begin();
  #else
  strip.begin();
  #endif

  // light blip of light to signal we are ready to listen
  colorAll(Color(0,0,11));
  delay(500);
  colorAll(Color(0,0,0));
}

void loop()
{
  //EthernetBonjour.run();
  // listen for connections
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  int packetSize = Udp.parsePacket();
  if(packetSize) {
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    vu(packetBuffer);
    #if defined (ISMATRIX)
    theMatrix.show();
    #else
    strip.show();
    #endif
  }

  // run the default pattern
  switch(defaultPattern) {
  case 1:
    p_rainbow();
    break;
  case 2:
    p_random(50);
    break;
  case 3:
    p_cylon(blue);
    break;
  case 5:
    p_cylon(red);
    break;
  case 4:
    colorAll(Color(0,0,0));
    cursor_x = cursor_x_orig;
    cursor_y = cursor_y_orig;
    break;
  }
}

