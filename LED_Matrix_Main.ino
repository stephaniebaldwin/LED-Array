// main script for LED array yay
// "line" refers to the set of 80 LEDs connected to on data pin
// "strip" is more general and can also refer to one of the 10-long strands

// import libraries
#include <FastLED.h>
#include <LiquidCrystal.h>    

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

const int debounceDelay = 25; // debounce timing for button inputs (ms)
const int ledUpdateRate = 16; // time between LED updates (ms) (~60 fps)

String patternNames[] = {"nothing lmao",              // 0
                         "red const",                 // 1
                         "Alternating RGB",           // 2
                         "100% rainbow"               // 3
                         "wave of LGBTGIFLMAOKFC"};   // 4

const int rs = 12;            // LCD consts
const int en = 13;
const int d4 = A5, d5 = A4, d6 = A3, d7 = A2;  

// declare variables
int incButtonState = 0;          // current (steady) state of the inc button
int incLastButtonState = 0;      // previous (steady) state of the inc button
int decButtonState = 0;          // current (steady) state of the dec button
int decLastButtonState = 0;      // previous (steady) state of the dec button
int resetButtonState = 0;        // current (steady) state of the reset button
int resetLastButtonState = 0;    // previous (steady) state of the reset button

int incLastFlickerableState = 0;          // raw state (for debounce)
unsigned long incLastDebounceTime = 0;    // the last time the output pin was toggled
int decLastFlickerableState = 0;          // raw state (for debounce)
unsigned long decLastDebounceTime = 0;    // the last time the output pin was toggled
int resetLastFlickerableState = 0;        // raw state (for debounce)
unsigned long resetLastDebounceTime = 0;  // the last time the output pin was toggled

int patternNumber = 0;      // current pattern num
unsigned long lastPatternUpdate = 0;  // time (ms) since last frame of animation

int cyclePosition = 0;                // the frame number within a moving pattern

// TODO: this probably shouldn't be a global variable, look into removing if SRAM availability becomes an issue
int frameCount    = 0;                // the number of intervals that a frame lasting longer than the pattern update cycle has been held for

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // declare LCD


void setup() {
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.print("Starting...");

  delay(3000); // power-up safety delay   TODO: do i really need this lol

  // show initial pattern info on LCD
  display_pattern(0);

  // initialize button pins as inputs
  pinMode(incButtonPin, INPUT);
  pinMode(decButtonPin, INPUT);
  pinMode(resetButtonPin, INPUT);

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
  
  // update the leds if enough time has passed for ~60 fps
  if ((currentTime - lastPatternUpdate) >= ledUpdateRate) {
    lastPatternUpdate = currentTime;

    switch (patternNumber) {  // update LED values based on the currently selected pattern
      case 0:
        pattern_0();
        break;
      case 1:
        pattern_1();
        break;
      case 2:
        pattern_2();
        break;
      case 3:
        pattern_3();
        break;
    }
  }
  FastLED.show();   // push updates to led strips
  

  // use button inputs to change patterns         TODO: double-check that this still works properly when the replacemet board comes in
  // read current button states
  incButtonState = digitalRead(incButtonPin);
  decButtonState = digitalRead(decButtonPin); 
  resetButtonState = digitalRead(resetButtonPin);

  // compare to previous state
  if (incButtonState != incLastButtonState) {   // debounce logic for inc button
    incLastDebounceTime = millis();             // reset debouncing timer
    incLastFlickerableState = incButtonState;   // update the last flickerable state
  }
  if ((millis() - incLastDebounceTime) > debounceDelay) {   // do things only if debounce period has passed
    incButtonState = incLastFlickerableState;
    if (incButtonState != incLastButtonState) {
      incLastButtonState = incButtonState;  // update last steady state
      if (incButtonState == HIGH) {   // check if rising edge
        patternNumber = patternNumber + 1;  // increase number
        if (patternNumber > (sizeof(patternNames) / sizeof(patternNames[0])) - 1) {  // loop the number back to zero if the last pattern was reached
          patternNumber = 0;
        }
        display_pattern(patternNumber);   // show current pattern info on LCD
      } 
    }
  }  

  if (decButtonState != decLastButtonState) {   // debounce logic for dec button
    decLastDebounceTime = millis();             // reset debouncing timer
    decLastFlickerableState = decButtonState;   // update the last flickerable state
  }
  if ((millis() - decLastDebounceTime) > debounceDelay) {   // do things only if debounce period has passed
    decButtonState = decLastFlickerableState;
    if (decButtonState != decLastButtonState) {
      decLastButtonState = decButtonState;  // update last steady state
      if (decButtonState == HIGH) { // rising edge
        if (patternNumber > 0) {  // decrease number if doing so will not result in a negative number
          patternNumber = patternNumber - 1;
        }
        else {
          patternNumber = (sizeof(patternNames) / sizeof(patternNames[0])) - 1;  // loop back to largest pattern index
        }
        display_pattern(patternNumber);   // show current pattern info on LCD
      } 
    }
  }

  if (resetButtonState != resetLastButtonState) {   // debounce logic for reset button
    resetLastDebounceTime = millis();             // reset debouncing timer
    resetLastFlickerableState = resetButtonState;   // update the last flickerable state
  }
  if ((millis() - resetLastDebounceTime) > debounceDelay) {   // do things only if debounce period has passed
    resetButtonState = resetLastFlickerableState;
    if (resetButtonState != resetLastButtonState) {
      resetLastButtonState = resetButtonState;  // update last steady state
      if (resetButtonState == HIGH) {   // rising edge
        patternNumber = 0;  // reset number
        display_pattern(patternNumber); // show current pattern info on LCD
      } 
    }
  }

}


// converts pixel location coords to a position on a list to place in giant leds structure
int locate_pixel(int x,int y,int z){
  // x refers to line num
  int index = x*NUM_LEDS_PER_STRIP*NUM_STRIPS_PER_LINE + y*NUM_LEDS_PER_STRIP + z;
  return index;

};


// diplay specified pattern name and number on LCD    
void display_pattern(int pattern_num){
  //assumes that LCD and patternNames structure are global
  lcd.clear();
  lcd.print("Pattern ");    // print pattern num to top row
  lcd.print(pattern_num);
  lcd.setCursor(0, 1);     // move cursor to beginning of line 1 (the bottom line)

  if (patternNames[pattern_num].length() > 16) {   // do fancy autoscrolling stuff to make long name fit
    lcd.print("add autoscrolling");
  }
  else {
    lcd.print(patternNames[pattern_num]);
  }
}


// increment counters or reset them if the end of the pattern has been reached
void increment_counters(int totalFrames, int cyclesPerFrame = 1){
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

  increment_counters(3, 30);  // update counters
}

// pattern 3 - All leds rainbow shift
void pattern_3(){
  CRGB currentColor = ColorFromPalette(RainbowColors_p, (cyclePosition * 2), 120); // get current color from built-in rainbow palette

  for (int i = 0; i < NUM_LEDS; i++) {  // fill all leds with current color
    leds[i] = currentColor;
  }

  increment_counters(127);  // update counters

}

// pattern 4 - rainbow wave left-to-right
void pattern_4(){

}


