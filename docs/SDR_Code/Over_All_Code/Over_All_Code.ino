#include <Wire.h>                          // I2C comms
#include <si5351.h>                        // Si5351 library
#include <LiquidCrystal_I2C.h>             // LCD library
#include <Audio.h>                         // Teensy Audio Library

//teensy pin 18 : sda, blue wire
//teensy pin 19 : scl, yellow wire
// pink mic
// whites ground
// black ptt
// green speaker

#define NO_HILBERT_COEFFS 70        // this defines the number of number coef

// Iowa Hills Hilbert transform filter coefficients
const short Hilbert_Plus_45_Coeffs[NO_HILBERT_COEFFS] = {
  (short)(32768 * -0.000287988910943357),
  (short)(32768 * -0.000383511439791303),
  (short)(32768 * -0.000468041804899774),
  (short)(32768 * -0.000529324432676899),
  (short)(32768 * -0.000569479602046985),
  (short)(32768 * -0.000616670267768531),
  (short)(32768 * -0.000731530748681977),
  (short)(32768 * -0.001002372095321225),
  (short)(32768 * -0.001525299390682192),
  (short)(32768 * -0.002370114347025230),
  (short)(32768 * -0.003539247773172147),
  (short)(32768 * -0.004932965382552984),
  (short)(32768 * -0.006337182914262393),
  (short)(32768 * -0.007448193692118567),
  (short)(32768 * -0.007940501940620482),
  (short)(32768 * -0.007570802072162988),
  (short)(32768 * -0.006296120449841751),
  (short)(32768 * -0.004371955618154949),
  (short)(32768 * -0.002391875073164555),
  (short)(32768 * -0.001236984700413469),
  (short)(32768 * -0.001922560128827416),
  (short)(32768 * -0.005356720327533458),
  (short)(32768 * -0.012055656297010635),
  (short)(32768 * -0.021882952959947619),
  (short)(32768 * -0.033888748300090733),
  (short)(32768 * -0.046312736456333638),
  (short)(32768 * -0.056783367797647665),
  (short)(32768 * -0.062699937453677912),
  (short)(32768 * -0.061735375084135742),
  (short)(32768 * -0.052358513976237808),
  (short)(32768 * -0.034257179158167443),
  (short)(32768 * -0.008554500746482946),
  (short)(32768 * 0.022249911747384360),
  (short)(32768 * 0.054622962942346594),
  (short)(32768 * 0.084568844473140448),
  (short)(32768 * 0.108316122839950818),
  (short)(32768 * 0.122979341462627859),
  (short)(32768 * 0.127056096658453188),
  (short)(32768 * 0.120656295327679283),
  (short)(32768 * 0.105420364259485699),
  (short)(32768 * 0.084152608145489444),
  (short)(32768 * 0.060257510644444748),
  (short)(32768 * 0.037105711921879434),
  (short)(32768 * 0.017464092086704748),
  (short)(32768 * 0.003100559033325746),
  (short)(32768 * -0.005373489802481697),
  (short)(32768 * -0.008418211280310166),
  (short)(32768 * -0.007286730644726664),
  (short)(32768 * -0.003638388931163832),
  (short)(32768 * 0.000858330713630433),
  (short)(32768 * 0.004847436504682235),
  (short)(32768 * 0.007476399317750315),
  (short)(32768 * 0.008440227567663121),
  (short)(32768 * 0.007898970420636600),
  (short)(32768 * 0.006314366257036837),
  (short)(32768 * 0.004261033495040515),
  (short)(32768 * 0.002261843500794377),
  (short)(32768 * 0.000680212977485724),
  (short)(32768 * -0.000319493110301691),
  (short)(32768 * -0.000751893569425181),
  (short)(32768 * -0.000752248417868501),
  (short)(32768 * -0.000505487955986662),
  (short)(32768 * -0.000184645628631330),
  (short)(32768 * 0.000087913008490067),
  (short)(32768 * 0.000253106348867209),
  (short)(32768 * 0.000306473486382603),
  (short)(32768 * 0.000277637042003169),
  (short)(32768 * 0.000207782317481292),
  (short)(32768 * 0.000132446796990356),
  (short)(32768 * 0.000072894261560354)
};

// Iowa Hills Hilbert transform filter coefficients
const short Hilbert_Minus_45_Coeffs[NO_HILBERT_COEFFS] = {
  (short)(32768 * -0.000072894261560345),
  (short)(32768 * -0.000132446796990344),
  (short)(32768 * -0.000207782317481281),
  (short)(32768 * -0.000277637042003168),
  (short)(32768 * -0.000306473486382623),
  (short)(32768 * -0.000253106348867259),
  (short)(32768 * -0.000087913008490148),
  (short)(32768 * 0.000184645628631233),
  (short)(32768 * 0.000505487955986583),
  (short)(32768 * 0.000752248417868491),
  (short)(32768 * 0.000751893569425298),
  (short)(32768 * 0.000319493110301983),
  (short)(32768 * -0.000680212977485245),
  (short)(32768 * -0.002261843500793748),
  (short)(32768 * -0.004261033495039842),
  (short)(32768 * -0.006314366257036280),
  (short)(32768 * -0.007898970420636345),
  (short)(32768 * -0.008440227567663343),
  (short)(32768 * -0.007476399317751102),
  (short)(32768 * -0.004847436504683540),
  (short)(32768 * -0.000858330713632029),
  (short)(32768 * 0.003638388931162351),
  (short)(32768 * 0.007286730644725833),
  (short)(32768 * 0.008418211280310565),
  (short)(32768 * 0.005373489802483816),
  (short)(32768 * -0.003100559033321630),
  (short)(32768 * -0.017464092086698697),
  (short)(32768 * -0.037105711921871905),
  (short)(32768 * -0.060257510644436532),
  (short)(32768 * -0.084152608145481672),
  (short)(32768 * -0.105420364259479538),
  (short)(32768 * -0.120656295327675800),
  (short)(32768 * -0.127056096658453216),
  (short)(32768 * -0.122979341462631633),
  (short)(32768 * -0.108316122839958146),
  (short)(32768 * -0.084568844473150454),
  (short)(32768 * -0.054622962942358168),
  (short)(32768 * -0.022249911747396132),
  (short)(32768 * 0.008554500746472333),
  (short)(32768 * 0.034257179158159054),
  (short)(32768 * 0.052358513976232306),
  (short)(32768 * 0.061735375084133286),
  (short)(32768 * 0.062699937453678217),
  (short)(32768 * 0.056783367797650072),
  (short)(32768 * 0.046312736456337288),
  (short)(32768 * 0.033888748300094730),
  (short)(32768 * 0.021882952959951244),
  (short)(32768 * 0.012055656297013388),
  (short)(32768 * 0.005356720327535105),
  (short)(32768 * 0.001922560128828006),
  (short)(32768 * 0.001236984700413229),
  (short)(32768 * 0.002391875073163812),
  (short)(32768 * 0.004371955618154038),
  (short)(32768 * 0.006296120449840938),
  (short)(32768 * 0.007570802072162439),
  (short)(32768 * 0.007940501940620253),
  (short)(32768 * 0.007448193692118624),
  (short)(32768 * 0.006337182914262643),
  (short)(32768 * 0.004932965382553323),
  (short)(32768 * 0.003539247773172483),
  (short)(32768 * 0.002370114347025498),
  (short)(32768 * 0.001525299390682370),
  (short)(32768 * 0.001002372095321316),
  (short)(32768 * 0.000731530748682004),
  (short)(32768 * 0.000616670267768521),
  (short)(32768 * 0.000569479602046963),
  (short)(32768 * 0.000529324432676881),
  (short)(32768 * 0.000468041804899765),
  (short)(32768 * 0.000383511439791304),
  (short)(32768 * 0.000287988910943362)
};

// Define Constants and Vaviables
const long tMbStart = 14000000;    // start of 20 Meter HF band
const long tMbEnd =   14350000;    // end of 20 Meter HF band
const long eMbStart = 3500000;     // Start of 80 Meter HF band
const long eMbEnd =   4000000;     // end of 80 Meter HF band
const long bandInit =  3750000;    // the bandInit is st to the USA 20 Meter AM calling frequency

volatile long oldfreq = 0;
volatile long freq = bandInit;
volatile long radix = 1000;                // how much to change the frequency by, clicking the rotary encoder will change this.
volatile int Even_Divisor = 0;             // This is used in generation of the 90 degree offset signal
volatile int oldEven_Divisor = 0;
volatile int displayFlag = 0;
volatile int bandPick = 0;     // this picks the 20 or 80 meter bands , 0:20, 1:80

//Teensy Audio things
static const int Mic_Gain = 0;          // Range of 0 to 63dB
static const int Lineout_Gain = 31;      // Range is 13 - 31. 13 = 3.16 Vp-p, 31 = 1.16 Vp-p 


// Rotary Encoder
int led = 13;
static const int pushPin = 39;    //green
static const int rotBPin = 36;    //yellow
static const int rotAPin = 35;    //red
volatile int aState;
volatile int bState;
volatile int aLastState;
volatile int bLastState;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile int license = 2;                 // this denotes the licence level, 0 is unlicenced
volatile int ssb = 0;

// Instantiate the Objects
LiquidCrystal_I2C lcd(0x3F, 20, 4);       // set the LCD address to either 0x27 or 0x3F for a 16 chars and 2 line display
Si5351 si5351;                            // name for the clock generator
AudioControlSGTL5000  audioShield;         // name for the teensy audio shield

// Audio shield
AudioInputI2S           audioInput;                                           // Name for the input to the audio shield (either line-in or mic)
AudioOutputI2S          audioOutput;                                          // Name for the output of the audio shield (either headphones or line-out)
AudioFilterFIR          RX_Hilbert_Plus_45;                                   // Name for the RX +45 Hilbert transform
AudioFilterFIR          RX_Hilbert_Minus_45;                                  // Name for the RX -45 Hilbert transform
AudioMixer4             RX_Summer;                                            // Name for the RX summer

/*
// Transmitter
AudioFilterFIR          TX_Hilbert_Plus_45;                                   // Name for the TX +45 Hilbert transform
AudioFilterFIR          TX_Hilbert_Minus_45;                                  // Name for the TX +45 Hilbert transform
AudioMixer4             TX_I_Sideband_Switch;                                 // Name for the sideband switching summer for the I channel

// Audio connections
AudioConnection         patchCord50(audioInput, 0, TX_Hilbert_Plus_45, 0);              // Mic audio to Hilbert transform +45
AudioConnection         patchCord55(audioInput, 0, TX_Hilbert_Minus_45, 0);             // Mic audio to  Hilbert transform -45
AudioConnection         patchCord60(TX_Hilbert_Plus_45, 0, TX_I_Sideband_Switch, 0);    // Hilbert transform +45 to receiver summer
AudioConnection         patchCord65(TX_I_Sideband_Switch, 0, audioOutput, 0);           // Output to the NE612
AudioConnection         patchCord70(TX_Hilbert_Minus_45, 0, audioOutput, 1);            // Output to the NE612
*/

// Audio connections
AudioConnection         patchCord5(audioInput, 0, RX_Hilbert_Plus_45, 0);     // Left channel in Hilbert transform +45
AudioConnection         patchCord10(audioInput, 1, RX_Hilbert_Minus_45, 0);   // Right channel in Hilbert transform -45
AudioConnection         patchCord15(RX_Hilbert_Plus_45, 0, RX_Summer, 0);     // Hilbert transform +45 to receiver summer
AudioConnection         patchCord20(RX_Hilbert_Minus_45, 0, RX_Summer, 1);    // Hilbert transform -45 to receiver summer
AudioConnection         patchCord25(RX_Summer, 0, audioOutput, 0);            // Receiver summer to receiver LPF

/*
//Audio pass through test instantiation
AudioConnection         patchCord5(audioInput, 0, audioOutput, 0);    // mic channel in to left channel out (Direct pass through)
AudioConnection         patchCord10(audioInput, 1, audioOutput, 1);    // mic channel in to right channel out (Direct pass through)
*/


void setup()
{
  // Set up input switches
  pinMode(rotAPin, INPUT_PULLUP);
  pinMode(rotBPin, INPUT_PULLUP);
  pinMode(pushPin, INPUT_PULLUP);

  // Set up interrupt pins
  attachInterrupt(digitalPinToInterrupt(rotAPin), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotBPin), ISRrotBChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pushPin), radixChange, RISING);

    // This is Initalizing the clock generator 
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    si5351.set_freq(1700000ULL, SI5351_CLK0);  
    si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
    si5351.set_freq(1700000ULL, SI5351_CLK1); 

    //set up the Audio Shield
    AudioNoInterrupts();
    AudioMemory(16);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_LINEIN);
    audioShield.volume(0.7);
    audioShield.unmuteLineout();
    AudioInterrupts();

    // Initialise the lcd
    lcd.begin();
    lcd.backlight();
    
    // print a message on the LCD.
    lcd.setCursor(0,0);                     //set to write on line 1, col 1
    lcd.print("   SDR Team 2.12");
    lcd.setCursor(0,1);
    lcd.print("PM James Bell");
    lcd.setCursor(0,2);
    lcd.print("Samuel Hussey");
    lcd.setCursor(0,3);
    lcd.print("Zachary Schneiderman");
    delay(3000);
    lcd.clear();

  updateDisplay();
}

void loop()
{
  if (radix == 1000000 && bandPick == 1)
   radix/=10;
   
  if(freq > tMbEnd){
    freq = eMbStart;
    bandPick = 1;
  }
  
  if(freq < eMbStart){
    freq = tMbEnd;
    bandPick = 0;
  }
  
  if(freq > eMbEnd && freq < tMbStart && bandPick == 1){
    freq = tMbStart;
    bandPick = 0;
  }

  if(freq > eMbEnd && freq < tMbStart && bandPick == 0){
    freq = eMbEnd;
    bandPick = 1;
  }
  
  if (freq != oldfreq)
  {
    updateDisplay();
    EvenDivisor();
    SendFrequency();
    oldfreq = freq;
  }

      if (displayFlag == 1)
      {
        updateDisplay();
        displayFlag = 0;
  }
}

// Interrupt routines
void ISRrotAChange()
{
  cli(); //Disable Interrupts
  aState = digitalRead(rotAPin);
  bState = digitalRead(rotBPin);
  
  if((aState && bState) && aFlag){
    freq-=radix;
    bFlag = 0;
    aFlag = 0;
  }
  
  else if(!aState && bState)
    bFlag = 1;

  delayMicroseconds(1000); //wait 1ms for bounce
  sei(); //Enable Interrupts
}

void ISRrotBChange()
{
  cli(); //Disable Interrupts
  aState = digitalRead(rotAPin);
  bState = digitalRead(rotBPin);
  
  if((aState && bState) && bFlag){
    freq+=radix;
    bFlag = 0;
    aFlag = 0;
  }
  
  else if(aState && !bState)
    aFlag = 1;

  delayMicroseconds(1000); //wait 1ms for debounce
  sei(); //Enable Interrupts
}

void radixChange()
{
  detachInterrupt(digitalPinToInterrupt(pushPin));
  displayFlag = 1;       //this tells the machine in the next loop to update the LCD
  radix/=10;
  if(radix<1)
    radix = 10000000;
  delayMicroseconds(5000);
  attachInterrupt(digitalPinToInterrupt(pushPin), radixChange, RISING);
}

void updateDisplay()
{
  cli(); //Disable Interrupts
  delayMicroseconds(5000);
  lcd.cursor();                                     // Turn on the cursor
  lcd.setCursor(5, 0);
  lcd.print(" SDR 2.12 ");
  lcd.setCursor(0, 1);                              // sets the Cursor to (col, row)
  lcd.print("                   ");
  lcd.setCursor(0,1);
  lcd.print("Freq: ");
  if(bandPick==1)
    lcd.print(" ");
  lcd.print(freq);
  //Displays selected Band
  if (bandPick == 0)
  {
    lcd.setCursor(16,1);
    lcd.print("20m");
  }
  else
  {
    lcd.setCursor(16,1);
    lcd.print("80m");
  }

   lcd.setCursor(0,2);
   lcd.print("Class: ");
   if(license==0)
     lcd.print("UnLic.");
   if(license==1)
     lcd.print("Tech.");
   if(license==2)
     lcd.print("Gen.");
   if(license==3)
     lcd.print("Xtra.");

   lcd.setCursor(16,2);
   if(ssb==0)
     lcd.print("LSB");
   if(ssb==1)
     lcd.print("USB");
   
  lcd.setCursor(0, 1);
    if (radix == 1)
      lcd.setCursor(13, 1);
    if (radix == 10)
      lcd.setCursor(12, 1);
    if (radix == 100)
      lcd.setCursor(11, 1);
    if (radix == 1000)
      lcd.setCursor(10, 1);
    if (radix == 10000)
      lcd.setCursor(9, 1);
    if (radix == 100000)
      lcd.setCursor(8, 1);
    if (radix == 1000000)
      lcd.setCursor(7, 1);
    if (radix == 10000000)
     lcd.setCursor(6, 1);
   sei(); //Enable Interrupts  
}

// http://www.qrp-labs.com/images/news/dayton2018/fdim2018.pdf page 15
//
// This is also part of the example codes given for the SI5351 clock generator
// Explained in the datasheets for the SI5351 
// https://cdn-learn.adafruit.com/downloads/pdf/adafruit-si5351-clock-generator-breakout.pdf
//
// For a quadrature (90-degee) LO outputs, just configure one output with 
// 0 phase offset register (in other words, leave it at the default 0); configure the 
// other output phase offset register to be the same as the even integer MultiSynth divider.
//
// The "MultiSynth divider" is the "Even_Divisor" and is posted in many places online.
// it does not seem to be consisitant and the data sheet for the device does not cover it in a way
// I understand.
void EvenDivisor()
{
  if (freq < 6850000)
  {
    Even_Divisor = 126;
  }
  if ((freq >= 6850000) && (freq < 9500000))
  {
    Even_Divisor = 88;
  }
  if ((freq >= 9500000) && (freq < 13600000))
  {
    Even_Divisor = 64;
  }
  if ((freq >= 13600000) && (freq < 17500000))
  {
    Even_Divisor = 44;
  }
  if ((freq >= 17500000) && (freq < 25000000))
  {
    Even_Divisor = 34;
  }
  if ((freq >= 25000000) && (freq < 36000000))
  {
    Even_Divisor = 24;
  }
  if ((freq >= 36000000) && (freq < 45000000)) {
    Even_Divisor = 18;
  }
  if ((freq >= 45000000) && (freq < 60000000)) {
    Even_Divisor = 14;
  }
  if ((freq >= 60000000) && (freq < 80000000)) {
    Even_Divisor = 10;
  }
  if ((freq >= 80000000) && (freq < 100000000)) {
    Even_Divisor = 8;
  }
  if ((freq >= 100000000) && (freq < 146600000)) {
    Even_Divisor = 6;
  }
  if ((freq >= 150000000) && (freq < 220000000)) {
    Even_Divisor = 4;
  }
}

// this sends the updated frequency to the clock generator. 
void SendFrequency()
{
  EvenDivisor();
  si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK1);
  si5351.set_phase(SI5351_CLK0, 0);
  si5351.set_phase(SI5351_CLK1, Even_Divisor);

if(Even_Divisor != oldEven_Divisor)
  {
    si5351.pll_reset(SI5351_PLLA);          // this resets the offset register to help keep the frequencys in sync 
    oldEven_Divisor = Even_Divisor;         // this stores the old value of the MultiSynth divider
  }
}
