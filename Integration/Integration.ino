/*

Galileo Audio Spectrum output to 8x8 matrix bicolor

SDA AN4
SCL AN5
Audio Input AN0
Port LED code from Adafruit Picollo
*/

#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <stdint.h>  //For New Arduino
#include <fix_fft.h>
#include <Adafruit_PWMServoDriver.h>
#define pin_adc 0

  // -------------------------------------------
  // ---------Start audio spectrum code---------
  // -------------------------------------------

  char im[128];
  char data[128];
  int analog_value[128];
  int i=0;
  int debug = 0;
  byte
    peak[8],      // Peak level of each column; used for falling dots
    dotCount = 0, // Frame counter for delaying dot-falling speed
    colCount = 0; // Frame counter for storing past column data
  int
    col[8][10],   // Column levels for the prior 10 frames
    minLvlAvg[8], // For dynamic adjustment of low & high ends of graph,
    maxLvlAvg[8], // pseudo rolling averages for the prior few frames.
    colDiv[8];    // Used when filtering FFT output to 8 columns
  
  Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

  static const uint8_t 
  // This is low-level noise that's subtracted from each FFT output column:
  noise[64]={ 8,6,6,5,3,4,4,4,3,4,4,3,2,3,3,4,
              2,1,2,1,3,2,3,2,1,2,3,1,2,3,4,4,
              3,2,2,2,2,2,2,1,3,2,2,2,2,2,2,2,
              2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,4 },
  // These are scaling quotients for each FFT output column, sort of a
  // graphic EQ in reverse.  Most music is pretty heavy at the bass end.
  eq[64]={
    255, 175,218,225,220,198,147, 99, 68, 47, 33, 22, 14,  8,  4,  2,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
  // When filtering down to 8 columns, these tables contain indexes
  // and weightings of the FFT spectrum output values to use.  Not all
  // buckets are used -- the bottom-most and several at the top are
  // either noisy or out of range or generally not good for a graph.
  col0data[] = {  2,  1,  // # of spectrum bins to merge, index of first
    111,   8 },           // Weights for each bin
  col1data[] = {  4,  1,  // 4 bins, starting at index 1
     19, 186,  38,   2 }, // Weights for 4 bins.  Got it now?
  col2data[] = {  5,  2,
     11, 156, 118,  16,   1 },
  col3data[] = {  8,  3,
      5,  55, 165, 164,  71,  18,   4,   1 },
  col4data[] = { 11,  5,
      3,  24,  89, 169, 178, 118,  54,  20,   6,   2,   1 },
  col5data[] = { 17,  7,
      2,   9,  29,  70, 125, 172, 185, 162, 118, 74,
     41,  21,  10,   5,   2,   1,   1 },
  col6data[] = { 25, 11,
      1,   4,  11,  25,  49,  83, 121, 156, 180, 185,
    174, 149, 118,  87,  60,  40,  25,  16,  10,   6,
      4,   2,   1,   1,   1 },
  col7data[] = { 37, 16,
      1,   2,   5,  10,  18,  30,  46,  67,  92, 118,
    143, 164, 179, 185, 184, 174, 158, 139, 118,  97,
     77,  60,  45,  34,  25,  18,  13,   9,   7,   5,
      3,   2,   2,   1,   1,   1,   1 },
  // And then this points to the start of the data for each of the columns:
  * const colData[]  = {
    col0data, col1data, col2data, col3data,
    col4data, col5data, col6data, col7data };

  // -------------------------------------------
  // ----------End audio spectrum code----------
  // -------------------------------------------

  // -------------------------------------------
  // ----------Start servo shield code----------
  // -------------------------------------------
  
  // Called this way, it uses the default address 0x40
  Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
  
  #define SERVOMIN  100 // this is the 'minimum' pulse length count (out of 4096)
  #define SERVOMAX  488 // this is the 'maximum' pulse length count (out of 4096)
  uint8_t servonum = 0;
  uint16_t pulseLength = 0;
  uint16_t pulselength0 = 0;
  uint16_t pulselength1 = 0;
  uint16_t pulselength2 = 0;
  uint16_t pulselength3 = 0;
  uint16_t pulselength4 = 0;
  uint16_t pulselength5 = 0;
  uint16_t pulselength6 = 0;
  uint16_t pulselength7 = 0;
  
  uint16_t amplitude[8] = {90, 90, 90, 90, 90, 90, 90, 90};
  uint16_t amplitude2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  // -------------------------------------------
  // -----------End servo shield code-----------
  // -------------------------------------------

void setup() 
{  
  // ---------Start audio spectrum code---------
  
  uint8_t * datacolumn;
  uint8_t i, j, nBins, binNum;

  memset(peak, 0, sizeof(peak));
  memset(col , 0, sizeof(col));

  for(i=0; i<8; i++) 
  {
    minLvlAvg[i] = 0;
    maxLvlAvg[i] = 512;
    datacolumn   = (uint8_t *)&colData[i];
    nBins        = datacolumn[0] + 2;
    binNum       = datacolumn[1];
    for(colDiv[i]=0, j=2; j<nBins; j++)
      colDiv[i] += datacolumn[j];
  }

  Serial.begin(9600);
  matrix.begin(0x70);
  
  // ----------End audio spectrum code----------
  
  // ----------Start servo shield code----------
  
  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at ~60 Hz updates
  
  // -----------End servo shield code-----------
}

void loop() 
{

}
