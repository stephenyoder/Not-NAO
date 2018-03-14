//encoder counts
//volatile long encoderValueM0;
//volatile long encoderValueM1;

int32_t encoderValueM0;
int32_t encoderValueM1;

// Encoder Counter prototypes
void enc_aM0(void);
void enc_bM0(void);
void enc_aM1(void);
void enc_bM1(void);

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once: 
  pinMode(7, INPUT);//change to correct pin numbers
  pinMode(8, INPUT);
  pinMode(4, INPUT);
  pinMode(3, INPUT);

  pinMode(BDPIN_LED_USER_3,OUTPUT);

  attachInterrupt(digitalPinToInterrupt(7), enc_aM0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(8), enc_bM0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(4), enc_aM1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), enc_bM1, CHANGE);
  
 // initialize encoder values
  encoderValueM0 = 0;
encoderValueM1 = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Left ");
  Serial.println(encoderValueM0);
  Serial.print("Right ");
  Serial.println(encoderValueM1);
  
}

/*******************************************************************************
* Update Motor Encoder Values
*******************************************************************************/
void enc_aM0()
{
  if (digitalRead(8) == HIGH)
  {
    if (digitalRead(7) == HIGH)
      encoderValueM0++;
    else
      encoderValueM0--;
  }
  else
  {
    if (digitalRead(7) == HIGH)
      encoderValueM0--;
    else
      encoderValueM0++;
  }
}

void enc_bM0()
{
  if (digitalRead(7) == HIGH)
  {
    if (digitalRead(8) == HIGH)
      encoderValueM0--;
    else
      encoderValueM0++;
  }
  else
  {
    if (digitalRead(8) == HIGH)
      encoderValueM0++;
    else
      encoderValueM0--;
  }
}

void enc_aM1()
{
  if (digitalRead(3) == HIGH)
  {
    if (digitalRead(4) == HIGH)
      encoderValueM1--;
    else
      encoderValueM1++;
  }
  else
  {
    if (digitalRead(4) == HIGH)
      encoderValueM1++;
    else
      encoderValueM1--;
  }
}

void enc_bM1()
{
  if (digitalRead(4) == HIGH)
  {
    if (digitalRead(3) == HIGH)
      encoderValueM1++;
    else
      encoderValueM1--;
  }
  else
  {
    if (digitalRead(3) == HIGH)
      encoderValueM1--;
    else
      encoderValueM1++;
  }
}

