/*
This script is used for controlling a self made LED CUBE with used Arduino Nano.
Other used components: 9x Shift register 74HC595N, 8x NPN BJT BD139

Author: Michael Kimmer
Year: 2022
*/

////////////////////////////////////////////////////////////////////////////////////////////////////// INCLUDES //////////////////////////////////////////////////////////////////////////////////////////////////////
// include file with defined ASCII characters
#include "font_6x8.c"  //variable -> console_font_5x8[] //RANGE 32..255 -> 224 letters !!!

//////////////////////////////////////////////////////////////////////////////////////////////////////  DEFINITIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////
// Starting values
#define FIRST_PROGRAM 0
#define FIRST_MESSAGE "Be Nice Bro "
#define PROGRAMMED_PROGRAMS 25

// define blinking ratio
#define FLASH_NUM 4
#define FLASH_TIME 500

// interrupts
#define INTERRUPT_DELAY_MS 200  // 0.2s
#define TIMER_CYCLE_PERIOD_S 60
#define TIMER_PRELOAD_VALUE 65535 - (15625 * 1)  // 1second timer //65535 - (16*10^10* 1second / 1024) //(used prescaler 1024)

// define PINs
#define PIN_data 16
#define PIN_CLK_serial 17
#define PIN_CLK_parallel 2
#define PIN_nRESET 19
#define PIN_INTERRUPT 3

//////////////////////////////////////////////////////////////////////////////////////////////////////  GLOBAL VARIABLES  //////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int my_interupt_flag = 0;
volatile int current_program = FIRST_PROGRAM;
volatile int timer_called = 0;

#define DISPLAY_BUFFER_LEN 32
volatile char string_to_display[DISPLAY_BUFFER_LEN] = FIRST_MESSAGE;

//////////////////////////////////////////////////////////////////////////////////////////////////////  FUNCTIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////

struct Point {  // Cpp without typedef
    int i;
    int j;
    int k;
};

// basic -- Fcns
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))  // already define sth here ???
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
float max_3(float a, float b, float c);
byte reverse_byte(byte b);
void delay_ms(unsigned long ms);  // max: 1342s = 22.3min

// interrupt -- Fcns
void ISR_fcn_button();

void process_serial();
int interrupt_processing();

// LED -- Fcns
void drawCube(byte cube[64], int number_flash, long int time_us);
void clear_cube(byte cube[64]);
void write_one_LED(byte cube[64], short i, short j, short k, short bit_value);

// load letter -- Fcns
void loadLetter(byte letter[8], int address);            // 1 layer
void moveLetter_toRight(byte cube[64], byte letter[8]);  // move in buffer
void placeLetter_toLayer(byte cube[64], byte letter[8], short layer);

// move the cube -- Fcns
void move_around_left_all(byte cube[64], int radius);  // moves all LEDs in given radius (square radius)
void move_around_left_text(byte cube[64]);
void push_letter_right(byte cube[64], byte letter[8], int speed);

// complexer -- Fcns
void write_spectrogram(byte cube[64], short spectrogram[8]);  // writes 8 levels to cube only

// COMPLETE -- Fcns
void static_64_lines(byte cube[64], int speed, byte value);
void static_8_layers(byte cube[64], int speed, byte value0, byte value1, byte value2, byte value3,
                     byte value4, byte value5, byte value6, byte value7);
void random_flashing(byte cube[64], int speed);
void hypercube(byte cube[64], int speed);
void in_de_creasing_Cube(byte cube[64], int speed);
void auto_snake(byte cube[64], int speed);
void letters_to_back(byte cube[64], int repeating, int speed, volatile char *text);
void push_text(byte cube[64], int speed, volatile char *text);
void rotate_four_rods(byte cube[64], int speed);
void rotate_plane(byte cube[64], int speed);
void running_dots_and_stripes(byte cube[64], int speed);
void filling_edges_from_two(byte cube[64], int speed);
void filling_edges_from_one(byte cube[64], int speed);
void draw_random_falling_spectrogram(byte cube[64], int speed);
void invert_ball(byte cube[64], int speed);
void invert_pattern(byte cube[64], int speed);
void random_up_down_Hana(byte cube[64], int N, int speed);
void random_up_down_ME(byte cube[64], int N, int speed);
void celtralWave(byte cube[64], int speed);
void celtralSpiral(byte cube[64], int speed);
void push_sin(byte cube[64], int speed);
void push_sin_pause(byte cube[64], int speed);
void squeeze_object(byte cube[64], int speed, int flash_in_rest); 
void droplet(byte cube[64], int speed);

//////////////////////////////////////////////////////////////////////////////////////////////////////  SETUP  //////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    // set pins
    pinMode(PIN_data, OUTPUT);
    pinMode(PIN_CLK_serial, OUTPUT);
    pinMode(PIN_CLK_parallel, OUTPUT);
    pinMode(PIN_nRESET, OUTPUT);

    digitalWrite(PIN_data, LOW);
    digitalWrite(PIN_CLK_serial, LOW);
    digitalWrite(PIN_CLK_parallel, LOW);
    digitalWrite(PIN_nRESET, HIGH);

    // interrupt - PIN
    pinMode(PIN_INTERRUPT, INPUT_PULLUP);
    delayMicroseconds(100);
    attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT), ISR_fcn_button, FALLING);  // ISR - Interrupt Service Routine

    // interrupt - Timer 1
    noInterrupts();  // disable all interrupts
    TCCR1A = 0;      // Timer/Counter Control Registers (TCCRnA/B)
    TCCR1B = 0;
    TCNT1 = TIMER_PRELOAD_VALUE;          // preload timer      // TCNTn = 65535 - (16x1010xTime in sec / Prescaler Value) //(used prescaler 1024)
    TCCR1B |= (1 << CS10) | (1 << CS12);  // 1024 prescaler
    TIMSK1 |= (1 << TOIE1);               // enable timer overflow interrupt ISR -- !! overflow
    interrupts();                         // enable all interrupts

    // Serial
    Serial.begin(115200);
    delay(1000);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////  LOOP  //////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
    // randomSeed(1001);
    byte cube[64];

    // wait between interrupts (debounce)
    delay_ms(INTERRUPT_DELAY_MS);
    my_interupt_flag = 0;  // some interrupt came in this delay (debounce)

    // Display current program
    char current_program_string[8] = "M";
    sprintf(current_program_string, "%d", current_program);
    letters_to_back(cube, /*repeating*/ 0, /*speed*/ 2, current_program_string);
    // letters_to_back(cube, /*repeating*/ 0, /*speed*/ 2, "N");

    delay_ms(500);
    if (my_interupt_flag) return;  // catch interrupt debounce from letters_to_back()

    switch (current_program) {
        case 0:
            while (!interrupt_processing())
                ;
            break;

        case 1:
            static_64_lines(cube, 10, 0xff);  // repair blinking ???
            break;

        case 2:
            static_8_layers(cube, 10, 0b00000000, 0b00111100, 0b01000010, 0b00000000, 0b00000000, 0b00100100, 0b00000000, 0b00000000);  // reversely
            break;

        case 3:
            random_flashing(cube, 3);
            break;

        case 4:
            hypercube(cube, 5);
            break;

        case 5:
            in_de_creasing_Cube(cube, 8);
            break;

        case 6:
            auto_snake(cube, 4);
            break;

        case 7:
            letters_to_back(cube, /*repeating*/ 1, /*speed*/ 2, string_to_display);
            break;

        case 8:
            push_text(cube, 6, string_to_display);
            break;

        case 9:
            rotate_four_rods(cube, 2);
            break;

        case 10:
            rotate_plane(cube, 3);
            break;

        case 11:
            draw_random_falling_spectrogram(cube, 10);
            break;

        case 12:
            running_dots_and_stripes(cube, 1);
            break;

        case 13:
            filling_edges_from_two(cube, 1);
            break;

        case 14:
            filling_edges_from_one(cube, 1);
            break;

        case 15:
            invert_ball(cube, 30);
            break;

        case 16:
            invert_pattern(cube, 5);
            break;

        case 17:
            random_up_down_Hana(cube, 10, 3);
            break;

        case 18:
            random_up_down_ME(cube, 4, 7);
            break;

        case 19:
            celtralWave(cube, 2);
            break;

        case 20:
            celtralSpiral(cube, 1);
            break;

        case 21:
            push_sin(cube, 2);
            break;

        case 22:
            push_sin_pause(cube, 2);
            break;

        case 23:
            squeeze_object(cube, 2, 5);
            break;
        case 24:
            droplet(cube, 3);
            break;
            
            
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////  FUNCTIONS  //////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////// basic -- Fcns /////////////////////////////////////////////////////
float max_3(float a, float b, float c) {
    return max(a, max(b, c));
}

byte reverse_byte(byte b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;  // nibbles
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;  // pairs
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;  // bits
    return b;
}

void delay_ms(unsigned long ms) {
    unsigned long i, T;

    T = 3200 * ms;  // max: 1342s = 22.3min
    for (i = 0; i < T; i++) {
        asm volatile("nop");
    }
}

///////////////////////////////////////////////////// interrupt -- Fcns /////////////////////////////////////////////////////
// interrupt - PIN
void ISR_fcn_button() {
    my_interupt_flag = 1;
    // change current program within interrupt_processing() !!

    timer_called = 0;  // reset timer counting !!
}

// TIMER1 - interrupt service routine for overflow
// 1second period !!!
ISR(TIMER1_OVF_vect) {
    TCNT1 = TIMER_PRELOAD_VALUE;  // 1second period  //preload timer      // TCNTn = 65535 - (16x1010xTime in sec / Prescaler Value) //(used prescaler 1024)
    timer_called += 1;

    if (timer_called >= TIMER_CYCLE_PERIOD_S) {
        timer_called = 0;  // reset timer counting !!

        my_interupt_flag = 1;
        // change current program within interrupt_processing() !!
    }
}

// Serial -- NOT an interrupt !!!
void process_serial() {
    int i, j, k;
    int received_len = 0, isNumeric = 1;

    //receive string
    String receivedData = Serial.readString();  // read until timeout
    receivedData.trim();                        // remove any \r \n whitespace at the end of the String

    Serial.print("LEDcube: Received: ");
    Serial.println(receivedData);

    // check numeric
    received_len = receivedData.length();
    for (i = 0; i < received_len; i++) {
        if (receivedData[i] < '0' || receivedData[i] > '9') {
            isNumeric = 0;
            break;
        }
    }

    // processing
    if (isNumeric) {
        current_program = receivedData.toInt();
    } else {
        current_program = 7;

        // copy receivedData to volatile char array !!
        for (i = 0; i < received_len && i < DISPLAY_BUFFER_LEN - 1; i++) string_to_display[i] = receivedData[i];
        // add ' 'to the end (if there is place)
        if (received_len + 1 <= DISPLAY_BUFFER_LEN - 1) {
            string_to_display[received_len] = ' ';
            received_len += 1;
        }
        // check '\0'
        if (received_len <= DISPLAY_BUFFER_LEN - 1)
            string_to_display[received_len] = '\0';
        else
            string_to_display[DISPLAY_BUFFER_LEN - 1] = '\0';
    }

    timer_called = 0;  // reset timer counting !!
}

int interrupt_processing() {
    //'Interrupt' from Serial
    if (Serial.available() > 0) {
        process_serial();
        my_interupt_flag = 1;  // for debouncing -- see the manual interupt of the first letter (solved by 'debounce') !!!!!!!!!
        return 1;
    }

    // interrupt seen on Pin or in Timer1 (or serial, see above)
    if (my_interupt_flag) {
        my_interupt_flag = 0;
        current_program = (current_program + 1) % PROGRAMMED_PROGRAMS;  // change current program !!
        return 1;
    }

    // No interrupt seen
    return 0;
}

///////////////////////////////////////////////////// LED -- Fcns /////////////////////////////////////////////////////
void drawCube_old(byte cube[64], int number_flash, long int time_us) {  // time in us !!!
    int i, k, n;                                                        // standard i,j,k orientation

    short transistor_order[8] = {7, 0, 6, 1, 5, 2, 4, 3};
    short layer_act;

    digitalWrite(PIN_CLK_serial, LOW);
    asm volatile("nop");

    for (n = 0; n < number_flash; n++) {
        // load sequence -- view from the front !!
        for (k = 0; k < 8; k++) {  // vertical
            layer_act = transistor_order[k];

            // 1st half -- horizontal (perpendicular)
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 1]);  // LSBFIRST no
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 3]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 5]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 7]);

            // load right Transistor -- vertical
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, (byte)1 << layer_act);

            // 2nd half
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 6]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 4]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 2]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 0]);

            asm volatile("nop");  ////////////////

            // storage register clock -- up/down -- (flash the layer !!)
            digitalWrite(PIN_CLK_parallel, HIGH);
            asm volatile("nop");
            digitalWrite(PIN_CLK_parallel, LOW);

            // Delay
            delayMicroseconds(time_us);  // Delaymicroseconds() handles only: between 3 and 16383 micro seconds

            // done one layer !!

        }  // k

        // done one flash
    }

    // done all flashing

    // reset for the last (upper) layer at the end

    delayMicroseconds(2 * time_us);  // for the upper layer -- not to blick -- time_us appr 500us edit?? !!!  //Delaymicroseconds() handles only: between 3 and 16383 micro seconds !!

    digitalWrite(PIN_nRESET, LOW);
    asm volatile("nop");
    digitalWrite(PIN_nRESET, HIGH);

    digitalWrite(PIN_CLK_parallel, HIGH);
    asm volatile("nop");  ////////////////
    digitalWrite(PIN_CLK_parallel, LOW);
}

void drawCube(byte cube[64], int number_flash, long int time_us) {  // time in us !!!
    int i, k, n;                                                    // standard i,j,k orientation

    short transistor_order[8] = {7, 0, 6, 1, 5, 2, 4, 3};
    short layer_act;

    digitalWrite(PIN_CLK_serial, LOW);
    // asm volatile("nop");

    for (n = 0; n < number_flash; n++) {
        // load sequence -- view from the front !!
        for (k = 0; k < 8; k++) {  // vertical
            layer_act = transistor_order[k];

            // 1st half -- horizontal (perpendicular)
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 1]);  // LSBFIRST no
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 3]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 5]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 7]);

            // load right Transistor -- vertical
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, (byte)1 << layer_act);

            // 2nd half
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 6]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 4]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 2]);
            shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, cube[8 * k + 0]);

            // asm volatile("nop"); ////////////////

            // storage register clock -- up/down -- (flash the layer !!)
            digitalWrite(PIN_CLK_parallel, HIGH);
            // asm volatile("nop");
            digitalWrite(PIN_CLK_parallel, LOW);

            // Delay
            delayMicroseconds(time_us);  // Delaymicroseconds() handles only: between 3 and 16383 micro seconds !!

            // done one layer !!

        }  // k

        // done one flash
    }

    // done all flashing

    // reset for the last (upper) layer at the end

    // for the same delay made manually reset !!
    for (i = 0; i < 9; i++) {
        shiftOut(PIN_data, PIN_CLK_serial, MSBFIRST, 0);
    }

    // delayMicroseconds(2*time_us); // for the upper layer -- not to blick -- time_us appr 500us edit?? !!!
    // digitalWrite(PIN_nRESET, LOW);
    // digitalWrite(PIN_nRESET, HIGH);

    digitalWrite(PIN_CLK_parallel, HIGH);
    digitalWrite(PIN_CLK_parallel, LOW);
}

void clear_cube(byte cube[64]) {
    int i;
    for (i = 0; i < 64; i++) {
        cube[i] = 0;
    }
}

void write_one_LED(byte cube[64], short i, short j, short k, short bit_value) {  // i,j,k standard orientation
    bitWrite(cube[8 * k + j], i, bit_value);
}

///////////////////////////////////////////////////// load letter -- Fcns /////////////////////////////////////////////////////
void loadLetter(byte letter[8], int address) {
    int k;
    for (k = 0; k < 8; k++) {
        letter[7 - k] = pgm_read_byte_near(console_font_6x8 + 8 * address + k - 32 * 8);  // 32 offset
    }
}

void moveLetter_toRight(byte cube[64], byte letter[8]) {  // move in buffer
    int i, j, k;
    byte bitt;

    // rewrite 6 elements on the right side back
    for (k = 0; k < 8; k++) {
        for (j = 2; j < 7; j++) {
            bitt = bitRead(letter[k], 7 - (j - 2));  // nahore
            bitWrite(cube[8 * k + j], 7, bitt);
        }
    }
}

void placeLetter_toLayer(byte cube[64], byte letter[8], short layer) {
    int i;

    for (i = 0; i < 8; i++) {
        cube[8 * i + layer] = reverse_byte(letter[i] >> 1);
    }
}

///////////////////////////////////////////////////// move the cube -- Fcns /////////////////////////////////////////////////////
void move_around_left_all(byte cube[64], int radius) {  // radius = 1..4
    int i, j, k;
    byte bitt;
    byte column_buffer;

    // backup left back column
    for (k = 0; k < 8; k++) {
        bitt = bitRead(cube[8 * k + 3 + radius], 4 - radius);
        bitWrite(column_buffer, k, bitt);
    }

    // left side
    for (k = 0; k < 8; k++) {
        for (j = 2 + radius; j > 3 - radius; j--) {  // for(j=6;j>-1;j--){
            bitt = bitRead(cube[8 * k + j], 4 - radius);
            bitWrite(cube[8 * k + j + 1], 4 - radius, bitt);
        }
    }

    // front side
    for (k = 0; k < 8; k++) {
        for (j = 4 - radius; j < 3 + radius; j++) {
            bitt = bitRead(cube[8 * k + 4 - radius], j + 1);
            bitWrite(cube[8 * k + 4 - radius], j, bitt);
        }
    }

    // right side
    for (k = 0; k < 8; k++) {
        for (j = 4 - radius; j < 3 + radius; j++) {
            bitt = bitRead(cube[8 * k + j + 1], 3 + radius);
            bitWrite(cube[8 * k + j], 3 + radius, bitt);
        }
    }

    // back side
    for (k = 0; k < 8; k++) {
        for (j = 2 + radius; j > 4 - radius; j--) {  // for(j=6;j>0;j--){
            bitt = bitRead(cube[8 * k + 3 + radius], j);
            bitWrite(cube[8 * k + 3 + radius], j + 1, bitt);
        }

        // last column
        bitt = bitRead(column_buffer, k);
        bitWrite(cube[8 * k + 3 + radius], 5 - radius, bitt);
    }
}

void move_around_left_text(byte cube[64]) {
    int i, j;
    byte bitt;

    // left side
    for (i = 0; i < 8; i++) {
        for (j = 6; j > -1; j--) {
            bitt = bitRead(cube[8 * i + j], 0);
            bitWrite(cube[8 * i + j + 1], 0, bitt);
        }
    }

    // front side
    for (i = 0; i < 8; i++) {
        cube[8 * i] >>= 1;
    }

    // right side
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 7; j++) {
            bitt = bitRead(cube[8 * i + j + 1], 7);
            bitWrite(cube[8 * i + j], 7, bitt);
        }
        bitWrite(cube[8 * i + 7], 7, 0);  // clear the last bit
    }
}

// move cube and add one letter
void push_letter_right(byte cube[64], byte letter[8], int speed) {
    int i, j, k;
    int n;
    byte bitt;
    byte letter_buffer[8];

    for (n = 0; n < 8; n++) {
        letter_buffer[n] = letter[n];
    }

    // rewrite 6 elements on the right side back one by one
    for (n = 0; n < 7; n++) {
        // move cube
        move_around_left_text(cube);

        // write one column
        for (k = 0; k < 8; k++) {
            bitt = bitRead(letter_buffer[k], 7);  // nahore
            letter_buffer[k] <<= 1;
            bitWrite(cube[8 * k + 7], 7, bitt);
        }

        // flash cube
        if (my_interupt_flag /*|| Serial.available() > 0*/) {
            return;
        }  //!!!!!!!!!!! edited JUMP OUT  !!! (NOT 'complete Fcn' !!!)
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

///////////////////////////////////////////////////// complexer -- Fcns /////////////////////////////////////////////////////
// only writes 8x9 levels to cube -- from left, how many LEDs from bottom (0..8 !!)
void write_spectrogram(byte cube[64], short spectrogram[8]) {
    byte i, j, k;

    clear_cube(cube);

    for (i = 0; i < 8; i++) {              // to right
        for (k = 0; k < 8; k++) {          // actual height
            if (k < spectrogram[i]) {      // only '<' !!
                for (j = 0; j < 8; j++) {  // to back -- still the same
                    cube[8 * k + j] |= (1 << i);
                }
            }
        }
    }
}

///////////////////////////////////////////////////// COMPLETE -- Fcns /////////////////////////////////////////////////////
void static_64_lines(byte cube[64], int speed, byte value) {
    int n;

    clear_cube(cube);
    for (n = 0; n < 64; n++) cube[n] = value;

    while (1) {
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void static_8_layers(byte cube[64], int speed, byte value0, byte value1, byte value2, byte value3,
                     byte value4, byte value5, byte value6, byte value7) {
    int j;

    clear_cube(cube);
    for (j = 0; j < 8; j++) cube[0 * 8 + j] = value0;
    for (j = 0; j < 8; j++) cube[1 * 8 + j] = value1;
    for (j = 0; j < 8; j++) cube[2 * 8 + j] = value2;
    for (j = 0; j < 8; j++) cube[3 * 8 + j] = value3;
    for (j = 0; j < 8; j++) cube[4 * 8 + j] = value4;
    for (j = 0; j < 8; j++) cube[5 * 8 + j] = value5;
    for (j = 0; j < 8; j++) cube[3 * 8 + j] = value6;
    for (j = 0; j < 8; j++) cube[7 * 8 + j] = value7;

    while (1) {
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void random_flashing(byte cube[64], int speed) {
    int i;

    clear_cube(cube);
    while (1) {
        for (i = 0; i < 64; i++) cube[i] = (byte)random(0, 256);

        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

// hypercube -- draw/remove
void hypercube(byte cube[64], int speed) {
    int i, j;

    while (1) {
        clear_cube(cube);

        // inner cube
        cube[8 * 2 + 2] = 60;
        cube[8 * 2 + 5] = 60;
        cube[8 * 5 + 2] = 60;
        cube[8 * 5 + 5] = 60;
        for (i = 3; i < 5; i++) {
            cube[8 * 2 + i] = 36;
            cube[8 * 5 + i] = 36;
            cube[8 * i + 2] = 36;
            cube[8 * i + 5] = 36;
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // joints
        cube[8 * 1 + 1] = 66;
        cube[8 * 1 + 6] = 66;
        cube[8 * 6 + 1] = 66;
        cube[8 * 6 + 6] = 66;
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // outer cube
        cube[0] = 255;
        cube[7] = 255;
        cube[8 * 7 + 0] = 255;
        cube[8 * 7 + 7] = 255;
        for (i = 1; i < 7; i++) {
            cube[i] = 129;
            cube[8 * 7 + i] = 129;
            cube[8 * i] = 129;
            cube[8 * i + 7] = 129;
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // delete inner cube
        cube[8 * 2 + 2] = 0;
        cube[8 * 2 + 5] = 0;
        cube[8 * 5 + 2] = 0;
        cube[8 * 5 + 5] = 0;
        for (i = 3; i < 5; i++) {
            cube[8 * 2 + i] = 0;
            cube[8 * 5 + i] = 0;
            cube[8 * i + 2] = 0;
            cube[8 * i + 5] = 0;
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // delete joints
        cube[8 * 1 + 1] = 0;
        cube[8 * 1 + 6] = 0;
        cube[8 * 6 + 1] = 0;
        cube[8 * 6 + 6] = 0;
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // joints
        cube[8 * 1 + 1] = 66;
        cube[8 * 1 + 6] = 66;
        cube[8 * 6 + 1] = 66;
        cube[8 * 6 + 6] = 66;
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // inner cube
        cube[8 * 2 + 2] = 60;
        cube[8 * 2 + 5] = 60;
        cube[8 * 5 + 2] = 60;
        cube[8 * 5 + 5] = 60;
        for (i = 3; i < 5; i++) {
            cube[8 * 2 + i] = 36;
            cube[8 * 5 + i] = 36;
            cube[8 * i + 2] = 36;
            cube[8 * i + 5] = 36;
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // remove outer cube
        cube[0] = 0;
        cube[7] = 0;
        cube[8 * 7 + 0] = 0;
        cube[8 * 7 + 7] = 0;
        for (i = 1; i < 7; i++) {
            cube[i] = 0;
            cube[8 * 7 + i] = 0;
            cube[8 * i] = 0;
            cube[8 * i + 7] = 0;
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        // delete joints
        cube[8 * 1 + 1] = 0;
        cube[8 * 1 + 6] = 0;
        cube[8 * 6 + 1] = 0;
        cube[8 * 6 + 6] = 0;
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void in_de_creasing_Cube(byte cube[64], int speed) {
    int n, i, j, k;

    clear_cube(cube);
    while (1) {
        // increase 1..4
        for (n = 1; n < 5; n++) {
            clear_cube(cube);
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    for (k = 0; k < 8; k++) {
                        if ((max_3(abs(i - 3.5), abs(j - 3.5), abs(k - 3.5)) < n) && (max_3(abs(i - 3.5), abs(j - 3.5), abs(k - 3.5)) > n - 1)) {
                            // if((abs(i-3.5) < n) && (abs(j-3.5) < n) && (abs(k-3.5) < n)    &&    (abs(i-3.5) > n-1) && (abs(j-3.5) > n-1) && (abs(k-3.5) > n-1)){
                            write_one_LED(cube, i, j, k, 1);
                        }
                    }
                }
            }
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        // decrease 3..2
        for (n = 3; n > 1; n--) {
            clear_cube(cube);
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    for (k = 0; k < 8; k++) {
                        if ((max_3(abs(i - 3.5), abs(j - 3.5), abs(k - 3.5)) < n) && (max_3(abs(i - 3.5), abs(j - 3.5), abs(k - 3.5)) > n - 1)) {
                            // if((abs(i-3.5) < n) && (abs(j-3.5) < n) && (abs(k-3.5) < n)    &&    (abs(i-3.5) > n-1) && (abs(j-3.5) > n-1) && (abs(k-3.5) > n-1)){
                            write_one_LED(cube, i, j, k, 1);
                        }
                    }
                }
            }
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        // increase 1..4
        for (n = 1; n < 5; n++) {
            clear_cube(cube);
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    for (k = 0; k < 8; k++) {
                        // if( (max_3(abs(i-3.5),abs(j-3.5),abs(k-3.5)) < n )  &&  (max_3(abs(i-3.5),abs(j-3.5),abs(k-3.5)) > n-1 ) ){
                        if ((abs(i - 3.5) < n) && (abs(j - 3.5) < n) && (abs(k - 3.5) < n) && (abs(i - 3.5) > n - 1) && (abs(j - 3.5) > n - 1) && (abs(k - 3.5) > n - 1)) {
                            write_one_LED(cube, i, j, k, 1);
                        }
                    }
                }
            }
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        // decrease 3..2
        for (n = 3; n > 1; n--) {
            clear_cube(cube);
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    for (k = 0; k < 8; k++) {
                        // if( (max_3(abs(i-3.5),abs(j-3.5),abs(k-3.5)) < n )  &&  (max_3(abs(i-3.5),abs(j-3.5),abs(k-3.5)) > n-1 ) ){
                        if ((abs(i - 3.5) < n) && (abs(j - 3.5) < n) && (abs(k - 3.5) < n) && (abs(i - 3.5) > n - 1) && (abs(j - 3.5) > n - 1) && (abs(k - 3.5) > n - 1)) {
                            write_one_LED(cube, i, j, k, 1);
                        }
                    }
                }
            }
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

    }  // end while
}

void auto_snake(byte cube[64], int speed) {
    // length of the snake == 16
    int n;
    short snake_direction;  // 0..5
    int tries;

start_snake:  // for goto
    short x[16] = {0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0};
    short y[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    short z[16] = {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6};

    clear_cube(cube);
    while (1) {
        tries = 0;

        // move body
        for (n = 15; n > 0; n--) {
            x[n] = x[n - 1];
            y[n] = y[n - 1];
            z[n] = z[n - 1];
        }

        // try new valid position (randomly 1000x)
        do {
            tries += 1;
            if (tries > 1000) {                      // game-over
                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, 20 * FLASH_NUM, FLASH_TIME);
                goto start_snake;
            }

            // decide head direction
            x[0] = x[1];
            y[0] = y[1];
            z[0] = z[1];
            snake_direction = (short)random(6);
            switch (snake_direction) {
                case 0:
                    x[0] = x[1] + 1;
                    break;
                case 1:
                    x[0] = x[1] - 1;
                    break;
                case 2:
                    y[0] = y[1] + 1;
                    break;
                case 3:
                    y[0] = y[1] - 1;
                    break;
                case 4:
                    z[0] = z[1] + 1;
                    break;
                case 5:
                    z[0] = z[1] - 1;
                    break;
            }
        } while (x[0] < 0 || y[0] < 0 || z[0] < 0 || x[0] > 7 || y[0] > 7 || z[0] > 7 ||
                 (x[0] == x[2] && y[0] == y[2] && z[0] == z[2]) || (x[0] == x[3] && y[0] == y[3] && z[0] == z[3]) || (x[0] == x[4] && y[0] == y[4] && z[0] == z[4]) ||
                 (x[0] == x[5] && y[0] == y[5] && z[0] == z[5]) || (x[0] == x[6] && y[0] == y[6] && z[0] == z[6]) || (x[0] == x[7] && y[0] == y[7] && z[0] == z[7]) ||
                 (x[0] == x[8] && y[0] == y[8] && z[0] == z[8]) || (x[0] == x[9] && y[0] == y[9] && z[0] == z[9]) || (x[0] == x[10] && y[0] == y[10] && z[0] == z[10]) ||
                 (x[0] == x[11] && y[0] == y[11] && z[0] == z[11]) || (x[0] == x[12] && y[0] == y[12] && z[0] == z[12]) || (x[0] == x[13] && y[0] == y[13] && z[0] == z[13]) ||
                 (x[0] == x[14] && y[0] == y[14] && z[0] == z[14]) || (x[0] == x[15] && y[0] == y[15] && z[0] == z[15]));

        // render cube
        clear_cube(cube);
        for (n = 0; n < 16; n++) {
            write_one_LED(cube, x[n], y[n], z[n], 1);
        }
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void letters_to_back(byte cube[64], int repeating, int speed, volatile char *text) {
    int i, j;
    byte letter[8];
    int N = strlen((char *)text);

    clear_cube(cube);
    do {
        for (j = 0; j < N; j++) {
            // each letter
            loadLetter(letter, (int)(text[j]));
            for (i = 0; i < 8; i++) {
                placeLetter_toLayer(cube, letter, i);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

                clear_cube(cube);
            }
        }
    } while (repeating);
}

void push_text(byte cube[64], int speed, volatile char *text) {
    int i;
    byte letter[8];
    int N = strlen((char *)text);

    clear_cube(cube);
    while (1) {
        for (i = 0; i < N; i++) {
            if (text[i] == ' ') {  // space differently (just width=1)
                move_around_left_text(cube);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

                continue;
            }
            loadLetter(letter, (int)(text[i]));
            push_letter_right(cube, letter, speed);  // flash inside !!!

            // flash -- inside !!!
            if (interrupt_processing()) return;  // JUMP OUT !!
        }
    }
}

void rotate_four_rods(byte cube[64], int speed) {
    int k, n, m;

    clear_cube(cube);

    ////rotate letter (IDK why)
    // byte letter[8];
    // loadLetter(letter, (int)('A'));
    // push_letter_right(cube, letter, 1); //flash inside !!!
    ////flash inside !!!
    // if (interrupt_processing()) return; // JUMP OUT !!

    for (k = 0; k < 8; k++) {
        // 4 vertical rods -- next to each other
        cube[8 * k + 3] = 0b00001111;

        // 4 vertical rods -- in corners
        // cube[8*k+0] = 129;
        // cube[8*k+7] = 129;
    }

    if (interrupt_processing()) return;  // JUMP OUT !!
    drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

    while (1) {
        for (n = 4; n > -1; n--) {             // which rod
            for (m = 0; m < n * 4 - 2; m++) {  // how many times to rotate
                move_around_left_all(cube, n);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }
        }
    }
}

void rotate_plane(byte cube[64], int speed) {
    int k, n;

    // load plane
    clear_cube(cube);
    for (k = 0; k < 8; k++) {
        cube[8 * k + 0] = 1;
        cube[8 * k + 1] = 2;
        cube[8 * k + 2] = 4;
        cube[8 * k + 3] = 8;
        cube[8 * k + 4] = 16;
        cube[8 * k + 5] = 32;
        cube[8 * k + 6] = 64;
        cube[8 * k + 7] = 128;
    }
    if (interrupt_processing()) return;             // JUMP OUT !!
    drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

    // rotate
    while (1) {
        for (n = 0; n < 7; n++) {
            move_around_left_all(cube, 4);                                  // RADIUS == 4
            if (n != 1 && n != 5) move_around_left_all(cube, 3);            // RADIUS == 3
            if (n == 1 || n == 3 || n == 5) move_around_left_all(cube, 2);  // RADIUS == 2
            if (n == 3) move_around_left_all(cube, 1);                      // RADIUS == 1

            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

    }  // end while
}

void draw_random_falling_spectrogram(byte cube[64], int speed) {
    int i;

    short spectrogram[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // from left, how many LEDs from bottom (0..8 !!)
    short new_spectrogram[8];

    clear_cube(cube);
    while (1) {
        for (i = 0; i < 8; i++) {
            new_spectrogram[i] = (short)random(0, 9);

            if (new_spectrogram[i] >= spectrogram[i]) {
                spectrogram[i] = new_spectrogram[i];
            } else {
                // fall by one only
                spectrogram[i] -= 1;
            }
        }

        // write whole spectrogram !!
        write_spectrogram(cube, spectrogram);

        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void running_dots_and_stripes(byte cube[64], int speed) {
    int i, j, k;

    int side;
    int led_pos;
    int set_compl = 7;  // 0 or 7
    int j_old = 0, k_old = 1;
    int state;  // whether to create, delete, etc

    clear_cube(cube);
    while (1) {
        for (state = 0; state < 3; state++) {
            for (side = 0; side < 4; side++) {
                if (side == 0) k = 0;
                if (side == 1) j = 7;
                if (side == 2) k = 7;
                if (side == 3) j = 0;

                for (led_pos = 0; led_pos < 7; led_pos++) {
                    if (side == 0) j = led_pos;
                    if (side == 1) k = led_pos;
                    if (side == 2) j = 7 - led_pos;
                    if (side == 3) k = 7 - led_pos;

                    switch (state) {
                        case 0:  // write and delete
                            write_one_LED(cube, 0, j, k, 1);
                            write_one_LED(cube, 7, j, k, 1);
                            write_one_LED(cube, 2, set_compl - j, set_compl - k, 1);
                            write_one_LED(cube, 5, set_compl - j, set_compl - k, 1);

                            write_one_LED(cube, 0, j_old, k_old, 0);
                            write_one_LED(cube, 7, j_old, k_old, 0);
                            write_one_LED(cube, 2, set_compl - j_old, set_compl - k_old, 0);
                            write_one_LED(cube, 5, set_compl - j_old, set_compl - k_old, 0);
                            break;
                        case 1:  // write
                            write_one_LED(cube, 0, j, k, 1);
                            write_one_LED(cube, 7, j, k, 1);
                            write_one_LED(cube, 2, set_compl - j, set_compl - k, 1);
                            write_one_LED(cube, 5, set_compl - j, set_compl - k, 1);
                            break;
                        case 2:                                    // delete
                            if (side == 0 && led_pos == 0) break;  // nicer when not deleting the first LED
                            write_one_LED(cube, 0, j, k, 0);
                            write_one_LED(cube, 7, j, k, 0);
                            write_one_LED(cube, 2, set_compl - j, set_compl - k, 0);
                            write_one_LED(cube, 5, set_compl - j, set_compl - k, 0);
                            break;
                    }
                    j_old = j;
                    k_old = k;

                    if (interrupt_processing()) return;  // JUMP OUT !!
                    drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
                }
            }
            if (state == 0) {
                write_one_LED(cube, 0, j_old, k_old, 0);
                write_one_LED(cube, 7, j_old, k_old, 0);
                write_one_LED(cube, 2, set_compl - j_old, set_compl - k_old, 0);
                write_one_LED(cube, 5, set_compl - j_old, set_compl - k_old, 0);
            }
        }

    }  // end while
}

void filling_edges_from_two(byte cube[64], int speed) {
    int led_pos;  // actual position in its line
    int state;    // whether to create, delete, etc

    clear_cube(cube);
    while (1) {
        for (state = 1; state > -1; state--) {
            for (led_pos = 0; led_pos < 8; led_pos++) {  // from (0,0,0) increment allways one axis
                write_one_LED(cube, led_pos, 0, 0, state);
                write_one_LED(cube, 0, led_pos, 0, state);
                write_one_LED(cube, 0, 0, led_pos, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }

            for (led_pos = 1; led_pos < 7; led_pos++) {  // allways one axis full --> increment the second
                write_one_LED(cube, 7, led_pos, 0, state);
                write_one_LED(cube, 0, 7, led_pos, state);
                write_one_LED(cube, led_pos, 0, 7, state);

                write_one_LED(cube, 7, 0, led_pos, state);
                write_one_LED(cube, led_pos, 7, 0, state);
                write_one_LED(cube, 0, led_pos, 7, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }

            for (led_pos = 0; led_pos < 8; led_pos++) {  // allways two axis full --> increment the third // to 8 !!
                write_one_LED(cube, led_pos, 7, 7, state);
                write_one_LED(cube, 7, led_pos, 7, state);
                write_one_LED(cube, 7, 7, led_pos, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }
        }  // end for(state)

        // another vertex
        for (state = 1; state > -1; state--) {
            for (led_pos = 0; led_pos < 8; led_pos++) {  // from (0,0,0) increment allways one axis
                write_one_LED(cube, 7 - led_pos, 7, 0, state);
                write_one_LED(cube, 7, 7 - led_pos, 0, state);
                write_one_LED(cube, 7, 7, led_pos, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }

            for (led_pos = 1; led_pos < 7; led_pos++) {  // allways one axis full --> increment the second
                write_one_LED(cube, 0, 7 - led_pos, 0, state);
                write_one_LED(cube, 7, 0, led_pos, state);
                write_one_LED(cube, 7 - led_pos, 7, 7, state);

                write_one_LED(cube, 0, 7, led_pos, state);
                write_one_LED(cube, 7 - led_pos, 0, 0, state);
                write_one_LED(cube, 7, 7 - led_pos, 7, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }

            for (led_pos = 0; led_pos < 8; led_pos++) {  // allways two axis full --> increment the third // to 8 !!
                write_one_LED(cube, 7 - led_pos, 0, 7, state);
                write_one_LED(cube, 0, 7 - led_pos, 7, state);
                write_one_LED(cube, 0, 0, led_pos, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }
        }  // end for(state)

    }  // end while
}

void filling_edges_from_one(byte cube[64], int speed) {
    int led_pos;  // actual position in its line
    int state;    // whether to create, delete, etc

    clear_cube(cube);
    while (1) {
        for (state = 0; state < 2; state++) {
            for (led_pos = 0; led_pos < 8; led_pos++) {      // from (0,0,0) increment allways one axis
                write_one_LED(cube, led_pos, 0, 0, !state);  // light up first
                write_one_LED(cube, 0, led_pos, 0, !state);
                write_one_LED(cube, 0, 0, led_pos, !state);

                write_one_LED(cube, led_pos, 7, 7, state);  // turn off last at the same time
                write_one_LED(cube, 7, led_pos, 7, state);
                write_one_LED(cube, 7, 7, led_pos, state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }

            for (led_pos = 1; led_pos < 7; led_pos++) {  // allways one axis full --> increment the second
                write_one_LED(cube, 7, led_pos, 0, !state);
                write_one_LED(cube, 0, 7, led_pos, !state);
                write_one_LED(cube, led_pos, 0, 7, !state);

                write_one_LED(cube, 7, 0, led_pos, !state);
                write_one_LED(cube, led_pos, 7, 0, !state);
                write_one_LED(cube, 0, led_pos, 7, !state);

                if (interrupt_processing()) return;  // JUMP OUT !!
                drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
            }
        }  // end for(state)

    }  // end while
}

void invert_ball(byte cube[64], int speed) {
    int i, j, k;

    // create ball
    clear_cube(cube);

    // for(i = 0; i < 64; i++) cube[i] = 255;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            for (k = 0; k < 8; k++) {
                // if((i+j+k) % 2) cube[8*k+j] |= (1 << i);
                if (((i - 3.5) * (i - 3.5) + (j - 3.5) * (j - 3.5) + (k - 3.5) * (k - 3.5)) <= 4 * 4) cube[8 * k + j] |= (1 << i);  // min dist from S to wall is 3.57071421
            }
        }
    }

    while (1) {
        for (i = 0; i < 64; i++) cube[i] ^= 255;

        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void invert_pattern(byte cube[64], int speed) {
    int i, j, k;

    // create ball
    clear_cube(cube);

    // for(i = 0; i < 64; i++) cube[i] = 255;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            for (k = 0; k < 8; k++) {
                if ((i + j + k) % 2) cube[8 * k + j] |= (1 << i);
                // if( ( (i-3.5)*(i-3.5)+(j-3.5)*(j-3.5)+(k-3.5)*(k-3.5)) <= 4*4) cube[8*k+j] |= (1 << i); //min dist from S to wall is 3.57071421
            }
        }
    }

    while (1) {
        for (i = 0; i < 64; i++) cube[i] ^= 255;

        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
    }
}

void random_up_down_Hana(byte cube[64], int N, int speed) {
    int i, j, k;

    int X_down[N], Y_down[N];  // can do it with N ?????
    int X_up[N], Y_up[N];

    int UP_coll[N], DOWN_coll[N];

    while (1) {
        clear_cube(cube);

        // pick N + N random points on top and bottom
        for (i = 0; i < N; i++) {
            X_down[i] = (int)random(0, 8);
            Y_down[i] = (int)random(0, 8);
            X_up[i] = (int)random(0, 8);
            Y_up[i] = (int)random(0, 8);
        }

        // check which LEDs will collide (repetitions ??)
        for (i = 0; i < N; i++) {
            DOWN_coll[i] = 0;
            UP_coll[i] = 0;
        }

        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (X_down[i] == X_up[j] && Y_down[i] == Y_up[j]) {
                    DOWN_coll[i] = 1;
                    UP_coll[j] = 1;
                }
            }
        }

        // set AND shine AND clear (for each height)
        for (k = 0; k < 8; k++) {
            // set them
            for (i = 0; i < N; i++) {  // down
                if (DOWN_coll[i] && k >= 4) continue;
                write_one_LED(cube, X_down[i], Y_down[i], k, 1);
            }
            for (i = 0; i < N; i++) {  // up
                if (UP_coll[i] && k >= 4) continue;
                write_one_LED(cube, X_up[i], Y_up[i], 7 - k, 1);
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, FLASH_NUM * speed, FLASH_TIME);

            // clear them
            for (i = 0; i < N; i++) {  // can clear all (nothing rests)
                write_one_LED(cube, X_down[i], Y_down[i], k, 0);
                write_one_LED(cube, X_up[i], Y_up[i], 7 - k, 0);
            }

        }  // k
    }
}

// start with N LEDs down, when the sides ends --> add N LEDs down
void random_up_down_ME(byte cube[64], int N, int speed) {
    int i, j, k, m;
    byte buff = 0;

    int dir = 0;  // 0/1 --- down/up

    byte layer_down[8];  // in bits !
    byte layer_up[8];

    for (i = 0; i < 8; i++) {
        layer_down[i] = 0;
        layer_up[i] = 0;
    }

    clear_cube(cube);

    while (1) {
        dir = !dir;

        // add N random points into bottom layer
        for (i = 0; i < N; i++) {
            if (!dir)
                layer_down[(int)random(0, 8)] |= ((byte)1 << (int)random(0, 8));
            else
                layer_up[(int)random(0, 8)] |= ((byte)1 << (int)random(0, 8));
        }

        // set AND shine AND clear (for each height)
        for (j = 0; j < 7; j++) {
            if (dir)
                k = 7 - j;
            else
                k = j;

            // collision?
            if (k == 4) {  //+dir ?????????
                for (i = 0; i < 8; i++) {
                    buff = (layer_down[i] & layer_up[i]);  // collisions
                    layer_down[i] &= ~buff;
                    layer_up[i] &= ~buff;

                    for (m = 0; m < 8; m++) cube[8 * m + i] = buff;  // fill the ccolumn !!
                }
            }

            // set them
            for (i = 0; i < 8; i++) {
                cube[8 * k + i] |= layer_down[i];
                cube[8 * (7 - k) + i] |= layer_up[i];
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, FLASH_NUM * speed, FLASH_TIME);

            // clear them
            clear_cube(cube);

        }  // k
    }
}

void celtralWave(byte cube[64], int speed) {
    int i, j, k;
    int height;
    int n, period = 7 + 6 - 1, increment = 1;
    int r_sq;

    int layerHeights[64], start_layerHeights[64];
    // for(i=0; i<64; i++) layerHeights[i] = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            r_sq = (i - 3.5) * (i - 3.5) + (j - 3.5) * (j - 3.5);  // of the current LED

            //           if(r_sq < 5*5) start_layerHeights[8*i+j] = 0;
            //           if(r_sq < 4*4) start_layerHeights[8*i+j] = -1;
            //           if(r_sq < 3*3) start_layerHeights[8*i+j] = -2;
            //           if(r_sq < 2*2) start_layerHeights[8*i+j] = -3;
            //           if(r_sq < 1*1) start_layerHeights[8*i+j] = -4;
            if (r_sq < 5 * 5) start_layerHeights[8 * i + j] = 0;
            if (r_sq < 4.5 * 4.5) start_layerHeights[8 * i + j] = -1;
            if (r_sq < 4 * 4) start_layerHeights[8 * i + j] = -2;
            if (r_sq < 3.5 * 3.5) start_layerHeights[8 * i + j] = -3;
            if (r_sq < 2.5 * 2.5) start_layerHeights[8 * i + j] = -4;
            if (r_sq < 1.5 * 1.5) start_layerHeights[8 * i + j] = -5;
            if (r_sq < 1 * 1) start_layerHeights[8 * i + j] = -6;
        }
    }

    while (1) {
        // UP
        clear_cube(cube);
        for (i = 0; i < 64; i++) layerHeights[i] = start_layerHeights[i];
        increment = 1;
        for (n = 0; n < period; n++) {
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    layerHeights[8 * i + j] += increment;
                }
            }

            // update LED cube
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    // OLD -- turn OFF
                    height = layerHeights[8 * i + j] - increment;
                    height = MIN(height, 7);
                    height = MAX(height, 0);
                    write_one_LED(cube, i, j, height, 0);
                    // write_one_LED(cube, height, j,  i, 0);

                    // NEW -- turn ON
                    height = layerHeights[8 * i + j];
                    height = MIN(height, 7);
                    height = MAX(height, 0);
                    write_one_LED(cube, i, j, height, 1);
                    // write_one_LED(cube, height, j,  i, 1);
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, FLASH_NUM * speed, FLASH_TIME);
        }

        // DOWN
        clear_cube(cube);
        for (i = 0; i < 64; i++) layerHeights[i] = 7 - start_layerHeights[i];
        increment = -1;
        for (n = 0; n < period; n++) {
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    layerHeights[8 * i + j] += increment;
                }
            }

            // update LED cube
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    // OLD -- turn OFF
                    height = layerHeights[8 * i + j] - increment;
                    height = MIN(height, 7);
                    height = MAX(height, 0);
                    write_one_LED(cube, i, j, height, 0);
                    // write_one_LED(cube, height, j,  i, 0);

                    // NEW -- turn ON
                    height = layerHeights[8 * i + j];
                    height = MIN(height, 7);
                    height = MAX(height, 0);
                    write_one_LED(cube, i, j, height, 1);
                    // write_one_LED(cube, height, j,  i, 1);
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, FLASH_NUM * speed, FLASH_TIME);
        }
    }
}

void celtralSpiral(byte cube[64], int speed) {
    int i, j, k;
    Point points[4];  // leading will be 0th
    int act_side = 0;
    int dir = 1;  // +1/-1

    // init
    clear_cube(cube);
    for (i = 0; i < 4; i++) {
        points[0].i = 0;
        points[0].j = 0;
        points[0].k = 0;
    }

    // loop
    while (1) {
        // compute new points
        write_one_LED(cube, points[3].i, points[3].j, points[3].k, 0);  // delete old
        // points[4] = points[3];
        points[3] = points[2];
        points[2] = points[1];
        points[1] = points[0];

        // corners
        if (points[0].i == points[0].k && points[0].i <= 3) {  // left-down

            if (act_side == 0) {
                points[0].k += 1;
            }

            else {
                // already at the back/front side -- change direction !!!!
                if ((points[0].j == 7 && dir == 1) || (points[0].j == 0 && dir == -1)) {
                    dir *= -1;
                }

                // change layer in j !!
                if (points[0].j <= 3) {  // front half
                    points[0].i += dir;
                    points[0].j += dir;
                    points[0].k += dir;
                } else {  // back half
                    points[0].i -= dir;
                    points[0].j += dir;
                    points[0].k -= dir;
                }

                act_side = 0;  // I know I switched j layer !!
            }

        } else if (points[0].i == 7 - points[0].k && points[0].i <= 3) {  // left-up
            points[0].i += 1;
            act_side = 1;
        } else if (points[0].i == points[0].k && points[0].i >= 4) {  // right-up
            points[0].k -= 1;
            act_side = 2;
        } else if (points[0].i == 7 - points[0].k && points[0].i >= 4) {  // left-up
            points[0].i -= 1;
            act_side = 3;
        }

        // only edges
        else if (act_side == 0) {  // left
            points[0].k += 1;
        } else if (act_side == 1) {  // up
            points[0].i += 1;
        } else if (act_side == 2) {  // right
            points[0].k -= 1;
        } else if (act_side == 3) {  // down
            points[0].i -= 1;
        }

        // write points[0] into the cube
        write_one_LED(cube, points[0].i, points[0].j, points[0].k, 1);

        // flash
        if (interrupt_processing()) return;  // JUMP OUT !!
        drawCube(cube, FLASH_NUM * speed, FLASH_TIME);
    }
}

void push_sin(byte cube[64], int speed) {
    int k;
    int i;

    int Nsamples = 28;
    //int samples[28] = {0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 6, 7, 7, 7, 7, 7, 6, 6, 5, 4, 4, 3, 2, 1, 1, 0, 0};  //1x freq -- generated in python sin_values.py
    int samples[28] = {0, 0, 1, 3, 4, 6, 7, 7, 7, 6, 4, 3, 1, 0, 0, 0, 1, 3, 4, 6, 7, 7, 7, 6, 4, 3, 1, 0}; //2x freq -- generated in python sin_values.py
    //int samples[28] = {0, 1, 4, 7, 7, 4, 1, 0, 1, 4, 7, 7, 4, 1, 0, 1, 4, 7, 7, 4, 1, 0, 1, 4, 7, 7, 4, 1}; //4x freq -- generated in python sin_values.py

    clear_cube(cube);
    while (1) {
        for (i = 0; i < Nsamples; i++) {
            k = samples[i];

            // move all
            move_around_left_all(cube, 4);

            // add new column to my position (i==0, j==0)
            //k = 
            write_one_LED(cube, 0, 0, k, 1);

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }
    }
    
}

void push_sin_pause(byte cube[64], int speed) {
    int k;
    int i;

    int Nsamples = 28;
    int samples[28] = {0, 0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 6, 7, 7, 7, 7, 7, 6, 6, 5, 4, 4, 3, 2, 1, 1, 0, 0};  // generated in python sin_values.py

    clear_cube(cube);
    while (1) {
        for (i = 0; i < Nsamples*2-1; i++) { //-1 nicer end
            k = samples[i%Nsamples];

            // move all
            move_around_left_all(cube, 4);

            // add new column to my position (i==0, j==0)
            //k = 
            if(i<Nsamples){
                write_one_LED(cube, 0, 0, k, 1);
            }
            else{
              write_one_LED(cube, 0, 0, k, 0); //!
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }
    }
    
}

void squeeze_object(byte cube[64], int speed, int flash_in_rest) {
    int i, j, k;

    byte image[64];
    clear_cube(image);

//    //RANDOM
//    for(i=0;i<64;i++){
//        image[i] = (byte)random(0, 256);
//    }

    //BALL
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            for (k = 0; k < 8; k++) {
                //if ((i + j + k) % 2) image[8 * k + j] |= (1 << i);
                if( ( (i-3.5)*(i-3.5)+(j-3.5)*(j-3.5)+(k-3.5)*(k-3.5)) <= 4*4) image[8*k+j] |= (1 << i); //min dist from S to wall is 3.57071421
            }
        }
    }


    clear_cube(cube);
    while(1){

        //copy image to the cube
        for(i=0;i<64;i++){
            cube[i] = image[i];
        }

        // flash in rest
        for(i=0; i<flash_in_rest; i++){
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        //move the image RIGHT by i + flash
        for(i=0; i<8; i++){
            clear_cube(cube);

            //copy moved + add covered
            for(j=0; j<64; j++){
                //copy moved image
                cube[j] = (image[j] << i); //move right

                //add unseen LEDs to the last LED
                for(k=8-i; k<8;k++){
                    if((image[j] & (1<<k)) != 0){
                        cube[j] |= 0x80;
                        break; //k
                    }
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }

        // flash in rest
        for(i=0; i<flash_in_rest; i++){
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        //move the image from RIGHT by i + flash
        for(i=7; i>=0; i--){
            clear_cube(cube);

            //copy moved + add covered
            for(j=0; j<64; j++){
                //copy moved image
                cube[j] = (image[j] << i); //move right

                //add unseen LEDs to the last LED
                for(k=8-i; k<8;k++){
                    if((image[j] & (1<<k)) != 0){
                        cube[j] |= 0x80;
                        break; //k
                    }
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }

        // flash in rest
        for(i=0; i<flash_in_rest; i++){
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        //move the image LEFT by i + flash
        for(i=0; i<8; i++){
            clear_cube(cube);

            //copy moved + add covered
            for(j=0; j<64; j++){
                //copy moved image
                cube[j] = (image[j] >> i); //move right

                //add unseen LEDs to the last LED
                for(k=0; k<i;k++){
                    if((image[j] & (1<<k)) != 0){
                        cube[j] |= 0x01;
                        break; //k
                    }
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }

        // flash in rest
        for(i=0; i<flash_in_rest; i++){
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);
        }

        //move the image from LEFT by i + flash
        for(i=7; i>=0; i--){
            clear_cube(cube);

            //copy moved + add covered
            for(j=0; j<64; j++){
                //copy moved image
                cube[j] = (image[j] >> i); //move right

                //add unseen LEDs to the last LED
                for(k=0; k<i;k++){
                    if((image[j] & (1<<k)) != 0){
                        cube[j] |= 0x01;
                        break; //k
                    }
                }
            }

            // flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }


    } //while


}

void droplet(byte cube[64], int speed){
    int i, j, k;
    int r;
    int X, Y;

    clear_cube(cube);

    while(1){

        //generate
        X = (int)random(0, 8);
        Y = (int)random(0, 8);

        //falling
        for(i = 7; i>= 1; i--){
            clear_cube(cube);
            write_one_LED(cube, X, Y, i, 1);

            //flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }


        //ground - radius r
        k = 0;
        for(r = 0; r < 5; r++){
            clear_cube(cube);

            //fill ring
            for(i = 0; i<8; i++) for(j = 0; j<8; j++){
                if( (((i-X)*(i-X)+(j-Y)*(j-Y)) <= r*r + 1) && (((i-X)*(i-X)+(j-Y)*(j-Y)) >= r*r - 1) ) cube[8*k+j] |= (1 << i); //min dist from S to wall is 3.57071421
            }

            //flash
            if (interrupt_processing()) return;  // JUMP OUT !!
            drawCube(cube, speed * FLASH_NUM, FLASH_TIME);

        }

            


    }



}
