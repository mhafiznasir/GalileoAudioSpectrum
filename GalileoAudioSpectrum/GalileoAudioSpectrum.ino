#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <stdint.h>  //For New Arduino
#define pin_adc 0
#include <fix_fft.h>
char im[128];
char data[128];
int analog_value[128];
int i=0;

void setup() {
  Serial.begin(9600);

}

void loop(){
  
  static long tt;
  int val;
  if (i < 128){
      analog_value[i] = analogRead(pin_adc);
      i++;  
   }
   else{
        for (i=0; i< 128;i++){ //Prepare data for FFT
            data[i] = analog_value[i] - 128;
            im[i] = 0;
        }
   //this could be done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
   // I am only interessted in the absolute value of the transformation
        for (i=0; i< 64;i++)
             data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);
   //do something with the data values 1..64 and ignore im
        i=0;

        for(int y = 0; y <64; y++)
        {
          Serial.print((uint8_t) data[y]);
          Serial.print(" ");
        }
        Serial.print("\n");
   }
   
   
}
