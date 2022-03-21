// Advanced microscope led ring light
//
// Set IN and OUT pins for your hardware:
//
#define LED_PIN 3
#define LED_COUNT 24
#define ANALOG_INPUT_RGB A3
#define ANALOG_INPUT_WHITE A4
#define ANALOG_INPUT_LED_POS A5
#define ANALOG_INPUT_LED_SHOW A2

// ## No changes under this line ##


#include <Adafruit_NeoPixel.h>
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);

int white = 255; 
int crgb = 255;

int show_led = LED_COUNT;
int pos_led = 0;

uint8_t cache_crgb;
uint8_t cache_white;
uint8_t cache_lpos; 
uint8_t cache_lcount;


// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings4brightness = 20;
const int numReadings4position = 5; 

// the buffer for the analog input
int readings_RGB[numReadings4brightness];      
int readings_WHITE[numReadings4brightness];
int readings_LEDC[numReadings4position];
int readings_LEDP[numReadings4position];


int index4brightness = 0;                  // the index of the current reading
int index4position = 0;                  // the index of the current reading

int total_RGB = 0;                  // the running total
int total_WHITE = 0;                  // the running total
int total_LEDC = 0;
int total_LEDP = 0;

int average_RGB = 0;                // the average
int average_WHITE = 0;                // the average
int average_LEDC = 0;
int average_LEDP = 0;

int inputPin_RGB = ANALOG_INPUT_RGB;
int inputPin_WHITE = ANALOG_INPUT_WHITE;
int inputPin_LEDC = ANALOG_INPUT_LED_SHOW;
int inputPin_LEDP = ANALOG_INPUT_LED_POS; 


void setup()
{
  uint16_t i;
  // initialize serial communication with computer:
  Serial.begin(9600);                   
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings4brightness; thisReading++) {
    readings_RGB[thisReading] = 0;
    readings_WHITE[thisReading] = 0;     
    readings_LEDC[thisReading] = 0;
    readings_LEDP[thisReading] = 0;
  }

  for (int thisReading = 0; thisReading < numReadings4position; thisReading++) {
    readings_RGB[thisReading] = 0;
    readings_WHITE[thisReading] = 0;     
    readings_LEDC[thisReading] = 0;
    readings_LEDP[thisReading] = 0;
  }

  strip.begin();
  strip.setBrightness(255);
  strip.show();      
}


// start with geting values from analog input, calculate it trough a average buffer
void calculateAvarage() {

  // subtract the last reading:
  total_RGB = total_RGB - readings_RGB[index4brightness];         
  total_WHITE = total_WHITE - readings_WHITE[index4brightness];
  total_LEDP = total_LEDP - readings_LEDP[index4position];
  total_LEDC = total_LEDC - readings_LEDC[index4position];
  
  // read from the sensor:  
  readings_RGB[index4brightness] = analogRead(inputPin_RGB);
  readings_WHITE[index4brightness] = analogRead(inputPin_WHITE);
  readings_LEDP[index4position] = analogRead(inputPin_LEDP);
  readings_LEDC[index4position] = analogRead(inputPin_LEDC);
   
  // add the reading to the total:
  total_RGB = total_RGB + readings_RGB[index4brightness];
  total_WHITE = total_WHITE + readings_WHITE[index4brightness];
  total_LEDP = total_LEDP + readings_LEDP[index4position];
  total_LEDC = total_LEDC + readings_LEDC[index4position];
         
  // advance to the next position in the array:  
  index4brightness = index4brightness + 1;                    
  index4position = index4position + 1;
  
  // if we're at the end of the array....wrap around to the beginning: 
  if (index4brightness >= numReadings4brightness) {
    index4brightness = 0;                                             
  }

  if (index4position >= numReadings4position) {
    index4position = 0;                                             
  }  

  // calculate the average:
  average_RGB = total_RGB / numReadings4brightness;     
  average_WHITE = total_WHITE / numReadings4brightness;
  average_LEDP = total_LEDP / numReadings4position;
  average_LEDC = total_LEDC / numReadings4position;
}



void setPixel(uint8_t crgb, uint8_t w, uint8_t lpos, uint8_t lcount) {
  uint16_t i;

  // deactivate all LEDs
  for ( i=0; i <= LED_COUNT-1; i++) {
     strip.setPixelColor(i, 0, 0, 0, 0);  
  }

  if ( lcount == 0 ) {
     strip.show();
     return;    
  }
  
  // set reading postion and length (count)
  for ( i=0; i<=lcount-1; i++) {   
    if( lpos+i <= LED_COUNT-1 ) {
      // ledOnOrOff[lpos+i] = true;
      strip.setPixelColor(lpos+i, crgb, crgb, crgb, w);
    } else {
      //ledOnOrOff[lpos+i-LED_COUNT] = true;
      strip.setPixelColor(lpos+i-LED_COUNT, crgb, crgb, crgb, w);
    }
  }
  
  strip.show();
  
}


void showDebugOutput() {
  Serial.print("CRGB: ");
  Serial.print(average_RGB);
  Serial.print(" WHITE: ");
  Serial.print(average_WHITE);
  Serial.print(" LED POS: ");   
  Serial.print(average_LEDP);
  Serial.print(" LED COUNT: ");
  Serial.print(average_LEDC);  
  Serial.print(" LED POS: ");
  Serial.print(pos_led);
  Serial.print(" LED SHOW: ");
  Serial.print(show_led);
  Serial.println();
}


bool hasValuesChanged(uint8_t crgb, uint8_t white, uint8_t lpos, uint8_t lcount) {
  bool hasChanged = false;
  
  if( crgb != cache_crgb) hasChanged = true;
  if( white != cache_white) hasChanged = true;
  if( lpos != cache_lpos) hasChanged = true; 
  if( lcount != cache_lcount) hasChanged = true;

  if(hasChanged){
    cache_crgb = crgb;
    cache_white = white;
    cache_lpos = lpos;
    cache_lcount = lcount;
    return true;
  } else {
    return false;  
  }
  
}


void loop() {

  calculateAvarage();
   
  // map analog readings, beautiful smoothed by hard and software to usefull range (led brightness = 0..255, pos&count = 0..LED_COUNT)  
  crgb = map(average_RGB, 0, 1200, 0, 255); 
  white = map(average_WHITE, 0, 1200, 0, 255); 
  pos_led = map(average_LEDP, 0, 1000, 0, LED_COUNT-1);
  show_led = map(average_LEDC, 0, 1000, 0, LED_COUNT);

  // caching values to avoid flickering
  if ( hasValuesChanged(crgb, white, pos_led, show_led) ) {  
    setPixel(crgb, white, pos_led, show_led); 
   // showDebugOutput();
  }
  
  delay(10);        // delay in between reads for stability            
}
