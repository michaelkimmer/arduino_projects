typedef unsigned char uint8_t;

struct RRC_PARAMS{
    int L;
    double alpha;
};
struct PILOT_PARAMS{ 
    int N;
    int M;
    int rec_factor;
    long int freqlength;
    int N_pil1_s;
    double Gain;
};

struct PARAMETERS{
    //coding
    uint8_t G[2][3];
    //modulation
    double Tp;
    int Ns;
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
    parameters.samples_g = parameters.rrc_params.L*parameters.Ns; //from def
    parameters.b_number = 8*(b_number_Bytes-1)+2;
    parameters.samples = parameters.Ns*parameters.b_number-parameters.Ns + parameters.samples_g;
    parameters.b_number_Bytes = b_number_Bytes;
    //pilot signal
    parameters.pilot_params.N = 131; //samples of pilote signal rec_factor*(0.5+1+0.5) %prime!!
    parameters.pilot_params.M = 3;
    parameters.pilot_params.rec_factor = 4;
    parameters.pilot_params.freqlength = 4096;
    parameters.pilot_params.Gain = 450;
    parameters.pilot_params.N_pil1_s = 2*parameters.pilot_params.rec_factor*parameters.pilot_params.N;
    parameters.N_pil_s = parameters.pilot_params.N_pil1_s + parameters.pilot_params.freqlength;
    //pilot_signal + modualated
    parameters.N_s_complete = parameters.N_pil_s + parameters.samples;


    return parameters;
}