/*
  Arduino LSM9DS1 - Simple Magnetometer

  This example reads the magnetic field values from the LSM9DS1
  sensor and continuously prints them to the Serial Monitor
  or Serial Plotter.

  The circuit:
  - Arduino Nano 33 BLE Sense

  created 10 Jul 2019
  by Riccardo Rizzo

  This example code is in the public domain.
*/

// jeste by slo potom najit poc. uhel podle dat z magnetometru !!!!

#include <Arduino_LSM9DS1.h>
#include <NRF52_MBED_TimerInterrupt.h>

#define TIMER_DURATION_us 10000  // 100Hz -- TIMER_DURATION = 10ms = 10000 us
#define TIMER_DURATION_s 0.01
#define g_const 9.80665
#define rad_const M_PI / 180

// global variables

volatile int interrupt_flag_3;

double omega_x = 0, omega_y = 0, omega_z = 0;
double omega_x_offset = 0, omega_y_offset = 0, omega_z_offset = 0;

double phi_x = 0, phi_y = 0, phi_z = 0;

double acc_x = 0, acc_y = 0, acc_z = 0;  // original in arduino orientation
double acc_x_offset = 0, acc_y_offset = 0, acc_z_offset = 0;

double a_x = 0, a_y = 0, a_z = 0;
double v_x = 0, v_y = 0, v_z = 0;
double x = 0, y = 0, z = 0;

// double m_x, m_y, m_z;

// Init NRF52 timer NRF_TIMER3
NRF52_MBED_Timer ITimer3(NRF_TIMER_3);

void TimerHandler3()  // timer interrupt -- 100Hz
{
    interrupt_flag_3 += 1;  // just set flag (with number)
}

void setup() {
    // init pins
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDB, HIGH);

    // init serial
    Serial.begin(115200);
    while (!Serial);  // start whe the terminal is opened !!!!!!!
    Serial.println("Started serial");

    // init IMU
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    } else {
        delay(2000); //2 seconds? !!!!!!!!!!!!

        Serial.print("Accelerometer sample rate = ");
        Serial.print(IMU.accelerationSampleRate());
        Serial.println("Hz");
        Serial.println();

        Serial.print("Gyroscope sample rate = ");  
        Serial.print(IMU.gyroscopeSampleRate());
        Serial.println("Hz");
        Serial.println();
  
        Serial.println(("Initialized IMU"));
    }

    // init timer3
    if (ITimer3.attachInterruptInterval(TIMER_DURATION_us, TimerHandler3)) {
        Serial.print(("Starting ITimer3 OK, millis() = "));
        Serial.println(millis());
    } else {
        Serial.println(("Can't set ITimer3. Select another freq. or timer"));
    }


    // read data -- MEASURE INITIAL OFFSET --- initial measurement weird??? !!!
    int m;
    int is_ok;
    int sum_count = 0;

    float buff_acc_x, buff_acc_y, buff_acc_z;
    float buff_omega_x, buff_omega_y, buff_omega_z;
    
    for (m = 0; m < 100; m++) {
        while (interrupt_flag_3 == 0);
            
        if (IMU.accelerationAvailable()) {                                           // 104 Hz. //jen zdrzeni? !!!
            is_ok = IMU.readAcceleration(buff_acc_x, buff_acc_y, buff_acc_z);  // returns in G
        }
        if (IMU.gyroscopeAvailable()) {                                                 // 104 Hz. //nebo 119 ?? !!!
            is_ok = IMU.readGyroscope(buff_omega_x, buff_omega_y, buff_omega_z);  // returns in deg/s
        }
        interrupt_flag_3 = 0;
    }

    // offset measurement
    for (m = 0; m < 100; m++) {
        while (interrupt_flag_3 == 0);

        // delay(10);
        if (IMU.accelerationAvailable()) {                      // 104 Hz. //jen zdrzeni? !!!
            is_ok = IMU.readAcceleration(buff_acc_x, buff_acc_y, buff_acc_z);  // returns in G
            if (!is_ok) digitalWrite(LEDR, LOW);
            acc_x = buff_acc_x;
            acc_y = buff_acc_y;
            acc_z = buff_acc_z;
        } else
            digitalWrite(LEDR, LOW);
        if (IMU.gyroscopeAvailable()) {                            // 104 Hz. //nebo 119 ?? !!!
            is_ok = IMU.readGyroscope(buff_omega_x, buff_omega_y, buff_omega_z);  // returns in deg/s
            if (!is_ok) digitalWrite(LEDB, LOW);
            omega_x = buff_omega_x;
            omega_y = buff_omega_y;
            omega_z = buff_omega_z;
        } else
            digitalWrite(LEDB, LOW);
        interrupt_flag_3 = 0;

        acc_x_offset += acc_x;
        acc_y_offset += acc_y;
        acc_z_offset += acc_z;
        omega_x_offset += omega_x;
        omega_y_offset += omega_y;
        omega_z_offset += omega_z;

        sum_count += 1;
    }

    acc_x_offset /= sum_count;
    acc_y_offset /= sum_count;
    acc_z_offset /= sum_count;
    omega_x_offset /= sum_count;
    omega_y_offset /= sum_count;
    omega_z_offset /= sum_count;

        //print offsets
        Serial.print("OFFSETS: (a_x a_y a_z) = (");
        Serial.print(acc_x_offset);
        Serial.print('\t');
        Serial.print(acc_y_offset);
        Serial.print('\t');
        Serial.print(acc_z_offset);
        Serial.print("),\t");

        Serial.print("(om_x om_y om_z) = (");
        Serial.print(omega_x_offset);
        Serial.print('\t');
        Serial.print(omega_y_offset);
        Serial.print('\t');
        Serial.print(omega_z_offset);
        Serial.println(")");
    
    
}

void loop() {
          acc_x_offset += 0;
        acc_y_offset += 0;
        acc_z_offset += 0;/////////////////////////////////////////////
        
    double time_passed;
    float buff_acc_x, buff_acc_y, buff_acc_z;
    float buff_omega_x, buff_omega_y, buff_omega_z;

    // read data
    if (interrupt_flag_3 > 0) {
        if (IMU.accelerationAvailable()) {              // 104 Hz. //jen zdrzeni? !!!
            IMU.readAcceleration(buff_acc_x, buff_acc_y, buff_acc_z);  // returns in G
            acc_x = buff_acc_x;
            acc_y = buff_acc_y;
            acc_z = buff_acc_z;
        }
        if (IMU.gyroscopeAvailable()) {                    // 104 Hz. //nebo 119 ?? !!!
            IMU.readGyroscope(buff_omega_x, buff_omega_y, buff_omega_z);  // returns in deg/s
            omega_x = buff_omega_x;
            omega_y = buff_omega_y;
            omega_z = buff_omega_z;
        }

        // time passed?
        //Serial.println(interrupt_flag_3); //!!!!!!!!!!!!!!!!!
        time_passed = TIMER_DURATION_s * interrupt_flag_3;
        interrupt_flag_3 = 0;

        // remove OFFSETS
        omega_x -= omega_x_offset;
        omega_y -= omega_y_offset;
        omega_z -= omega_z_offset;

        acc_x -= acc_x_offset;
        acc_y -= acc_y_offset;
        acc_z -= acc_z_offset;

        // basic units
        omega_x *= rad_const;
        omega_y *= rad_const;
        omega_z *= rad_const;

        acc_x *= g_const;
        acc_y *= g_const;
        acc_z *= g_const;

        // first compute new angle --> more accurate location
        phi_x += omega_x * time_passed;
        phi_y += omega_y * time_passed;
        phi_z += omega_z * time_passed;

        // adjust acceleration orientation by angle !!!!! -- matrix multiplication on paper?
        double c_x, s_x, c_y, s_y, c_z, s_z;
        c_x = cos(-phi_x);
        s_x = sin(-phi_x);
        c_y = cos(-phi_y);
        s_y = sin(-phi_y);
        c_z = cos(-phi_z);
        s_z = sin(-phi_z);

        double A[3][3];  // rotation matrix -- is cos/sin computing long???
        A[0][0] = c_z * c_y;
        A[0][1] = -s_z * c_x + c_z * s_y * s_x;
        A[0][2] = s_z * s_x + c_z * s_y * c_x;
        A[1][0] = s_z * c_y;
        A[1][1] = c_z * c_x + s_z * s_y * s_x;
        A[1][2] = -c_z * s_x + s_z * s_y * c_x;
        A[2][0] = -s_y;
        A[2][1] = c_y * s_x;
        A[2][2] = c_y * c_x;

        /*
        a_x = A[0][0]*acc_x + A[0][1]*acc_y + A[0][2]*acc_z;
        a_y = A[1][0]*acc_x + A[1][1]*acc_y + A[1][2]*acc_z;
        a_z = A[2][0]*acc_x + A[2][1]*acc_y + A[2][2]*acc_z;
      */
        a_x = acc_x;
        a_y = acc_y;
        a_z = acc_z;

        // compute velocity
        v_x += a_x * time_passed;
        v_y += a_y * time_passed;
        v_z += a_z * time_passed;

        // compute location
        x += v_x * time_passed;
        y += v_y * time_passed;
        z += v_z * time_passed;

        /*
        // print data
        Serial.print("(x y z) = (");
        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.print(z);
        Serial.print("),\t");

        Serial.print("(v_x v_y v_z) = (");
        Serial.print(v_x);
        Serial.print('\t');
        Serial.print(v_y);
        Serial.print('\t');
        Serial.print(v_z);
        Serial.print("),\t");

        Serial.print("(a_x a_y a_z) = (");
        Serial.print(a_x);
        Serial.print('\t');
        Serial.print(a_y);
        Serial.print('\t');
        Serial.print(a_z);
        Serial.print("),\t");

        // print data
        Serial.print("(phi_x phi_y phi_z) = (");
        Serial.print(phi_x);
        Serial.print('\t');
        Serial.print(phi_y);
        Serial.print('\t');
        Serial.print(phi_z);
        Serial.print("),\t");;

        Serial.print("(om_x om_y om_z) = (");
        Serial.print(omega_x);
        Serial.print('\t');
        Serial.print(omega_y);
        Serial.print('\t');
        Serial.print(omega_z);
        Serial.println(")");
        

*/
        //print only data
        Serial.print(a_x);
        Serial.print(' ');
        Serial.print(a_y);
        Serial.print(' ');
        Serial.print(a_z);
        Serial.print(" ");

        Serial.print(omega_x);
        Serial.print(' ');
        Serial.print(omega_y);
        Serial.print(' ');
        Serial.println(omega_z);

    }

    /*
    if (IMU.magneticFieldAvailable()) { //20 Hz
      IMU.readMagneticField(m_x, m_y, m_z);
    }*/

    /*
        digitalWrite(LEDB, LOW);
        delay(500);
        digitalWrite(LEDB, HIGH);
        delay(500);
    */
}
