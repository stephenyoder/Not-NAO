//Last modified 11/30 by S.Y.
#include <SoftwareSerial.h>
#include <PololuQik.h>

volatile long encoderValueM0;
volatile long encoderValueM1;

double newPosM0;  // counts
double oldPosM0 = 0;  // counts
double newPosM1;  // counts
double oldPosM1 = 0;  // counts

const uint8_t r = 5;  // inches
const uint8_t l = 11; // inches

unsigned long newTime;  // ms
unsigned long oldTime = 0;  // ms
unsigned long ts; // ms

double velM0;
double velM1;
double phiM0; // rad/s
double phiM1; // rad/s
double omega = 0; // rad/s
double filteredOmega = 0;

volatile double newTheta;  // rad
volatile double oldTheta = PI/2; // rad
volatile double setHeading;  // rad

double Kp, Kd;
double error = 0;
double pd_out = 0;

double outLeft = 0;
double outRight = 0;

bool ramp = false; 
bool inc_done = false;
bool dec_done = false;

enum Command { NONE, STRAIGHT, LEFT, RIGHT};
Command command = NONE;  // 0:none, 1:straight, 2:left, 3:right

double calcOmega(void);

// Encoder Counter prototypes
void enc_aM0(void);
void enc_bM0(void);
void enc_aM1(void);
void enc_bM1(void);

// pd task callback prototype
void pdCallback(void);
void forward(void);
void turn_left(void);
void halt(void);

/*
Required connections between Arduino and qik 2s12v10:

      Arduino   qik 2s12v10
---------------------------
          GND - GND
Digital Pin 2 - TX
Digital Pin 3 - RX
Digital Pin 4 - RESET

DO NOT connect the 5V output on the Arduino to the 5V output on the qik 2s12v10!
*/
PololuQik2s12v10 qik(2, 3, 4);
// **************** FOR ROBOT BASE ******************
// Motor 0 is the left and Motor 1 is the right
// **************************************************

// USED FOR ARDUINAO USB SERIAL CONNECTION

// ***************************************

#define BUFSIZE 512

char inbuf[BUFSIZE];

volatile boolean booted = false;
volatile boolean moving = false;
volatile boolean tagRead = false;
volatile boolean commandRead = false;

char prev_data[] = {0, 0};
char data[] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  Serial2.begin(57600);
  qik.init();
  
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(20, INPUT);
  pinMode(21, INPUT);

  attachInterrupt(digitalPinToInterrupt(18), enc_aM0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(19), enc_bM0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(20), enc_aM1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(21), enc_bM1, CHANGE);
  
}

void loop() { 
  //if(commandRead){  //this loop is commented out for the test
  //  processCommand(inbuf); 
  //  for (int j=0;j<BUFSIZE;j++) inbuf[j]=0;
  //    commandRead = false;
    //Serial.println("test");
  }
  if(tagRead) {
    if((prev_data[0] == data[4]) && (prev_data[1] == data[5]));
    else
    {
      if((data[4] == 01) && (data[5] == 45))
      {
        Serial.println("012D");
        if(moving)
          halt();
      }
      else if((data[4] == 01) && (data[5] == 52))
      {
        Serial.println("0134");
        if(moving)
          halt();
      }
      else
      {
  //      for(uint8_t i = 0; i < 8; i++)
  //      {
  //        Serial.print(data[i],HEX);
  //      }
  //      Serial.println();
      }
    }
    prev_data[0] = data[4];
    prev_data[1] = data[5];
    
    tagRead = false;
  }

  //test
forward();
delay(1000);
turn_left();
delay(1000);
turn_right();
delay(1000);
//end test

  pdCallback();
  delay(4);
}

void serialEvent() {
  int i = 0;
  while (Serial.available()) {
    // get the new tag:
    char inChar = Serial.read();
    if(inChar != '\n' && inChar != '\r')  {
      inbuf[i] = inChar;
    }
    i++;
    delay(1);
  }
  commandRead = true;
}

void serialEvent2() {
  if(tagRead == false)
  {
    while (Serial2.available()) {
      // get the new tag:
      Serial2.readBytes(data, 8);
      tagRead = true;
    }
  }
}

void processCommand(char * command){
  // Echo the command with added \r\n
  //Serial.println(command);
  if (strcmp(command,"forward")==0) {
    forward();
  } 
  else if (strcmp(command,"halt")==0) {
    halt();
  }
  else if (strcmp(command,"left")==0) {
    turn_left();
  }
  else if (strcmp(command,"booted")==0) {
    qik.init();
  }
//  if(!moving)
//    forward();
//  else
//    halt();
}

void forward(){
  moving = true;

  ramp = true;
  
  command = STRAIGHT;
  
  //encoderValueM0 = 0;
  //encoderValueM1 = 0;
  
  outLeft = 0;
  outRight = 0;
  
  setHeading = PI/2;
  oldTheta = PI/2;
  //filteredOmega = 0;
}

void turn_left(){
  moving = true;
  
  inc_done = false;

  command = LEFT;

  //encoderValueM0 = 0;
  //encoderValueM1 = 0;
  
  outLeft = 0;
  outRight = 0;
  
  setHeading = 6*PI/10; //108 degrees
  oldTheta = PI/2;
}

void turn_right(){
  moving = true;
  
  dec_done = false;

  command = RIGHT;

  //encoderValueM0 = 0;
  //encoderValueM1 = 0;
  
  outLeft = 0;
  outRight = 0;
  
  setHeading = 4*PI/10; //72 degrees
  oldTheta = PI/2;
}


void halt(){
  moving = false;

  ramp = false;

  command = STRAIGHT;
  
  //encoderValueM0 = 0;
  //encoderValueM1 = 0;
  
  outLeft = 0;
  outRight = 0;
  
  setHeading = PI/2;
  oldTheta = PI/2;
}

void pdCallback()
{
  switch(command)
  {
    case STRAIGHT:
      Kp = 7;
      Kd = 20;
      newPosM0 = encoderValueM0;
      newPosM1 = encoderValueM1;
      newTime = millis();
      ts = (newTime - oldTime);
      oldTime = newTime;
    
      //velM0 = (newPosM0-oldPosM0) * 1000 /(ts);  // this is for daq
      //velM1 = (newPosM1-oldPosM1) * 1000 /(ts);  // this is for daq
      
      phiM0 = (newPosM0 - oldPosM0) * 1000 * 2*PI / (3408 * ts);
      phiM1 = (newPosM1 - oldPosM1) * 1000 * 2*PI / (3408 * ts);
      
      oldPosM0 = newPosM0;
      oldPosM1 = newPosM1;
    
      omega = (r*phiM1/(2*l)-r*phiM0/(2*l));
      filteredOmega = 0.3679*filteredOmega+0.6321*omega;
      newTheta = oldTheta + filteredOmega * ts / 1000;  // replaced omega with filteredOmega
      error = setHeading - newTheta;
      pd_out = error*Kp - filteredOmega*Kd;

      outRight += pd_out;

      if(ramp)
      {
        outLeft = 40;
        if(outLeft >= 39.9)
        {
          ramp = false;
        }
      }

      if(outLeft > 127)
        outLeft = 127;
      else if(outLeft < -127)
        outLeft = -127;
      
      if(outRight > 127)
        outRight = 127;
      else if(outRight < -127)
        outRight = -127;

      outRight = outLeft;

      qik.setM0Speed(outLeft); //sets the left motor to rotate
      qik.setM1Speed(-outRight); //sets the right motor to rotate
      
      oldTheta = newTheta;

//      Serial.print(ts);
//      Serial.print(" ");
//      Serial.print(velM0);  // this is for daq
//      Serial.print(" ");
//      Serial.print(velM1);
//      Serial.print(newTheta*180/PI);
//      Serial.println();
      break;
    
    case LEFT:
      Kp = 9;
      Kd = 12;
      newPosM0 = encoderValueM0;
      newPosM1 = encoderValueM1;
      newTime = millis();
      ts = (newTime - oldTime);
      oldTime = newTime;
    
      //velM0 = (newPosM0-oldPosM0) * 1000 /(ts);  // this is for daq
      //velM1 = (newPosM1-oldPosM1) * 1000 /(ts);  // this is for daq
      
      phiM0 = (newPosM0 - oldPosM0) * 1000 * 2*PI / (3408 * ts);
      phiM1 = (newPosM1 - oldPosM1) * 1000 * 2*PI / (3408 * ts);
      oldPosM0 = newPosM0;
      oldPosM1 = newPosM1;
    
      omega = (r*phiM1/(2*l)-r*phiM0/(2*l));
      filteredOmega = 0.3679*filteredOmega+0.6321*omega;
      newTheta = oldTheta + filteredOmega * ts / 1000;  // replaced omega with filteredOmega
      error = setHeading - newTheta;
      pd_out = error*Kp - filteredOmega*Kd;

      outLeft -= pd_out/2;
      outRight += pd_out/2;

      if(outLeft > 127)
        outLeft = 127;
      else if(outLeft < -127)
        outLeft = -127;
      
      if(outRight > 127)
        outRight = 127;
      else if(outRight < -127)
        outRight = -127;

      if(!inc_done && ((newTheta+PI/5) >= setHeading)) //gradually increase heading angle by pi/20 until heading angle is reached
      {
        setHeading += PI/20;
        if(newTheta >= PI)
        {
          setHeading = PI;
          inc_done = true;
          forward();
        }
      }

      qik.setM0Speed(outLeft);
      qik.setM1Speed(-outRight);
      
      oldTheta = newTheta;
//      Serial.print(ts);
//      Serial.print(" ");
//      Serial.print(velM0);  // this is for daq
//      Serial.print(" ");
//      Serial.print(velM1);
//      Serial.println();
      break;
  }


    case RIGHT: //needs to be verified
      Kp = 9; //needs verification
      Kd = 12; //needs verification
      newPosM0 = encoderValueM0;
      newPosM1 = encoderValueM1;
      newTime = millis();
      ts = (newTime - oldTime);
      oldTime = newTime;
    
      //velM0 = (newPosM0-oldPosM0) * 1000 /(ts);  // this is for daq
      //velM1 = (newPosM1-oldPosM1) * 1000 /(ts);  // this is for daq
      
      phiM0 = (newPosM0 - oldPosM0) * 1000 * 2*PI / (3408 * ts);
      phiM1 = (newPosM1 - oldPosM1) * 1000 * 2*PI / (3408 * ts);
      oldPosM0 = newPosM0;
      oldPosM1 = newPosM1;
    
	//the following calculations need to be verified
      omega = (r*phiM1/(2*l)-r*phiM0/(2*l));
      filteredOmega = 0.3679*filteredOmega+0.6321*omega;
      newTheta = oldTheta + filteredOmega * ts / 1000;  // replaced omega with filteredOmega
      error = setHeading - newTheta;
      pd_out = error*Kp - filteredOmega*Kd;

      outLeft += pd_out/2;
      outRight -= pd_out/2;

      if(outLeft > 127)
        outLeft = 127;
      else if(outLeft < -127)
        outLeft = -127;
      
      if(outRight > 127)
        outRight = 127;
      else if(outRight < -127)
        outRight = -127;

      if(!dec_done && ((newTheta-PI/5) <= setHeading)) //gradually decrease heading angle by pi/20 until heading angle is reached
      {
        setHeading -= PI/20;
        if(newTheta <= 0)
        {
          setHeading = 0;
          dec_done = true;
          forward();
        }
      }

      qik.setM0Speed(-outLeft); //needs to be verified
      qik.setM1Speed(outRight); //needs to be verified
      
      oldTheta = newTheta;
//      Serial.print(ts);
//      Serial.print(" ");
//      Serial.print(velM0);  // this is for daq
//      Serial.print(" ");
//      Serial.print(velM1);
//      Serial.println();
      break;
  }


//  Serial.print(newTheta*180/PI);
//  Serial.print(" ");
//  Serial.print(ts);
//  Serial.println();
}

//double calcOmega()
//{
//  double omega;
//  
//  newPosM0 = encoderValueM0;
//  newPosM1 = encoderValueM1;
//  
//  omega = ((newPosM1 - oldPosM1)-(newPosM0 - oldPosM0))*POS_2_VEL;
//  
//  oldPosM0 = newPosM0;
//  oldPosM1 = newPosM1;
//  
//  return omega;
//}

void enc_aM0()
{
  if (digitalRead(19) == HIGH)
  {
    if (digitalRead(18) == HIGH)
      encoderValueM0++;
    else
      encoderValueM0--;
  }
  else
  {
    if (digitalRead(18) == HIGH)
      encoderValueM0--;
    else
      encoderValueM0++;
  }
}

void enc_bM0()
{
  if (digitalRead(18) == HIGH)
  {
    if (digitalRead(19) == HIGH)
      encoderValueM0--;
    else
      encoderValueM0++;
  }
  else
  {
    if (digitalRead(19) == HIGH)
      encoderValueM0++;
    else
      encoderValueM0--;
  }
}

void enc_aM1()
{
  if (digitalRead(21) == HIGH)
  {
    if (digitalRead(20) == HIGH)
      encoderValueM1--;
    else
      encoderValueM1++;
  }
  else
  {
    if (digitalRead(20) == HIGH)
      encoderValueM1++;
    else
      encoderValueM1--;
  }
}

void enc_bM1()
{
  if (digitalRead(20) == HIGH)
  {
    if (digitalRead(21) == HIGH)
      encoderValueM1++;
    else
      encoderValueM1--;
  }
  else
  {
    if (digitalRead(21) == HIGH)
      encoderValueM1--;
    else
      encoderValueM1++;
  }
}

