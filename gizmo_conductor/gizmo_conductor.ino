#include <Servo.h>
#include <Wire.h>
#include <MIDI.h>

#define arraySize 9

int arduino_id = 1;

MIDI_CREATE_DEFAULT_INSTANCE();

int throttle = 0;
Servo drone;

const int stepPin = 3; 
const int dirPin = 4; 
int currentStep = 0, deltaStep = 0;
int timeNow, timeStep;
bool pulse = true;

const int pitchArray[arraySize] = {336, 343, 345, 364, 398, 418, 438, 482, 517};
const int playerArray[arraySize] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int stepArray[arraySize] = {600, 600, 600, 550, 400, 350, 300, 200, 0};
const int throttleArray[arraySize] = {2000, 1900, 1800, 1600, 1400, 1300, 1200, 1100, 1000};

void setup() {
  // put your setup code here, to run once:
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(listen_midi);
  
  drone.attach(9);
  drone.writeMicroseconds(throttle);
  
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  timeNow = millis();
  timeStep = timeNow + 1;

  Wire.begin(arduino_id);
  
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  MIDI.read();
  play_note();
  if (Serial.available()) {
    listen_serial();
  }
}

void listen_midi(byte channel, byte pitch, byte velocity) {
  check_table(pitch);  
}

void listen_serial() {
  String command = Serial.readString();
  Serial.println(command);
  int first = command.indexOf(',');
  int second = command.indexOf(',',first+1);
  int player_id = command.substring(0,first).toInt();
  int targetStep = command.substring(first+1,second).toInt();
  int throttleValue = command.substring(second+1).toInt();
  process_note(player_id, targetStep, throttleValue);
}

void check_table(int pitch) {
  int index;
  for (int x = 0; x < arraySize; x++) {
    if (pitchArray[x] == pitch) {
      index = x;
      break;  
    }
  }
  
  int player_id = playerArray[index];
  int targetStep = stepArray[index];
  int throttleValue = throttleArray[index];

  process_note(player_id, targetStep, throttleValue);
}

void process_note(int player_id, int targetStep, int throttleValue) {
  if (player_id == arduino_id) {
    deltaStep = targetStep - currentStep;
    throttle = throttleValue; 
  } else {
    send_event(player_id, targetStep, throttleValue);  
  }
}


void send_event(int player_id, int targetStep, int throttleValue) {
  Wire.beginTransmission(player_id);
  Wire.write(String(targetStep).c_str());
  Wire.write(",");
  Wire.write(String(throttleValue).c_str());
  Wire.endTransmission();
}

void play_note() {
  change_throttle();
  change_step();
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
