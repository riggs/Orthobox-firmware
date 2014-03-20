char read;
void setup() {
  Serial.begin(9600);
  Serial.println("version: peggy_dev");
}

void loop() {
  if (Serial.available()){
    read = Serial.read();
    if (read == 't') {
      Serial.println("ed: 10 t: 100");
      Serial.println("ew: 40 t: 140");
      Serial.println("ed: 13 t: 180");
      delay(1000*15);
      Serial.println("ew: 3 t: 14999");
      Serial.println("tc: success t: 15000");
    }
  }
}
