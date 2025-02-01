// main script for LED array yay
// "line" refers to the set of 80 LEDs connected to on data pin
// "strip" refers to one of the 10-long strands

// import libraries
#include <FastLED.h>
#include <LiquidCrystal.h>   
#include <Bounce2.h>  

// define constants and variables
// declare LED constants (size, data pins)
#define NUM_LEDS_PER_STRIP  10     // z
#define NUM_STRIPS_PER_LINE 8      // y
#define NUM_LINES 5                // x
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE * NUM_LINES
#define DATA_PIN_0 3
#define DATA_PIN_1 5
#define DATA_PIN_2 6
#define DATA_PIN_3 9
#define DATA_PIN_4 10

CRGB leds[NUM_LEDS];  // define single giant LED matrix

// declare button selector constants and variables
// declare constants
const int incButtonPin = 2;   // button consts
const int decButtonPin = 4;   
const int brightnessButtonPin = 7;   

const int ledUpdateRate = 16; // time between LED updates (ms) (~60 fps)

String patternNames[] = {"nothing lmao",                        // 0
                         "red const",                           // 1
                         "Alternating RGB",                     // 2
                         "100% rainbow",                        // 3
                         "wave of rainbow (left-to-right)",     // 4
                         "Raindrops",                           // 5
                         "Rainbow Raindrops",                   // 6
                         "3D Diagonal Waves",                   // 7
                         "8",                                   // 8    original idea sounds like too much of a pain to implement, if u think of anything good put it here
                         "9",                                   // 9
                         "coord test"};

const int rs = 12;            // LCD consts
const int en = 13;
const int d4 = A5, d5 = A4, d6 = A3, d7 = A2;  

// define custom palettes

// define gradient palette (blue for wave, raindrops)
// use https://rgbcolorpicker.com/ to change if it looks ugly
DEFINE_GRADIENT_PALETTE(wavePalette) {  // brightest at index 255, fades to black (lower index => deeper under wave)
  0,   0,   0,   0,     // black
  128, 56,  207, 195,   // some teal thing in the middle idk
  255, 101, 225, 255    // really bright blue ouchie my eyeballs
};


// declare variables
Bounce2::Button incButton = Bounce2::Button();    // create object for inc button
Bounce2::Button decButton = Bounce2::Button();    // create object for dec button
Bounce2::Button brightnessButton = Bounce2::Button();  // create object for brightness inc button

uint8_t       patternNumber = 0;       // current pattern num
unsigned long lastPatternUpdate = 0;  // time (ms) since last frame of animation

unsigned int cyclePosition = 0;       // the frame number within a moving pattern
unsigned int frameCount    = 0;       // the number of intervals that a frame lasting longer than the pattern update cycle has been held for

int brightness = 80;    // pattern brightness (0 - 255)          

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // declare LCD
unsigned int lcdCyclePosition = 0;          // state counter for scrolling text
unsigned int lcdFrameCount = 0;             // to keep track of how long the current state has been held for


void setup() {
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.print("Starting...");

  delay(3000); // power-up safety delay   TODO: do i really need this lol

  // show initial pattern info on LCD
  display_pattern(patternNumber);

  // set up buttons
  incButton.attach(incButtonPin, INPUT);  // specify button pin
  incButton.interval(5);                  // define debounce period (ms)
  incButton.setPressedState(HIGH);        // define HIGH as corresponding to the presed state

  decButton.attach(decButtonPin, INPUT);  // specify button pin
  decButton.interval(5);                  // define debounce period (ms)
  decButton.setPressedState(HIGH);        // define HIGH as corresponding to the presed state

  brightnessButton.attach(brightnessButtonPin, INPUT);  // specify button pin
  brightnessButton.interval(5);                  // define debounce period (ms)
  brightnessButton.setPressedState(HIGH);        // define HIGH as corresponding to the presed state

  // tell FastLED the type of LEDs, data structure, and number for the 6 data lines
  FastLED.addLeds<NEOPIXEL, DATA_PIN_0>(leds, 0,   NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 0 (start at index 0)
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds, 80,  NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 1 (start at index 80)
  FastLED.addLeds<NEOPIXEL, DATA_PIN_2>(leds, 160, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 2 (start at index 160)
  FastLED.addLeds<NEOPIXEL, DATA_PIN_3>(leds, 240, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 3 (start at index 240)
  FastLED.addLeds<NEOPIXEL, DATA_PIN_4>(leds, 320, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 4 (start at index 320)

  Serial.begin(9600);   // set up serial monitor (FOR TESTING ONLY)
}

// want to alternate btwn button stuff and displaying the current pattern
void loop() {
  unsigned long currentTime = millis();        // get current time
  
  // update things if enough time has passed for ~60 fps
  if ((currentTime - lastPatternUpdate) >= ledUpdateRate) {
    lastPatternUpdate = millis();   // update time of the last pattern updaate
    switch (patternNumber) {  // update LED values based on the currently selected pattern
      case 0:
        pattern_0();  break;
      case 1:
        pattern_1();  break;
      case 2:
        pattern_2();  break;
      case 3:
        pattern_3();  break;
      case 4:
        pattern_4();  break;
      case 5:
        pattern_5();  break;
      case 6:
        pattern_6();  break;
      case 7:
        pattern_7();  break;
      case 8:
        pattern_8();  break;
      case 9:
        pattern_9();  break;
      case 10:
        coord_test(); break;
    }
    FastLED.show();   // push updates to led strips

    if (patternNames[patternNumber].length() > 16) {  // scroll LCD periodically if text requires scrolling
      if (lcdFrameCount >= 60) {
        display_pattern(patternNumber);   // show current pattern info on LCD
        lcdFrameCount = 0;  // reset timing counter
      }
      else {
        lcdFrameCount++;
      }
    }
    update_pattern();   // check buttons and change current pattern if needed
  }


}



// converts pixel location coords to a position on a list to place in giant leds structure
// TODO: test this
int locate_pixel(int x,int y,int z){
  // x refers to line num (data lines 0-5, perpendicular to front view)
  // y refers to the position of the strip (horizontal if looking from the front)
  // z refers to the pixel location on the script (vertical going down in front view)
  int index = x*NUM_LEDS_PER_STRIP*NUM_STRIPS_PER_LINE + y*NUM_LEDS_PER_STRIP + z;

  return index;

};

/*
#define NUM_LEDS_PER_STRIP  10     // z  TEMP SO I DONT HAVE TO SCROLL ALL THE WAY UP
#define NUM_STRIPS_PER_LINE 8      // y
#define NUM_LINES 5                // x
*/


// use button inputs to change patterns  
void update_pattern() {       
  incButton.update();  decButton.update();  brightnessButton.update(); // update button states

  if (incButton.pressed()) {  // inc button rising edge
    patternNumber = patternNumber + 1;  // increase number
    if (patternNumber > (sizeof(patternNames) / sizeof(patternNames[0])) - 1) {  // loop the number back to zero if the last pattern was reached
      patternNumber = 0;
    }
    display_pattern(patternNumber);   // show current pattern info on LCD
    reset_frame_position();
    pattern_0();     // clear pixels
  }
  else if (decButton.pressed()) {   // dec button
    if (patternNumber > 0) {  // decrease number if doing so will not result in a negative number
      patternNumber = patternNumber - 1;
    }
    else {
      patternNumber = (sizeof(patternNames) / sizeof(patternNames[0])) - 1;  // loop back to largest pattern index
    }
    display_pattern(patternNumber);   // show current pattern info on LCD
    reset_frame_position();    // reset animations for LEDs and LCD
    pattern_0();     // clear pixels
  }
  else if (brightnessButton.isPressed()) {  // brightness inc button
    brightness += 5;  // increase LED brightness
    if (brightness > 255) {   // reset brightness if max was reached
      brightness = 0;
    }
  }
}


// diplay specified pattern name and number on LCD    
// TODO: LCD text changes to nonsense sometimes
void display_pattern(int pattern_num){
  //assumes that LCD and patternNames structure are global
  String name = patternNames[pattern_num] + "  ";   // get name of selected pattern

  lcd.clear();
  lcd.print("Pattern ");    // print pattern num to top row
  lcd.print(pattern_num);
  lcd.setCursor(0, 1);     // move cursor to beginning of line 1 (the bottom line)

  if (name.length() > 16) {   // do fancy autoscrolling stuff to make long name fit
    if ((lcdCyclePosition + 16) <= (name.length() - 1)){  // if the selected part of the word doesnt clip off the back (no wrapping)
      lcd.print(name.substring(lcdCyclePosition, lcdCyclePosition + 16));
    }
    else {   // wrapping
      lcd.print(name.substring(lcdCyclePosition, name.length()) + name.substring(0, (lcdCyclePosition - (name.length() - 16))));
    }

    // increment lcd counter
    if (lcdCyclePosition > (name.length() - 2)){   // reset cycle when the end of the string is reached
      lcdCyclePosition = 0;
    }
    else {
      lcdCyclePosition++;
    }
  }
  else {
    lcd.print(name);
  }
}


// increment counter or reset if the end of the pattern has been reached
void increment_counters(int totalFrames){
  if (cyclePosition >= (totalFrames - 1)){  // reset cycle if the last frame has been reached
    cyclePosition = 0;
  }
  else {
    cyclePosition++;
  }
}

// increment counters for patterns that hold each state for more than one interval
void increment_counters_longer_interval(int totalFrames, int cyclesPerFrame = 1){
  if (frameCount >= cyclesPerFrame){  // change frame if the current one has been held for long enough
    if (cyclePosition >= (totalFrames - 1)){  // reset cycle if the last frame has been reached
      cyclePosition = 0;
    }
    else {
      cyclePosition++;
    }
    frameCount = 0;   // reset frame duration counter
  }
  else{
    frameCount++;
  }
}

// reset the frame and frame duration counters so patterns always start at their beginning
void reset_frame_position(){
  cyclePosition = 0;
  frameCount = 0;
  lcdCyclePosition = 0;
}

// pattern 0 - Nothing (all off)
void pattern_0(){   // const pattern, no counter 
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

}

// pattern 1 - Constant red for all
void pattern_1(){   // const pattern, no counter 
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(brightness, 0, 0);
  }
}

// pattern 2 - All leds cycle thru all R, G, B discontinuously
void pattern_2(){  
  // takes (global) counters to determine the frame within the cycle and the duration that the current frame has been held for
  CRGB color = CRGB::Black;   // create var to store current color

  switch (cyclePosition){   // choose led color based on the current frame
    case 0:   // red
      color = CRGB(brightness, 0, 0);   break;
    case 1:   // green
      color = CRGB(0, brightness, 0);   break;
    case 2:   // blue
      color = CRGB(0, 0, brightness);   break;
  }

  for (int i = 0; i < NUM_LEDS; i++) {    // apply current color to all LEDs
    leds[i] = color;
  }

  increment_counters_longer_interval(3, 30);  // update counters
}

// pattern 3 - All leds rainbow shift
void pattern_3(){
  CRGB currentColor = ColorFromPalette(RainbowColors_p, cyclePosition, brightness); // get current color from built-in rainbow palette

  for (int i = 0; i < NUM_LEDS; i++) {  // fill all leds with current color
    leds[i] = currentColor;
  }

  increment_counters(256);  // update counter
}

// pattern 4 - rainbow wave left-to-right
void pattern_4(){
  for (int y = 0; y < NUM_STRIPS_PER_LINE; y++) {       // loop thru each column (when looking from the front. Contains 6*10 LEDs)
    // determine current color for the selected "column"
    CRGB color = CRGB::Black;   // variable containing color for the current "column"

    if (cyclePosition + y*32 > 255) {
      color = ColorFromPalette(RainbowColors_p, cyclePosition + y*32 - 256, brightness);
    }
    else {
      color = ColorFromPalette(RainbowColors_p, cyclePosition + y*32, brightness);
    }

    // apply current color to the selected "column"
    for (int x = 0; x < NUM_LINES; x++) {     // loop thru each strip
      for (int z = 0; z < NUM_LEDS_PER_STRIP; z++) {  // loop thru each LED on a strip
        leds[locate_pixel(x,y,z)] = color;
      }
    }
  }
  increment_counters(256);  // update counter
}

// pattern 5 - raindrops
void pattern_5() {
  // idk how to deal with raindrops overlapping thru cycle ends so raindrops will be offset in time, but
  // all of them will finish before more are added with the next cycle
  // aim for this to run @ 60 fps, use lots of gradients so raindrops don't fall at lightspeed lol
  uint8_t numPerSpawnTime = 3;      // how many drops are created in each spawning event
  uint8_t numSpawnTimes = 1;        // how many spawning events are in a cycle
  unsigned int    timeBetweenSpawns = 120;  // # frames (~60 fps) between spawning events    TODO: i just put a number, it's probably wrong

  // at the start of a spawn event:  
  // TODO: this just spawns once at the beginning-ish of a cycle rn
  if (cyclePosition == 10) {

    for (int i = 0; i < numSpawnTimes; i++) {   // spawn however many drops
      // select random xy
      int dropX = random(NUM_LINES - 1);
      int dropY = random(NUM_STRIPS_PER_LINE - 1);    // TODO: add checks that prevent duplicate spawn locations(??)
    }
  }

  increment_counters(numSpawnTimes * timeBetweenSpawns);   // update counter    


}

// draw a droplet and its associated gradient at a given point in time, ADD to pixel set 
void drop_fall(CRGBPalette16 palette, int x, int y, int t) {
  // palette: custom or built-in gradient palette
  // x,y: horizontal location
  // t:   time RELATIVE to drop spawning (t = 0 -> drop first appears)
  // brightness is reduced far ahead and behind droplet "core"

  const float brightnessModifiers[15] = {0.5, 0.8, 0.95, 1, 0.95, 0.9, 0.85, 0.75, 0.65, 0.55, 0.4, 0.3, 0.2, 0.1, 0.0};

  int currentZ = 0;  // the current pixel being worked on
  while (true) {
    if (t - currentZ < 0) {   // don't bother looping thru pixels that are under the drop cus those are obviously off
      break;
    }
    if (t >= currentZ) {
      
    }

    currentZ++;
  }

  // TODO: need to SET the end of the drop to zero

}

// pattern 6 - rainbow raindrops
void pattern_6() {   

}

// pattern 7 - 3D wave
void pattern_7() {
  float closenessThreshold = 0.2;      // how far the pixel location can be from the actual function output for the pixel to be turned on
  int8_t waveValue = 0;                // the "vertical position for a wave". surface corresponds to total depth, fades to black as value decreases
  int8_t waveDepth = 6;                // how far pixels will be lit up (includes black at the very bottom) (includes surface)

  CRGBPalette16 activatedPalette = wavePalette;   // activate palette for this pattern

  // TODO: will probably need to make the wave smoother somehow
  // TODO: scale brightness with position
  // TODO: LOOKS FLAT
  for (int x = 0; x < NUM_LINES; x++) {   // fill colors if a pixel is on or under the wave surface
    for (int y = 0; y < NUM_STRIPS_PER_LINE; y++) {
      for (int z = 0; z < NUM_LEDS_PER_STRIP; z++) {
        // check if wave is close to the current pixel and a wave hasn't already been drawn for the current column
        if ((abs(pattern_7_wave_func(x, y, ((5 * cyclePosition)/ 113)) - z) <= closenessThreshold))  {  // 5 controls spd w/ time
          waveValue = waveDepth;  // update waveValue to surface (max) value
        }
        else if (waveValue > 0) {
          waveValue = waveValue - 1;  // decrease waveValue if currently in a wave
        }

        // draw colors 
        int colorIndex = floor((float(waveValue)/waveDepth)*255);   // choose index based on current depth
        int modifiedBrightness = floor((float(waveValue)/waveDepth)*brightness);   // scale brightness with depth
        leds[locate_pixel(x,y,z)] = ColorFromPalette(activatedPalette, colorIndex, modifiedBrightness);  // light pixel according to the current color index
      }
    }
  }

  increment_counters(710);  // update counters (2pi ~ 710/113)
}

// define wave func that moves diagonally // to xy plane for pattern 7
int pattern_7_wave_func(int x, int y, int t) {   
  int v = 0.5;  // set wave density
  return 2*sin(v*x + v*y - t) + 2;  // compute z value (sin is in radians), A and c are hard-coded
}

// pattern 8 - idk
void pattern_8() {

}

// pattern 9  - idk
void pattern_9() {
  // idk
}

// (unofficial) pattern 10 - coord test
void coord_test() {
  // start all traces @ origin, red inc along x, green inc along y, blue inc along z
  // if one axis reaches the end before the others, simply hold the pixel at the end of that axis
  // TODO: weirdness (things lighting when they're not supposed to)
  
  if (cyclePosition >= 10) {   // clear pixels before the start of each cycle      
    pattern_0();
  }
  else if (cyclePosition > 0) {    
    // fill x-axis:   
    if (cyclePosition > NUM_LINES) {
      leds[locate_pixel(NUM_LINES - 1, NUM_STRIPS_PER_LINE - 1, NUM_LEDS_PER_STRIP - 1)] = CRGB(brightness, 0, 0);  // set last pixel on the axis 
    }
    else {
      leds[locate_pixel(cyclePosition, 0, 0)] += CRGB(brightness, 0, 0);  // set current pixel to red 
    }

    // fill y-axis:
    if (cyclePosition > NUM_STRIPS_PER_LINE) {
      leds[locate_pixel(0, NUM_STRIPS_PER_LINE - 1, 0)] = CRGB(0, brightness, 0);  // set last pixel on the axis
    }
    else {
      leds[locate_pixel(0, cyclePosition, 0)] += CRGB(0, brightness, 0);  // set current pixel to green
      leds[3] = CRGB(0, brightness, 0);  // set current pixel to green
    }

    // fill z-axis:
    if (cyclePosition > NUM_LEDS_PER_STRIP) {
      leds[locate_pixel(0, 0, NUM_LEDS_PER_STRIP - 1)] = CRGB(0, 0, brightness);  // set last pixel on the axis
    }
    else {
      leds[locate_pixel(0, 0, cyclePosition)] += CRGB(0, 0, brightness);  // set current pixel to blue 
    }
  }

  increment_counters_longer_interval(11, 30);   // update counters
  

}


