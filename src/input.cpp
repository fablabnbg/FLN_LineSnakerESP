#include <FastLED.h>

#include <pindef.h>

#define NUM_LEDS 6
#define DATA_PIN 13
CRGB input_leds[NUM_LEDS];

#define DEB_IO   0
/*
  HC-SR04 Ping duration sensor:
*/

  #define GOOD CHSV(GOODHue, Satur, Bright)
  #define BAD  CHSV(BADHue, Satur, Bright)

  int maximumRange = 500; // Maximum range needed
  int minimumRange = 0;   // Minimum range needed
  long duration0;         // Duration used to calculate duration
  long duration1;         // Duration used to calculate duration
  long duration2;         // Duration used to calculate duration
  int outVal0 = -1;
  int outVal1 = -1;
  int outVal2 = -1;
  bool LED0 = false;
  bool LED1 = false;
  bool LED2 = false;
  int GOODHue = 80;
  int BADHue = 2;
  int Satur = 254;
  int Bright = 240;

void input_setup() {
	pinMode(PIN_INPUT_TRIG_0, OUTPUT);
	pinMode(PIN_INPUT_ECHO_0, INPUT);
	pinMode(PIN_INPUT_TRIG_1, OUTPUT);
	pinMode(PIN_INPUT_ECHO_1, INPUT);
	pinMode(PIN_INPUT_TRIG_2, OUTPUT);
	pinMode(PIN_INPUT_ECHO_2, INPUT);
	Serial.printf("Add %d leds to WS2812-leds for input feedback on pin %d\n", NUM_LEDS, PIN_INPUT_LED);
	FastLED.addLeds<NEOPIXEL, PIN_INPUT_LED>(input_leds, NUM_LEDS);
}
 

void input_loop() {
	// blocks for less than 2ms (1836µs + runtime)
	digitalWrite(PIN_INPUT_TRIG_0, LOW);
	delayMicroseconds(2);
	digitalWrite(PIN_INPUT_TRIG_0, HIGH);
	delayMicroseconds(10);
	digitalWrite(PIN_INPUT_TRIG_0, LOW);
	duration0 = pulseIn(PIN_INPUT_ECHO_0, HIGH, maximumRange + 100); // Limit wait time to maximumRange + 100µs

	digitalWrite(PIN_INPUT_TRIG_1, LOW);
	delayMicroseconds(2);
	digitalWrite(PIN_INPUT_TRIG_1, HIGH);
	delayMicroseconds(10);
	digitalWrite(PIN_INPUT_TRIG_1, LOW);
	duration1 = pulseIn(PIN_INPUT_ECHO_1, HIGH, maximumRange + 100);

	digitalWrite(PIN_INPUT_TRIG_2, LOW);
	delayMicroseconds(2);
	digitalWrite(PIN_INPUT_TRIG_2, HIGH);
	delayMicroseconds(10);
	digitalWrite(PIN_INPUT_TRIG_2, LOW);
	duration0 = pulseIn(PIN_INPUT_ECHO_2, HIGH, maximumRange + 100);

	if (duration0 >= maximumRange || duration0 <= minimumRange) {
		outVal0 = 1;
		//digitalWrite(OUT_PIN_0, LOW); FIXME: Set here flag for main loop
		LED0 = 0;
		input_leds[4] = BAD;
		input_leds[5] = BAD;
	} else {
		outVal0 = duration0;
		if ((outVal1 == 1) & (outVal2 == 1)) {
			//digitalWrite(OUT_PIN_0, HIGH); FIXME: Set here flag for main loop
			LED0 = true;
			input_leds[4] = GOOD;
			input_leds[5] = GOOD;
		}
	}

	if (duration1 >= maximumRange || duration1 <= minimumRange) {
		outVal1 = 1;
		//  digitalWrite(OUT_PIN_1, LOW); FIXME: Set here flag for main loop
		LED1 = 0;
		input_leds[2] = BAD;
		input_leds[3] = BAD;
	} else {
		outVal1 = duration1;
		if ((outVal0 == 1) & (outVal2 == 1)) {
			//    digitalWrite(OUT_PIN_1, HIGH); FIXME: Set here flag for main loop
			LED1 = true;
			input_leds[2] = GOOD;
			input_leds[3] = GOOD;
		}
	}

	if (duration2 >= maximumRange || duration2 <= minimumRange) {
		outVal2 = 1;
		//  digitalWrite(OUT_PIN_2, LOW); FIXME: Set here flag for main loop
		LED2 = 0;
		input_leds[0] = BAD;
		input_leds[1] = BAD;
	} else {
		outVal2 = duration2;
		if ((outVal1 == 1) & (outVal0 == 1)) {
			//    digitalWrite(OUT_PIN_2, HIGH); FIXME: Set here flag for main loop
			LED2 = true;
			input_leds[0] = GOOD;
			input_leds[1] = GOOD;
		}
	}

	Serial.print(outVal0);
	Serial.print("   ");
	Serial.print(outVal1);
	Serial.print("   ");
	Serial.println(outVal2);
}
