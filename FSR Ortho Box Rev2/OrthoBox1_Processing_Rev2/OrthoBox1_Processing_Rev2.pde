import processing.serial.*;

int mySerialPort = 0; // Change this to be your arduino serial port,
                      // 0 is the default.
Serial myPort;

PFont font1;
PImage img;
final int WIDTH=500;  //width of GUI in pixels
final int HEIGHT=500;  //height of GUI in pixels
final int CENTER_X=WIDTH/2;  //x coordinate of center of speedometer
final int CENTER_Y=(HEIGHT/2);    //y coordinate of center of speedometer
final int FONTSIZE=20;
int time0=0;
int numberSensors = 7;
boolean record=false;
int[] sensorPin = new int [numberSensors];
float[] analogValue = new float [numberSensors];
int portX1, portY1;
int portWidth=80;
int portHeight=30;
int portColor;
color buttonHighlight;
boolean overPort=false;
PrintWriter output, output1;  //output file

void setup() //function to setup GUI
{
  size(WIDTH, HEIGHT);  //setting size of GUI
  background(255);
  smooth();
  //img = loadImage("crest.jpg");
  // Open an Arduino on mySerialPort
  font1 = createFont("Times New Roman", FONTSIZE);  //setting font to Times New Roman
  textFont(font1);
  portColor = color(255);
  buttonHighlight = color(204);  //setting highlight color to grey

  //setting position of buttons
  portX1 = 50;
  portY1 = 25;
}

void draw()  //draw function like loop() in arduino, which is repeated until the program is closed
{
//  image(img, 164, 10);                                                          //value sent is the analog value read from the Pin A0
  button();
  if(overPort==true)
  {
    myPort = new Serial(this, Serial.list()[mySerialPort], 9600);  //connecting with available COM port
    myPort.bufferUntil('\n');
    overPort=false; 
  }
  record=recordData(analogValue);
  if(record)  //if record is true, the analog value read is printed into a file
  {
    output.print(millis()-time0);
    for(int i = 0; i < numberSensors; i++)
    {
      output.print("\t" + analogValue[i]);
    }
    output.println();
  } 
}

void serialEvent(Serial myPort)
{
  String inString = myPort.readStringUntil('\n');
  //println(inString);
  if(inString != null)
  {
    inString = trim(inString);
    float[] aValues = float(split(inString, ","));
    if(aValues.length>=numberSensors)
    {
      for(int i = 0; i < numberSensors; i++)
        analogValue[i] = aValues[i];
    }
  }
}
boolean recordData(float[] value)
{
  if(value[0] == 1 && value[1]==1 && record==false) //checking photointerrupter values
  {
    output= createWriter("Ortho 1--RAW"+" "+ month()+"-"+day()+" " + hour() + "-" + minute() + "-" + second()+".txt"); //creates a text file
    output.print("Time" +  "," +"Int 1" + "," +"Int 2" + "," + "Value" + "," + "No." + "," + "Gnd" + "," + "E1" + "," + "E2");
    output.println();
    output.println("(ms)");
    output1= createWriter("Ortho 1"+" "+ month()+"-"+day()+" " + hour() + "-" + minute() + "-" + second()+".txt"); //creates a text file
    output1.print("Time" +  "," + "No." + "," +  "E1" + "," + "E2");
    output1.println();
    output1.println("(ms)");
    time0=millis();
    record=true;    
  } 
  else if(record==true)  //if recording in process
  {
    if(value[0]==0 || value[1]==0)    //instrument kept back in holder
    {
      record=false; //stop recording
      output1.println(millis()-time0 + "," + analogValue[3] + "," + analogValue[5] + "," + analogValue[6]); //final values  
      output.close();
      output1.close(); //both files closed
    }
  }
  return record;
}
  
void button() //function to create the buttons 
{
  smooth();
  rectMode(CENTER);
  stroke(80);
  strokeWeight(2);
  
  for(int i=0; i<Serial.list().length; i++)
  {
    if(overButton(portX1, portY1+portHeight*i, portWidth, portHeight) && portColor!=color(145))
      fill(204);
    else  fill(portColor);
    rect(portX1, portY1+portHeight*i, portWidth, portHeight);
    fill(80);
    text(Serial.list()[i], 20, portHeight*(1+i));
  }
  if(overButton(50, 25, 80, 30))  fill(buttonHighlight);
  else fill(portColor);
  
}

void mousePressed()
{
  for(int i = 0; i<Serial.list().length; i++)
  {
    if(overButton(portX1, portY1+portHeight*i, portWidth, portHeight) && portColor!=color(145))
    {
      mySerialPort=i;
      overPort=true;
      portColor = color(145);
    } 
  }
}

boolean overButton(int x, int y, int a, int b) //checks if the mouse is over a button
{
  if(mouseX>x-(a/2) && mouseX<x+(a/2) && mouseY>y-(b/2) && mouseY<y+(b/2)) return true; 
  else  return false;
}

