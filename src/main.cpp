
#define DEB_MOVESNAKE   0
#define DEB_INSERT_CAKE 1


#include <Arduino.h>
#include <FastLED.h>

#include <pindef.h>

#define NUM_LEDS 700
const byte BUT_LEFT  = 1;
const byte BUT_SET   = 2;
const byte BUT_RIGHT = 3;

#define COLOR_BACKGROUND     CHSV(BGHue, BGSatur, BGBright)


extern bool LED0;
extern bool LED1;
extern bool LED2;


CRGB leds[NUM_LEDS];

// for User Input
/* from analog Joystick
  int SensorY = A0;
  int SensorX = A1;
  int YRead = 0;
  int XRead = 0;
  int YRange = 1;
  int XRange = 1;
  int State0 = 0;
  int State1 = 0;
  int State2 = 0; */
int ioTrig = 0;

// Position moved Pixel
int Dot = 0;
int XDot = 1;
int XDotFakt = 6; // >= 4   4=1:1 5=5:4 6=3:2 Speed Snake:Dot

// color Stripe
int BGHue = 2;
int BGBright = 150;
int BGSatur = 255;

// color of moved pixel
int PXColor = 160;
int PXBright = 250;
int PXSatur = 255;

// Snake
#define MIN_SNAKE_DOT_NUMBER  (NUM_LEDS / 10)
#define MAX_SNAKE_DOT_NUMBER  (NUM_LEDS)
#define MAX_SNAKE_LEN         (NUM_LEDS / 11)
#define SNAKE_INC_VALUE       (NUM_LEDS / 100)
int  SnakeDot    = MIN_SNAKE_DOT_NUMBER; // initial start point
byte SnakeLenght = NUM_LEDS / 10 / 3;
int  SnakeDir    = 1;                    // Step with 1 for ging up  or -1 for going down

int PrevSnakeDot = 1;
int SNColor = 96;
int SNBright = 250;
int SNSatur = 255;

// Cake
#define CAKE_GOAL_POS   (NUM_LEDS - (2*(NUM_LEDS / 10)))
int CakeDot[4];
int LastCakeDot = 1;
int CKColor = 55;
int CKBright = 254;
int CKSatur = 255;

byte CakeCnt  = 0;    // Count number of valid cakes in CakePool
int CakeDotPool[3];   // Cakes are inserted in sorted order biggest first
#define MAX_CAKE_POOL_NUM  (sizeof(CakeDotPool) / sizeof(CakeDotPool[0]) )

// CakeMem
int MaxCakeDotPos = 0;
int CakeMem[4];

// Delay  // TODO: Delay by counting millis in loop (was set to zero anyhow)
//int SpeedDelay = 0;

// *******
int  State0 = false;
int  State1 = false;
int  State2 = false;

void input_setup();
void input_loop();


void Sensor() {
  /* from analog Joystick
    YRead = analogRead(SensorY);
    XRead = analogRead(SensorX);
    YRange = map(YRead, 0, 900, 3, 1);
    XRange = map(XRead, 0, 900, 3, 1);*/
//  State0 = digitalRead(inPin);
//  State1 = digitalRead(inPin1);
//  State2 = digitalRead(inPin2);
	State0 = LED0;
	State1 = LED1;
    State2 = LED2;


  if (State0 == true) {
    ioTrig = 1;
  }
  if (State1 == true) {
    ioTrig = 2;
  }
  if (State2 == true) {
    ioTrig = 3;
  }
  if ((State0 == false) & (State1 == false) & (State2 == false)) {
    ioTrig = 0;
  }
}

void sort(int a[], int size) {
  for (int i = 0; i < (size - 1); i++) {
    for (int o = 0; o < (size - (i + 1)); o++) {
      if (a[o] < a[o + 1]) {
        int t = a[o];
        a[o] = a[o + 1];
        a[o + 1] = t;
      }
    }
  }
}

void insertNewCake( int cakePos ) {
  int i;
  if ( CakeCnt < MAX_CAKE_POOL_NUM) {
    CakeDotPool[CakeCnt++] = cakePos;
    if (CakeCnt >= MAX_CAKE_POOL_NUM) {
      CakeCnt = MAX_CAKE_POOL_NUM - 1;
    }
  } else {  // array is full look if new element is greater then last one
    if ( CakeDotPool[MAX_CAKE_POOL_NUM - 1] < cakePos) {
      leds[ CakeDotPool[MAX_CAKE_POOL_NUM - 1]] = CHSV(BGHue, BGSatur, BGBright);
      CakeDotPool[MAX_CAKE_POOL_NUM - 1] = cakePos;
    }
  }
  sort(CakeDotPool, MAX_CAKE_POOL_NUM);
#if DEB_INSERT_CAKE
  Serial.print(CakeCnt);
  Serial.print("== ");
  Serial.println(cakePos);
  for(i=0; i<MAX_CAKE_POOL_NUM; i++) {
    Serial.print(i);
    Serial.print(", ");
    Serial.println(CakeDotPool[i]);
  }
#endif
}

void MovePlayerDot() {
	static uint32_t PrevMillis = 0;
  switch (ioTrig) {
    case BUT_LEFT:
      if (XDot > 3) XDot--;
      Dot = abs(XDot * 4 / XDotFakt);
      leds[Dot] = CHSV(PXColor, PXSatur, PXBright);
      //delay(SpeedDelay);
      break;

    case BUT_SET:
      if (LastCakeDot > 3) {
        LastCakeDot = 1;
      }
      if ((millis() - PrevMillis) > 500) {
        insertNewCake(Dot);
        leds[CakeDot[LastCakeDot]] = CHSV(BGHue, BGSatur, BGBright);
        CakeDot[LastCakeDot] = Dot;
        PrevMillis = millis();
        LastCakeDot = LastCakeDot + 1;
      }
      break;

    case BUT_RIGHT:
      if (XDot < (NUM_LEDS * XDotFakt / 4)) XDot = XDot + 1;
      Dot = abs(XDot * 4 / XDotFakt);
      leds[Dot] = CHSV(PXColor, PXSatur, PXBright);
//      delay(SpeedDelay);
      break;
  }
}


void MoveSnake() {

#if DEB_MOVESNAKE
  Serial.print(SnakeDot);
  Serial.print(F(", "));
  Serial.print(SnakeDir);
  Serial.print(F("-- "));
#endif
#if 1
  PrevSnakeDot = SnakeDot;
  SnakeDot += SnakeDir;

  if ( SnakeDir > 0 ) {  // positiv == is going up
    if ( SnakeDot < (MAX_SNAKE_DOT_NUMBER) ) {
      ; // All is well boarder not reached
    } else {
      SnakeDir = -SnakeDir;
      SnakeDot = (MAX_SNAKE_DOT_NUMBER - 1) - SnakeLenght;
    }
  }
  else {    // negative  == going down
    if ( SnakeDot <= MIN_SNAKE_DOT_NUMBER ) {
      SnakeDir = -SnakeDir;
      SnakeDot = MIN_SNAKE_DOT_NUMBER + SnakeLenght;
    }
  }
  leds[SnakeDot] = CHSV(SNColor, SNSatur, SNBright);
  leds[SnakeDot - (SnakeLenght * SnakeDir)] = CHSV(BGHue, BGSatur, BGBright);
  
#if DEB_MOVESNAKE
  Serial.print(SnakeDot);
  Serial.print(F(", "));
  Serial.print(SnakeDir);
  Serial.print(F(", "));
  Serial.println(SnakeDot - (SnakeLenght * SnakeDir));
#endif

#else
  if ((PrevSnakeDot < SnakeDot) && (SnakeDot < (NUM_LEDS))) {
    PrevSnakeDot = SnakeDot;
    SnakeDot++;
    if (SnakeDot > (NUM_LEDS - 1))
    { PrevSnakeDot = NUM_LEDS + 1;
    }
    leds[SnakeDot] = CHSV(SNColor, SNSatur, SNBright);
    leds[SnakeDot - SnakeLenght] = CHSV(BGHue, BGSatur, BGBright);
  }
  if ((PrevSnakeDot > SnakeDot) & (SnakeDot > (NUM_LEDS / 10)) ) {
    PrevSnakeDot = SnakeDot;
    SnakeDot--;
    if (SnakeDot < ((NUM_LEDS / 10) + 2)) {
      PrevSnakeDot = (SnakeDot - 1);
    }
    leds[SnakeDot] = CHSV(SNColor, SNSatur, SNBright);
    if (SnakeDot < (NUM_LEDS - SnakeLenght)) {
      leds[SnakeDot + SnakeLenght] = CHSV(BGHue, BGSatur, BGBright);
    }
  }
#endif
}

void MovePx() {
  leds[Dot] = CHSV(PXColor, PXSatur, PXBright);
  leds[Dot - 1] = CHSV(PXColor, PXSatur, PXBright);
  leds[Dot + 1] = CHSV(PXColor, PXSatur, PXBright);
  leds[Dot - 2] = CHSV(BGHue, BGSatur, BGBright);
  leds[Dot + 2] = CHSV(BGHue, BGSatur, BGBright);
}

void HomeRun() {
  // reset Cakes
  for (int i = 1; i < 4; i++) {
    leds[CakeDot[i]] = CHSV(BGHue, BGSatur, BGBright);
    CakeDot[i] = 0;
  }
  // Blinky Blinky

  for (int d = Dot; d > 3  ; d = d - 3) {
    leds[d] = CHSV(PXColor, PXSatur, PXBright);
    leds[d + 1] = CHSV(PXColor, PXSatur, PXBright);
    leds[d - 1] = CHSV(PXColor, PXSatur, PXBright);
    MoveSnake();
    FastLED.show();
    leds[d] = CHSV(BGHue, BGSatur, BGBright);
    leds[d + 1] = CHSV(BGHue, BGSatur, BGBright);
    leds[d - 1] = CHSV(BGHue, BGSatur, BGBright);
  }
  Dot = 2;
  XDot = 2;
}
void Collision() {
  for (int i = 1; i < 3; i++) {
    for (int Col = 1; Col < 14; Col++) {
      leds[(SnakeDot + 2  + Col)] = CHSV((PXColor + (10 * Col)), PXSatur, PXBright);
      leds[(SnakeDot + 2  + (abs(Col / 3)))] = CHSV((PXColor + (10 * Col)), PXSatur, PXBright);
      leds[(SnakeDot + 2  + (abs(Col / 4)))] = CHSV((PXColor + (10 * Col)), PXSatur, PXBright);
      leds[SnakeDot] = CHSV(PXColor, PXSatur, PXBright);
      FastLED.show();
      leds[(SnakeDot + 2  + Col)] = CHSV(SNColor, SNSatur, SNBright);
      leds[(SnakeDot + 2  + (abs(Col / 3)))] = CHSV(SNColor, SNSatur, SNBright);
      leds[(SnakeDot + 2  + (abs(Col / 4)))] = CHSV(SNColor, SNSatur, SNBright);
      FastLED.show();
      leds[(SnakeDot + 2  + Col)] = CHSV(BGHue, BGSatur, BGBright);
      leds[(SnakeDot + 2  + (abs(Col / 3)))] = CHSV(BGHue, BGSatur, BGBright);
      leds[(SnakeDot + 2  + (abs(Col / 4)))] = CHSV(BGHue, BGSatur, BGBright);
    }
  }
  //PrevSnakeDot = SnakeDot - 3;
  SnakeDir = -SnakeDir;
  HomeRun();
}



void SetCake() {
  for (int i = 1; i < 4; i++) {
    leds[CakeDot[i]] = CHSV(CKColor, CKSatur, CKBright);
  }
}


void SortCake() {
#if 1
  MaxCakeDotPos = 0;
  for (byte i = 1; i < 4 ; i++) {
    MaxCakeDotPos = max(MaxCakeDotPos, CakeDot[i] );
  }
#else
  if ((CakeDot[3] >= CakeDot[2]) & (CakeDot[3] >= CakeDot[1])) {
    CakeMem[3] = CakeDot[3];
    CakeMem[2] = CakeDot[2];
    CakeMem[1] = CakeDot[1];
  }
  if ((CakeDot[2] >= CakeDot[3]) & (CakeDot[2] >= CakeDot[1])) {
    CakeMem[3] = CakeDot[2];
    CakeMem[2] = CakeDot[3];
    CakeMem[1] = CakeDot[1];
  }
  if ((CakeDot[1] >= CakeDot[3]) & (CakeDot[1] >= CakeDot[2])) {
    CakeMem[3] = CakeDot[1];
    CakeMem[2] = CakeDot[3];
    CakeMem[1] = CakeDot[2];
  }
#endif
}

void SnakeCake() {
  //PrevSnakeDot = SnakeDot - 1;
  SnakeDir = -SnakeDir;
  CakeMem[3] = 0;
  MaxCakeDotPos = 0;
  for (int i = 1; i < 4; i++) {
    CakeDot[i] = CakeMem[i];      // TODO:
    LastCakeDot = 3;
  }
}

void Goal() {
  int Dot2 = Dot;
  for (Dot; Dot < NUM_LEDS; Dot++) {
    leds[Dot - 2] = CHSV(SNColor, SNSatur, SNBright);
    leds[Dot] = CHSV(PXColor, PXSatur, PXBright);
    FastLED.show();
  }

   //for (int Dot = NUM_LEDS; Dot > 3; Dot--) {
   // leds[Dot] = CHSV(PXColor, PXSatur, PXBright);
    //leds[Dot-1] = CHSV(PXColor, PXSatur, PXBright);
   // for (int Dot2 = 0; Dot2 <= NUM_LEDS; Dot+2) {
    //leds[Dot2] = CHSV(BGHue, BGSatur, BGBright);
    //FastLED.show();
 // }
    
  //}
  //FastLED.show();
  //for (int Dot = 0; Dot <= NUM_LEDS; Dot++) {
  //  leds[Dot] = CHSV(BGHue, BGSatur, BGBright);
  //  FastLED.show();
  //}
  SnakeLenght = NUM_LEDS / 10 / 3;
  //PrevSnakeDot = SnakeDot - 3;
  SnakeDir = -SnakeDir;
  HomeRun();

}



void MoniPrint() {
//  int time = millis();
  Serial.print(CakeDot[0]);
  Serial.print(F("   "));
  Serial.print(CakeDot[1]);
  Serial.print(F("    "));
  Serial.print(CakeDot[2]);
  Serial.print(F("    "));
  Serial.print(CakeDot[3]);
  Serial.print(F("   "));
  Serial.print(SnakeDot);
  Serial.print(F("   "));
  Serial.print(CakeMem[3]);
  Serial.print(F("   "));
  Serial.print(CakeMem[2]);
  Serial.print(F("   "));
  Serial.print(CakeMem[1]);
  Serial.print(F("   "));
  Serial.println(Dot);
}





void setup() {
  Serial.begin(74880);		// 74880 is boot loader baud rate of ESP8266, so use it for app, too.
  Serial.println("FLN_LineSnaker ESP 0.0.1");
  Serial.flush();

  // LEDS
  //FastLED.setMaxPowerInMilliWatts(7000);
  Serial.printf("Add %d leds to WS2812-leds for line snaker on pin %d\n", NUM_LEDS, PIN_SNAKE_LED);
  FastLED.addLeds<NEOPIXEL, PIN_SNAKE_LED>(leds, NUM_LEDS);

  // Init Delay
  //delay(2000);
  input_setup();

  // Fill with BG Background
  Serial.println("Start background animation");
  for (int Dot = 0; Dot < NUM_LEDS; Dot++) {
    leds[Dot] = CHSV(BGHue, BGSatur, BGBright);
    //leds[Dot+1] = CHSV(BGHue, BGSatur, BGBright);
    //FastLED.show();  // don't animate because it will trigger WDT
  }
  FastLED.show();
  Serial.println("Background animation finished");
  // Reset Dot Position
  Dot = 2;

  // Set Cake Dots and CakeMem
  for (int i = 0; i < 4; i++) {
    CakeDot[i] = 0;
    CakeMem[i] = 0;
    LastCakeDot = 1;
  }
  Serial.println("setup finished()");
  Serial.flush();
}

void loop() {
	static uint32_t last = 0;
	static uint32_t skip_count = 0;
	static uint32_t loop_count = 0;
	if (millis() - last > 50) {
		last = millis();
		loop_count++;
	} else {
		skip_count++;
		return;			// run only once per 50ms
	}


  Sensor();

  MovePlayerDot();

  MoveSnake();

  MovePx();

  //  Collision Snake Dot
  if (((Dot == SnakeDot) or ((Dot - 1) == SnakeDot)) && (Dot > (NUM_LEDS / 10))) {
    Collision();
  }

  SetCake();
  SortCake();

  //  Collison Goal
  if ( (MaxCakeDotPos > CAKE_GOAL_POS) && (MaxCakeDotPos == SnakeDot) ) {
    Goal();
  }

  // Collision Snake Cake
  if (MaxCakeDotPos == SnakeDot) {
    SnakeCake();
#if 0
    SnakeLenght += SNAKE_INC_VALUE;
    if (SnakeLenght > MAX_SNAKE_LEN ) {
      SnakeLenght = MAX_SNAKE_LEN;
    }
#endif
    SnakeLenght = min(MAX_SNAKE_LEN, SnakeLenght + SNAKE_INC_VALUE );
  }



  // Hide everything set to zero
  leds[0] = COLOR_BACKGROUND;

  leds[CAKE_GOAL_POS] = CHSV(PXColor+30, PXSatur, PXBright);
  leds[NUM_LEDS / 10] = CHSV(PXColor+30, PXSatur, PXBright);

  input_loop();
  FastLED.show();
  //MoniPrint();

  if (loop_count % 20 == 0) {			// Once per seconds
	  Serial.printf("%06ld: Loops: %d / Skipped: %d [%.1f %%]\n", millis(), loop_count, skip_count, loop_count / (loop_count + skip_count) * 100);
  }
}


