int ledPin[]={
  32, 34, 36, 38, 40, 42, 46, 48, 50, 52, 44};
  
int num = 11;
int delaytime = 50;
int i;
void setup() {
  Serial.begin(9600);
  for (i = 0;i<num;i++) 
    pinMode(ledPin[i],OUTPUT);
}

void loop() {
//  Serial.println(analogRead(A9));
  for (i = 0;i < num; i++) {
    digitalWrite(ledPin[i],HIGH);
    delay(delaytime);
  }
  for (i = 0;i < num; i++) {
    digitalWrite(ledPin[i],LOW);
    delay(delaytime);
  }
}
