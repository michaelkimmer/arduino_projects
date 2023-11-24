#include "arduinoFFT.h"
//#include <math.h>

int LED_offset = 2;
int TRANS_offset = 15;

void delay_ms(unsigned long ms);
void drawLetter(unsigned char letter[8], int time);


int main(){
  uint8_t letter[8];
  int i, j;

  //FFT
  arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

  const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
  const double signalFrequency = 1000;
  const double samplingFrequency = 44000;
  const uint8_t amplitude = 100;
  /*
  These are the input and output vectors
  Input vectors receive computed results from FFT
  */
  double vReal[samples];
  double vImag[samples];

  //double period;
  //period = 1.0/signalFrequency; //23us

  int sensorValue = 0;        //ADC
  double Voltage = 0; 

  for(i = 0; i<samples; i++){
      vReal[i] = 0;
  }



  while(1){

    for(i = 0; i < samples; i++){
      sensorValue = analogRead(A0);
      Voltage = (double)sensorValue/1024*5;
      vReal[i] = Voltage;
      delayMicroseconds(23);
    }

  FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes in vReal*/

  for(i = 0; i < samples/2; i++){
      vReal[i] = vReal[i]*vReal[i];
  }

  




  }
}



///////////////////////////////////////////FCNs//////////////////////////////////
void delay_ms(unsigned long ms){
  unsigned long i;
  unsigned long T;
  T = 3200*ms;
  for (i = 0; i< T; i++){
    asm volatile("nop");
  }

}
void drawLetter(unsigned char letter[8], int time) {  //time in us !!!
  int i;
  int j;

  for (i = 0; i < 5; i++) {

    for (j = 0; j < 8; j++) {
      if ((letter[j] & ((uint8_t)1 << (7 - i))) == 0) {
        digitalWrite(LED_offset + j, LOW);
      } else {
        digitalWrite(LED_offset + j, HIGH);
      }
    }

    digitalWrite(TRANS_offset + i, HIGH);
    delayMicroseconds(time);

    for (j = 0; j < 8; j++) {  //odvedeni naboje!!
      digitalWrite(LED_offset + j, LOW);
    }

    //delayMicroseconds(2*time);
    digitalWrite(TRANS_offset + i, LOW);
  }
}