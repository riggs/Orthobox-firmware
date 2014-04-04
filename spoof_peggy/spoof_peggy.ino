char read;
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
void setup() {
  Serial.begin(9600);
  write_packet(VERSIONMSG,2L,UNUSED_ARG);
}

void loop() {
  if (Serial.available()){
    read = Serial.read();
    if (read == 't') {
      write_packet(ERROR_DROP_MSG,10,100);
      write_packet(ERROR_WALL_MSG,40,140);
      write_packet(ERROR_DROP_MSG,13,180);
      delay(15000);
      write_packet(ERROR_WALL_MSG,3,14000);
      write_packet(TEST_COMPLETE_SUCCESS_MSG,15000,UNUSED_ARG);
    }
  }
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
