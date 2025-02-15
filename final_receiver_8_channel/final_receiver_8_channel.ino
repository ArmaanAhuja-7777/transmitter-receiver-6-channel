#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_3 = 0;
int ch_width_4 = 0;
int ch_width_5 = 0;
int ch_width_6 = 0;
int ch_width_7 = 0;  // New analog channel width (mapped to pin 0)
int ch_width_8 = 0;  // New digital channel width (mapped to pin 7)

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;
Servo ch6;
Servo ch7;  // Servo for new analog channel (mapped to pin 0)
Servo ch8;  // Servo for new digital channel (mapped to pin 7)

struct Signal {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte aux1;
  byte aux2;
  byte aux3;  // New digital channel
  byte aux4;  // New analog channel
};

Signal data;
const uint64_t pipeIn = 0xE9E8F0F0E1LL;  
RF24 radio(9, 10); 

void ResetData() {
  data.roll = 127;  
  data.pitch = 127;  
  data.throttle = 0;  // Set throttle to 0 for safety
  data.yaw = 127;  
  data.aux1 = 0;  
  data.aux2 = 127;
  data.aux3 = 0;  // New digital channel reset
  data.aux4 = 127;  // New analog channel reset
}

void setup() {
  radio.begin();
  radio.startListening();  
  radio.openReadingPipe(1, pipeIn);  
  // radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  ResetData();
  Serial.begin(115200);
  
  ch1.attach(8);
  ch2.attach(2);
  ch3.attach(3);
  ch4.attach(4);
  ch5.attach(5);
  ch6.attach(6);
  ch7.attach(0);  // Attach servo for new analog channel to pin 0
  ch8.attach(7);  // Attach servo for new digital channel to pin 7
  delay(1000);
}

unsigned long lastRecvTime = 0;

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));
    lastRecvTime = millis();  
  }
}

void loop() {
  recvData();
  
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    ResetData();
  }

  ch_width_1 = map(data.roll, 0, 255, 1000, 2000);     
  ch_width_2 = map(data.pitch, 0, 255, 1000, 2000);     
  ch_width_3 = map(data.throttle, 0, 255, 1000, 2000);     
  ch_width_4 = map(data.yaw, 0, 255, 1000, 2000);        
  ch_width_5 = map(data.aux1, 0, 1, 1000, 2000);     
  ch_width_6 = map(data.aux2, 0, 255, 1000, 2000);
  
  // Map the new channels like other servo channels
  ch_width_7 = map(data.aux4, 0, 255, 1000, 2000);  // New analog channel to pin 0
  ch_width_8 = map(data.aux3, 0, 1, 1000, 2000);  // New digital channel to pin 7

  ch1.writeMicroseconds(ch_width_1);
  ch2.writeMicroseconds(ch_width_2);
  ch3.writeMicroseconds(ch_width_3);
  ch4.writeMicroseconds(ch_width_4);
  ch5.writeMicroseconds(ch_width_5);
  ch6.writeMicroseconds(ch_width_6);
  
  // Control the new servo-compatible channels (mapped to pin 0 and pin 7)
  ch7.writeMicroseconds(ch_width_7);  // New analog channel to pin 0
  ch8.writeMicroseconds(ch_width_8);  // New digital channel to pin 7

//   if (now - lastRecvTime <= 1000) {
//     Serial.print("Throttle: "); Serial.print(data.throttle);
//     Serial.print(" Yaw: "); Serial.print(data.yaw);
//     Serial.print(" Roll: "); Serial.print(data.roll);
//     Serial.print(" Pitch: "); Serial.print(data.pitch);
//     Serial.print(" Aux1: "); Serial.print(data.aux1);
//     Serial.print(" Aux2: "); Serial.print(data.aux2);
//     Serial.print(" Aux3: "); Serial.print(data.aux3);
//     Serial.print(" Aux4: "); Serial.println(data.aux4);
//   }
}
