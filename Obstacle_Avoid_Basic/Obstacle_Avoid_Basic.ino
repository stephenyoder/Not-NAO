int trigPin = 50;
int echoPin = 51;

long duration;
double distance;
boolean obstacle = false;
double distanceToStopCentimeters = 70;

void setup() {
  for(int i = 0; i <= 16; i=i+2){
    pinMode(trigPin + i, OUTPUT);
    pinMode(echoPin + i, INPUT);
  }
  pinMode(10, OUTPUT);
  pinMode(11, INPUT);
  Serial.begin(9600);
}

void loop() {
  for(int i = 0; i <= 16; i=i+2){
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
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" [cm] ");
    //Serial.print(trigPin + i);
    //Serial.println(echoPin + i);

    if(distance <= distanceToStopCentimeters)
      obstacle = true;

    if(obstacle)
      Serial.println("Object detected");
    else
      Serial.println("Clear");
  }
  digitalWrite(10, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(10, HIGH);
  delayMicroseconds(10);
  digitalWrite(10, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(11, HIGH);
  // Calculating the distance
  distance= duration*0.034/2; //cm
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" [cm] ");
  
  if(distance <= distanceToStopCentimeters)
    obstacle = true;

  if(obstacle) {
    Serial.println("Object detected");
  }
  else {
    Serial.println("Clear");
  }
}
