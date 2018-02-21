int trigPin = 50;
int echoPin = 51;

long duration;
double distance;
boolean obstacle = false;
double distanceToStopCentimeters = 70;

//sonar arrays on the side are 10,11 & 14,15
//sonar arrays next to side are 50,51 & 16,17

void setup() {
  for(int i = 0; i <= 16; i=i+2){
    initializeSensors(i);
  }
  initializeSensors(10, 11);
  Serial.begin(9600);
}

void loop() {
  for(int i = 0; i <= 16; i=i+2){
    readSensorData(i);
    printSensorData(i, distance);
  }
  
  readSensorData(10, 11);
  printSensorData(10, 11, distance);
}

void initializeSensors(int i){
    pinMode(trigPin + i, OUTPUT);
    pinMode(echoPin + i, INPUT);
}

void initializeSensors(int trig, int echo){
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}
  
void readSensorData(int i){
    // Clears the trigPin
    digitalWrite(trigPin + i, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin + i, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin + i, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin + i, HIGH);
    // Calculating the distance
    distance= duration*0.034/2; //cm
}

void printSensorData(int i, double dist){    
    Serial.print("Sensor");
    Serial.print(i);
    Serial.print(" Distance: ");
    Serial.print(distance);
    Serial.println(" [cm] ");
    //Serial.print(trigPin + i);
    //Serial.println(echoPin + i);
}

boolean obstacleDetected(){
    if(distance <= distanceToStopCentimeters)
      obstacle = true;
    if(obstacle)
      Serial.println("Object detected");
    else
      Serial.println("Clear");
  }

void readSensorData(int pin1, int pin2){
  digitalWrite(pin1, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(pin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin1, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(pin2, HIGH);
  // Calculating the distance
  distance= duration*0.034/2; //cm
}

void printSensorData(int pin1, int pin2, double dist){
  // Prints the distance on the Serial Monitor
  Serial.print("Sensor9 Distance: ");
  Serial.print(distance);
  Serial.println(" [cm] ");
}


