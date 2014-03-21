/* Orthobox pokey
 March 11, 2014
 Peter O'Hanley
 */
/* TODO wierd failure case: if you hold the tool touching the metal, it will lock.*/
//TODO make topled actually do something
//TODO fix the problem with the last pin not getting its light turned on
int sensorPin[]={
  A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};
int ledPin[]={
  32, 34, 36, 38, 40, 42,/*44*/ 46, 48, 50, 52};
int topLed = 44;

int analogval;

#define PRECONNECT 0
#define PRETEST 1
#define VALIDATE 2
#define TEST 3
#define PRACTICE 4
#define TESTREADY 5
#define WALLERROR 6
int state;

#define VERSIONMSG 'v'
#define TESTMSG 't'
#define PRACTICEMSG 'p'
#define VALIDATEMSG 'a'
#define VERSIONSTRING "version: pokey_dev"
#define SENSOR_COUNT 10

#define TEST_TIMELIMIT 60000000
#define TEST_COMPLETE_TIMEOUT_STR "tc: timeout"
#define TEST_COMPLETE_SUCCESS_STR "tc: success"
#define ERROR_MIN_LENGTH 100

int pintouched = 0;
int cursens;
char read;
#define VALIDATE_PIN_OK "ok: " //then the number

int tool = A0;
long error_start = 0;
long error_length;
#define TOOL_LOWER_LIMIT 100
#define PUSH_THRESHOLD 100
long test_start_time;

#define PHOTO_COUNT 2 //2
int photo[] = {
  0, 0};
int photopin[] = {
  28, 30};
int pin_order[] = {
  0,1,2,3,4,5,6,7,8,9};
//int pin_order_test[] = {
//  0,1,2,3,4,5,6,7,8,9}; //going to change?
int pin_order_test[] = {
  0,2,3,4,5,6,7,8,9};

void setup() {
//  state = PRECONNECT;
  state = PRETEST;
  Serial.begin(9600);
  Serial.println(VERSIONSTRING);
  pinMode(topLed,OUTPUT);
  pinMode(photopin[0], INPUT);
  pinMode(photopin[1], INPUT);

  for(int i=0; i<SENSOR_COUNT; i++) 
    pinMode(ledPin[i], OUTPUT);
  randomSeed(analogRead(15));  //setting a constant seed to produce the same random sequence everytime
  
}

void loop() {
  for (int i = 0;i < PHOTO_COUNT;i++) 
    photo[i] = digitalRead(photopin[i]);

  switch(state) {
  case PRECONNECT:
    if (Serial.available() && (read = Serial.read()) == VERSIONMSG) {
      Serial.println(VERSIONSTRING);
      state = PRETEST;
    } //can't do anything without a version, can we?
    //still, we will want to be able to easily test the box out without running software
    //TODO: add swaps to other states in PRECONNECT state
    break;
  case PRETEST:
    if (Serial.available()) {
      read = Serial.read();
      switch (read) {
      case VALIDATEMSG:
        state = VALIDATE;
        cursens = 0;
        break;
      case PRACTICEMSG:
        state = PRACTICE;
        cursens = 0;
        break;
      case TESTMSG:
        //somehow need to start test time

        arrcpy(pin_order,pin_order_test,SENSOR_COUNT);
        state = TESTREADY;
        cursens = 0;
      }
    }
    break;
  case VALIDATE:
    //TODO VALIDATE is a state that could potentially fail, (ie if one of the sensors is broken)
    //so it needs to read from the serial port
    //check for pressure against the currently highlighted pin.
    //if found move to next
    digitalWrite(ledPin[cursens],HIGH);//next LED on
    analogval = analogRead(sensorPin[cursens]);
    if (analogval > PUSH_THRESHOLD) {
      //report successful hit, move to next one
      digitalWrite(ledPin[cursens],LOW);
      Serial.print(VALIDATE_PIN_OK);
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
    if (toolremoved()) {
      Serial.println("test ready");
      test_start_time = millis();
      state = TEST;
    }
    break;
  case TEST:
    //check for pressure against the currently highlighted pin.
    //if found move to next. If the ground pin has low analog value,
    //sound buzzer. Keep track of errors
    if (elapsedtime() > TEST_TIMELIMIT) {
      Serial.print(TEST_COMPLETE_TIMEOUT_STR);
      Serial.print(" t: ");
      Serial.println(elapsedtime());
      digitalWrite(ledPin[pin_order[cursens]],LOW);
      //        for (int i = 0;i <= SENSOR_COUNT;i++) digitalWrite(ledPin[pin_order[i]],LOW);
      state = PRETEST;
      cursens = 0;
    } else {
      digitalWrite(ledPin[pin_order[cursens]],HIGH);//next LED on
      analogval = analogRead(sensorPin[pin_order[cursens]]);
      if (analogval > PUSH_THRESHOLD) {
        //report successful hit, move to next one
        digitalWrite(ledPin[pin_order[cursens]],LOW);

        Serial.print("p: ");
        Serial.print(cursens);
        Serial.print(" t: ");
        Serial.println(elapsedtime());

        cursens++;
      }
      if (analogRead(tool) < TOOL_LOWER_LIMIT) {
        error_start = millis();
        state = WALLERROR;
      }
      if (cursens >= SENSOR_COUNT) {
        Serial.print(TEST_COMPLETE_SUCCESS_STR);
        Serial.print(" t: ");
        Serial.println(elapsedtime());
        state = PRETEST;
        cursens = 0;
      }
    }
    break;
  case PRACTICE:
    //practicing is just testing with randomized pin order
    shuffle(pin_order,SENSOR_COUNT);
    state = TESTREADY;
    break;
  case WALLERROR:
    if (analogRead(tool) < TOOL_LOWER_LIMIT) {
      //still error
    } else {
        error_length = millis() - error_start;
        if (error_length > ERROR_MIN_LENGTH) {
          Serial.print("e: ");
          Serial.print(error_length);
          Serial.print(" t: ");
          Serial.println(elapsedtime());
          state = TEST;
        }
    }
    break;
  }

}
void shuffle(int arr1[], int len) {
  int r=-1;
  int temp;
  for (int a=0; a<len;a++) {
    r = random(a,len); 
    temp = arr1[a];
    arr1[a] = arr1[r];
    arr1[r] = temp;
  }
}

void arrcpy(int* dest, int* src, int len) {
  for (int i = 0; i < len; i++) {
    dest[i] = src[i];
  }
}

long elapsedtime(void) {
  return millis() - test_start_time;
}

int toolremoved(void) {
  //if open, read returns 1
  int out = 1;
  for (int i = 0; i < PHOTO_COUNT;i++) {
    out = out & photo[i];
  }
  return out;
}


