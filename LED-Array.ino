// main script for LED array yay
// "line" refers to the set of 80 LEDs connected to on data pin
// "strip" refers to one of the 10-long strands

// import libraries
#include <FastLED.h>
#include <LiquidCrystal.h>   
#include <Bounce2.h>  

// define constants and variables
// declare LED constants (size, data pins)
#define NUM_LEDS_PER_STRIP  10     
#define NUM_STRIPS_PER_LINE 8     
#define NUM_LINES 6
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE * NUM_LINES
#define DATA_PIN_0 3
#define DATA_PIN_1 5
#define DATA_PIN_2 6
#define DATA_PIN_3 9
#define DATA_PIN_4 10
#define DATA_PIN_5 11

CRGB leds[NUM_LEDS];  // define single giant LED matrix

// declare button selector constants and variables
// declare constants
const int incButtonPin = 2;   // button consts
const int decButtonPin = 4;   
const int resetButtonPin = 7;   

const int ledUpdateRate = 16; // time between LED updates (ms) (~60 fps)

String patternNames[] = {"nothing lmao",              // 0
                         "red const",                 // 1
                         "Alternating RGB",           // 2
                         "100% rainbow",              // 3
                         "wave of LGBTGIFLMAOKFC"};   // 4

const int rs = 12;            // LCD consts
const int en = 13;
const int d4 = A5, d5 = A4, d6 = A3, d7 = A2;  

// declare variables
Bounce2::Button incButton = Bounce2::Button();    // create object for inc button
Bounce2::Button decButton = Bounce2::Button();    // create object for dec button
Bounce2::Button resetButton = Bounce2::Button();  // create object for reset button

unsigned int patternNumber = 0;       // current pattern num
unsigned long lastPatternUpdate = 0;  // time (ms) since last frame of animation

unsigned int cyclePosition = 0;       // the frame number within a moving pattern

unsigned int frameCount    = 0;       // the number of intervals that a frame lasting longer than the pattern update cycle has been held for

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

  resetButton.attach(resetButtonPin, INPUT);  // specify button pin
  resetButton.interval(5);                  // define debounce period (ms)
  resetButton.setPressedState(HIGH);        // define HIGH as corresponding to the presed state

  // tell FastLED the type of LEDs, data structure, and number for the 6 data lines
  FastLED.addLeds<NEOPIXEL, DATA_PIN_0>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 0
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 1
  FastLED.addLeds<NEOPIXEL, DATA_PIN_2>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 2
  FastLED.addLeds<NEOPIXEL, DATA_PIN_3>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 3
  FastLED.addLeds<NEOPIXEL, DATA_PIN_4>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 4
  FastLED.addLeds<NEOPIXEL, DATA_PIN_5>(leds, NUM_LEDS_PER_STRIP * NUM_STRIPS_PER_LINE);    // line 5
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
int locate_pixel(int x,int y,int z){
  // x refers to line num
  int index = x*NUM_LEDS_PER_STRIP*NUM_STRIPS_PER_LINE + y*NUM_LEDS_PER_STRIP + z;
  return index;

};


// use button inputs to change patterns  
void update_pattern() {       
  incButton.update();  decButton.update();  resetButton.update(); // update button states

  if (incButton.pressed()) {  // inc button rising edge
    patternNumber = patternNumber + 1;  // increase number
    if (patternNumber > (sizeof(patternNames) / sizeof(patternNames[0])) - 1) {  // loop the number back to zero if the last pattern was reached
      patternNumber = 0;
    }
    display_pattern(patternNumber);   // show current pattern info on LCD
    reset_frame_position();
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
  }
  else if (resetButton.pressed()) {  // reset button
    patternNumber = 0;  // reset number
    display_pattern(patternNumber); // show current pattern info on LCD
    reset_frame_position();
  }
}


// diplay specified pattern name and number on LCD    
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
    leds[i] = CRGB::Red;
  }
}

// pattern 2 - All leds cycle thru all R, G, B discontinuously
void pattern_2(){  
  // takes (global) counters to determine the frame within the cycle and the duration that the current frame has been held for
  switch (cyclePosition){   // set led colors based on the current frame
    case 0:   // red
      for (int i = 0; i < NUM_LEDS; i++) {    // TODO: avoid having the loop written for each frame since it's all uniform
        leds[i] = CRGB::Red;
      }
      break;

    case 1:   // green
      for (int i = 0; i < NUM_LEDS; i++) {    // TODO: avoid having the loop written for each frame since it's all uniform
        leds[i] = CRGB::Green;
      }
      break;

    case 2:   // blue
      for (int i = 0; i < NUM_LEDS; i++) {    // TODO: avoid having the loop written for each frame since it's all uniform
        leds[i] = CRGB::Blue;
      }
      break;
  }

  increment_counters_longer_interval(3, 30);  // update counters
}

// pattern 3 - All leds rainbow shift
void pattern_3(){
  CRGB currentColor = ColorFromPalette(RainbowColors_p, cyclePosition, 60); // get current color from built-in rainbow palette

  for (int i = 0; i < NUM_LEDS; i++) {  // fill all leds with current color
    leds[i] = currentColor;
  }

  increment_counters(256);  // update counter
}

// pattern 4 - rainbow wave left-to-right
void pattern_4(){

}


