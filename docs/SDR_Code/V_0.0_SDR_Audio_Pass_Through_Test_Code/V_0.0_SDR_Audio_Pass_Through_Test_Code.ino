//Audio Pass through Test code for SDR, from Charlie Morris, call sign: ZL2CT, additional annotations added by James Bell
#include <Wire.h>                          // I2C comms
#include <si5351.h>                        // Si5351 library
#include <LiquidCrystal_I2C.h>             // LCD library
#include <Audio.h>                         // Teensy audio library

// Define Constants and Vaviables
static const long bandStart = 1000000;     // start of HF band
static const long bandEnd =   30000000;    // end of HF band
static const long bandInit =  3690000;     // where to initially set the frequency
volatile long oldfreq = 0;
volatile long freq = bandInit ;
volatile long radix = 1000;                // how much to change the frequency by, clicking the rotary encoder will change this.
volatile int updatedisplay = 0;

// Rotary Encoder
static const int pushPin = 39;
static const int rotBPin = 36;
static const int rotAPin = 35;
volatile int rotState = 0;
volatile int rotAval = 1;
volatile int rotBval = 1;
volatile int rotAcc = 0;

// Instantiate the Objects
LiquidCrystal_I2C lcd(0x3F, 16, 2);       // Set the LCD address to either 0x27 or 0x3F for a 16 chars and 2 line display (refer to completed_Code_annotated(9/26/2018)
Si5351 si5351;                            // The Si5351 DDS
AudioControlSGTL5000    audioShield;      // The Teensy audio CODEC on the audio shield


// Audio shield
AudioInputI2S           audioInput;                                   // What we call the input to the audio shield
AudioOutputI2S          audioOutput;                                  // What we call the output of the audio shield
AudioConnection         patchCord5(audioInput, 0, audioOutput, 0);    // Left channel in to left channel out
AudioConnection         patchCord10(audioInput, 1, audioOutput, 1);   // Right channel in to right channel out

void setup()
{
  // Setup input switches
  pinMode(rotAPin, INPUT);
  pinMode(rotBPin, INPUT);
  pinMode(pushPin, INPUT);
  digitalWrite(rotAPin, HIGH);
  digitalWrite(rotBPin, HIGH);
  digitalWrite(pushPin, HIGH);

  // Setup interrupt pins
  attachInterrupt(digitalPinToInterrupt(rotAPin), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotBPin), ISRrotBChange, CHANGE);

  // Setup the lcd
  lcd.begin();
  lcd.backlight();

  // Setup the DDS
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.set_freq((freq * 100ULL), SI5351_PLL_FIXED, SI5351_CLK0);

  // Setup the audio shield
  AudioNoInterrupts();
  AudioMemory(16);
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_LINEIN);
  audioShield.volume(0.7);
  audioShield.unmuteLineout();
  AudioInterrupts();

  UpdateDisplay();
}


void loop()
{
  if (freq != oldfreq)
  {
    UpdateDisplay();
    SendFrequency();
    oldfreq = freq;
  }

  if (digitalRead(pushPin) == LOW)
  {
    delay(10);
    while (digitalRead(pushPin) == LOW)
    {
      if (updatedisplay == 1)
      {
        UpdateDisplay();
        updatedisplay = 0;
      }
    }
    delay(50);
  }
}


// Interrupt routines
void ISRrotAChange()
{
  if (digitalRead(rotAPin))
  {
    rotAval = 1;
    UpdateRot();
  }
  else
  {
    rotAval = 0;
    UpdateRot();
  }
}


void ISRrotBChange()
{
  if (digitalRead(rotBPin))
  {
    rotBval = 1;
    UpdateRot();
  }
  else
  {
    rotBval = 0;
    UpdateRot();
  }
}


void UpdateRot()
{
  switch (rotState)
  {

    case 0:                                         // Idle state, look for direction
      if (!rotBval)
        rotState = 1;                               // CW 1
      if (!rotAval)
        rotState = 11;                              // CCW 1
      break;

    case 1:                                         // CW, wait for A low while B is low
      if (!rotBval)
      {
        if (!rotAval)
        {
          // either increment radixindex or freq
          if (digitalRead(pushPin) == LOW)
          {
            updatedisplay = 1;
            if (radix == 1000000)
              radix = 100000;
            else if (radix == 100000)
              radix = 10000;
            else if (radix == 10000)
              radix = 1000;
            else if (radix == 1000)
              radix = 100;
            else if (radix == 100)
              radix = 10;
            else if (radix == 10)
              radix = 1;
            else
              radix = 1000000;
          }
          else
          {
            freq = (freq + radix);
            if (freq > bandEnd)
              freq = bandEnd;
          }
          rotState = 2;                             // CW 2
        }
      }
      else if (rotAval)
        rotState = 0;                               // It was just a glitch on B, go back to start
      break;

    case 2:                                         // CW, wait for B high
      if (rotBval)
        rotState = 3;                               // CW 3
      break;

    case 3:                                         // CW, wait for A high
      if (rotAval)
        rotState = 0;                               // back to idle (detent) state
      break;

    case 11:                                        // CCW, wait for B low while A is low
      if (!rotAval)
      {
        if (!rotBval)
        {
          // either decrement radixindex or freq
          if (digitalRead(pushPin) == LOW)
          {
            updatedisplay = 1;
            if (radix == 1)
              radix = 10;
            else if (radix == 10)
              radix = 100;
            else if (radix == 100)
              radix = 1000;
            else if (radix == 1000)
              radix = 10000;
            else if (radix == 10000)
              radix = 100000;
            else if (radix == 100000)
              radix = 1000000;
            else
              radix = 1;
          }
          else
          {
            freq = (freq - radix);
            if (freq < bandStart)
              freq = bandStart;
          }
          rotState = 12;                            // CCW 2
        }
      }
      else if (rotBval)
        rotState = 0;                               // It was just a glitch on A, go back to start
      break;

    case 12:                                        // CCW, wait for A high
      if (rotAval)
        rotState = 13;                              // CCW 3
      break;

    case 13:                                        // CCW, wait for B high
      if (rotBval)
        rotState = 0;                               // back to idle (detent) state
      break;
  }
}


void UpdateDisplay()
{
  lcd.cursor();                                     // Turn on the cursor
  lcd.setCursor(0, 0);
  lcd.print("        ");
  lcd.setCursor(0, 0);
  lcd.print(freq);
  lcd.setCursor(10, 0);
  lcd.print("ZL2CTM");

  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);

  if (freq > 9999999)
  {
    if (radix == 1)
      lcd.setCursor(7, 0);
    if (radix == 10)
      lcd.setCursor(6, 0);
    if (radix == 100)
      lcd.setCursor(5, 0);
    if (radix == 1000)
      lcd.setCursor(4, 0);
    if (radix == 10000)
      lcd.setCursor(3, 0);
    if (radix == 100000)
      lcd.setCursor(2, 0);
    if (radix == 1000000)
      lcd.setCursor(1, 0);

  }
  if (freq <= 9999999)
  {
    if (radix == 1)
      lcd.setCursor(6, 0);
    if (radix == 10)
      lcd.setCursor(5, 0);
    if (radix == 100)
      lcd.setCursor(4, 0);
    if (radix == 1000)
      lcd.setCursor(3, 0);
    if (radix == 10000)
      lcd.setCursor(2, 0);
    if (radix == 100000)
      lcd.setCursor(1, 0);
    if (radix == 1000000)
      lcd.setCursor(0, 0);
  }
}



void SendFrequency()
{
  si5351.set_freq((freq * 4) * 100ULL, SI5351_PLL_FIXED, SI5351_CLK0);
}
