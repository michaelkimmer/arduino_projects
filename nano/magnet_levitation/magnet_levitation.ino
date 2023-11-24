
/*
Magnet levitaion 

Author: Michael Kimmer
Year: 2023
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////  DEFINITIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////
// Starting values


// interrupts
#define TIMER_CYCLE_PERIOD_S 5
//#define TIMER_PRELOAD_VALUE 65535 - (15625 * 1)  // 1second timer //65535 - (16*10^6* 1second / 1024) //(used prescaler 1024)
#define TIMER_PRELOAD_VALUE 65535 - (16 * 1)  // 1second timer //65535 - (16*10^3* 1milisecond / 1024) //(used prescaler 1024) //not ideal 1ms !!

// define PINs   --ADC:  0-1023 for 10 bits or 0-4095 for 12 bits
#define PIN_field_X A0
#define PIN_field_Y A1

#define PIN_coil_EnA 5
#define PIN_coil_EnB 6
#define PIN_coil_IN1 7
#define PIN_coil_IN2 8
#define PIN_coil_IN3 9
#define PIN_coil_IN4 10

struct PID_values
{
    float P;
    float I;
    float D;
    int field_last_val;
};

//Global structure definition
struct PID_values PID_params_X = {.P = 3, .I = 1, .D = 0.3, .field_last_val = 0}; /////////////////////////////////////////
struct PID_values PID_params_Y = PID_params_X;


struct PID_values PID_component_X = {.P = 0, .I = 0, .D = 0, .field_last_val = 0};
struct PID_values PID_component_Y = PID_component_X;

//////////////////////////////////////////////////////////////////////////////////////////////////////  GLOBAL VARIABLES  //////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int flag_interupt_Timer1 = 0;
volatile int timer_called = 5;


    int hall_field_x_offset = 570;
    int hall_field_y_offset = 570;

//////////////////////////////////////////////////////////////////////////////////////////////////////  FUNCTIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////


// basic -- Fcns
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))  // already define sth here ???
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
void delay_ms(unsigned long ms);

// interrupt -- Fcns
void ISR_fcn_button();

void process_serial();
int interrupt_processing();

void set_coil(int coil_num, int direction);
void read_hall(int *hall_field_x, int *hall_field_y);


//////////////////////////////////////////////////////////////////////////////////////////////////////  SETUP  //////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    // set pins
    pinMode(PIN_field_X, INPUT);
    pinMode(PIN_field_Y, INPUT);
    pinMode(PIN_coil_EnA, OUTPUT);
    pinMode(PIN_coil_EnB, OUTPUT);
    pinMode(PIN_coil_IN1, OUTPUT);
    pinMode(PIN_coil_IN2, OUTPUT);
    pinMode(PIN_coil_IN3, OUTPUT);
    pinMode(PIN_coil_IN4, OUTPUT);

    digitalWrite(PIN_coil_EnA, HIGH);
    digitalWrite(PIN_coil_EnB, HIGH);
    digitalWrite(PIN_coil_IN1, LOW);
    digitalWrite(PIN_coil_IN2, LOW);
    digitalWrite(PIN_coil_IN3, LOW);
    digitalWrite(PIN_coil_IN4, LOW);

    // interrupt - Timer 1
    noInterrupts();  // disable all interrupts
    TCCR1A = 0;      // Timer/Counter Control Registers (TCCRnA/B)
    TCCR1B = 0;
    TCNT1 = TIMER_PRELOAD_VALUE;          // preload timer      // TCNTn = 65535 - (16x1010xTime in sec / Prescaler Value) //(used prescaler 1024)
    TCCR1B |= (1 << CS10) | (1 << CS12);  // 1024 prescaler
    TIMSK1 |= (1 << TOIE1);               // enable timer overflow interrupt ISR -- !! overflow
    interrupts();                         // enable all interrupts



    //read actual offset
    read_hall(&hall_field_x_offset, &hall_field_y_offset);
    delay(100);
    read_hall(&hall_field_x_offset, &hall_field_y_offset);

    // Serial
    Serial.begin(115200);
    delay(1000);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////  LOOP  //////////////////////////////////////////////////////////////////////////////////////////////////////
int coil_x_dir = 0;
int coil_y_dir = 0;



void loop() {
    
    int hall_field_x, hall_field_y;

    if (flag_interupt_Timer1)
    {
        flag_interupt_Timer1 = 0;

        


        // Test 1 --- each 3 seconds read magnetic field and print the values to Serial
        read_hall(&hall_field_x, &hall_field_y);
//        Serial.print("Read x-field: ");
//        Serial.print(hall_field_x);
//        Serial.print("/tRead y-field: ");
//        Serial.println(hall_field_y);


        // Compute directions
        if(hall_field_x-hall_field_x_offset < -1) coil_x_dir = 1;
        else if(hall_field_x-hall_field_x_offset > 1) coil_x_dir = -1;
        else coil_x_dir = 0;

        if(hall_field_y-hall_field_y_offset < -1) coil_y_dir = 1;
        else if(hall_field_y-hall_field_y_offset > 1) coil_y_dir = -1;
        else coil_y_dir = 0;


        // With PWM
          analogWrite(PIN_coil_EnB, abs(hall_field_x-hall_field_x_offset));
          analogWrite(PIN_coil_EnA, abs(hall_field_y-hall_field_y_offset));

//          digitalWrite(PIN_coil_EnB, HIGH);
//          digitalWrite(PIN_coil_EnA, LOW);


        //Test 0 
        set_coil(1, coil_x_dir);
        set_coil(0, coil_y_dir);




    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////  FUNCTIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////// basic -- Fcns /////////////////////////////////////////////////////
void delay_ms(unsigned long ms) {
    unsigned long i, T;

    T = 3200 * ms;  // max: 1342s = 22.3min
    for (i = 0; i < T; i++) {
        asm volatile("nop");
    }
}

///////////////////////////////////////////////////// interrupt -- Fcns /////////////////////////////////////////////////////

// TIMER1 - interrupt service routine for overflow
// 1second period !!!
ISR(TIMER1_OVF_vect) {
    TCNT1 = TIMER_PRELOAD_VALUE;  // 1second period  //preload timer      // TCNTn = 65535 - (16x1010xTime in sec / Prescaler Value) //(used prescaler 1024)
    timer_called += 1;

    if (timer_called >= TIMER_CYCLE_PERIOD_S) {
        timer_called = 0;  // reset timer counting !!

        flag_interupt_Timer1 = 1;
        // change current program within interrupt_processing() !! ?????!!!!!
    }
}

// Serial -- NOT an interrupt !!!
void process_serial() {
    int i, j, k;
    int received_len = 0, isNumeric = 1;

    //receive string
    String receivedData = Serial.readString();  // read until timeout
    receivedData.trim();                        // remove any \r \n whitespace at the end of the String

    Serial.print("MagnetLevitation: Received: ");
    Serial.println(receivedData);

    // check numeric

    // processing

    // reset timer counting !!
    timer_called = 0;  
}

int interrupt_processing() {
    //'Interrupt' from Serial
    if (Serial.available() > 0) {
        process_serial();
        //flag_interupt_Timer1 = 1;  // for debouncing -- see the manual interupt of the first letter (solved by 'debounce') !!!!!!!!!
        return 1;
    }

    // interrupt seen in Timer1 
    if (flag_interupt_Timer1) {
        flag_interupt_Timer1 = 0;
         // change current program !!
        return 1;
    }

    // No interrupt seen
    return 0;
}

///////////////////////////////////////////////////// Coils -- Fcns /////////////////////////////////////////////////////


void set_coil(int coil_num, int direction)
{
    switch(coil_num){
        case 0:
            switch(direction){
                case -1:
                    digitalWrite(PIN_coil_IN1, LOW);
                    digitalWrite(PIN_coil_IN2, HIGH);
                    break;
                case 0:
                    digitalWrite(PIN_coil_IN1, LOW);
                    digitalWrite(PIN_coil_IN2, LOW);
                    break;
                case 1:
                    digitalWrite(PIN_coil_IN1, HIGH);
                    digitalWrite(PIN_coil_IN2, LOW);
                    break;
            }
            break;

        case 1:
            switch(direction){
            case -1:
                    digitalWrite(PIN_coil_IN3, LOW);
                    digitalWrite(PIN_coil_IN4, HIGH);
                    break;
            case 0:
                    digitalWrite(PIN_coil_IN3, LOW);
                    digitalWrite(PIN_coil_IN4, LOW);
                    break;
            case 1:
                    digitalWrite(PIN_coil_IN3, HIGH);
                    digitalWrite(PIN_coil_IN4, LOW);
                    break;
            }
            break;
    }

}

void read_hall(int *hall_field_x, int *hall_field_y)
{

    // read Analog values
    *hall_field_x = analogRead(PIN_field_X);
    *hall_field_y = analogRead(PIN_field_Y);

}

void update_PID(int hall_field_x, int hall_field_y)
{
    // update P component
    PID_component_X.P = PID_params_X.P * hall_field_x;
    PID_component_Y.P = PID_params_Y.P * hall_field_y;

    // update I component
    PID_component_X.I += PID_params_X.I * hall_field_x;
    PID_component_Y.I += PID_params_Y.I * hall_field_y;

    // update D component
    PID_component_X.D = PID_params_X.D * (hall_field_x - PID_component_X.field_last_val);
    PID_component_Y.D = PID_params_Y.D * (hall_field_y - PID_component_Y.field_last_val);

    //at the end save the last value of field
    PID_component_X.field_last_val = hall_field_x;
    PID_component_Y.field_last_val = hall_field_y;
    
}
