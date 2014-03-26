#define PINCOUNT 2
#define FOREACHPIN for(int i = 0, pin = pins[i]; i < PINCOUNT;pin=pins[i++])
int pins[] = {28,30};
void setup() {
  FOREACHPIN
    pinMode(pin,INPUT);
  Serial.begin(9600);
}

void loop() {
  FOREACHPIN {
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(digitalRead(pin));
  }
  delay(500);
}
