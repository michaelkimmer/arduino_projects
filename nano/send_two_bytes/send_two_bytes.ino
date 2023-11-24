


// define
#define SERIAL_BAUD 115200

// define PINs
#define PIN_data 16
#define PIN_CLK 17
#define PIN_nRESET 19

// define FCNs
void write_2bytes(byte msg[2]);

//////////////////////////////////////////////////////////////////////////////////////////////////////  SETUP  //////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    // set pins
    pinMode(PIN_data, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_nRESET, OUTPUT);

    digitalWrite(PIN_data, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_nRESET, HIGH);


    // Serial to PC
    Serial.begin(SERIAL_BAUD);
    delay(1000);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////  LOOP  //////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    String receivedData;
    char charBuf[50];
    byte msg[2] = {0, 0};

    // Anything received in serial
    if (Serial.available() > 0)
    {
        // receive string
        receivedData = Serial.readString(); // read until timeout
        receivedData.trim();                       // remove any \r \n whitespace at the end of the String
        receivedData.toCharArray(charBuf, 50);

        // process data !!
        sscanf(charBuf, "%d %d", &msg[0], &msg[1]);



        // send 2 bytes
        write_2bytes(msg);

        // Confirm back to PC
        Serial.print("Processed: ");
        Serial.println(receivedData);
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////  FCNs  //////////////////////////////////////////////////////////////////////////////////////////////////////

void write_2bytes(byte msg[2])
{
    digitalWrite(PIN_nRESET, HIGH);
    shiftOut(PIN_data, PIN_CLK, MSBFIRST, msg[0]);
    shiftOut(PIN_data, PIN_CLK, MSBFIRST, msg[1]);
    digitalWrite(PIN_nRESET, HIGH);
}
