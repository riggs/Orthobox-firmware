/*
March 23, 2014
Peter O'Hanley
Medjules
*/
//We track errors as errors on any wire, at any time.
#define UNKNOWN -69
#define UU UNKOWN

#define CONE_COUNT 3

/* wire that is the test obstacle */
int trial_wires[] = {UU, UU, UU};
int cone_bases[] = {UU,UU,UU};
int tool_pin = UU;
int cone_leds[] = {UU,UU,UU};

int ringloc;

int state, prevstate;
#define ROULETTE 0
#define ROULETTE_SETUP 1
#define TEST_SETUP 2
#define TEST 3
#define TEST_OVER_WIN 4
#define TEST_OVER_ERRORS 5
#define TEST_OVER_TIMEOUT 6
#define WIRE_ERROR 7
#define WTF_ERROR 8

int target_cone; /* % CONE_COUNT */
int prev_cone; /* where the ring was */

#define ROULETTE_TIME (1000 * 5)
int roulette_start;
int cur_led;

#define TEST_TIMELIMIT (1000 * 60 * 5)
int test_start;

void setup() {
  for (int i = 0; i < CONE_COUNT; i++)
    pinMode(cone_leds[i],OUTPUT);
  Serial.begin(9600);
  random.begin(UU);
  state = ROULETTE_SETUP;
}

void loop() {
  switch(state) {
  case ROULETTE_SETUP:
    roulette_start = millis();
    state = ROULETTE;
    break;
  case ROULETTE:
    if ((millis() - roulette_start) > ROULETTE_TIME ) {
      digitalWrite(cone_leds[cur_led],LOW);
      //TODO make it spiral around until it reaches the target cone
      digitalWrite(cone_leds[target_cone],HIGH);
      state = TEST_SETUP;
    }
    break;
  case TEST_SETUP:
    prev_cone = find_ring();
    //TODO get rid of this blocking loop
    while (ring_present(prev_cone));
    test_start = millis();
    state = TEST;
    break;
  case TEST:
    if (elapsed_time() > TEST_TIMELIMIT) {
      state = TEST_OVER_TIMEOUT;
    } else {
      if (digitalRead(tool_pin)) {
        prevstate = state;
        state = WIRE_ERROR;
      } else if (ring_present(target_cone)) {
        state = TEST_OVER_WIN;
      }
    }
    break;
