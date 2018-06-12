#include <Arduino.h>
// http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <AccelStepper.h>
// https://github.com/kroimon/Arduino-SerialCommand
#include <SerialCommand.h>

#define motorStepPin 4
#define motorDirPin 5
#define motorOnOffPin 16
#define switchOffPin 14

#define defaultSpeed 6400
#define defaultStartSpeed 1600
#define defaultAcceleration 1600

AccelStepper motor(AccelStepper::DRIVER, motorStepPin, motorDirPin);
SerialCommand sCmd;

bool isRotating = false;


// ---------------- params -----------------

int getParam() {
  char *arg = sCmd.next();
  if (arg == NULL) {
    return 0;
  }

  return atoi(arg);
}

void setNextParamAsMaxSpeed() {
  int value = getParam();
  Serial.print("motor.setMaxSpeed: ");
  Serial.println(value);
  motor.setMaxSpeed(value);
}

// void setNextParamAsStartSpeed() {
//   int value = getParam();
//   Serial.print("motor.setPullInSpeed: ");
//   Serial.println(value);
//   motor.setPullInSpeed(value);
// }

void setNextParamAsAcceleration() {
  int value = getParam();
  Serial.print("motor.setAcceleration: ");
  Serial.println(value);
  motor.setAcceleration(value);
}

void setDefaultParams() {
  motor.setMaxSpeed(defaultSpeed);
//   motor.setPullInSpeed(defaultStartSpeed);
  motor.setAcceleration(defaultAcceleration);
}

// ----------------- Helpers -----------------

void motorOn() {
  Serial.println("turn motor on");
  digitalWrite(motorOnOffPin, HIGH);
}

void motorOff() {
  // Serial.println("turn motor off");
  // digitalWrite(motorOnOffPin, LOW);
}

// ----------------- Commands -----------------

void unrecognizedCommand(const char *command) {
  Serial.print("- unrecognizedCommand: ");
  Serial.println(command);
}

void setSettings() {
  Serial.println("- setSettings");  
  setNextParamAsMaxSpeed();
//   setNextParamAsStartSpeed();
  setNextParamAsAcceleration();
}

void startRotation() {
  Serial.println("- startRotation");
  Serial.println("motor.run: 999999");
  motor.moveTo(999999);
}

void stopRotation() {
  Serial.println("- stopRotation");
  Serial.println("motor.stop");
  motor.stop();
}

void rotateSteps() {
  Serial.println("- rotateSteps");

  int steps = getParam();
  Serial.print("motor.moveTo: ");
  Serial.println(steps);
  motor.moveTo(steps);
}

// ----------------- Loop Helpers -----------------

void checkSwitch() {
  // // read the state of the switch button
  if(digitalRead(switchOffPin) == HIGH && motor.isRunning()) {
    Serial.println("Switch was triggered, motor was on");
  } 

  Serial.println("checkSwitch");

  Serial.print(" 14 ");
  Serial.print(digitalRead(14));
  Serial.print(" ");

}

void rotateIfNeeded() {
  if (motor.distanceToGo() != 0) {
    motor.run();
    isRotating = true;
    Serial.println(motor.distanceToGo());
  } else if(isRotating) {
    isRotating = false
    Serial.println("- Done running. Motor stops");
  }
}

// ----------------- Setup -----------------

void setup(){
  Serial.begin(9600);
  Serial.println("- Setup start");

  pinMode(motorStepPin, OUTPUT);

  sCmd.addCommand("SET_SETTINGS", setSettings);
  sCmd.addCommand("START_ROTATION", startRotation);
  sCmd.addCommand("STOP_ROTATION", stopRotation);
  sCmd.addCommand("ROTATE_X_STEPS", rotateSteps);
  sCmd.setDefaultHandler(unrecognizedCommand);

  setDefaultParams();
  motorOn();

  Serial.println("- Setup end");
}

void loop() {
  // check for new commands the whole time
  sCmd.readSerial();

  // checkSwitch();
  rotateIfNeeded();
}
