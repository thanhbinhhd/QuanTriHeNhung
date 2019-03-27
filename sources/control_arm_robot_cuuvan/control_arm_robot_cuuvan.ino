#include<Servo.h> // include server library
#include "PS2X_lib.h"  //for v1.6
/******************************************************************
   set pins connected to PS2 controller:
     - 1e column: original
     - 2e colmun: Stef?
   replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        4  //14    
#define PS2_CMD        10  //15
#define PS2_SEL        8  //16
#define PS2_CLK        11  //17

/******************************************************************
   select modes of PS2 controller:
     - pressures = analog reading of push-butttons
     - rumble    = motor rumbling
   uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

void saveCurrentState();
void replay();
void increaseAngle(int ser_index);
void decreaseAngle(int ser_index);
void servo_catch();
void servo_release();
void gamePadConfig();

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

PS2X ps2x; // create PS2 Controller Class
int val; // initial value of input
int t_step; // step to replay
int g_si; //store current step when pause
int g_i; // store current servo when stop
HaServo motors[4]; // Array to stores info of servo motors
State history[50]; // Store 100 state of arm
bool playing;

//right now, the library does NOT support hot pluggable controllers, meaning
//you must always either restart your Arduino after you connect the controller,
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

void setup() {
  t_step = 0; // init t_step to 0
  playing = 0; // init playing is false

  Serial.begin(9600); // Serial comm begin at 9600bps

  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it

  gamePadConfig(); // config gamePad

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
  gamePadControl();
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
  //  writeServo(3, 0);  // write angle
  Serial.print("\n\rCatching");
  while (motors[3].angle > 0) {
    writeServo(3, motors[3].angle - 1);
    delay(10);
  }
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

void gamePadConfig() {
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************

  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  if (error == 0) {
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
    if (pressures)
      Serial.println("true ");
    else
      Serial.println("false");
    Serial.print("rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }
  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  //  Serial.print(ps2x.Analog(1), HEX);

  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
    case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
  }
}

void gamePadControl() {
  if (type == 2) { //Guitar Hero Controller, Không thể test với nhánh này do không có gamepad này.
    ps2x.read_gamepad(); //read controller

    if (ps2x.ButtonPressed(GREEN_FRET))
      Serial.println("Green Fret Pressed");
    if (ps2x.ButtonPressed(RED_FRET))
      Serial.println("Red Fret Pressed");
    if (ps2x.ButtonPressed(YELLOW_FRET))
      Serial.println("Yellow Fret Pressed");
    if (ps2x.ButtonPressed(BLUE_FRET))
      Serial.println("Blue Fret Pressed");
    if (ps2x.ButtonPressed(ORANGE_FRET))
      Serial.println("Orange Fret Pressed");

    if (ps2x.ButtonPressed(STAR_POWER))
      Serial.println("Star Power Command");

    if (ps2x.Button(UP_STRUM))         //will be TRUE as long as button is pressed
      Serial.println("Up Strum");
    if (ps2x.Button(DOWN_STRUM))
      Serial.println("DOWN Strum");

    if (ps2x.Button(PSB_START))        //will be TRUE as long as button is pressed
      Serial.println("Start is being held");
    if (ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");

    if (ps2x.Button(ORANGE_FRET)) {    // print stick value IF TRUE
      Serial.print("Wammy Bar Position:");
      Serial.println(ps2x.Analog(WHAMMY_BAR), DEC);
    }
  }
  else { //DualShock Controller, Loại gamepad thầy đưa cho là loại này
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed

    if (ps2x.Button(PSB_PAD_UP)) {     //will be TRUE as long as button is pressed
      increaseAngle(1);
    }
    if (ps2x.Button(PSB_PAD_RIGHT)) {
      decreaseAngle(0);
    }
    if (ps2x.Button(PSB_PAD_LEFT)) {
      increaseAngle(0);
    }
    if (ps2x.Button(PSB_PAD_DOWN)) {
      decreaseAngle(1);
    }
    if (ps2x.Button(PSB_TRIANGLE)) {
      increaseAngle(2);
    }
    if (ps2x.Button(PSB_CROSS)) {
      decreaseAngle(2);
    }
    if (ps2x.ButtonPressed(PSB_SQUARE)) {
      Serial.print("\n\rButtonPressed(PSB_SQUARE)");
      servo_catch();
    }
    if (ps2x.ButtonPressed(PSB_CIRCLE)) {
      Serial.print("\n\rButtonPressed(PSB_CIRCLE)");
      servo_release();
    }
    if (ps2x.NewButtonState()) {
      if (ps2x.ButtonReleased(PSB_L2)) {
        playing = false;
        backToFirstStep();
        Serial.println("Reset to first step1");
      }
      if (ps2x.ButtonPressed(PSB_R2)) {
        //pass
        Serial.print("\n\rReplaying1...");
        playing = true;
      }
      if (ps2x.ButtonPressed(PSB_START)) {
        t_step = 0;
        saveCurrentState();
        Serial.print("\n\rReset recording1...\n");
      }
      if (ps2x.ButtonPressed(PSB_SELECT)) {
        saveCurrentState();
        Serial.print("\n\rSaveCurrentState, total step1: ");
        Serial.print(t_step, "\n");
      }
    }
  }
  delay(15);
}
