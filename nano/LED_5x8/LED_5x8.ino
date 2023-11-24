
#include "font_5x8.c"  //variable -> console_font_5x8[] //pointer
//RANGE 32..255 -> 224 letters !!!

int LED_offset = 2;
int TRANS_offset = 15;

////////////////////////////////////////////////////  FUNCTIONS   //////////////////////////////////////

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

void bufferLetter(unsigned char *buffer, int address) {
  int i;
  for (i = 0; i < 8; i++) {
    buffer[i] = pgm_read_byte_near(console_font_5x8 + 8 * address + i - 32 * 8);  //32 offset
  }
}

////////////////////////////////////////   MAIN  /////////////////////////////////////

int main() {
  //char text[] = "Hana smrdi, je to tak. Ha Ha  "; //30 chars
  //char text[] = "HANA SMRDI, JE TO TAK, HAHAHA  ";
  char text[] = "_ Bez si dat tu svoji nejakou chalku, vole Hano a necum mi na display HAHA    ";
  int length = 77;
  //char text[] = "Bez si koupit to pivo, Ondro !! _";
  //int length = 32;
  //char text[] = "Uvidime se na maturaku, Pepo _";
  //int length = 29;


  unsigned char letter[8];
  unsigned char buffer[8];
  int i, j, n;

  //led pins
  for (i = 0; i < 8; i++) {
    pinMode(LED_offset + i, OUTPUT);
    digitalWrite(LED_offset + i, LOW);
  }
  //transistor pins
  for (i = 0; i < 8; i++) {
    pinMode(TRANS_offset + i, OUTPUT);
    digitalWrite(TRANS_offset + i, LOW);
  }

  //button
  pinMode(14, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  //null variables
  for (i = 0; i < 8; i++) {
    letter[i] = 0;
    buffer[i] = 0;
  }


  int x[30], y[30];
  /////////////////////////////////////  LOOP  //////////////////////////////////////
  while (1) {

    while (1) {  //writes moving text[]
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }
      for (n = 0; n < length; n++) {
        if (!digitalRead(14)) break;
        bufferLetter(buffer, (int)text[n]);
        for (i = 0; i < 6; i++) {  //6. is space !!!!!!!!!!!!!!!!
          for (j = 0; j < 30; j++) {
            if (!digitalRead(14)) break;
            drawLetter(letter, 1000);
          }
          for (j = 0; j < 8; j++) {
            letter[j] <<= 1;
            letter[j] |= ((uint8_t)128 & buffer[j]) >> 4;
            buffer[j] <<= 1;
          }
        }
      }
    }

    //////////////////////////////////////

    while (1) {  //writes stacic text[]
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }
      for (n = 0; n < length; n++) {
        if (!digitalRead(14)) break;

        bufferLetter(buffer, (int)text[n]);  //load data
        for (j = 0; j < 8; j++) {
          letter[j] = buffer[j];
        }


        for (j = 0; j < 100; j++) {
          if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
        }
      }
    }

    ///////////////////////////////////////////////////////
    //writes 30 random points
    while (1) {  
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }
      //n = (int)random(0, 10);

      for (i = 0; i < 30; i++) {
        x[i] = (int)random(0, 5);
        y[i] = (int)random(0, 8);
      }  //mam body

      for (i = 0; i < 30; i++) {  //rozsvitim 1 po 2.
        if (!digitalRead(14)) break;
        letter[y[i]] |= (uint8_t)1 << (7 - x[i]);
        for (j = 0; j < 10; j++) {
          drawLetter(letter, 1000);
        }
      }
      for (i = 0; i < 30; i++) {  //zhasinam 1 po 2.
        if (!digitalRead(14)) break;
        letter[y[29 - i]] &= ((uint8_t)1 << (7 - x[29 - i])) ^ ((uint8_t)255);
        for (j = 0; j < 10; j++) {
          drawLetter(letter, 1000);
        }
      }
    }

    //////////////////////////////////
    //moving dosts from above
    while (1) {  
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      for (i = 0; i < 8; i++) {
         letter[i] = 0;
        }
      break;
      }
      n = (int)random(0, 3); //0-2 body

      for (i = 0; i < n; i++) {  //mam body
        x[i] = (int)random(0, 5); //misto 5 -> mensi pravd na 2
      }  

      for (j = 0; j < 7; j++) {  //posunu letter dolu
          letter[7-j] = letter[7-j-1];
      }

      letter[0] = 0;
      for (j = 0; j < n; j++) {  //vlozim random body
          letter[0] |= (uint8_t)1 << (7-x[j]);
      }

      for (i = 0; i < 100; i++) {  //rozsvitim
          if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
      }
    }

    ///////////////////////////////
    //tetris
    while (1) {  
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }
      
      //posunu letter dolu, kdyz pod nim nic neni - bitwise
      for (i = 0; i < 7; i++) {  
        for (j = 0; j < 5; j++) { 
          if( (letter[7-i] & ((uint8_t)1<<(7-j)) ) == 0){ //pod diodou prazdno
            letter[7-i] |= ( letter[7-i-1] & ((uint8_t)1<<(7-j)) ); //dam do spodni
            letter[7-i-1] &= ~((uint8_t)1<<(7-j)); //vymazu vrchni
          }
          
        }
      }

      do{
        x[0] = (int)random(0, 5);
      }while((letter[0] & ((uint8_t)1 << (7-x[0]))) != 0);

      letter[0] |= (uint8_t)1 << (7-x[0]); //pridam nahodnou nahoru


      for (i = 0; i < 80; i++) {  //rozsvitim
          if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
      }

      if (letter[0] >= 0xf1 && letter[1] >= 0xf1){ // hezci
        for (i = 0; i < 8; i++) {
          letter[i] = 0;
        }
      }
    }
    //////////////////////////
    //random blinking
    while (1) {
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }

      for (i = 0; i < 8; i++) {
        letter[i] =(uint8_t)random(0, 255);
      }

      for (j = 0; j < 5; j++) {
        if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
      }
      
    }

    //////////////////////////////
    //one random blinking
    while (1) {
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }

      x[0] += (int)random(0, 3) - 1; //-1 .. 1 posun x[0]
      y[0] += (int)random(0, 3) - 1;

      if(x[0] < 0) x[0] = 1;
      if(y[0] < 0) y[0] = 1;
      if(x[0] > 4) x[0] = 3;
      if(y[0] > 7) y[0] = 6;
      
      letter[y[0]] = letter[y[0]] ^ ((uint8_t)1 << (7 - x[0]));
      

      for (j = 0; j < 10; j++) {
        if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
      }
      
    }

    /////////////////////////
    //rotace
      for (i = 0; i < 8; i++) { //initial state
        letter[i] = 0;
      }
      letter[0] = 0x80;
      letter[7] = 0x08;
      j = 0;

    while(1){
      if (!digitalRead(14)){ //Button
      delay_ms(500);
      break;
      }

      for (i = 0; i < 7; i++) {  //rozsvitim
          if (!digitalRead(14)) break;
          drawLetter(letter, 1000);
      }


      //rotace
      if((letter[1] == 0x80) || (letter[0] == 0x80)) j == 0; //bug fix :D

      if((letter[0] != 0x08) && (j == 0)){
        letter[0] >>= 1;
        j = 1; //moved?
      }
      else j = 0;

      if((letter[7] != 0x08) && (j == 0)){
        for(i=6; i>=0; i--){
          letter[i+1] |= (letter[i] & ((uint8_t)1 << 3));
          letter[i] &= ~((uint8_t)1 << 3);
        }
        j = 1;
      }
      else j = 0;

      if((letter[7] != 0x80) && (j == 0)){
        letter[7] <<= 1;
        j = 1; //moved?
      }
      else j = 0;

      if((letter[0] != 0x80) && (j == 0)){
        for(i=0; i<7; i++){
          letter[i] |= (letter[i+1] & ((uint8_t)1 << 7));
          letter[i+1] &= ~((uint8_t)1 << 7);
        }
        j = 1;
      }
      else j = 0;



    }
  }

  ////////////////////////
  

  ////////////////////////////////////////////////
}

///////////////////////////////////FUNCTIONS///////////////////////////////////////////////
