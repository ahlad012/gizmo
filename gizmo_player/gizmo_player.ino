#include <Servo.h>
#include <Wire.h>

int arduino_id = 4;

int throttle = 0;
Servo drone;

const int stepPin = 3; 
const int dirPin = 4; 
int currentStep = 0, deltaStep = 0;
int timeNow, timeStep;
bool pulse = true;

void setup() {
  // put your setup code here, to run once:
  drone.attach(9);
  drone.writeMicroseconds(throttle);
  
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  timeNow = millis();
  timeStep = timeNow + 1;
  
  Wire.begin(arduino_id);
  Wire.onReceive(receive_event);
  
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  change_step();
}

void receive_event(int howMany) {
  String s;
  while (0 < Wire.available()) {
    char c = Wire.read();
    if (c == ',') {
      deltaStep = s.toInt() - currentStep;
      s = "";
    } else {
      s += c;  
    }
  }
  throttle = s.toInt();
  change_throttle();
}

void change_step() {
  timeNow = millis();
  if (deltaStep != 0 && timeNow >= timeStep) {
    digitalWrite(dirPin, (deltaStep > 0) ? LOW : HIGH);
    digitalWrite(stepPin, (pulse == true) ? HIGH : LOW);
    timeStep = timeNow + 1;
    pulse = !pulse;
    if (pulse == false) {
      if (deltaStep > 0) { 
        deltaStep--;
        currentStep++;
      } else {
        deltaStep++;
        currentStep--;
      }
    }
  }
}

void change_throttle() {
  drone.writeMicroseconds(throttle);
}
