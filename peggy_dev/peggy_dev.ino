/* Orthobox peggy
 March 11, 2014
 Peter O'Hanley
 */
 #define UNKNOWN_PIN 0
//TODO make topled actually do something
//DONE proper drop error handling like wall errors
//TODO the students can currently move directly from one wall to the other. If this is 
//  not desired, it can be fixed.
/* pin numbers
3 0
4 1
5 2
looking from the front*/
int sensorPin[]={
  A1,A2,A3,A4,A5,A6};
int peg_cutoff[] = {
  60,100,400,70,70,60};
#define PIN_LOWER_LIMIT 300
#define SENSOR_BLOCKED 1
#define SENSOR_CLEAR 0

int topLed = 30; //LED on top of box
int dropsensor = 28; //thing in front
int analogval;
int dropcount = 0;
#define PIN_DROP_COOLDOWN 500

#define PRETEST 0
#define VALIDATE 1
#define TESTREADY 2
#define TESTAB 3
#define TESTBC 4
#define TESTCB 5
#define TESTBA 6
#define WALLERROR 7
#define DROPERROR 8


/* The columns are named A, B, C ; numbered as you'd expect*/

int state, prevstate; /* stack depth of 1 will be fine */

#define VERSIONMSG 'v'
#define TESTMSG 't'
#define VALIDATEMSG 'a'
#define VERSIONSTRING "version: peggy_dev"
#define INVALID_START_STATE "error: invalid start state"
#define SENSOR_COUNT 6

#define TEST_TIMELIMIT 600000
#define TEST_COMPLETE_TIMEOUT_STR "tc: timeout"
#define TEST_COMPLETE_SUCCESS_STR "tc: success"

int pintouched = 0;
int cursens;
char read;
#define VALIDATE_PIN_OK "ok: " //then the number

int tool = A0;
long error_start = 0;
long error_length;
#define TOOL_LOWER_LIMIT 100
long test_start_time;

#define PHOTO_COUNT 2 //2
#define PHOTO_BLOCKED 0
#define PHOTO_CLEAR 1
int photo[] = {
  0, 0};
int photopin[] = {
  24, 26};

int pinDropped;
void setup() {
  state = PRETEST;

  pinMode(topLed,OUTPUT);
  pinMode(photopin[0], INPUT);
  pinMode(photopin[1], INPUT);

  Serial.begin(9600);
  //TODO ensure this pin is actually not used
  randomSeed(analogRead(15));
  Serial.println(VERSIONSTRING);
}

void loop() {
  for (int i = 0;i < PHOTO_COUNT;i++) photo[i] = digitalRead(photopin[i]);
  switch(state) {
  case PRETEST:
    if (Serial.available()) {
      read = Serial.read();
      switch (read) {
      case VALIDATEMSG:
        state = VALIDATE;
        cursens = 0;
        break;
      case TESTMSG:
        //somehow need to start test time
        state = TESTREADY;
      }
    }
    break;
  case VALIDATE:
    //TODO VALIDATE is a state that could potentially fail, (ie if one of the sensors is broken)
    //so it needs to read from the serial port
    //TODO this validate is broken
    /* check each photosensor in turn for coverage.*/
    if (digitalRead(sensorPin[cursens]) == PHOTO_BLOCKED) {
      Serial.println(VALIDATE_PIN_OK);
      Serial.println(cursens);
      cursens++;
    }
    if (cursens >= SENSOR_COUNT) {
      Serial.println("ok: all"); // not in production
      state = PRETEST;
      cursens = 0;
    }
    break;
  case TESTREADY:
  //TODO implement selecting between ABC and CBA
    if (toolremoved()) {
      Serial.println("test ready");
      if (columnstate(0,SENSOR_BLOCKED)) {
        state = TESTAB;
        test_start_time = millis();
        digitalWrite(topLed,HIGH);
        Serial.println("mode: testABC");
      } else if (columnstate(2,SENSOR_BLOCKED)) {
        state = TESTCB;
        test_start_time = millis();
        digitalWrite(topLed,HIGH);
        Serial.println("mode: testCBA");
      } else {
        for (int i = 0;i < SENSOR_COUNT;i++)
        Serial.println(peg_sensor_read(i));
        Serial.println(INVALID_START_STATE);
        delay(1000);
      }
    }
    break;
  case TESTAB:
    if (elapsedtime() > TEST_TIMELIMIT) {
      timeout();
    } else {
      if (columnstate(0,SENSOR_CLEAR)
       && columnstate(1,SENSOR_BLOCKED)) {
        state = TESTBC;
        Serial.println("testab done");
      }
      if (analogRead(tool) < TOOL_LOWER_LIMIT) {
        error_start = millis();
        state = WALLERROR;
        prevstate = TESTAB;
      }
      if (digitalRead(dropsensor)  == PHOTO_BLOCKED) {
        error_start = millis();
        state = DROPERROR;
        prevstate = TESTAB;
      }
    }
    break;
  case TESTBC:
    if (elapsedtime() > TEST_TIMELIMIT) {
      timeout();
    } else {
      if (columnstate(1,SENSOR_CLEAR)
       && columnstate(2,SENSOR_BLOCKED)) {
        succeed();
      }
      if (analogRead(tool) < TOOL_LOWER_LIMIT) {
        error_start = millis();
        state = WALLERROR;
        prevstate = TESTBC;
      }
      if (digitalRead(dropsensor)  == PHOTO_BLOCKED) {
        error_start = millis();
        state = DROPERROR;
        prevstate = TESTBC;
      }
    }
    break;
    case TESTCB:
    if (elapsedtime() > TEST_TIMELIMIT) {
      timeout();
    } else {
      if (columnstate(2,SENSOR_CLEAR)
       && columnstate(1,SENSOR_BLOCKED)) {
        state = TESTBA;
        Serial.println("testcb done");
      }
      if (analogRead(tool) < TOOL_LOWER_LIMIT) {
        error_start = millis();
        state = WALLERROR;
        prevstate = TESTCB;
      }
      if (digitalRead(dropsensor)  == PHOTO_BLOCKED) {
        error_start = millis();
        state = DROPERROR;
        prevstate = TESTCB;
      }
    }
    break;
  case TESTBA:
    if (elapsedtime() > TEST_TIMELIMIT) {
      timeout();
    } else {
      if (columnstate(1,SENSOR_CLEAR)
       && columnstate(0,SENSOR_BLOCKED)) {
        succeed();
      }
      if (analogRead(tool) < TOOL_LOWER_LIMIT) {
        error_start = millis();
        state = WALLERROR;
        prevstate = TESTBA;
      }
      if (digitalRead(dropsensor)  == PHOTO_BLOCKED) {
        error_start = millis();
        state = DROPERROR;
        prevstate = TESTBA;
      }
    }
    break;
  case WALLERROR:
    if (analogRead(tool) < TOOL_LOWER_LIMIT) {
      //still error
    } else {
      //while (analogRead(tool) < TOOL_LOWER_LIMIT);
        error_length = millis() - error_start;
        Serial.print("ew: ");
        Serial.print(error_length);
        Serial.print(" t: ");
        Serial.println(elapsedtime());
        state = prevstate;
    }
    break;
  case DROPERROR:
    if (digitalRead(dropsensor) == PHOTO_CLEAR) {
      error_length = millis() - error_start;
      Serial.print("ed: ");
      Serial.print(error_length);
      Serial.print(" t: ");
      Serial.println(elapsedtime());
      state = prevstate;
    }
    break;
  }
}


long elapsedtime(void) {
  return millis() - test_start_time;
}

int toolremoved(void) {
  //TODO actually do this as a loop through the photoresistors
  //there just had to be two photothings didn't there
  //if open, read returns 1
  int out = 1;
  for (int i = 0; i < PHOTO_COUNT;i++) {
    out = out & photo[i];
  }
  return out;
}

void timeout(void) {
  digitalWrite(topLed,LOW);
  Serial.print(TEST_COMPLETE_TIMEOUT_STR);
  Serial.print(" t: ");
  Serial.println(elapsedtime());
  state = PRETEST;
}
void succeed(void) {
  digitalWrite(topLed,LOW);
  Serial.print(TEST_COMPLETE_SUCCESS_STR);
  Serial.print(" t: ");
  Serial.println(elapsedtime());
  state = PRETEST;
}

int columnstate(int col, int state) {
  switch (col) {
    case 0:
      return (peg_sensor_read(3) == state)
        && (peg_sensor_read(4) == state)
        && (peg_sensor_read(5) == state);
    case 1:
    //no sensors there
        return 1;
    case 2:
      return (peg_sensor_read(0) == state)
        && (peg_sensor_read(1) == state)
        && (peg_sensor_read(2) == state);
  }
}

int peg_sensor_read(int peg) {
  return (analogRead(sensorPin[peg]) < peg_cutoff[peg])
    ? SENSOR_BLOCKED
    : SENSOR_CLEAR;
}
