/*
  TestHarness.pde - Tests the basic functions of the WH2 sensor library
  Decodes received packets and writes a summary of each packet to the Arduino's
  serial port
  Created by Luc Small on 30 April 2012.
  Released into the public domain.
*/

#include <WeatherSensorWH2.h>

#define RF_IN 2
#define LED_PACKET A2
#define LED_ACTIVITY  A1

volatile byte got_interval = 0;
volatile byte interval = 0;

volatile unsigned long old = 0, packet_count = 0; 
volatile unsigned long spacing, now, average_interval;

WeatherSensorWH2 weather;

ISR(TIMER1_COMPA_vect)
{
  static byte count = 0;
  static byte was_hi = 0;
  
  
  if (digitalRead(RF_IN) == HIGH) {
    digitalWrite(LED_ACTIVITY, HIGH);
    count++;
    was_hi = 1; 
  } else {
    digitalWrite(LED_ACTIVITY, LOW);
    if (was_hi) {
      was_hi = 0;
      interval = count;
      got_interval = 1;
      count = 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Weather Sensor WH2 Test Harness");
  
  pinMode(LED_PACKET, OUTPUT);
  pinMode(LED_ACTIVITY, OUTPUT);
  pinMode(RF_IN, INPUT);

  TCCR1A = 0
//    | (1 << COM1A1) //compare output mode for channel A
//    | (1 << COM1A0)
//    | (1 << COM1B1) //compare output mode for channel B
//    | (1 << COM1B0)
//    | (1 << WGM11)  // waveform generation mode. 0000 = normal,
//    | (1 << WGM10)  // 0001 = PWM phase correct 8bit
      ;

  TCCR1B = 0
//    | (1 << ICNC1)  // input capture noise canceler
//    | (1 << ICES1)  // input capture edge select, 1 = rising edge
//    | (1 << WGM13)  // waveform generation mode.
      | (1 << WGM12)
//    | (1 << CS12)   // 000 = stopped, 001 = clk/1,
//    | (1 << CS11)   // 010 = clk/8, 011 = clk/64
      | (1 << CS10)   // 100 = clk/256, 101 = clk/1024
      ;               // 110 = Clock source on T1, clock on falling edge
                      // 111 = Clock source on T1, clock on rising edge

  TCCR1C = 0
//    | (1 << FOC1A)  // force output compare for channel A
//    | (1 << FOC1B)  // force output compare for channel B
      ;


  OCR1A = 399; // 0...399, 0...399 -> Timer triggers every 400 cycles (400 / 16 MHz = 25 uS)

  TIMSK1 = TIMSK1                 // don't throw away the old value
//    | (1 << ICIE1)  // Counter 1 input capture interrupt enable
//    | (1 << OCIE1B) // output compare b match interrupt enable
      | (1 << OCIE1A) // output compare a match interrupt enable
//    | (1 << TOIE1)  // counter1 overflow interrupt enable
      ;

// TIFR1 = TIFR1                   // not sure whether inactive is 0 or 1
//    | (1 << ICF1)   // counter1 input capture flag
//    | (1 << OCF1B)  // output compare b match flag
//    | (1 << OCF1A)  // output compare a match flag
//    | (1 << TOV1)   // counter 1 overflow flag
//    ;

// GTCCR = GTCCR
//    | (1 << TSM)    // counter synchronization mode.
//    | (1 << PSRSYNC) // prescaler reset
//    ;

 
  /* enable interrupts */
  sei();
}

void loop() {
  byte i;
  byte *packet;
  byte *min_interval;
  byte *max_interval;
  unsigned int *mean_interval;

  if (got_interval) {
    weather.accept(interval);  
    if (weather.acquired()) {
      now = millis();
      spacing = now - old;
      old = now;
      packet_count++;
      average_interval = now / packet_count;     
   
      Serial.print("Spacing: ");
      Serial.println(spacing, DEC);
      Serial.print("Packet count: ");
      Serial.println(packet_count, DEC);
    
      Serial.print("Average spacing: ");
      Serial.println(average_interval, DEC);
  
      min_interval = weather.get_min_interval();
      max_interval = weather.get_max_interval();
      mean_interval = weather.get_mean_interval();
  
      Serial.print("Pulse stats: Hi: ");
      Serial.print(min_interval[1], DEC);
      Serial.print(" - ");
      Serial.print(max_interval[1], DEC);
      Serial.print(" (~");
      Serial.print(mean_interval[1], DEC);
      Serial.print(" Mean), Lo: ");
      Serial.print(min_interval[0], DEC);
      Serial.print(" - ");
      Serial.print(max_interval[0], DEC);
      Serial.print(" (~");
      Serial.print(mean_interval[0], DEC);
      Serial.println(" Mean)");

      // flash green led to say got packet
      digitalWrite(LED_PACKET, HIGH);
      delay(100);
      digitalWrite(LED_PACKET, LOW);
       
      packet = weather.get_packet();
      for(i=0;i<5;i++) {
        Serial.print("0x");
        Serial.print(packet[i], HEX);
        Serial.print("/");
        Serial.print(packet[i], DEC);
        Serial.print(" ");
      }  
      Serial.print("crc: ");
      Serial.print(weather.calculate_crc(), HEX);
      Serial.println((weather.valid() ? " OK" : " BAD"));
  
      Serial.print("Sensor ID: 0x");
      Serial.println(weather.get_sensor_id(), HEX);

      Serial.print("Humidity: ");
      Serial.print(weather.get_humidity(), DEC);
      Serial.println("%");
      
      Serial.print("Temperature: ");
      Serial.print(weather.get_temperature_formatted());
      Serial.print(" C  [");
      Serial.print(weather.get_temperature(), DEC);
      Serial.println("]");
      Serial.println("--------------");
   }
   
   got_interval = 0; 
  }
  
}









