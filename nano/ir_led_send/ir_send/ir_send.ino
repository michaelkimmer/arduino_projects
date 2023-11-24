struct color{
  uint8_t color1;
  uint8_t color2;
  uint8_t color3;
};

int seq_length = 32;
int color_sequence[32] = {0,0,0,0, 1,0,1,0, 1,1,1,1, 2,1,2,1, 2,2,2,2, 3,2,3,2, 3,3,3,3, 0,3,0,3};  //nejde 0303?

void myPWM(int time_us){ //50%, 38kHz~0.00002631578s=26.31578us
  //16MHz~62.5ns ->421cyklu/5 = 84       /4 = 105
  int cykles_num = time_us/26.25;
  
  for(int j=0;j<cykles_num;j++){
    
  digitalWrite(12, HIGH);
  for (int i=0;i<42;i++){
    asm volatile("nop");
  }
  digitalWrite(12, LOW);
  for (int i=0;i<42;i++){
    asm volatile("nop");
  } //both takes 26.250us             

  }

}

void writeByte(uint8_t byte){ //delay after fnc!!
  for(int i=7;i>=0;i--){
    if((byte & (1<<i)) != 0){
      myPWM(580);
    }

    delayMicroseconds(550);
  }
}


int main(void) {
  
  // put your setup code here, to run once:
  struct color colors[3];

  //red
  colors[0].color1 = 213;
  colors[0].color2 = 223;
  colors[0].color3 = 173;
  //blue
  colors[1].color1 = 213;
  colors[1].color2 = 175;
  colors[1].color3 = 221;
  //green
  colors[2].color1 = 213;
  colors[2].color2 = 111;
  colors[2].color3 = 237;

pinMode(12, OUTPUT);
digitalWrite(12, LOW);

int i=0;
while(1){ 
  for(i=0; i<seq_length;i++){ //celkem 200ms
  myPWM(9000);
  delayMicroseconds(4500);
  writeByte(0b11111111);
  writeByte(0b10101010);
  writeByte(colors[color_sequence[i]].color1);
  writeByte(colors[color_sequence[i]].color2);
  writeByte(colors[color_sequence[i]].color3);
  writeByte(0b01010101);
  //takes 120 ms

  //delayMicroseconds(80000); //nejde - velky na int(2B)
  delayMicroseconds(30000);
  delayMicroseconds(30000);
  delayMicroseconds(10000);
  //delay(80); //nejde - proc???

  /*for(j =0; j<384000; j++) { //120ms/62.5ns/5 = 384000
    asm volatile("nop");
  }*/
  }
}
}