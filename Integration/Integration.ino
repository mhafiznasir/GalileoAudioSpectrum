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
#include <stdint.h> //For New Arduino
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
//  int debug = 0;
  int debug = 1;
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
  col0data[] = { 0 },
  col1data[] = { 0 },
  col2data[] = { 0 },
  col3data[] = { 0 },
  col4data[] = { 0 },
  col5data[] = { 0 },
  col6data[] = { 0 },
  col7data[] = { 0 },
      
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
  
  #define SERVOMIN  100 // This is the 'minimum' pulse length count (out of 4096)
  #define SERVOMAX  560 // This is the 'maximum' pulse length count (out of 4096)
  uint8_t servonum = 0;
  uint16_t pulseLength = 0;
  
  uint16_t amplitude[8] = {90, 90, 90, 90, 90, 90, 90, 90};
  uint16_t amplitude2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  uint8_t pistonHeight[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t prevPistonHeight[8] = {0, 0, 0, 0, 0, 0, 0, 0};

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
    {
      colDiv[i] += datacolumn[j];
    }
  }

  Serial.begin(9600);
  matrix.begin(0x70);
  
  // ----------End audio spectrum code----------
  
  // ----------Start servo shield code----------
  
  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at ~60 Hz updates
  
  // -----------End servo shield code-----------
}
    uint8_t m = 120;
void loop() 
{
  uint8_t  x, L, nBins, binNum, weighting, c;
  uint8_t * datacolumn;
  uint16_t minLvl, maxLvl;
  int level, y, sum;
  static long tt;
  int val;
  
  if(debug)  Serial.println("Start sampling audio");
    
//    if (m==30) m=120;
//       else if (m==120) m=30;
  // Move each servo motor according to piston height
    //x = 0;
  for(x=0; x<8; x++) 
  {
    //double m = getDegreeFromHeight(pistonHeight[x]);
    m = map(pistonHeight[x], 0, 10, 30, 120);
    
    pulseLength = map(m, 0, 180, SERVOMIN, SERVOMAX);
    pwm.setPWM(x, 0, pulseLength);
    
    if(debug)  
    {
      Serial.print("Going to move motor ");
      Serial.println(x);
      Serial.print("Degree: ");
      Serial.println(m);
      Serial.print("PistonHeight: ");
      Serial.println(pistonHeight[x]);
      Serial.print("pulseLength: ");
      Serial.println(pulseLength);
    }
  }
  
  i = 0;
  while(i < 128)
  {
    analog_value[i] = analogRead(pin_adc);
    i++;  
  }
  
  if(debug)  Serial.println("Finish sampling audio");

  for (i=0; i<128; i++)
  { 
    // Prepare data for FFT
    data[i] = analog_value[i];
    im[i] = 0;
  }
  
  // This could be done with the fix_fftr function without the im array.
  if(debug)  Serial.println("Start Fourier Transform");
  
  fix_fft(data, im, 7, 0);
  
  // I am only interested in the absolute value of the transformation
  if(debug)  Serial.println("Get absolute value transformation");
  
  for (i=0; i<64; i++)
  {
    data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);    
    
    // Remove noise and eq level
    if(data[i] < noise[i])
    {
      data[i] = 0; 
    }
    else
    {
      data[i] = ((data[i] - noise[i])*( (256L - eq[i])) >> 4);
    }
  }
  
  delay(2);
  
  // Fill background w/colors, then idle parts of columns will erase
  matrix.fillRect(0, 0, 8, 3, LED_RED);    // Upper section
  matrix.fillRect(0, 3, 8, 2, LED_YELLOW); // Mid
  matrix.fillRect(0, 5, 8, 3, LED_GREEN);  // Lower section

  if(debug)  Serial.println("Downsample spectrum output to 8 columns");
  
  // Downsample spectrum output to 8 columns:  
  for(x=0; x<8; x++) 
  {
    datacolumn   = (uint8_t *)&colData[x];
    nBins  = datacolumn[0] + 2;
    binNum = datacolumn[1];
    
    if(debug)  Serial.println("Weighted Average summation");
    
    for(sum=0, i=2; i<nBins; i++)
    {
      sum += (data[binNum++])*(datacolumn[i]); // Weighted
    }

    if(debug)  Serial.println("Averaging weighted average");
    
    col[x][colCount] = sum / colDiv[x]; // Average
    minLvl = maxLvl = col[x][0];
    
    if(debug)  Serial.println("Get range of prior 10 frames");
    
    for(i=1; i<10; i++) 
    { 
      // Get range of prior 10 frames
      if(col[x][i] < minLvl)      minLvl = col[x][i];
      else if(col[x][i] > maxLvl) maxLvl = col[x][i];
    }
    
    // minLvl and maxLvl indicate the extents of the FFT output, used
    // for vertically scaling the output graph (so it looks interesting
    // regardless of volume level).  If they're too close together though
    // (e.g. at very low volume levels) the graph becomes super coarse
    // and 'jumpy'...so keep some minimum distance between them (this
    // also lets the graph go to zero when no sound is playing)
    
    if(debug)  Serial.println("Vertically scaling the output graph");
    
    if(debug)
    {
      Serial.print("minLvlAvg : ");
      Serial.println(minLvlAvg[x]);
      Serial.print("maxLvlAvg : ");
      Serial.println(maxLvlAvg[x]);
    }
    
    if((maxLvl - minLvl) < 8) 
    {
      maxLvl = minLvl + 8;
    }
    
    minLvlAvg[x] = (minLvlAvg[x] * 7 + minLvl) >> 3; // Dampen min/max levels
    maxLvlAvg[x] = (maxLvlAvg[x] * 7 + maxLvl) >> 3; // (fake rolling average)
    
    // Temp swap
    int temp_swap = 0;
    if (maxLvlAvg[x] < minLvlAvg[x]) 
    {
      temp_swap    = minLvlAvg[x]; //Min = min | Max = max | Temp = min 
      minLvlAvg[x] = maxLvlAvg[x]; //Min = max | Max = max | Temp = min
      maxLvlAvg[x] = temp_swap;    //Min = max | Max = min | Temp = min
    }

    if(debug)  Serial.println("Fixed point scale based on dynamic min/max levels");
    
    // Second fixed-point scale based on dynamic min/max levels:
    level = 10L * (col[x][colCount] - minLvlAvg[x]) / (long)(maxLvlAvg[x] - minLvlAvg[x]);

    if(debug)  Serial.println("Clip the output and convert it to byte");
    if(debug)  {
      Serial.print("x:");
      Serial.println(x);
      Serial.print("colCount:");
      Serial.println(colCount);
      Serial.print("col[x][colCount]:");
      Serial.println(col[x][colCount]);
      Serial.println("minLvlAvg[x]:");
      Serial.println(minLvlAvg[x]);
      Serial.println("maxLvlAvg[x]:");
      Serial.println(maxLvlAvg[x]);
    }
    // Clip output and convert to byte:
    if(level < 0L)      c = 0;
    else if(level > 10) c = 10; // Allow dot to go a couple pixels off top
    else                c = (uint8_t)level;
    
    pistonHeight[x] = level;

    if(debug)  Serial.println("Keep the dot on the top");
    
    if(c > peak[x]) peak[x] = c; // Keep dot on top

    if(peak[x] <= 0) 
    { 
      // Empty column?
      matrix.drawLine(x, 0, x, 7, LED_OFF);
      continue;
    } 
    else if(c < 8) 
    { 
      // Partial column?
      matrix.drawLine(x, 0, x, 7 - c, LED_OFF);
    }

    // The 'peak' dot color varies, but doesn't necessarily match
    // the three screen regions...yellow has a little extra influence.
    if(debug)  Serial.println("Draw pixels");
    
    y = 8 - peak[x];
    if(y < 2)      matrix.drawPixel(x, y, LED_RED);
    else if(y < 6) matrix.drawPixel(x, y, LED_YELLOW);
    else           matrix.drawPixel(x, y, LED_GREEN);
  }

  //matrix.writeDisplay();
  
  // Every third frame, make the peak pixels drop by 1:
  if(debug)  Serial.println("Make pixel drop by 1 every 3 frame dotcount");
  
  if(++dotCount >= 3) 
  {
    dotCount = 0;
    for(x=0; x<8; x++) 
    {
      if(peak[x] > 0) peak[x]--;
    }
  }
  
  if(++colCount >= 10) colCount = 0;   
  
  if(debug)  Serial.println("Going to move motor");
  
  // Move each servo motor according to piston height
//  for(x=0; x<8; x++) 
//  {
//    //int m = getDegreeFromHeight(pistonHeight[x]);
//    int m = 0;
//    if(debug)  
//    {
//      Serial.print("Going to move motor ");
//      Serial.println(x);
//      Serial.print("Degree: ");
//      Serial.println(m);
//      Serial.print("PistonHeight: ");
//      Serial.println(pistonHeight[x]);
//    }
//    pulseLength = map(m, 0, 180, SERVOMIN, SERVOMAX);
//    //pulseLength = map(0, 0, 180, SERVOMIN, SERVOMAX);
//    pwm.setPWM(x, 0, pulseLength);
//  }
  
  delay(0);
//  for(x=0; x<8; x++) 
//  {
//    //double m = getDegreeFromHeight(pistonHeight[x]);
////    int m = map(pistonHeight[x], 0, 255, 30, 120);
//
//    
//    
//    pulseLength = map(30, 0, 180, SERVOMIN, SERVOMAX);
////    pulseLength = map(0, 0, 180, SERVOMIN, SERVOMAX);
//    pwm.setPWM(x, 0, pulseLength);
//    delay(2000);
//    pulseLength = map(120, 0, 180, SERVOMIN, SERVOMAX);
////    pulseLength = map(0, 0, 180, SERVOMIN, SERVOMAX);
//    pwm.setPWM(x, 0, pulseLength);
//    if(debug)  
//    {
//      Serial.print("Are we here ");
//      Serial.println(x);
//      Serial.print("Degree: ");
//      Serial.println(m);
//      Serial.print("PistonHeight: ");
//      Serial.println(pistonHeight[x]);
//      Serial.print("pulseLength: ");
//      Serial.println(pulseLength);
//    }
//  }
//  delay(2000);
}

double getDegreeFromHeight(double height)
{
  return height / 10 * 90 + 30;
}
