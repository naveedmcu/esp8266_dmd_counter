/*
Server SoftAP with 2x2 dot matrix 64*32
Esp8266 base code

 */

// P10 LED pin     = esp8266 pins
// a               = d0
// b               = d6
// clk             = D5
// sck             = D3
// r               = D7
// noe             = D8
// gnd             = gnd
// SERVER
// Receiving string $123;
#include <ESP8266WiFi.h>
#include <elapsedMillis.h>
#include <SPI.h>
#include <DMD2.h>
#include <Ticker.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/SystemFont5x7v2.h>
#include <fonts/Font5x16.h>
#include <fonts/Arial14.h>
// #include "fonts/Arial_black_16.h"
// #include "Arial_black_16.h"
#include "fonts/Arial_Black_16.h"
#include "fonts/Arial_Black_20.h"
#include "fonts/Arial_Narrow_20.h"
#include "fonts/Droid_Sans_12.h"
#include "fonts/Droid_Sans_16.h"
#include "fonts/Droid_Sans_24.h"
#include "fonts/digit_32.h"
#include "fonts/digit_bolt_36.h"
#include "fonts/Droid_Sans_36.h"
#include "fonts/Droid_Sans_64.h"
#include "fonts/Droid_Sans_96.h"

#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
#define LED_ON 0
#define LED_OFF 1

#define SBUFF 500
#define MAX_WORLD_COUNT 50
#define MIN_WORLD_COUNT 1
elapsedSeconds delay_Sec;
struct sr_t
{
  char raw[SBUFF] = {0};
  int16_t ind;
  bool done;
};
sr_t sr;
int onetime = 1;
char msg[SBUFF];
byte sms_block = 0;
char *Words[MAX_WORLD_COUNT];
byte word_count;
const int COUNTDOWN_FROM = 100200;
int counter = COUNTDOWN_FROM;
/*Ticker*/

Ticker scheduledTicker;
const uint8_t *FONT = Arial14;
// const uint8_t *FONT = SystemFont5x7;

SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
// DMD_TextBox box(dmd, -4, 0, 32, 16); // "box" provides a text box to automatically write to/scroll the display
// DMD_TextBox box(dmd); // "box" provides a text box to automatically write to/scroll the display
DMD_TextBox box(dmd, 0, 0); // "box" provides a text box to automatically write to/scroll the display
const char *MESSAGE = "abcdefghijklmnopqrstuvwxyz";
// the setup routine runs once when you press reset:
int currentVal = 0;
int previousVal = 1;
void ScreenUpdate();
void numberDisplay(unsigned int uiTime);
void readSerialData(uint8_t c);
byte split_message(char *str);
void print_message(byte word_count);
void ShowClockNumbers(unsigned int uiTime, byte bColonOn);
/**/
void DisplayScanning()
{
  dmd.scanDisplay();
}
/**/
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  /**/
  /*END*/
  /*THIS IS FOR DMD*/
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  // dmd.beginNoTimer();
  dmd.begin();
}

// the loop routine runs over and over again forever:
void loop()
{
  // dmd.selectFont(Arial_Black_16_ISO_8859_1);
  // dmd.selectFont(Arial_Black_16);
  // dmd.selectFont(Arial_Black_20);
  // dmd.selectFont(Droid_Sans_12);
  // dmd.selectFont(Droid_Sans_12);
  // dmd.selectFont(Droid_Sans_16);
  // dmd.selectFont(Droid_Sans_24);
  // dmd.selectFont(Droid_Sans_36);
  // dmd.selectFont(digit_bolt_36);
  // dmd.selectFont(Arial14);
  // dmd.selectFont(Arial14);
  // dmd.selectFont(Font5x7);
  dmd.selectFont(Arial_Black_20);
  // dmd.selectFont(Arial_Narrow_20);
  // dmd.selectFont(heattenschweiler);
  dmd.drawString(0, 0, "===");
  while (1)
  {
    if (Serial.available())
    {
      readSerialData(Serial.read());
    }
    /**/
    if (onetime == 2)
    {
      if (delay_Sec >= 2)
      {
        delay_Sec = 0;
        if (word_count >= MIN_WORLD_COUNT)
        {
          print_message(word_count);
          // ShowClockNumbers(1234, true);
        }
      }
    }
    /**/
    if (sr.done)
    {
      onetime = 2;
      dmd.setBrightness(255);
      // dmd.selectFont(Arial_Black_16);
      Serial.println(sr.raw);         // String
      Serial.println(strlen(sr.raw)); // String lenght
      memcpy(msg, sr.raw, strlen(sr.raw) + 1);
      word_count = split_message(msg);
      sms_block = 0;
      sr.done = false;
    }
  }
}

void print_message(byte word_count)
{
  if (sms_block < word_count)
  {
    dmd.clearScreen();
    dmd.drawString(0, 0, Words[sms_block]);
    sms_block++;
  }
  else
  {
    sms_block = 0;
  }
}
void ShowClockNumbers(unsigned int uiTime, byte bColonOn)
{
  dmd.drawChar(0, 0, '0' + ((uiTime % 10000) / 1000), GRAPHICS_ON); // thousands
  dmd.drawChar(7, 0, '0' + ((uiTime % 1000) / 100), GRAPHICS_ON);   // hundreds
  dmd.drawChar(17, 0, '0' + ((uiTime % 100) / 10), GRAPHICS_ON);    // tens
  dmd.drawChar(25, 0, '0' + (uiTime % 10), GRAPHICS_ON);            // units
  if (bColonOn)
    dmd.drawChar(15, 0, ':', GRAPHICS_OR); // clock colon overlay on
  else
    dmd.drawChar(15, 0, ':', GRAPHICS_NOR); // clock colon overlay off
}

byte split_message(char *str)
{
  byte word_count = 0;
  char *item;
  item = (char *)malloc(500 * sizeof(char));
  item = strtok(str, " ,"); // getting first word (uses space & comma as delimeter)

  while (item != NULL)
  {
    if (word_count >= MAX_WORLD_COUNT)
    {
      break;
    }
    Words[word_count] = item;
    item = strtok(NULL, " ,"); // getting subsequence word
    word_count++;
  }
  return word_count;
}
void readSerialData(uint8_t c)
{
  static bool incomming = false;
  char character = (char)c;
  if (incomming)
  {
    sr.raw[sr.ind] = character;
    sr.raw[sr.ind + 1] = 0;
    sr.ind++;
    if (sr.ind == SBUFF)
    {
      sr.ind = 0;
      sr.raw[sr.ind] = 0;
    }
    if (character == ';')
    {
      sr.raw[sr.ind - 1] = 0;
      sr.raw[sr.ind] = 0;
      sr.ind = 0;
      sr.done = true;
      incomming = false;
    }
  }
  if (character == '$')
  {
    sr.ind = 0;
    sr.raw[sr.ind] = 0;
    incomming = true;
  }
}

void numberDisplay(unsigned int uiTime)
{
  dmd.clearScreen();
  dmd.drawChar(0, 0, '0' + ((uiTime % 10000) / 1000), GRAPHICS_ON);   // thousands
  dmd.drawChar(16, 0, '0' + ((uiTime % 1000) / 100), GRAPHICS_ON);    // hundreds
  dmd.drawChar(16 + 16, 0, '0' + ((uiTime % 100) / 10), GRAPHICS_ON); // tens
  dmd.drawChar(16 + 16 + 16, 0, '0' + (uiTime % 10), GRAPHICS_ON);    // units
}