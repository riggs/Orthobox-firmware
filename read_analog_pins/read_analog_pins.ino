int pushPin[]={
  A1,A2,A3,A4,A5,A6,A7,A8,A9,A10};
  
int num = 6;
int delaytime = 50;
int i;
int thresh[] = {60,100,400,70,70,70};
void setup() {
  Serial.begin(9600);
}

void loop() {
//  Serial.println(analogRead(A9));
  for (i = 0;i < num; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(analogRead(pushPin[i]) < thresh[i]);
  }
  delay(400);
}
