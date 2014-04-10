void write_packet(char messagetype, long foo, long bar) {
  switch (messagetype) {
    case ERROR_WIRE_MSG:
      Serial.write(ERROR_WIRE_MSG);
      Serial.write((char) 10);
      write_long(foo);
      write_long(bar);
      break;
    case TEST_COMPLETE_MSG: //foo = type, bar = time
      Serial.write(TEST_COMPLETE_MSG);
      Serial.write((char) 10);
      write_long(foo);
      write_long(bar);
      break;
    case CONE_COMPLETE_MSG://foo = cone, bar = time
      Serial.write(CONE_COMPLETE_MSG);
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
    default:
      Serial.write(GOT_A_BUG_MSG);
      Serial.write((char)2);
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
