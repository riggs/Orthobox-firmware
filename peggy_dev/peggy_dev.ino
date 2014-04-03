/* Orthobox peggy
 March 11, 2014
 Peter O'Hanley
 */
 #define UNKNOWN_PIN 0
//DONE make topled actually do something
//DONE proper drop error handling like wall errors
//TODO the students can currently move directly from one wall to the other. If this is 
//  not desired, it can be fixed.
//TODO make the full test be AB -> BC -> CB -> BA (or the other direction)
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

const char VERSIONMSG = 'v';
const char TESTMSG = 't';
const char PRACTICEMSG = 'p';
const char VALIDATEMSG = 'a';
const char TEST_READY_MSG = 'e';
const char ERROR_WALL_MSG = 'w';
const char ERROR_DROP_MSG = /* the */ 'd';
const char PIN_TOUCHED_MSG = 'u';
const char TEST_COMPLETE_TIMEOUT_MSG = 'i';
const char TEST_COMPLETE_SUCCESS_MSG = 's';
const char VALIDATE_PIN_OK = 'k'; //then the number
#define UNUSED_ARG 0 //for readability

unsigned long dong = 2L;
#define VERSIONCONST dong

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

#define INVALID_START_STATE "error: invalid start state"
#define SENSOR_COUNT 6

#define TEST_TIMELIMIT 180000

int pintouched = 0;
int cursens;
char read;

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
  write_packet(VERSIONMSG,VERSIONCONST,UNUSED_ARG);
}

void loop() {
  for (int i = 0;i < PHOTO_COUNT;i++) photo[i] = digitalRead(photopin[i]);
  switch(state) {
  case PRETEST:
    if (Serial.available()) {
      read = Serial.read();
      switch (read) {
      case VERSIONMSG:
        write_packet(VERSIONMSG,VERSIONCONST,UNUSED_ARG);
      break;  
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
      write_packet(VALIDATE_PIN_OK,cursens,UNUSED_ARG);
      cursens++;
    }
    if (cursens >= SENSOR_COUNT) {
      write_packet(VALIDATE_PIN_OK,SENSOR_COUNT,UNUSED_ARG);
      state = PRETEST;
      cursens = 0;
    }
    break;
  case TESTREADY:
  //TODO implement selecting between ABC and CBA
    if (toolremoved()) {
      write_packet(TEST_READY_MSG,UNUSED_ARG,UNUSED_ARG);
      if (columnstate(0,SENSOR_BLOCKED)) {
        state = TESTAB;
        test_start_time = millis();
        digitalWrite(topLed,HIGH);
//        Serial.println("mode: testABC");
      } else if (columnstate(2,SENSOR_BLOCKED)) {
        state = TESTCB;
        test_start_time = millis();
        digitalWrite(topLed,HIGH);
//        Serial.println("mode: testCBA");
      } /*else {
        for (int i = 0;i < SENSOR_COUNT;i++)
        Serial.println(peg_sensor_read(i));
        Serial.println(INVALID_START_STATE);
        delay(1000);
      }*/
    }
    break;
  case TESTAB:
    if (elapsedtime() > TEST_TIMELIMIT) {
      timeout();
    } else {
      if (columnstate(0,SENSOR_CLEAR)
       && columnstate(1,SENSOR_BLOCKED)) {
        state = TESTBC;
//        Serial.println("testab done");
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
//        Serial.println("testcb done");
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
        error_length = millis() - error_start;
        write_packet(ERROR_WALL_MSG,error_length,elapsedtime());
        state = prevstate;
    }
    break;
  case DROPERROR:
    if (digitalRead(dropsensor) == PHOTO_CLEAR) {
      error_length = millis() - error_start;
      write_packet(ERROR_DROP_MSG,error_length,elapsedtime());
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
  write_packet(TEST_COMPLETE_TIMEOUT_MSG,elapsedtime(),UNUSED_ARG);
  state = PRETEST;
}
void succeed(void) {
  digitalWrite(topLed,LOW);
  write_packet(TEST_COMPLETE_SUCCESS_MSG,elapsedtime(),UNUSED_ARG);
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
void write_packet(char messagetype, long foo, long bar) {
  switch (messagetype) {
    case ERROR_WALL_MSG:
      Serial.write(ERROR_WALL_MSG);
      Serial.write((char) 10);
      write_long(foo);
      write_long(bar);
      break;
    case ERROR_DROP_MSG:
      Serial.write(ERROR_DROP_MSG);
      Serial.write((char) 10);
      write_long(foo);
      write_long(bar);
      break;
    case TEST_COMPLETE_SUCCESS_MSG:
      Serial.write(TEST_COMPLETE_SUCCESS_MSG);
      Serial.write((char) 6);
      write_long(foo);
      break;
    case TEST_COMPLETE_TIMEOUT_MSG:
      Serial.write(TEST_COMPLETE_TIMEOUT_MSG);
      Serial.write((char) 6);
      write_long(foo);
      break;
    case PIN_TOUCHED_MSG:
      Serial.write(PIN_TOUCHED_MSG);
      Serial.write((char) 10);
      write_long(foo);
      write_long(bar);
      break;
    case TEST_READY_MSG:
      Serial.write(TEST_READY_MSG);
      Serial.write((char) 2);
      break;
    case VERSIONMSG:
      Serial.write(VERSIONMSG);
      Serial.write((char) 6);
      write_long(foo);
      break;
    case VALIDATE_PIN_OK:
      Serial.write(VALIDATE_PIN_OK);
      Serial.write((char) 6);
      write_long(foo);
      break;
  }
}
void write_long(long var) {
  Serial.write(var);
  Serial.write(var>>8);
  Serial.write(var>>16);
  Serial.write(var>>24);
}
void write_int(int var) {
  Serial.write(var);
  Serial.write(var>>8);
}
