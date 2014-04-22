#define PINCOUNT 10
#define FOREACHPIN for(int i = 0, pin = pins[i]; i < PINCOUNT;pin=pins[++i])

int pins[]={
  A0,A1,A2,A3,A4,A5,A6,A7,A8,A9};
int num = 6;
int delaytime = 1000;
int thresh[] = {60,100,400,70,70,70};
int ground_pin = 15;

void setup() {
  Serial.begin(9600);
  
  pinMode(ground_pin,OUTPUT);
  digitalWrite(ground_pin,LOW);
}

void loop() {
  FOREACHPIN {
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(analogRead(pin));// < thresh[i]);
  }
  delay(delaytime);
}
