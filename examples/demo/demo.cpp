#include "Particle.h"
#include "LPD6803.h"
#include "math.h"

/*****************************************************************************/
// Original code by Bliptronics.com Ben Moyes 2009
// Use this as you wish, but please give credit, or at least buy some of my LEDs!

// Code cleaned up and Object-ified by ladyada, should be a bit easier to use

// Adapted for Spark Core by Paul Kourany, Dec. 2014

// Adapted for Particle Photon by Grant Sellers, Sep. 2018
/*****************************************************************************/

// Number of LPD6803 drivers in the strip to be controlled
#define NUM_LEDS 130

// SPI pins on the Particle Photon
#define DATA_PIN D2
#define CLOCK_PIN D4

// Particle handle functions for controlling lights remotely
int setMode(String);
int setColor(String);
int setSpeed(String);
int setPulseWidth(String);

// State functions, for how LEDs light up
void resetLights();
void solidLights();
void pulseLights();
void fadeLights();
void rainbowLights();

// Color utility functions
unsigned int Color(byte,byte,byte);
unsigned int Wheel(byte);
unsigned int InterpolateColors(unsigned int , unsigned int , byte );

// State values for the modeof LED patterns
enum state {RAINBOW, PULSE, FADE, SOLID};

// Default global variables for the state of the controller.
enum state ledState = SOLID;
unsigned int color = Color(0,31,0);
int speed = 100;
int pulseWidth = 40;

// Global index values for counting through rounds of patterns
int i_led = 0;
int j_led = 0;
int k_led = 0;

// Initialize LED strip
LPD6803 strip = LPD6803(NUM_LEDS, DATA_PIN, CLOCK_PIN);

void setup(){

  //buffer for the color string
  char buffer[10];

  // Makes functions and variables visible to Particle Photon board.
  Particle.function("setMode",setMode);
  Particle.function("setColor",setColor);
  Particle.function("setSpeed",setSpeed);
  Particle.variable("mode",ledState);
  Particle.variable("color",sprintf(buffer,"%X",color));
  Particle.variable("speed",speed);

  // Set the CPU
  strip.setCPUmax(50);

  // My LED strip has a problem where the first 50 swap the g and b colors
  // Seems like a hardware issue, but its easily fixed in software with this fix.
  strip.fixColorIssue(47);

  // Start and run the LEDs
  strip.begin();

  // Call if you want to flash white before your default setting, since at this point,
  // no data has been sent.
  // strip.show();
}

void loop() {
  // Simple state machine for controlling lights, since you don't want
  // Particle.function calls to stall the board
  switch (ledState){
    case RAINBOW:
      rainbowLights();
      break;
    case SOLID:
      solidLights();
      break;
    case PULSE:
      pulseLights();
      break;
    case FADE:
      fadeLights();
      break;
  }
  i_led++;
  j_led++;
  k_led++;
  // Used to tell when pulses have reached the end of the line
  if (i_led>strip.numPixels()){
    i_led=0;
  }
  // Used to tell when the rainbow cycle should end
  if (j_led>96){
    j_led=0;
  }
  // Used to tell when the fade cycle should end
  if (k_led>64){
    k_led = 0;
  }

  // Update lights and wait for the next loop.
  strip.show();
  delay(speed);
}

int setMode(String str)
{
  resetLights();

  // Parse the string into the different modes
  if (str=="pulse"){
    ledState = PULSE;
  } else if (str=="solid"){
    ledState = SOLID;
  } else if (str == "fade"){
    ledState = FADE;
  } else if (str == "rainbow"){
    ledState = RAINBOW;
  } else{
    return 0;
  }
  return 1;
}

int setColor(String str){

 // Pars ethe string into various colors.
 if (str=="r"){
   color = Color(0,0,31);
 } else if (str=="g"){
   color = Color(0,31,0);
 } else if (str=="b"){
   color = Color(31,0,0);
 } else if (str == "y"){
   color = Color(0,31,31);
 } else if (str == "c"){
   color = Color(31,31,0);
 } else if (str == "p"){
   color = Color(31,0,31);
 } else if (str=="w"){
   color = Color(31,31,31);
 } else if (str=="k"){
   color = Color(0,0,0);
 }
 return 0;
}

int setSpeed(String str){
  // Expect an integer string, so do a simple conversion
  speed = atoi(str);
  return 1;
}

int setPulseWidth(String str){
  // Expect an integer string, so do a simple conversion
  pulseWidth = atoi(str);
  return 1;
}

void resetLights(){
  // Reset counter so mode starts fresh at the beginning of the cycle.
  i_led=0;
  j_led=0;
  k_led=0;

  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
}
void solidLights(){
  // Turn all the pixels in the strip to the same color
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
}

void pulseLights(){
  // Send a scanning pulse around the LED strip that cycles through.

  // Find where the trailing end should be
  int i_off = i_led - pulseWidth;
  if (i_off<0){
    i_off += strip.numPixels();
  }

  // Turn on the head of the LEDs, turn off the tail
  strip.setPixelColor(i_led,color);
  strip.setPixelColor(i_off,Color(0,0,0));
}


void fadeLights(){
  int newColor = InterpolateColors(color,0,32-abs(32-k_led%64));
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, newColor);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void rainbowLights() {
  for (int i=0; i < strip.numPixels(); i++) {
    // tricky math! we use each pixel as a fraction of the full 96-color wheel
    // (thats the i / strip.numPixels() part)
    // Then add in j which makes the colors go around per pixel
    // the % 96 is to make the wheel cycle around
    strip.setPixelColor(i, Wheel( ((i * 96 / strip.numPixels()) + j_led) % 96) );
  }
}

/* Helper functions */

// Create a 15 bit color value from R,G,B
unsigned int Color(byte r, byte g, byte b)
{
  //Take the lowest 5 bits of each value and append them end to end
  return( ((unsigned int)g & 0x1F )<<10 | ((unsigned int)b & 0x1F)<<5 | (unsigned int)r & 0x1F);
}

//Input a value 0 to 127 to get a color value.
//The colours are a transition r - g -b - back to r
unsigned int Wheel(byte WheelPos)
{
  byte r,g,b;
  switch(WheelPos >> 5)
  {
    case 0:
      r=31- WheelPos % 32;   //Red down
      g=WheelPos % 32;      // Green up
      b=0;                  //blue off
      break;
    case 1:
      g=31- WheelPos % 32;  //green down
      b=WheelPos % 32;      //blue up
      r=0;                  //red off
      break;
    case 2:
      b=31- WheelPos % 32;  //blue down
      r=WheelPos % 32;      //red up
      g=0;                  //green off
      break;
  }
  return(Color(r,g,b));
}

unsigned int InterpolateColors(unsigned int color1, unsigned int color2, byte pos)
{
  byte r1,g1,b1,r2,g2,b2;
  r1 = (byte)(color1       ) & 0x1F;
  g1 = (byte)(color1 >> 5  ) & 0x1F;
  b1 = (byte)(color1 >> 10 ) & 0x1F;
  r2 = (byte)(color2       ) & 0x1F;
  g2 = (byte)(color2 >> 5  ) & 0x1F;
  b2 = (byte)(color2 >> 10 ) & 0x1F;
  return Color(r1-(r1-r2)*pos/32,g1-(g1-g2)*pos/32,b1-(b1-b2)*pos/32);
}
