//complete code for SDR, from Charlie Morris, call sign: ZL2CT, additional annotations added by James Bell
#include <Wire.h>                          // I2C comms
#include <si5351.h>                        // Si5351 library
#include <LiquidCrystal_I2C.h>             // LCD library


// Define Constants and Vaviables
static const long bandStart = 1000000;     // start of HF band (New Zealand)
static const long bandEnd =   30000000;    // end of HF band   (New Zealand)
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
LiquidCrystal_I2C lcd(0x3F, 16, 2);       // set the LCD address to either 0x27 or 0x3F (what do these numbers mean?) for a 16 chars and 2 line display
Si5351 si5351;


void setup()
{
  // Set up input switches
  pinMode(rotAPin, INPUT);
  pinMode(rotBPin, INPUT);
  pinMode(pushPin, INPUT);
  digitalWrite(rotAPin, HIGH);
  digitalWrite(rotBPin, HIGH);
  digitalWrite(pushPin, HIGH);

  // Set up interrupt pins
  attachInterrupt(digitalPinToInterrupt(rotAPin), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotBPin), ISRrotBChange, CHANGE);

  // Initialise the lcd
  lcd.begin();
  lcd.backlight();

  // Initialise the DDS (what is the DDS?)
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.set_freq((freq * 100ULL), SI5351_PLL_FIXED, SI5351_CLK0);

  UpdateDisplay();
}


void loop()
{
  if (freq != oldfreq)
  {
    UpdateDisplay();                       // Functions in the arduino librarys?
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
        rotState = 1;                               // CW 1 (continous wave = CW )
      if (!rotAval)
        rotState = 11;                              // CCW 1 (Coherent CW, a variant of CW radiocommunication)
      break;

    case 1:                                         // CW, wait for A low while B is low (A is ?,B is ?, these are binaray values on some switch?)
      if (!rotBval)
      {
        if (!rotAval)
        {
          // either increment radixindex or freq
          if (digitalRead(pushPin) == LOW)
          {
            updatedisplay = 1;
            if (radix == 1000000)                  // Reduce by one 0 to traverse LCD location each time the "pushPin" is "LOW" (A binary state)
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
              radix = 1000000;                    // This causes the device to loop through the locations
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
            updatedisplay = 1;                  // this is the reverse of the LCD loop before. 
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
              radix = 1;                          //Loops back to start again
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
  lcd.print("TEST01");

  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);

  if (freq > 9999999)             // this is the top of the HF band for New Zealand
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
  if (freq <= 9999999)                      // this is the top of the HF band for New Zealand
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


