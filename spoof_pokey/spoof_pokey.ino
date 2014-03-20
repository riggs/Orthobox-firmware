char read;
void setup() {
  Serial.begin(9600);
  Serial.println("version: pokey_dev");
}

void loop() {
  if (Serial.available()){
    read = Serial.read();
    if (read == 't') {
      Serial.println("p: 0 t: 59");
      Serial.println("e: 40 t: 90");
      Serial.println("p: 1 t: 100");
      delay(1000*15);
      Serial.println("p: 3 t: 14999");
      Serial.println("tc: success t: 15000");
    }
  }
}
