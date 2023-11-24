void pilot_signal(struct PARAMETERS parameters){ 
    //parameters.pilot_params.N_pil1_s = 2*parameters.pilot_params.rec_factor*parameters.pilot_params.N;
    long int n, m;

    //FIRST CAZAC PILOT SIGNAL
    //N is prime (ODD!!) -- also length of basic sequence!!
    #define N  parameters.pilot_params.N
    #define M  parameters.pilot_params.M
    #define rec_factor  parameters.pilot_params.rec_factor

    double complex basic_seq[N];
    for(n =0; n<N; n++){
        basic_seq[n] = cexp(I*M_PI*M/N* n*(n+1)) *parameters.pilot_params.Gain; //row!!
    }
    
    //proper sequence
    double complex pil_s;
    for(n = 0;n<N/2;n++){
        for(m = 0; m<rec_factor; m++){ 
            pil_s = basic_seq[N/2 + n]; //first half (//N/2 -1/2 )
            Serial.println("%f + %fi, ", creal(pil_s), cimag(pil_s));
        }
    }
    for(n = 0;n<N;n++){
        for(m = 0; m<rec_factor; m++){ 
            pil_s = basic_seq[n]; //main sequence (N )
            Serial.println("%f + %fi, ", creal(pil_s), cimag(pil_s));
        }
    }
    for(n = 0;n<N/2 +1;n++){
        for(m = 0; m<rec_factor; m++){
            pil_s = basic_seq[n]; //last half (//N/2 +1/2 )
            Serial.println("%f + %fi, ", creal(pil_s), cimag(pil_s));
        }
    }


    //SECOND CONSTANT PILOT SIGNAL
    for(m = parameters.pilot_params.N_pil1_s; m< parameters.N_pil_s; m++){
        pil_s = 1*parameters.pilot_params.Gain;
        Serial.println("%f + %fi, ", creal(pil_s), cimag(pil_s));
    }


}

