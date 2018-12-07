#include <USBComposite.h>
#include <WS2812B.h>

char *manufacture = "bongorian";
char *product = "flickclick";

//define pins
#define LEDPIN PA7

#define JOY1A PA1
#define JOY1B PA2
#define JOY4A PB0
#define JOY4B PB1

#define MODE1 PC14
#define MODE2 PC15
#define ENT_SW PA0

#define PWMPIN PA8

#define MU 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define UPLEFT 5
#define UPRIGHT 6
#define DOWNLEFT 7
#define DOWNRIGHT 8
#define LEFTRORTATE 9
#define RIGHTRORTATE 10

const uint8_t rows[4] = {PA15, PB3, PB4, PB5};
const int rowCount = sizeof(rows) / sizeof(rows[0]);
const uint8_t cols[4] = {PB6, PB7, PB8, PB9};
const int colCount = sizeof(cols) / sizeof(cols[0]);
byte curkeys[rowCount][colCount];
byte oldkeys[rowCount][colCount];
unsigned int islongpresskeys[rowCount][colCount];

//define numbers
#define NUM_LEDS 4
const uint8_t notes[4][4] = {{47, 48, 49, 50}, {42, 43, 44, 45}, {37, 38, 39, 40}, {32, 33, 34, 35}};
const int numNotes = sizeof(notes) / sizeof(*notes);

//define hensuu
byte curmode;
byte oldmode;
unsigned int vertical, holizonal, sub1, sub2; //方向,ねじりの検出
byte switches[2] = {0, 0};

//instances
USBMIDI midi;
WS2812B strip = WS2812B(NUM_LEDS);

void setup()
{
  Serial.begin(115200);
  for (int x = 0; x < rowCount; x++)
  {
    pinMode(rows[x], OUTPUT);
    digitalWrite(rows[x], HIGH);
  }
  for (int x = 0; x < colCount; x++)
  {
    pinMode(cols[x], INPUT_PULLUP);
  }
  pinMode(JOY1A, INPUT);
  pinMode(JOY1B, INPUT);
  pinMode(JOY4A, INPUT);
  pinMode(JOY4B, INPUT);
  pinMode(MODE1, INPUT);
  pinMode(MODE2, INPUT);
  //midi init
  USBComposite.setProductId(0x0075);
  USBComposite.setManufacturerString(manufacture);
  USBComposite.setProductString(product);
  midi.begin();
  //led init
  strip.begin();
  strip.show();
  rainbowCycle(20);
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.setPixelColor(1, strip.Color(0, 0, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 0));
  strip.setPixelColor(3, strip.Color(0, 0, 0));
  delay(1000);
}

void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85)
  {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else
  {
    if (WheelPos < 170)
    {
      WheelPos -= 85;
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else
    {
      WheelPos -= 170;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
  }
}

void checkSwitch()
{
  switches[0] = digitalRead(MODE1);
  switches[1] = digitalRead(MODE2);
}

void checkFlick()
{
  vertical = analogRead(JOY1A);
  holizonal = analogRead(JOY1B);
  sub1 = analogRead(JOY4A);
  sub2 = analogRead(JOY4B);
  if (vertical < 500)
  {
    setMode(UP);
  }
  else if (vertical > 4000)
  {
    setMode(DOWN);
  }
  else if (holizonal > 4000)
  {
    setMode(LEFT);
  }
  else if (holizonal < 500)
  {
    setMode(RIGHT);
  }
  else
  {
    setMode(MU);
  }
}

void setMode(byte status)
{
  oldmode = curmode;
  curmode = status;
  setLed(curmode);
}

bool checkModechange()
{
  if (oldmode == curmode)
  {
    return false;
  }
  else
  {
    return true;
  }
}

void setLed(byte mode)
{
  switch (mode)
  {
  case MU:
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
    break;
  case UP:
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    break;
  case DOWN:
    strip.setPixelColor(0, strip.Color(0, 255, 255));
    strip.show();
    break;
  case LEFT:
    strip.setPixelColor(0, strip.Color(128, 0, 255));
    strip.show();
    break;
  case RIGHT:
    strip.setPixelColor(0, strip.Color(128, 255, 0));
    strip.show();
    break;
  case UPLEFT:
    strip.setPixelColor(0, strip.Color(255, 0, 192));
    strip.show();
    break;
  case UPRIGHT:
    strip.setPixelColor(0, strip.Color(255, 192, 0));
    strip.show();
    break;
  case DOWNLEFT:
    strip.setPixelColor(0, strip.Color(0, 64, 255));
    strip.show();
    break;
  case DOWNRIGHT:
    strip.setPixelColor(0, strip.Color(0, 255, 64));
    strip.show();
    break;
  case LEFTRORTATE:
    strip.setPixelColor(0, strip.Color(0, 150, 0));
    strip.show();
    break;
  case RIGHTRORTATE:
    strip.setPixelColor(0, strip.Color(0, 150, 0));
    strip.show();
    break;
  }
}

void readMatrix()
{
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
  {
    byte curRow = rows[rowIndex];
    digitalWrite(curRow, LOW);
    for (int colIndex = 0; colIndex < colCount; colIndex++)
    {
      byte curCol = cols[colIndex];
      oldkeys[rowIndex][colIndex] = curkeys[rowIndex][colIndex]; //過去の状態格納
      curkeys[rowIndex][colIndex] = !digitalRead(curCol);
      if ((oldkeys[rowIndex][colIndex] == 1) && (curkeys[rowIndex][colIndex] == 1))
      {
        islongpresskeys[rowIndex][colIndex]++;
      }
      else
      {
        islongpresskeys[rowIndex][colIndex] = 0;
      }
    }
    digitalWrite(curRow, HIGH);
  }
}

void printMatrix()
{
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
  {
    if (rowIndex < 10)
      Serial.print(F("0"));
    Serial.print(rowIndex);
    Serial.print(F(": "));
    for (int colIndex = 0; colIndex < colCount; colIndex++)
    {
      Serial.print(curkeys[rowIndex][colIndex]);
      if (colIndex < colCount)
        Serial.print(F(", "));
    }
    Serial.println("");
  }
  Serial.println("");
}

void setNote(byte mode)
{
  int shift = mode * 8 - 28;
  switch (mode)
  {
  case LEFT:
    isFletactive(shift);
    break;
  case RIGHT:
    isFletactive(shift);
    break;
  case UP:
  case DOWN:
  case MU:
    isFletactive(0);
  }
}

void isFletactive(int shift)
{
  for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
  {
    for (int colIndex = 0; colIndex < colCount; colIndex++)
    {
      if ((curkeys[rowIndex][colIndex] == 1) && (islongpresskeys[rowIndex][colIndex] == 0))
      {
        midi.sendNoteOn(0, notes[rowIndex][colIndex] + shift, 127);
      }
      else if ((curkeys[rowIndex][colIndex] == 1) && (islongpresskeys[rowIndex][colIndex] != 0))
      {
      }
      else
      {
        midi.sendNoteOff(0, notes[rowIndex][colIndex] + shift, 127);
      }
    }
  }
}

void AlloldNoteOff(byte mode)
{
  int oldshift = mode * 8 - 28;
  if (mode == MU)
  {
    oldshift = 0;
  }
  else if (mode)
    for (int rowIndex = 0; rowIndex < rowCount; rowIndex++)
    {
      for (int colIndex = 0; colIndex < colCount; colIndex++)
      {
        midi.sendNoteOff(0, notes[rowIndex][colIndex] + oldshift, 127);
      }
    }
}

void loop()
{
  checkFlick();
  readMatrix();
  if (checkModechange())
  {
    AlloldNoteOff(oldmode);
  }
  setNote(curmode);
}
