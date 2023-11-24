double RRC(struct PARAMETERS parameters, double *g){
    #define err 0.00000000000000000001
    long int i;
    double f_max;
    double delay;
    delay = parameters.Tp*floor(parameters.rrc_params.L*parameters.Ns/2); //floor !!!

    double t_g[parameters.samples_g]; 
    //double g[samples_g];
    for(i=0; i< parameters.samples_g; i++){
        t_g[i] = i*parameters.Tp - delay;
        g[i] = 1/(sqrt(parameters.Ts)*(1-16*parameters.rrc_params.alpha*parameters.rrc_params.alpha*t_g[i]*t_g[i]/(parameters.Ts*parameters.Ts))) * (sin((1-parameters.rrc_params.alpha)*M_PI*t_g[i]/parameters.Ts)/(M_PI*t_g[i]/parameters.Ts) + 4*parameters.rrc_params.alpha*cos((1+parameters.rrc_params.alpha)*M_PI*t_g[i]/parameters.Ts)/M_PI);
    
        if (fabs(t_g[i]-0) <= err){
             g[i] = 1/sqrt(parameters.Ts)*(1-parameters.rrc_params.alpha+4*parameters.rrc_params.alpha/M_PI);
        }
        if ((fabs(t_g[i] - parameters.Ts/4/parameters.rrc_params.alpha) <=err) || (fabs(t_g[i] + parameters.Ts/4/parameters.rrc_params.alpha) <=err) ){
            g[i] = parameters.rrc_params.alpha/M_PI/sqrt(2*parameters.Ts)*((M_PI-2)*cos(M_PI/4/parameters.rrc_params.alpha)+(M_PI+2)*sin(M_PI/4/parameters.rrc_params.alpha));
        }
    }

    f_max = (1+parameters.rrc_params.alpha)/(2*parameters.Ts); //fmax different from cutting of infinite RRC !!
    return f_max;
}