#include<SPI.h>

byte bytes[100];
uint32_t cnt = 0;

volatile boolean received;
volatile byte Slavereceived;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("");
  Serial.println("SPI dummy slave start");

  // Sets MISO as OUTPUT
  // Have to Send data to Master IN
  pinMode(MISO, OUTPUT);

  //Turn on SPI in Slave Mode
  SPCR |= _BV(SPE);

  //Interrupt ON is set for SPI commnucation
  received = false;
  SPI.attachInterrupt();
}

ISR (SPI_STC_vect) {
  // Value received from master 
  // if store in variable slavereceived
  Slavereceived = SPDR;
  // Sets received as True
  received = true;
}

void loop() {
  if (received) {
    bytes[cnt++] = Slavereceived;
    received= false;
    if (cnt == 91) {
      dump_byte_array(bytes, cnt);
    }
  }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    if (i % 16 == 15) {
      Serial.println("");
    } else {
      Serial.print(" ");
    }
  }
}
