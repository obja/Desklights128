#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define PIN 6
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

//default pattern on startup, 0 = off, 6 = current IP address
int defaultPattern = 0;

String inputString = "";
boolean stringComplete = false;

//set grid size here
uint16_t max_x = 30;
uint16_t max_y = 8;

Adafruit_NeoMatrix theMatrix = Adafruit_NeoMatrix(30, 8, PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

//Matrix Scrolling
unsigned long prevFrameTime = 0L;             // For animation timing
#define FPS 10                                // Scrolling speed
uint8_t       msgLen        = 0;              // Empty message
int           msgX          = 16; // Start off right edge
String writeCharStr = "";

int aax,aay,aayE,aaxE;
uint32_t aac;

// --------------------------------------- START FUNCTIONS ----------------------------------- //
int antiDelay(unsigned long nowTime, int delayTime) {
  unsigned long arduinoTime = millis();
  //Serial.print(F("antidelay, arduinoTime: ")); Serial.print(arduinoTime); Serial.print(F(", nowTime: ")); Serial.print(nowTime); Serial.print(F(", delayTime: ")); Serial.println(delayTime);
  while (arduinoTime - nowTime < delayTime) {
    arduinoTime = millis();
    if(Serial1.available()) {
      defaultPattern = 0;
      Serial.println("antidel returning");
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

//cylon colors
uint32_t red[6] = {  Color(255, 0, 0),  Color(200, 0, 0),  Color(150, 0, 0),  Color(100, 0, 0),  Color(50, 0, 0),  Color(0, 0, 0) };
uint32_t orange[6] = {  Color(255, 128, 0),  Color(200, 100, 0),  Color(150, 75, 0),  Color(100, 50, 0),  Color(50, 25, 0),  Color(0, 0, 0) };
uint32_t yellow[6] = {  Color(255, 255, 0),  Color(200, 200, 0),  Color(150, 150, 0),  Color(100, 100, 0),  Color(50, 50, 0),  Color(0, 0, 0) };
uint32_t green[6] = {  Color(0, 255, 0),  Color(0, 200, 0),  Color(0, 150, 0),  Color(0, 100, 0),  Color(0, 50, 0),  Color(0, 0, 0) };
uint32_t blue[6] = {  Color(0, 0, 255),  Color(0, 0, 200),  Color(0, 0, 150),  Color(0, 0, 100),  Color(0, 0, 50),  Color(0, 0, 0) };
uint32_t purple[6] = {  Color(127, 0, 255),  Color(100, 0, 200),  Color(75, 0, 150),  Color(50, 0, 100),  Color(25, 0, 50),  Color(0, 0, 0) };
//end cylon colors

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
int p_rainbow() {
  int i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < theMatrix.numPixels(); i++) {
      theMatrix.setPixelColor(i, Wheel( ((i * 256 / theMatrix.numPixels()) + j) & 255) );
    }
    theMatrix.show();
    if(Serial1.available()) {
      defaultPattern = 0;
      return 0;
    }
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

void fetchData(int *inData) {
  int x, y, r, g, b, xE, yE, use_hex = 0;
  int s, reply = 1;
  int delayTime = 100;
  int id = -1;
  uint32_t c = theMatrix.Color(0, 0, 0);


  while(inputString != "") {
    /*Serial.println("Fetching data");
    char data = Serial1.read();
    Serial.print("Data: ");Serial.println(data);
    while(!Serial1.available()) {;}
    Serial.println("Reading value");
    Serial1.readStringUntil('='); //read '='
    while(!Serial1.available()) {;}
    Serial.print("Value: ");
    String value = Serial1.readStringUntil('\n');
    value.trim();
    Serial.println(value);*/

    if(inputString.length() < 2) {
      Serial.print("break, ");Serial.println(inputString);
      inputString = "";
      break;
    }

    char data = inputString.charAt(0);
    inputString.remove(0,inputString.indexOf('='));
    String value = inputString.substring(1,inputString.indexOf('\r'));
    value.trim();
    inputString.remove(0,inputString.indexOf('\r')+1);
    //Serial.print("Datastr: ");Serial.println(inputString);Serial.println("done");
    
    Serial.print("Data: ");Serial.println(data);
    Serial.print("Value: ");Serial.println(value);
  
    switch (data) {
      case 'i':
        id = value.toInt();
        break;
      case 'x':
        x = value.toInt();
        break;
      case 'y':
        y = value.toInt();
        break;
      case 'h':
        char test[6];
        value.toCharArray(test, value.length());
        c = hexColor(test);
        use_hex = 1;
        break;
      case 'r':
        r = value.toInt();
        break;
      case 'g':
        g = value.toInt();
        break;
      case 'b':
        b = value.toInt();
        break;
      case 's':
        s = value.toInt();
        break;
      case 'c':
        xE = value.toInt();
        break;
      case 'u':
        yE = value.toInt();
        break;
      case 'z':
        reply = value.toInt();
        break;
      case 'd':
        delayTime = value.toInt();
        break;
      case 't':
        writeCharStr = value;
        break;
      case 'v':
        vu(String(value));
        theMatrix.show();
        defaultPattern = 0;
        break;
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

void setup() {
  Serial.begin(38400); //debug printing
  Serial1.begin(38400); //wifi module
  delay(5000);
  Serial1.end();
  delay(100);
  Serial1.begin(38400);
  theMatrix.begin();
  theMatrix.show();
  inputString.reserve(200);
}

void loop() {
  String incoming = "";
  String command = "";
  int inData[10];

  /*if(stringComplete) {
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
  }*/
  
  if(stringComplete) {//(Serial1.available() > 6) {
    /*int i = 0;
    int availData = Serial1.parseInt();
    Serial.print("Incoming data size: ");Serial.println(availData);
    Serial.print("Current data size: ");Serial.println(Serial1.available());
    while(Serial1.available() < availData) {
      if(i>3) {
        Serial1.end();
        delay(100);
        Serial1.begin(38400);
        return;
      }
      delay(250);
      Serial.print(i);Serial.print(", Current data size: ");Serial.println(Serial1.available());
      i++;
    }
    //Serial.print(Serial1.available());
    Serial.println("reading data");
    Serial1.setTimeout(2000);
    while(!Serial1.available()) {;}
    incoming = Serial1.readStringUntil('\n');
    while(!Serial1.available()) {;}
    incoming = Serial1.readStringUntil('\n');
    Serial.print("Got data: ");Serial.println(incoming);
    incoming.remove(0,1);
    command = incoming.substring(0, incoming.indexOf("\r\n"));
    command.trim();
    Serial.print("Command: ");Serial.println(command);
    fetchData(inData);
    Serial.print("after fetchData, X: ");Serial.println(inData[2]);
    Serial.print("Current data size: ");Serial.println(Serial1.available());
    */
    Serial.print("Data: ");Serial.print(inputString);Serial.println("done");
    command = inputString.substring(1,inputString.indexOf('\r'));
    command.trim();
    inputString.remove(0,inputString.indexOf('\r')+1);
    Serial.println(command);
    //Serial.print("Data: ");Serial.println(inputString);Serial.println("done");
    fetchData(inData);
    
    if(command == "pixel") {
      inputString = "";
      stringComplete = false;
      theMatrix.setPixelColor(inData[0], inData[1]);
      theMatrix.show();
      defaultPattern = 0;
    }
    else if(command == "off") {
      inputString = "";
      stringComplete = false;
      colorAll(theMatrix.Color(0,0,0));
      theMatrix.setCursor(1,1);
    }
    else if(command == "show") {
      inputString = "";
      stringComplete = false;
      defaultPattern = 0;
      theMatrix.show();
    }
    else if(command == "wipe") {
      inputString = "";
      stringComplete = false;
      colorWipe(inData[1], inData[6]);
    }
    else if(command == "color") {
      inputString = "";
      stringComplete = false;
      colorAll(inData[1]);
    }
    else if(command == "alert") {
      inputString = "";
      stringComplete = false;
      alert(inData[1], inData[6]);
    }
    else if(command == "default") {
      inputString = "";
      stringComplete = false;
      defaultPattern = inData[7];
      colorAllDef(theMatrix.Color(0,0,0));
    }
    else if(command == "write") {
      inputString = "";
      stringComplete = false;
      theMatrix.setTextColor(inData[1]);
      defaultPattern = 7;
    }
    else if(command == "test") {
      inputString = "";
      stringComplete = false;
      if(inData[7] == 0) {
        lightTest(inData[6]);
      }
      else if(inData[7] == 1) {
        gridTest(inData[6]);
      }
    }
    else if(command == "alertArea") {
      inputString = "";
      stringComplete = false;
      aax = inData[2];
      aay = inData[3];
      aaxE = inData[4];
      aayE = inData[5];
      aac = inData[1];
      defaultPattern = 10;
    }
    Serial.println("Finished data");
    inputString = "";
    stringComplete = false;
  }
  
  unsigned long t = millis(); // Current elapsed time, milliseconds.
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
    colorAll(theMatrix.Color(0,0,0));
    theMatrix.setCursor(1, 1);
    break;
  case 5:
    p_cylon(red);
    break;
  case 6:
    if((t - prevFrameTime) >= (1000L / FPS)) { // Handle scrolling
      theMatrix.fillScreen(0);
      theMatrix.setCursor(msgX, 0);
      theMatrix.print("");//Ethernet.localIP());
      msgLen = String("").length();//Ethernet.localIP()).length();
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
  case 10:
      for(int p=0;p<200;p++) {
        alertArea(aac, aax, aay, aaxE, aayE);
        aac--;
        delay(5);
      }
      for(int p=0;p<200;p++) {
        alertArea(aac, aax, aay, aaxE, aayE);
        aac++;
        delay(5);
      }
    break;
  }
}

void serialEvent1() {
  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read();
    Serial.print("Processing data, ");Serial.println(inputString);
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    /*if (inChar == '\r') {
      inputString += '\n';
    }*/
    if(inputString.indexOf("rstrstrstrst") > 0) {
      Serial.println("Restarting...");
      delay(1000);
      CPU_RESTART
    }
    if (inChar == '\n') {
      stringComplete = true;
      Serial.println("Processed data");
    }
  }
}
