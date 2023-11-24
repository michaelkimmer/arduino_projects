void modulation(struct PARAMETERS parameters, uint8_t c[2][parameters.b_number_Bytes], double *g){
    long int n, m;
 
    //QPSK modulation
    double complex q[parameters.b_number];
    for (n =0; n<parameters.b_number_Bytes-1; n++){
        for(m =0; m<8; m++){
            q[n*8+m] = 1./sqrt(2)*(-1 +2*((c[0][n] >> (7-m)) & (uint8_t)1 )  +  I*(-1+2*((c[1][n] >> (7-m)) & (uint8_t)1 ))) *parameters.modulation_Gain;
        }  
    }
    n = parameters.b_number_Bytes-1;
    for(m = 0; m<2; m++){
        q[n*8+m] = 1./sqrt(2)*(-1 +2*((c[0][n] >> (7-m)) & (uint8_t)1 )  +  I*(-1+2*((c[1][n] >> (7-m)) & (uint8_t)1 ))) *parameters.modulation_Gain;
    } 

    /*
    //padding
    long int q_padded_length;
    q_padded_length = parameters.Ns* parameters.b_number - parameters.Ns+1;

    double complex q_padded[q_padded_length];
    for (n = 0; n<parameters.Ns* parameters.b_number - parameters.Ns+1; n++){ //upsampling of q by Ns (could be bypassed?)
        if (n%parameters.Ns == 0){
            q_padded[n] = q[n/parameters.Ns]; 
        }
        else{
        q_padded[n] = 0;
        }
    }
    double complex *q_padded;
    

    //s = conv(q_padded, g);
    for(n = 0; n< parameters.samples; n++){ 
        s[n] = 0;
        for(m =0; m< parameters.samples_g; m++){
            if((n-m >= 0) && (n-m<q_padded_length)){
                s[n] += q_padded[n-m]*g[m];
            }
        }
    }
    */

    //without q_padded
    long int q_padded_length;
    q_padded_length = parameters.Ns* parameters.b_number - parameters.Ns+1;

    double complex q_padded;
    double complex s_n;
    for(n = 0; n< parameters.samples; n++){ 
        s_n = 0;
        for(m =0; m< parameters.samples_g; m++){
            if( ((n-m)%4 == 0) && (n-m >= 0) && (n-m<q_padded_length) ){
                q_padded = q[(n-m)/4];
                s_n += q_padded*g[m];
            }
        }
        Serial.println("%f + %fi, ", creal(s_n), cimag(s_n));
    }

}