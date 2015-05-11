#define WEBDUINO_FAIL_MESSAGE "NOT ok\n"
#define WEBDUINO_COMMANDS_COUNT 20

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
int defaultPattern = 0;
Adafruit_2801Matrix theMatrix = Adafruit_2801Matrix(max_x, max_y, dataPin, clockPin,NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,WS2801_RGB);


//Matrix Scrolling
unsigned long prevFrameTime = 0L;             // For animation timing
#define FPS 10                                // Scrolling speed
uint8_t       msgLen        = 0;              // Empty message
int           msgX          = 16; // Start off right edge
String writeCharStr = "";

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

/* SNAKE SETUP */

int snakeButton = 0;

int hrow=0,hcol=0;//sets the row and col of the snake head
bool game = true;//game is good
bool start = false;//start the game with true
bool ignoreNextTimer=false;//tells the game timer weither or not it should run
//When true the game timer will not update due to the update by a keypress
int sx=8,sy=4;//set snake location
long previousMillis = 0;//used for the game timer
long interval = 500; //how long between each update of the game
unsigned long currentMillis = 0;//used for the game timer

int sDr=1,sDc=0;//used to keep last direction, start off going up

//int array[Y * X];
uint16_t SetElement(uint16_t, uint16_t);//2D array into a number

#define X 16//this is the depth of the field
#define Y 8//this is the length of the field

int gameBoard[X][Y] = //game field, 0 is empty, -1 is food, >0 is snake
{
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0}
};

void checkKeyboard() {
    if (snakeButton == 1) {
      Serial.println("[Left]");
      if (game&&start) {
        moveSnake(-1,0);
	ignoreNextTimer=true;
      }
    }
    else if (snakeButton == 2) { 
      Serial.println("[Right]");
      if (game&&start) {
        moveSnake(1,0);
	ignoreNextTimer=true;
      }
    }
    else if (snakeButton == 3) {
      Serial.println("[Up]");
      if (game&&start) {
        moveSnake(0,1);
	ignoreNextTimer=true;
      }
    }
    else if (snakeButton == 4) {
      Serial.println("[Down]");
      if (game&&start) {
        moveSnake(0,-1);
	ignoreNextTimer=true;
      }
    }
    else if (snakeButton == 5) {
      resetGame();
    }
    else if (snakeButton == 6) {
      start = true;
      drawBoard();
    }
    snakeButton = 0;
}

void updateGame() {
  if (game && start) {
    moveSnake(sDr,sDc);
  }
  if (game && start) {
    drawBoard();
  }
}

void resetGame() {
  resetBoard();
  sDr=1; //move up
  sDc=0;
  loadSnake();
  placeFood();
  findSnakeHead();//find where the snake is starting from
  game=true;
  start=false;
  ignoreNextTimer=false;
  drawBoard();
}

void placeFood() {
  int rx=0,ry=0;
  rx = random(0,X-1);
  ry = random(0,Y-1);
  if (gameBoard[rx][ry]>0) {
    while(gameBoard[rx][ry]>0) {
      rx = random(0,X-1);
      ry = random(0,Y-1);
      if (gameBoard[rx][ry]==0) {
        gameBoard[rx][ry]=-1;
	break;
      }
    }
  }
  else {
    gameBoard[rx][ry]=-1;
  }
}

void loadSnake() {
  gameBoard[sx][sy]=1;
}

void resetBoard() {
  for(int x=0;x<X;x++) {
    for(int y =0;y<Y;y++) {
      gameBoard[x][y]=0;
    }
  }
  loadSnake();
}

void gameOver() {
  game = false;
  start = false;
  for(int light=0;light<255;light++) {
    for(int i =0;i< theMatrix.numPixels();i++) {
      theMatrix.setPixelColor(i,Color(light,0,0));
    }
    theMatrix.show();
    delay(15);
  }
  theMatrix.show();
}

void moveSnake(int row, int col) {
  sDr = row;
  sDc = col;
  int new_r=0,new_c=0;
  new_r=hrow+row;
  new_c=hcol+col;
  if (new_r>=X||new_r<0||new_c>=Y||new_c<0) {
    gameOver();
  }
  else if(gameBoard[new_r][new_c]>0) {
    gameOver();
  }
  else if (gameBoard[new_r][new_c]==-1) {
    gameBoard[new_r][new_c] = 1+gameBoard[hrow][hcol];
    hrow=new_r;
    hcol=new_c;
    placeFood();
    drawBoard();
  }
  else {
    gameBoard[new_r][new_c] = 1+gameBoard[hrow][hcol];
    hrow=new_r;
    hcol=new_c;
    removeTail();
    drawBoard();
  }
}

void removeTail() {
  for (int x=0;x<X;x++) {
    for (int y=0;y<Y;y++) {
      if(gameBoard[x][y]>0) {
        gameBoard[x][y]--;
      }
    }
  }
}

void drawBoard() {
  clear_dsp();
  for (int x=0;x<X;x++) {
    for (int y=0;y<Y;y++) {
      if(gameBoard[x][y]==-1) {//food
        theMatrix.setPixelColor(g2p(x+1,y+1),Color(0,255,0)); //green, food
      }
      else if(gameBoard[x][y]==0) { //empty
        theMatrix.setPixelColor(g2p(x+1,y+1),Color(0,0,0)); //off
      }
      else {
        theMatrix.setPixelColor(g2p(x+1,y+1),Color(0,0,255)); //blue, snake
      }
    }
  }
  theMatrix.show();
}

void findSnakeHead() {
  hrow=0;//clearing out old location
  hcol=0;//clearing out old location
  for (int x=0;x<X;x++) {
    for (int y=0;y<Y;y++) {
      if (gameBoard[x][y]>gameBoard[hrow][hcol]) {
        hrow=x;
	hcol=y;
      }
    }
  }
}


void clear_dsp() {
  for(int i =0;i< theMatrix.numPixels();i++) {
    theMatrix.setPixelColor(i,Color(0,0,0));
  }
  theMatrix.show();
}

uint16_t SetElement(uint16_t row, uint16_t col)
{
	//array[width * row + col] = value;
	return Y * row+col;
}



/* END SNAKE SETUP */

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

//cylon colors

uint32_t red[6] = { //red
  Color(255,0,0),
  Color(200,0,0),
  Color(150,0,0),
  Color(100,0,0),
  Color(50,0,0),
  Color(0,0,0),
};

uint32_t orange[6] = { //orange
  Color(255,128,0),
  Color(200,100,0),
  Color(150,75,0),
  Color(100,50,0),
  Color(50,25,0),
  Color(0,0,0),
};

uint32_t yellow[6] = { //yellow
  Color(255,255,0),
  Color(200,200,0),
  Color(150,150,0),
  Color(100,100,0),
  Color(50,50,0),
  Color(0,0,0),
};

uint32_t green[6] = { //green
  Color(0,255,0),
  Color(0,200,0),
  Color(0,150,0),
  Color(0,100,0),
  Color(0,50,0),
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

uint32_t purple[6] = { //blue
  Color(127,0,255),
  Color(100,0,200),
  Color(75,0,150),
  Color(50,0,100),
  Color(25,0,50),
  Color(0,0,0),
};

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
  server.print(F("<script> function process() { var url='write?l=' + document.getElementById('url').value.length + '&c=' + document.getElementById('url').value; location.href=url; return false; } </script>")); //script for form submitting
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

void drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      theMatrix.setPixelColor(y0, x0, color);
    } else {
      theMatrix.setPixelColor(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void drawFastVLine(int16_t x, int16_t y,
				 int16_t h, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void drawFastHLine(int16_t x, int16_t y,
				 int16_t w, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

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

// set all pixels to a "Color" value
void colorAll(uint32_t c) {
  Serial.println("Matrix");
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
  int theLength;
  writeCharStr = "";
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while(strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if((rc != URLPARAM_EOS)) {
      switch(name[0]) {
        case 'l':
           theLength = atoi(value);
           break;
        case 'c':
        for(int i = 0; i<theLength; i++) {
          writeCharStr += (value[i]);
          Serial.write((value[i]));Serial.println();
        }
           break;
      }
    }
  }
  Serial.println(writeCharStr);
  defaultPattern = 7;
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
  colorAll(c);
  delay(wait);
  colorAll(Color(0,0,0));
}

// flash color "c" on x/y/c/u as a rectangle
void alertArea(uint32_t c, int x, int y, int xE, int yE) {
  int w = xE-x;
  int h = yE-y;
  for (int16_t i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, c);
  }
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

  printOk(server);
  colorWipe(c, delay);
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

void cmd_snakemove(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  while (strlen(url_tail)) {
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if ((rc != URLPARAM_EOS)) {
      if (name[0] == 'i') {
        snakeButton = atoi(value);
        Serial.println(snakeButton);
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

void cmd_alertArea(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  int r;
  int g;
  int b;
  int use_hex = 0;
  uint32_t c;
  int x,xE,y,yE;

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
      case 'x':
        x = atoi(value);
        break;
      case 'y':
        y = atoi(value);
        break;
      case 'c':
        xE = atoi(value);
        break;
      case 'u':
        yE = atoi(value);
        break;
      }
    }
  }

  if (use_hex == 0) {
    c = Color(r,g,b);
  }

  alertArea(c, x, xE, y, yE);
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
  Serial1.begin(9600);
  Ethernet.begin(mac,ip);
  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);
  delay(1000); //resetting should fix our issues with not connecting intiially
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
  EthernetBonjour.addServiceRecord(tableNameDLAr,80,MDNSServiceTCP);
  EthernetBonjour.addServiceRecord(tableNameHTTPAr,80,MDNSServiceTCP);
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
  webserver.addCommand("snake", &cmd_snakemove);
  webserver.addCommand("alertArea", &cmd_alertArea);
  webserver.begin();
  Udp.begin(localPort);
  
  /* SNAKE SETUP */
  hrow=sx;//set the row of the snake head
  hcol=sy;//set the col of the snake head
  randomSeed(analogRead(28));//used to help make a better random number
  resetGame();//clear and set the game
  /* END SNAKE SETUP */
  
  theMatrix.begin();
  theMatrix.setCursor(1,1);
  theMatrix.setTextWrap(false);
  
  //theMatrix.setRemapFunction(g2p); //this is causing issues with matrix.write() ? tested it at tinker night and was scrolling fine commented out.
  
  // light blip of light to signal we are ready to listen
  colorAll(Color(0,0,11));
  delay(500);
  colorAll(Color(0,0,0));
  
  Serial.begin(9600);
  Serial.println(Ethernet.localIP());
}

void loop()
{
  if(Serial1.available() > 0) {
    defaultPattern = 9;
  }
  unsigned long t = millis(); // Current elapsed time, milliseconds.
  EthernetBonjour.run();
  // listen for connections
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  int packetSize = Udp.parsePacket();
  if(packetSize) {
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    vu(packetBuffer);
    theMatrix.show();
  }
  
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
  case 4:
    colorAll(Color(0,0,0));
    cursor_x = cursor_x_orig;
    cursor_y = cursor_y_orig;
    break;
  case 5:
    p_cylon(red);
    break;
  case 6:
    if((t - prevFrameTime) >= (1000L / FPS)) { // Handle scrolling
      theMatrix.fillScreen(0);
      theMatrix.setCursor(msgX, 0);
      theMatrix.print(Ethernet.localIP());
      msgLen = String(Ethernet.localIP()).length();
      if(--msgX < (msgLen * -6)) msgX = 15; // We must repeat!
      theMatrix.show();
      prevFrameTime = t;
    }
    break;
  case 7:
    if((t - prevFrameTime) >= (1000L / FPS)) { // Handle scrolling
      theMatrix.fillScreen(0);
      theMatrix.setCursor(msgX, 0);
      theMatrix.print(writeCharStr);
      msgLen = String(writeCharStr).length();
      if(--msgX < (msgLen * -6)) msgX = 15; // We must repeat!
      theMatrix.show();
      prevFrameTime = t;
    }
    break;
  case 8:
    p_cylon(red);
    p_cylon(orange);
    p_cylon(yellow);
    p_cylon(green);
    p_cylon(blue);
    p_cylon(purple);
    break;
  case 9:
    if(Serial1.available() > 0) {
       snakeButton = Serial1.read();
       Serial.println(snakeButton);
    }
    currentMillis = millis();
    if(currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (game&&start&&ignoreNextTimer==false) {
        drawBoard();
	updateGame();
      }
      ignoreNextTimer=false;//resets the ignore bool
    }
    checkKeyboard();
    break;
  }
}

