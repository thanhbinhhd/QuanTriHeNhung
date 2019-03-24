#include<Servo.h> // include server library

void saveCurrentState();
void replay();
void increaseAngle(int ser_index);
void decreaseAngle(int ser_index);
void servo_catch();
void servo_release();

typedef struct HaServo {
  Servo ser;
  int MIN;
  int MAX;
  int angle;
} HaServo;

typedef struct State {
  int angles[4];
} State;

const char TO_LEFT = 'a'; // Spin to left
const char TO_RIGHT = 'd'; // Spin to right
const char TO_UP = 'w'; // lift up
const char TO_DOWN = 's'; // let down
const char TO_FRONT = '8'; // push to front
const char TO_BACK = '2'; // pull to back
const char TO_CATCH = '4'; // Catch something
const char TO_RELEASE = '6'; // Release
const char TO_RECORD = 'r'; // Start/Reset recording
const char TO_REPLAY = 'e'; // Stop recording and start replaying
const char TO_STOP_REPLAYING = 'x'; // Strop replaying and change servo to first state
const char ENTER = 13; // Save current state
const char PAUSE = 'p'; // Pause while replaying
const char CONTINUE = 'c'; // Continue when paused

int val; // initial value of input
int t_step; // step to replay
int g_si; //store current step when pause
int g_i; // store current servo when stop
HaServo motors[4]; // Array to stores info of servo motors
State history[100]; // Store 100 state of arm
bool playing;

void setup() {
  t_step = 0; // init t_step to 0
  playing = 0; // init playing is false
  Serial.begin(9600); // Serial comm begin at 9600bps

  motors[0].ser.attach(9);
  motors[0].MIN = 10;
  motors[0].MAX = 170;
  motors[0].angle = 90;

  motors[1].ser.attach(6);
  motors[1].MIN = 10;
  motors[1].MAX = 170;
  motors[1].angle = 90;

  motors[2].ser.attach(5);
  motors[2].MIN = 10;
  motors[2].MAX = 170;
  motors[2].angle = 90;

  motors[3].ser.attach(3);
  motors[3].MIN = 0;
  motors[3].MAX = 140;
  motors[3].angle = 140;

  for (int i = 0; i < 4; i++) {
    motors[i].ser.write(motors[i].angle); // init angle
  }
  Serial.print("\n\rWellcome to ARM robot Quan tri he nhung!\n\r");
  //Save first state
  saveCurrentState();
}

void loop() {
  if (Serial.available()) // if serial value is available
  {
    val = Serial.read();// then read the serial value
    switch (val) {
      case TO_LEFT:
        increaseAngle(0); break;
      case TO_RIGHT:
        decreaseAngle(0); break;
      case TO_UP:
        increaseAngle(1); break;
      case TO_DOWN:
        decreaseAngle(1); break;
      case TO_FRONT:
        increaseAngle(2); break;
      case TO_BACK:
        decreaseAngle(2); break;
      case TO_CATCH:
        servo_catch(); break;
      case TO_RELEASE:
        servo_release(); break;
      case TO_RECORD:
        t_step = 0;
        saveCurrentState();
        Serial.print("\n\rReset recording...\n"); break;
      case ENTER:
        saveCurrentState();
        Serial.print("\n\rSaveCurrentState, total step: ");
        Serial.print(t_step, "\n"); break;
      case TO_REPLAY:
        Serial.print("\n\rReplaying...");
        playing = true;
        break;
      case TO_STOP_REPLAYING:
        playing = false;
        backToFirstStep();
        break;
      case CONTINUE:
        Serial.print("\n\rContinue");
        playing = true;
        replayFromPause(true);
        break;
      default:
        break;
    }
  }
  if (playing) {
    replayFromPause(false);
  }
}

void increaseAngle(int ser_index) {
  motors[ser_index].angle += (motors[ser_index].angle > motors[ser_index].MAX) ? 0 : 1; //position of servo motor increases by 1 ( anti clockwise) until MAX
  writeServo(ser_index, motors[ser_index].angle); // the servo will move according to position
  delay(10); //delay for the servo to get to the position
}

void decreaseAngle(int ser_index) {
  motors[ser_index].angle -= (motors[ser_index].angle < motors[ser_index].MIN) ? 0 : 1; //position of servo motor decreases by 1 ( anti clockwise) until MIN
  writeServo(ser_index, motors[ser_index].angle); // the servo will move according to position
  delay(10); //delay for the servo to get to the position
}
void servo_catch() { // Catch something
  // Set angle of motors 3 to 0 (close)
  writeServo(3, 0);  // write angle
}

void servo_release() {
  // Set angle of motors 3 to 140 (open)
  writeServo(3, 140); // Write Angle
}
void writeServo(int ser_index, int angle) {
  motors[ser_index].angle = angle;
  motors[ser_index].ser.write(angle);   // Write angle to servo
}

void smoothSpin(int motor, int newAngle) {
  int oldAngle = motors[motor].angle;
  if (oldAngle < newAngle) { // If old Angle <  new Angle increase angle until equal new angle
    while (oldAngle < newAngle) {
      increaseAngle(motor);
      oldAngle++;
    }
  } else {  // If old Angle >  new Angle decrease angle until equal new angle
    while (oldAngle > newAngle) {
      decreaseAngle(motor);
      oldAngle--;
    }
  }
}
void saveCurrentState() {
  for (int i = 0; i < 4; i++) {
    history[t_step].angles[i] = motors[i].angle; // save each angle of motor into t_step of history array
  }
  t_step++; // Increase step
}
void backToFirstStep() {
  // Come back to start state
  for (int i = 0; i < 4; i++) {
    smoothSpin(i, history[0].angles[i]);
  }
}
bool checkStop() {
  if (Serial.available()) {
    char x = Serial.read();
    if (x == TO_STOP_REPLAYING) {
      playing = false;
      Serial.print("\n\rStop to back to first step!");
      backToFirstStep();
    }
    if (x == PAUSE) {
      playing = false;
      Serial.print("\n\rPaused");
    }
  }
  return playing;
}
void replayFromPause(bool m_continue) {
  int si, i;
  if (m_continue) {
    i = g_i;
    si = g_si;
  } else {
    backToFirstStep();
    i = 0;
    si = 1;
  }
  Serial.print("\n\rStart replaying!");
  for (; si < t_step && playing; si++) { // For each step
    for (; i < 4 && playing; i++) {
      checkStop();
      smoothSpin(i, history[si].angles[i]); // Spin each servo motor to angle of current step
      g_si = si; g_i = i;
    }
    i = 0;
  }
  // Stop replaying
  Serial.print("\n\rEnd replaying!\n\r");
}
