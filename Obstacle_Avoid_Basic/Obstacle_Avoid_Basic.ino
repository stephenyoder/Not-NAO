//#include <NewPing.h>
//#include <PololuQik.h>

//PololuQik2s12v10 qik(2, 3, 4);

#define SONAR_NUM     10 // Number of sensors.
//#define MAX_DISTANCE 200 // Maximum distance (in cm) to ping.
//#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

//unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
//unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
//uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

const double distanceToStopCentimeters = 80; //distance at which we consider an obstacle in the way
long duration;
double distance;
boolean obstacle = false;
int sonarPins[SONAR_NUM][2] = { //Sensor object array
//{trigger pin, echo pin}
  {50, 51}, //sonar0
  {52, 53}, //  .
  {54, 55}, //  .
  {56, 57}, //  .
  {58, 59}, //  .
  {60, 61}, //  .
  {62, 63}, //  .
  {64, 65}, //  .
  {66, 67}, //  .
  {10, 11}, //sonar9
  };

/* code from last year
  NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(50, 51, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(52, 53, MAX_DISTANCE),
  NewPing(54, 55, MAX_DISTANCE),
  NewPing(56, 57, MAX_DISTANCE),
  NewPing(58, 59, MAX_DISTANCE),
  NewPing(60, 61, MAX_DISTANCE),
  NewPing(62, 63, MAX_DISTANCE),
  NewPing(64, 65, MAX_DISTANCE),
  NewPing(66, 67, MAX_DISTANCE),
  NewPing(10, 11, MAX_DISTANCE)
};
*/
void setup() {

  //sets trigger & echo pins as outputs & inputs, respectively
  for(uint8_t i = 0; i < SONAR_NUM; i++){
    pinMode(sonarPins[i][1], OUTPUT); //trigger pins
    pinMode(sonarPins[i][2], INPUT);  //echo pins
  }

  //qik.init();
  
<<<<<<< HEAD
  Serial.begin(9600);
=======
  Serial.begin(115200);

  delay(100);
>>>>>>> 79f4ec3491e9b0bd30e7d7cce4e38e6aa4ebe027
  
/*  // Initialize timmers for sonar array
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
*/

/*    
  qik.setM0Speed(64);
  qik.setM1Speed(-64);
*/
}

void loop() {

  for (uint8_t i = 0; i < SONAR_NUM; i++) { //loops through all 10 sonar sensors
    //clear trigger 
    digitalWrite(sonarPins[i][1], LOW);
<<<<<<< HEAD
    delayMicroseconds(2);
=======
    delayMicroseconds(5);
>>>>>>> 79f4ec3491e9b0bd30e7d7cce4e38e6aa4ebe027
    
    // Sets the trigger on HIGH for 10 us
    digitalWrite(sonarPins[i][1], HIGH);
    delayMicroseconds(10);
    //reads echo and returns the sound wave travel time in us
<<<<<<< HEAD
    duration = digitalRead(sonarPins[i][2]); //pulseIn(sonarPins[i][2], HIGH);
=======
    duration = pulseIn(sonarPins[i][2], HIGH);
    delayMicroseconds(10);
    // Sets the trigger LOW
    digitalWrite(sonarPins[i][1], LOW);

>>>>>>> 79f4ec3491e9b0bd30e7d7cce4e38e6aa4ebe027

    //calculate distance in cm.
    distance = duration * 0.034/2;  

    if(distance <= distanceToStopCentimeters)
      obstacle = true;

    if(obstacle) { //obstacle detected
<<<<<<< HEAD
        Serial.println("Obstacle detected");
        //add code to stop motors
    }
    else { //obstacle not detected
        
=======
      Serial.println("Obstacle detected");
    }
    else { //obstacle not detected
     Serial.println("Clear");
>>>>>>> 79f4ec3491e9b0bd30e7d7cce4e38e6aa4ebe027
    }
      
    Serial.print("Object Distance: ");
    Serial.print(distance);
<<<<<<< HEAD
    Serial.print(" [cm]");
    Serial.print(sonarPins[i][1]);
    Serial.println(sonarPins[i][2]);
=======
    Serial.println(" cm");
>>>>>>> 79f4ec3491e9b0bd30e7d7cce4e38e6aa4ebe027
  }
}

/* code from last year
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = MAX_DISTANCE;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  // Other code that *DOESN'T* analyze ping results can go here.
}
*/

/*
void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}
*/

/*
void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  // The following code would be replaced with your code that does something with the ping results.
  obstacle = false;
  
  for (uint8_t i = 3; i < 5; i++) {
    if(cm[i] <= 150)
      obstacle = true;
    Serial.print(i);
    Serial.print("=");
    Serial.print(cm[i]);
    Serial.print("cm ");
  }
  Serial.println();

  if(obstacle) {
    qik.setM0Speed(0);
    qik.setM1Speed(0);
  }
  else {
    qik.setM0Speed(64);
    qik.setM1Speed(-64);
  }
}
*/
