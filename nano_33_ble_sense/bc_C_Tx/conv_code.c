typedef unsigned char uint8_t;

void conv_code(struct PARAMETERS parameters, unsigned char c[2][parameters.b_number_Bytes], unsigned char b[parameters.b_number_Bytes]){
   long int i, n ,p;
   uint8_t b_np;

    for(i=0;i<parameters.b_number_Bytes; i++){
        c[0][i] = 0;
        c[1][i] = 0;
    }

    //bitwise conv code
    for (n=1;n<parameters.b_number_Bytes; n++){
        b_np = b[n]; //b[n-p] made by shifts
        for (p = 0; p<3; p++){
      
                c[0][n] ^= parameters.G[0][p] * b_np;
                c[1][n] ^= parameters.G[1][p] * b_np;

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
                c[0][n] ^= parameters.G[0][p] * b_np;
                c[1][n] ^= parameters.G[1][p] * b_np;

                //shift b[n-p] 
                b_np >>= 1;
            
        } 


}