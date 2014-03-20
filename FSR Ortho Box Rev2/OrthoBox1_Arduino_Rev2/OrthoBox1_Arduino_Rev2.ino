/* ORTHO BOX 1 - FSR orthoscope test box
Author: Yashovardhan Sand
CREST, MDC
Version 1.0
March 3rd, 2014
*/
//10 force sensors
int p = 10; 
int AnalogPin[]={14, 1, 3, 4, 5, 6, 7, 8, 9, 10};
int DigitalPin[]={26, 52, 48, 46, 40, 42, 32, 34, 36, 38};
int AnalogValue[10];
int pin; //pin for random LED
int error1=0;
int error2=0;
int touch=0; //counter for every sensor pushed
long newBeg=0; //new beginning time when box touched
long time0=0;
int pInt1=28;
int pInt2=30;
int pI1, pI2;
int flag=0;
int topLed=51; //LED on top of box
int gndPin=15; //pin for buzzer
int i=0;
void setup()
{
  pinMode(topLed,OUTPUT); //top LED
  pinMode(pInt1, INPUT); //photointerrupter 1
  pinMode(pInt2, INPUT); //photointerrupter 2
  for(int j=0; j<10; j++)
    pinMode(DigitalPin[j], OUTPUT);    //setting all LED as output pins
  Serial.begin(9600); //Begin Serial Communication with a baud rate of 9600
  randomSeed(2342);  //setting a constant seed to produce the same random sequence everytime
  shuffle(AnalogPin, DigitalPin, p);
  pin=i;
}

void loop() 
{
  pI1=digitalRead(pInt1); //reading photointerrupter 1
  pI2=digitalRead(pInt2); //reading photointerrupter 2
  if(i<p)
  {
    if(flag==1)
    {
      digitalWrite(DigitalPin[pin], HIGH);  //turning on random LED
      AnalogValue[pin]=analogRead(AnalogPin[pin]);  //checking force applied on sensor
      if(analogRead(gndPin)<100) //if box touched by instrument
      {
        newBeg=millis();  //new beginning time when box is touched
        while(analogRead(gndPin)<100); //waiting till contact is broken
        if(millis()-newBeg>50 && millis()-newBeg<1000)  error1++;  //error 1 for 50ms<contact time<1000ms
        else if(millis()-newBeg>1000) error2++; //error 2 for contact time>1000ms
      }
      if(AnalogValue[pin] > 400) //if force applied  >1/5th of max., then sensor touch recorded 
      {
        touch++; //number of sensors touched increments
        digitalWrite(DigitalPin[pin], LOW); //current LED turns off
        i=i+1;
        pin=i;  //next LED is chosen
      }
    }
    if(pI1==1 && pI2==1 && flag==0) //if no instrument is sheathed in the box
    {
      digitalWrite(topLed, HIGH); //turning on top led to start test
      time0=millis(); //inital time recorded
      flag=1;
    }
    if(pI1==0 || pI2==0) //if instrument is kept in either hole
    {
      digitalWrite(topLed, LOW); //top led is kept off
      digitalWrite(DigitalPin[pin], LOW); //led for test is kept off
      time0=millis();
      pI1=0; // both interrupter values are made 0 to allow instrument to be kept back in any hole
      pI2=0;
      flag=0;
   }
   else if(millis()-time0>=60000)  //test length 60s
   {
     digitalWrite(topLed, LOW); //after time limit all LED's are turned off
     digitalWrite(DigitalPin[pin], LOW);
     pI1=0;
     pI2=0;
     flag=0;
   }
  }
  else  
  {
    if(pI1==0 || pI2==0)
      digitalWrite(topLed, LOW);
  }
  //  Serial.print(flag);
//  Serial.print(',');
//values sent to computer via serial port
    Serial.print(pI1);  //photointerrupter 1
    Serial.print(',');
    Serial.print(pI2);  //photointerrupter 2
    Serial.print(',');
    Serial.print(AnalogValue[pin]);  //force on sensor
    Serial.print(',');
    Serial.print(touch);  //touch recorded
    Serial.print(',');
    Serial.print(analogRead(gndPin));  //instrument grounded
    Serial.print(',');
    Serial.print(error1);  //error1
    Serial.print(',');
    Serial.println(error2); //error 2
}
void shuffle(int arr1[], int arr2[], int len)
{
  int r=-1;
  for (int a=0; a<len;a++)
  {
    r = random(a,len); 
    int temp1 = arr1[a];
    arr1[a] = arr1[r];
    arr1[r] = temp1;  
    int temp2 = arr2[a];
    arr2[a] = arr2[r];
    arr2[r] = temp2;  
  }
}
  
