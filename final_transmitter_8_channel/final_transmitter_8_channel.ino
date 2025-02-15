  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>

  const uint64_t pipeOut = 0xE9E8F0F0E1LL;  
  RF24 radio(9, 10); 

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

  // Adaptive throttle calibration variables
  int throttleMin = 1023;  // Start with max possible value
  int throttleMax = 0;      // Start with min possible value
  int throttleMid = 512;    // Default midpoint

  void ResetData() {
    data.throttle = 0;  
    data.yaw = 127;
    data.pitch = 127;   
    data.roll = 127;    
    data.aux1 = 0;    
    data.aux2 = 127;
    data.aux3 = 0;  // New digital channel reset
    data.aux4 = 127;  // New analog channel reset
  }

  void calibrateThrottle() {
    int rawValue = analogRead(A0);

    // Update min and max dynamically
    if (rawValue < throttleMin) throttleMin = rawValue;
    if (rawValue > throttleMax) throttleMax = rawValue;

    // Update the midpoint
    throttleMid = (throttleMin + throttleMax) / 2;
  }

  int getMappedThrottle() {
    int throttleRaw = analogRead(A0);
    calibrateThrottle();  // Continuously refine min/max

    if (throttleRaw < throttleMid) {
      return 0;  // Below mid, set throttle to zero
    } else {
      return map(throttleRaw, throttleMid, throttleMax, 0, 255);
    }
  }

  int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse) {
    val = constrain(val, lower, upper);
    if (val < middle)
      val = map(val, lower, middle, 0, 128);
    else
      val = map(val, middle, upper, 128, 255);
    return (reverse ? 255 - val : val);
  }

  void setup() {
    radio.begin();
    radio.stopListening();  
    radio.openWritingPipe(pipeOut);  
    // radio.setAutoAck(false);  // Enable Auto Acknowledgment
    radio.setCRCLength(RF24_CRC_16);  // Set CRC to 16-bit for better error checking
    radio.setDataRate(RF24_1MBPS);  // Use 1MBps for better reliability
    radio.setPALevel(RF24_PA_LOW); // Increase power for a stronger signal
    radio.setChannel(108);  // Use a clean channel
    ResetData();
    Serial.begin(115200);
    delay(1000);
  }

  void loop() {
    // Adaptive throttle mapping
    data.throttle = getMappedThrottle();  

    // Map joystick values for yaw, pitch, and roll
    data.yaw = mapJoystickValues(analogRead(A1), 12, 524, 1020, false);  
    data.roll = mapJoystickValues(analogRead(A3), 12, 524, 1020, true);  
    data.pitch = mapJoystickValues(analogRead(A2), 12, 524, 1020, false);  

    // Read switch and potentiometer
    data.aux1 = digitalRead(2);  
    data.aux2 = mapJoystickValues(analogRead(A7), 12, 524, 1020, true);  

    // Read new digital and analog channels
    data.aux3 = digitalRead(4);  // New digital channel
    data.aux4 = mapJoystickValues(analogRead(A6), 12, 524, 1020, false);  // New analog channel

    // Send data and check if transmission was successful
    bool success = radio.write(&data, sizeof(Signal));
    if (success) {
      Serial.println("Data Sent Successfully");
    } else {
      Serial.println("Send Failed");
    }

    // Serial output for debugging
    if (success) {
      Serial.print("Throttle: "); Serial.print(data.throttle);
      Serial.print(" Yaw: "); Serial.print(data.yaw);
      Serial.print(" Roll: "); Serial.print(data.roll);
      Serial.print(" Pitch: "); Serial.print(data.pitch);
      Serial.print(" Aux1: "); Serial.print(data.aux1);
      Serial.print(" Aux2: "); Serial.print(data.aux2);
      Serial.print(" Aux3: "); Serial.print(data.aux3);  // Print new digital channel
      Serial.print(" Aux4: "); Serial.println(data.aux4);  // Print new analog channel
    }
  }
