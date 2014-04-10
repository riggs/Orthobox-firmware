/*
March 23, 2014
Peter O'Hanley
Medjules
*/
//We track errors as errors on any wire, at any time.
//for DMD, just test each cone once, then boogie
#define UNKNOWN -69
#define UU -69
#define UNUSED_ARG -1

#define CONE_COUNT 3

/* wire that is the test obstacle */
int trial_wires[] = {A0, A1, A2}; //work on white
int cone_bases[] = {A3,A4,A5}; //work on white
int t_button[] = {9,10,11}; //front button

int cone_leds[] = {2,4,7};
#define OFF 0
#define BLINKING 1
#define ON 2
int led_mode[] = {OFF, OFF, OFF};
int blink_state[] = {LOW,LOW,LOW};
long blink_time[] = {100,100,100};
long last_blink[] = {-100,-100,-100};

int buzzer = 8; // confirmed on white

int state, prevstate;
#define PRETEST 1
#define TEST_SETUP 2
#define TEST 3
#define TEST_OVER_WIN 4
#define TEST_OVER_ERRORS 5
#define TEST_OVER_TIMEOUT 6
#define PRE_WIRE_ERROR 7
#define WIRE_ERROR 8
#define ROULETTE 9

//const rather than define because who knows how arduino combines these files
const char TEST_COMPLETE_MSG = 'c';
  //completion reasons
  const long OVER_SUCCESS = 1L;
  const long OVER_TIMEOUT = 2L;
  const long OVER_FAILURE = 3L;
const char ERROR_WIRE_MSG = 'w';
const char GOT_A_BUG_MSG = '!';
const char VERSIONMSG = 'v';
  const long VERSION = 3L;
const char TEST_READY_MSG = 'r';
const char CONE_COMPLETE_MSG = 'o';

const char START_TEST_CMD = 't';
#define CONES_TO_WIN 3
#define RING_CONTACT(pin) (analogRead(pin)<512)
const long TEST_TIMELIMIT = 180000;

int target_cone; /* % CONE_COUNT */
int prev_cone; /* where the ring was */

int cur_led;
int ground_pin = 13;
long test_start = -10000;
long error_start;
int cones_completed = 0;
char read;
void setup() {
  for (int i = 0; i < CONE_COUNT; i++){
    pinMode(trial_wires[i],INPUT);
    pinMode(cone_bases[i],INPUT);
    pinMode(t_button[i], OUTPUT);
    pinMode(cone_leds[i],OUTPUT);
    led_mode[i] = OFF;
  }
  
  pinMode(ground_pin,OUTPUT);
  digitalWrite(ground_pin,LOW);
  Serial.begin(9600);
  randomSeed(A6);
  write_packet(VERSIONMSG,VERSION,UNUSED_ARG);
  state = ROULETTE;
}

void loop() {
//  for (int i = 0;i<CONE_COUNT;i++) {
//    Serial.print("w:");
//    Serial.print(i);
//    Serial.print(": ");
//    Serial.println(analogRead(trial_wires[i]));
//  }
//  delay(500);
  //handle blinking and so on
  for(int i=0;i<CONE_COUNT;++i) {
    int led = cone_leds[i];
    
    switch(led_mode[i]) {
      case OFF:
        digitalWrite(led,LOW);
        break;
      case ON:
        digitalWrite(led,HIGH);
        break;
      case BLINKING:
        if (millis()-last_blink[i] > blink_time[i]) {
          //TODO make this preserve times so lights with different starts won't drift.
          last_blink[i] = millis();
          blink_state[i] = blink_state[i] == LOW ? HIGH : LOW;
          digitalWrite(led,blink_state[i]);
        }
    }
  }
  
  switch(state) {
    case ROULETTE:
      //never happens for real. TODO, make the light-spiral be non-blocking
      spin_leds();
      state = PRETEST;
      break;
    case PRETEST:
      //do spinny lights :D
      for (int i = 0;i<CONE_COUNT;++i) {
        led_mode[i] = ON;
      }
      
      if (Serial.available()) {
        read = Serial.read();
        if (read == START_TEST_CMD) {
          state = TEST_SETUP;
          for (int i = 0;i<CONE_COUNT;++i) {
            led_mode[i] = OFF;
          }
        } else {
          //todo send a "you got a bug" message here
          Serial.print("got: ");
          Serial.println(read);
        }
      }
      break;
  case TEST_SETUP:
    prev_cone = find_ring();
//    Serial.println("found ring");
    //TODO get rid of this blocking loop
    while (RING_CONTACT(cone_bases[prev_cone]));
    target_cone = (prev_cone+1)%CONE_COUNT;
    test_start = millis();
    led_mode[target_cone] = BLINKING;
    write_packet(TEST_READY_MSG,UNUSED_ARG,UNUSED_ARG);
    state = TEST;
    cones_completed = 0;
    break;
  case TEST:
    if (elapsed_time() > TEST_TIMELIMIT) {
      state = TEST_OVER_TIMEOUT;
    } else {
      if (RING_CONTACT(trial_wires[prev_cone]) 
      || RING_CONTACT(trial_wires[target_cone])) {
        prevstate = state;
        error_start = millis();
        state = PRE_WIRE_ERROR;
      } else if (RING_CONTACT(cone_bases[target_cone])) {
        cones_completed++;
        state = TEST_OVER_WIN;
      }
    }
    break;
  case TEST_OVER_WIN:
    if (cones_completed == CONES_TO_WIN) {
      write_packet(TEST_COMPLETE_MSG,OVER_SUCCESS,elapsed_time());
      led_mode[target_cone] = OFF;
      state = ROULETTE;
    } else {
      write_packet(CONE_COMPLETE_MSG,target_cone,elapsed_time());
      led_mode[target_cone] = OFF;
      prev_cone = target_cone;
      target_cone = (prev_cone+1)%CONE_COUNT;
      led_mode[target_cone] = BLINKING;
      state = TEST;
      test_start = millis();
    }
    break;
  case TEST_OVER_TIMEOUT:
    write_packet(TEST_COMPLETE_MSG,OVER_TIMEOUT,elapsed_time());
    state = ROULETTE;
    break;
  case PRE_WIRE_ERROR:
    tone(buzzer,440);
    for (int i = 0;i<CONE_COUNT;i++)
      led_mode[i] = BLINKING; //this will override any other settings. do we need those?
    state = WIRE_ERROR;
    break;
  case WIRE_ERROR:
    if (RING_CONTACT(trial_wires[0])
    || RING_CONTACT(trial_wires[1])
    || RING_CONTACT(trial_wires[2])) {
      //continue
    } else {
      long error_dur = millis() - error_start;
      long error_end = elapsed_time();
      write_packet(ERROR_WIRE_MSG,error_dur,error_end);
      state = prevstate;
      noTone(buzzer);
      for (int i = 0;i<CONE_COUNT;i++)
        led_mode[i] = i==target_cone ? BLINKING : OFF; 
    }
    break;
  }
}
void spin_leds() {
  #define LEDON(n) digitalWrite(cone_leds[n],HIGH)
  #define LEDOFF(n) digitalWrite(cone_leds[n],LOW)
//  int times[] = {250, 200, 150, 100, 50, 0};
  for (int k = 0; k < 25;++k) {
    for (int j = 0;j < 3;++j) {
//      for (int i = 0;i < CONE_COUNT;i++) {
        int a = j, b = (j+1)%CONE_COUNT,c=(j+2)%CONE_COUNT;
        LEDON(a);
        LEDOFF(b);
        LEDOFF(c);
        delay((25*5) - (k*5));
//      }
    }
  }
}
int find_ring() {
  //if it isn't on the base deal with it in the morning
  int i = 0;
  do 
    if (RING_CONTACT(cone_bases[i])) return i;
    else i++;
  while (true);
}
    
long elapsed_time() {
  return millis()-test_start;
}
