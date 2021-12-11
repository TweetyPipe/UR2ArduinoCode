#include <Servo.h>

const int magnet = 8;
const int LED = 13;
float xpos;
float ypos;
float width;
const float sarm = 5.0;
const float larm = 10.0;
float a = 0.0;
float c = 0.0;
float angleX = 0.0;
float motorB = 0.0;
float motorM = 0.0;
float currentB = 0.0;
float currentM = 0.0;
int shapeCat = 0;

const byte buffSize = 40;
unsigned int inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;
byte coordinates[3];

Servo base;
Servo mid;
Servo mag;

void setup() {
  ///set up arduinos and start serial monitoring
  pinMode(magnet, OUTPUT);
  pinMode(LED, OUTPUT);
  base.attach(3);
  mid.attach(5);
  mag.attach(6);
  Serial.begin(9600);
  Serial.println("begin");
  base.write(20);
  mid.write(0);
}

void loop() 
{
  //read in the coordinates
  getDataFromPC();

  //if there are coordinates to read
  if(newDataFromPC){

  shapeCat = coordinates[0];
  xpos = coordinates[1];
  ypos = coordinates[2];

  //print the catagory and the coordinates so I can monitor them 
  Serial.print("<");
  Serial.print("D");
  Serial.print(shapeCat);
  Serial.print(",");
  Serial.print(xpos);
  Serial.print(",");
  Serial.print(ypos);
  Serial.print(">");

  //determine if it's square or triangle
    determineCase();
  //pick up the shape
    magnetON();
  //take shape to the proper side
    sortShape();
  //drop shape
    magnetOFF();
  //go back to start
    moveMotor(0,0);
  //report the coordinates of the shape it just picked up
    sendCoordinatesToPC();
    newDataFromPC = false;
    sendEnableCmd();
  }   
}
//pick up shape
void magnetON()
{
  delay(1000);
  mag.write(180);
  digitalWrite(magnet, HIGH);
  delay(1000);
  mag.write(0);
}
//drop shape
void magnetOFF()
{
  digitalWrite(magnet, LOW);
  delay(2000);
}

//decicde which side and move it there
void sortShape()
{
  moveMotor(shapeCat*180, 180);
  
}

//moves the motors slowly
void moveMotor(float baseMotor, float midMotor)
{  
  base.write(currentB);
  delay(2000);
  
  float i = 0.0;
  float j = 0.0;
  //moving base motor
  if (currentB <= baseMotor)//move forward
  {
    for (i= currentB ; i <= baseMotor; i++)
    {
       base.write(i);
       delay(20);
    }
    currentB = i;
  }
  else if (currentB > baseMotor)//move backward
  {
    for (i= currentB ; i >= baseMotor; i--)
    {
       base.write(i);
       delay(20);
    }
    currentB = i;
  }

  //moving mid motor
  mid.write(currentM);
  delay(2000);
  if (currentM <= midMotor) //move forward
  {
      for (j= currentM; j<= midMotor; j++)
    {
      mid.write(j);
      delay(20);
    }
    currentM =  j;
  }
  else if (currentM > midMotor)//move backward
  {
    for (j= currentM; j>= midMotor; j--)
    {
      mid.write(j);
      delay(20);
    }
    currentM =  j;
  }
 
}

//read the coordinates
void getDataFromPC() 
{
  if(Serial.available() > 0) 
  {
    char x = Serial.read();
    // the order of these IF clauses is significant
    if (x == endMarker) 
    {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      coordinates[0] = inputBuffer[0];
      coordinates[1] = inputBuffer[1];
      coordinates[2] = inputBuffer[2];
    }
  
    if(readInProgress) 
    {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) 
      {
        bytesRecvd = buffSize - 1;
      }
    }
  
    if (x == startMarker) 
    {
      bytesRecvd = 0;
      readInProgress = true;
    }
  }
}

//calculate motor angles
void calculateAngles()
{
  float height = ypos+(5.5*2);
  //hypotenuse, or Direct Distance
  float dirDist = sqrt(sq(width*0.5)+sq(height*0.5));
  Serial.print("Direct Distance: ");
  Serial.println(dirDist);
  //law of Cosines to find angle a
  a = acos((sq(larm)+sq(dirDist)-sq(sarm))/(2*larm*dirDist));
  Serial.print("radian cos of a is = ");
  Serial.println((sq(larm)+sq(dirDist)-sq(sarm))/(2*larm*dirDist));
  //find angle of hypotenuse
  angleX = atan(height/width);
  //convert a to degrees
  a = (a*180)/3.14;
  Serial.print("degree Angle a:");
  Serial.println(a);
  //convert angleX to degrees
  angleX = (angleX*180)/3.14;
  Serial.print("AngleX is = ");
  Serial.println(angleX);
  //Law of cosines to find mid arm angle
  c = acos((sq(sarm)+sq(larm)-sq(dirDist))/(2*sarm*larm));
  //convert to degrees
  c = (c*180)/3.14;
  Serial.print("degree Angle c:");
  Serial.println(c); 
}
//moving directors for left, right, or midle
void determineCase()
{
  if (xpos < 23 && xpos > 0)
    {
      if (xpos > 11)
      {
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        width = xpos-11;
        calculateAngles();
        motorB = 180 - (angleX - a)+10.0;
        motorM = c-10.0;
        Serial.print("Base Angle: ");
        Serial.println(motorB);
        Serial.print("Mid Angle: ");
        Serial.println(motorM);
      }
      else if (xpos < 11)
      {
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        width = 11-xpos;
        calculateAngles();
        motorB = (angleX + a)+17.0;
        motorM = c-12.0;
        Serial.print("Base Angle: ");
        Serial.println(motorB);
        Serial.print("Mid Angle: ");
        Serial.println(motorM);
      }
      else if(xpos == 11)
      {
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        width = 0.1;
        calculateAngles();
        motorB = (angleX + a)+15.0;
        motorM = c-10.0;
        Serial.print("Base Angle: ");
        Serial.println(motorB);
        Serial.print("Mid Angle: ");
        Serial.println(motorM);
      }
    }
    else
    {
      base.write(90);
    }
    moveMotor(motorB, motorM);
}

//sending coordinates that were recieved to the UI
void sendCoordinatesToPC()
{
  // send the point data to the PC
  Serial.print("<P");
  Serial.print(coordinates[0]);
  Serial.print(",");
  Serial.print(coordinates[1]);
  Serial.println(">");
}

void sendEnableCmd()
{
  // send the suspend-false command
  Serial.println("<S0>");
}
