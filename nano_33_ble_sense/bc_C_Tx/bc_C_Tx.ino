#include <stdlib.h>
#include <math.h>
//#include "CComplex.h"


//////////////////////
struct RRC_PARAMS{
    long int L;
    double alpha;
};
struct PILOT_PARAMS{ 
    long int N;
    long int M;
    long int rec_factor;
    long int freqlength;
    long int N_pil1_s;
    double Gain;
};

struct PARAMETERS{
    //coding
    uint8_t G[2][3];
    //modulation
    double Tp;
    long int Ns;
    double Ts;
    double modulation_Gain;
    //rrc
    struct RRC_PARAMS rrc_params;
    long int samples_g;
    long int b_number_Bytes; //pass into structure
    long int b_number;
    long int samples;
    //pilot signal
    struct PILOT_PARAMS pilot_params;
    long int N_pil_s;
    //complete s
    long int N_s_complete;
};

///////////////////////global var///////////////////
//RRC
double g[40]; //parameters.samples_g
double q[20]; //complex //need 10 for modualation


/////////////////////////////////"main"/////////////////////////////////////////////////
 void setup(){
   
  Serial.begin(115200); //1048576

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDB, HIGH);



}

void loop(){
  long int i, j, n; 
  char str[30]; 

  digitalWrite(LEDR, LOW);
  
  //GENERATE some b without fopen !!! -- SD later
  long int b_number_Bytes = 32;
  struct PARAMETERS parameters;
  parameters = parameters_load(b_number_Bytes+1);

  uint8_t b[parameters.b_number_Bytes];
  for(n=0; n<b_number_Bytes-1; n++){
    b[n] = n%256;
  }
  b[b_number_Bytes-1] = 0;


/*
    //PRINT b bits
    Serial.write("b=\n");
    for(i = 0; i< parameters.b_number_Bytes; i++){
        for(j = 0; j< 8; j++){
            sprintf(str, "%d ", ((b[i] >> (7-j)) & (uint8_t)1 ) );
            Serial.write(str);
            write_complex((int16_t)pil_s[0], (int16_t)pil_s[1]);
        }
    }*/

  
    digitalWrite(LEDR, HIGH);
    
    
    ////////////////////Convolution coding/////////////////////////
    uint8_t c[parameters.b_number_Bytes][2];
     // c arguments reversed because of C!!
    conv_code(parameters, c, b); 
/*
    //PRINT c bits
    Serial.write("\nc0=\n");
    for(i = 0; i< parameters.b_number_Bytes; i++){
        for(j = 0; j< 8; j++){
          sprintf(str, "%d ", ((c[0][i] >> (7-j)) & (uint8_t)1 ) );
          Serial.write(str);
          write_complex(int16_t real, int16_t imag)
        }
    }
    Serial.write("\nc1=\n");
    for(i = 0; i< parameters.b_number_Bytes; i++){
        for(j = 0; j< 8; j++){
          sprintf(str, "%d ", ((c[1][i] >> (7-j)) & (uint8_t)1 ));
          Serial.write(str);
          write_complex(int16_t real, int16_t imag)
        }
    }*/
    
    digitalWrite(LEDG, LOW);
    ///RRC generation beforheand //////////////////////
     RRC(parameters);

    /////////////////////pilot signals/////////////////////
    //double complex s[parameters.N_s_complete];
    pilot_signal(parameters);

    digitalWrite(LEDG, HIGH);
    
    ////////////////////Modulation BPSK, QPSK/////////////////
    //RRC - g global



    digitalWrite(LEDB, LOW); //!!!!!!!!!!!!!!!!!!!!!!!

    //modulation QPSK
    modulation(parameters, c); 

    digitalWrite(LEDB, HIGH);

    ////////////////////////END/////////////////////////  
}

//////////////////////////////////other FUNCTIONS/////////////////////////////
void c_rotation(double *real, double *imag, double phase){
  double sin_phase, cos_phase;
  sin_phase = sin(phase);
  cos_phase = cos(phase);

  double real_old;
  real_old = *real;

  *real = (*real)*cos_phase - (*imag)*sin_phase;
  *imag = real_old*sin_phase + (*imag)*cos_phase;
}
//////////////////
void write_complex(int16_t real, int16_t imag){

while(Serial.availableForWrite() < 1);
Serial.write((int8_t)(real>>8));
while(Serial.availableForWrite() < 1);
Serial.write((int8_t)real);

while(Serial.availableForWrite() < 1);
Serial.write((int8_t)(imag>>8));
while(Serial.availableForWrite() < 1);
Serial.write((int8_t)imag);

}

///////////////////////////////////////////////////FUNCTIONS//////////////////////////////////////////////////
struct PARAMETERS parameters_load(int b_number_Bytes){
    struct PARAMETERS parameters;

    //coding
    //parameters.G[2][3] = {{1,0,1}, {1,1,1}};
    parameters.G[0][0] = 1; parameters.G[0][1] = 0; parameters.G[0][2] = 1;
    parameters.G[1][0] = 1; parameters.G[1][1] = 1; parameters.G[1][2] = 1;
    //modulation
    parameters.Tp = 0.00000512;  //1/195312.5 = 0.512e-6; %of the SDRu
    parameters.Ns = 4; //samples in one symbol
    parameters.Ts = parameters.Ns*parameters.Tp; //symbol period
    parameters.modulation_Gain = 2;
    //rrc
    parameters.rrc_params.L = 10; //L*Ns = used samples of RRC pulse
    parameters.rrc_params.alpha = 0.5; //roll-off factor
    parameters.samples_g = parameters.rrc_params.L * parameters.Ns; //from def
    parameters.b_number = 8*(b_number_Bytes-1)+2;
    parameters.samples = parameters.Ns*parameters.b_number-parameters.Ns + parameters.samples_g;
    parameters.b_number_Bytes = b_number_Bytes;
    //pilot signal
    parameters.pilot_params.N = 29; //131 //samples of pilote signal rec_factor*(0.5+1+0.5) %prime!!
    parameters.pilot_params.M = 3;
    parameters.pilot_params.rec_factor = 4;
    parameters.pilot_params.freqlength = 50;//4096
    parameters.pilot_params.Gain = 450;
    parameters.pilot_params.N_pil1_s = 2*parameters.pilot_params.rec_factor*parameters.pilot_params.N;
    parameters.N_pil_s = parameters.pilot_params.N_pil1_s + parameters.pilot_params.freqlength;
    //pilot_signal + modualated
    parameters.N_s_complete = parameters.N_pil_s + parameters.samples;


    return parameters;
}

void conv_code(struct PARAMETERS parameters, uint8_t c[][2], uint8_t b[]){
   long int i, n ,p;
   uint8_t b_np;

    for(i=0;i<parameters.b_number_Bytes; i++){
        c[i][0] = 0;
        c[i][1] = 0;
    }

    //bitwise conv code
    for (n=1;n<parameters.b_number_Bytes; n++){
        b_np = b[n]; //b[n-p] made by shifts
        for (p = 0; p<3; p++){
      
                c[n][0] ^= parameters.G[0][p] * b_np;
                c[n][1] ^= parameters.G[1][p] * b_np;

                //shift b[n-p] and add bit from previous b[n-p-1] 
                b_np >>= 1;
                b_np |= ((b[n-p-1] << (7-p)) & ((uint8_t)1 << 7));
        }
    }
    //last byte has only 2 used bits
    n = 0; 
        b_np = b[n]; //b[n-p] made by shifts
        for (p = 0; p<3; p++){ 
            //if (n-p >= 0) { //&& (n-p <= b_number-1) holds
                c[n][0] ^= parameters.G[0][p] * b_np;
                c[n][1] ^= parameters.G[1][p] * b_np;

                //shift b[n-p] 
                b_np >>= 1;
            
        } 

}

void pilot_signal(struct PARAMETERS parameters){ 
    //parameters.pilot_params.N_pil1_s = 2*parameters.pilot_params.rec_factor*parameters.pilot_params.N;
    long int n, m, position;
    char str[30];

    //FIRST CAZAC PILOT SIGNAL
    //N is prime (ODD!!) -- also length of basic sequence!!
    #define N  parameters.pilot_params.N
    #define M  parameters.pilot_params.M
    #define rec_factor  parameters.pilot_params.rec_factor

    double  basic_seq[2*N]; //complex
    double phase;
    for(n =0; n<N; n++){
        phase = M_PI*M/N* n*(n+1);
        basic_seq[2*n] = cos(phase) *parameters.pilot_params.Gain; //row!!
        basic_seq[2*n+1] = sin(phase) *parameters.pilot_params.Gain;
    }
    
    //proper sequence
    double pil_s[2]; //complex
    for(n = 0;n<N/2;n++){
        for(m = 0; m<rec_factor; m++){ //first half (//N/2 -1/2 )
            position = N/2 + n;
            pil_s[0] = basic_seq[2*position]; 
            pil_s[1] = basic_seq[2*position +1];
           //sprintf(str, "%f + %fi,\n", pil_s[0], pil_s[1]);
            //Serial.write(str);
            write_complex((int16_t)pil_s[0], (int16_t)pil_s[1]);
        }
    }
    for(n = 0;n<N;n++){
        for(m = 0; m<rec_factor; m++){ 
            position = n;
            pil_s[0] = basic_seq[2*position]; 
            pil_s[1] = basic_seq[2*position +1];
            //sprintf(str, "%f + %fi,\n", pil_s[0], pil_s[1]);
            //Serial.write(str);
            write_complex((int16_t)pil_s[0], (int16_t)pil_s[1]);
        }
    }
    for(n = 0;n<N/2 +1;n++){
        for(m = 0; m<rec_factor; m++){
            position = n;
            pil_s[0] = basic_seq[2*position]; 
            pil_s[1] = basic_seq[2*position +1];
            //sprintf(str, "%f + %fi,\n", pil_s[0], pil_s[1]);
            //Serial.write(str);
            write_complex((int16_t)pil_s[0], (int16_t)pil_s[1]);
        }
    }


    //SECOND CONSTANT PILOT SIGNAL
    pil_s[0] = 1*parameters.pilot_params.Gain;
    for(m = parameters.pilot_params.N_pil1_s; m< parameters.N_pil_s; m++){  
        //sprintf(str, "%f + %fi,\n", pil_s[0], 0.0);
        //Serial.write(str);
        write_complex((int16_t)pil_s[0], (int16_t)0);
    }


}

void RRC(struct PARAMETERS parameters){
  long int i;
    #define err 0.0000000000000001

    double delay;
    delay = parameters.Tp*(parameters.rrc_params.L*parameters.Ns/2); //floor !!!
    

    double t_g[parameters.samples_g]; 

    
     //Serial.write(parameters.samples_g);
   
    for(i=0; i< 40; i++){ 
    
        t_g[i] = i*parameters.Tp - delay;
        
        g[i] = 1.0 /(sqrt(parameters.Ts) *(1.0-16*parameters.rrc_params.alpha*parameters.rrc_params.alpha*t_g[i]*t_g[i]/(parameters.Ts*parameters.Ts))) * (sin((1-parameters.rrc_params.alpha)*M_PI*t_g[i]/parameters.Ts)/(M_PI*t_g[i]/parameters.Ts) + 4*parameters.rrc_params.alpha*cos((1+parameters.rrc_params.alpha)*M_PI*t_g[i]/parameters.Ts)/M_PI);
    
        if (fabs(t_g[i]-0) <= err){
             g[i] = 1/sqrt(parameters.Ts)*(1-parameters.rrc_params.alpha+4*parameters.rrc_params.alpha/M_PI);
        }
        if ((fabs(t_g[i] - parameters.Ts/4/parameters.rrc_params.alpha) <=err) || (fabs(t_g[i] + parameters.Ts/4/parameters.rrc_params.alpha) <=err) ){
            g[i] = parameters.rrc_params.alpha/M_PI/sqrt(2*parameters.Ts)*((M_PI-2)*cos(M_PI/4/parameters.rrc_params.alpha)+(M_PI+2)*sin(M_PI/4/parameters.rrc_params.alpha));
        }
        
    }


}

void modulation(struct PARAMETERS parameters, uint8_t c[][2]){ 
    long int n, m, k, p;
    long int sample;
    char str[30];
    double s_n[2]; //complex //actual only
 
    // has to be without saving q, q_padded !!
    int bits_inByte;
    long int q_padded_length;
    q_padded_length = parameters.Ns* parameters.b_number - parameters.Ns+1;

    
    for(n = 0; n< 20; n++){
      q[n] = 0;
    } 
    


    for (n =0; n<parameters.b_number_Bytes; n++){
      if(n = parameters.b_number_Bytes-1){
        bits_inByte = 2;
      }
      else{
        bits_inByte = 8;
      }

      for(m =0; m<bits_inByte; m++){

          //QPSK modulation
          for(k = 9; k>0; k--){ //shift old q
            q[2*k] = q[2*(k-1)];
            q[2*k+1] = q[2*(k-1)+1];
          }
          q[0] = 1./sqrt(2)*(-1 +2*((c[n][0] >> (7-m)) & (uint8_t)1 ))*parameters.modulation_Gain;
          q[1] = 1./sqrt(2)*(-1+2*((c[n][1] >> (7-m)) & (uint8_t)1 ))*parameters.modulation_Gain;
          


          
          for(k = 0; k< 4; k++){ //padding
            sample =4*(8*n+m)+k; //potom musi byt jeste vetsi na zbytek posledniho pulsu
              s_n[0] = 0;
              s_n[1] = 0;
              for(p =0; p< parameters.rrc_params.L; p++){ //conv
                  if(  (n-m >= 0) && (n-m<q_padded_length) ){
                    s_n[0] += q[2*p]*g[4*p+k];
                    s_n[1] += q[2*p + 1]*g[4*p+k];
                  }

              }
              //sprintf(str, "%f + %fi,\n", s_n[0], s_n[1]);
              //Serial.write(str);
              write_complex(s_n[0], s_n[1]);
          }





        }  
    }

}






