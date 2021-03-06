/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Yoonseok Pyo, Leon Jung, Darby Lim, HanCheol Cho */

#include "OpenCR_core_config.h"
#include "pitches.h"
#include "SimpleTimer.h"

//#include <SoftwareSerial.h>
#include <PololuQik.h>

//the following is for the sonar sensors
#include <ros.h>
#include <ros/time.h>
#include <sensor_msgs/Range.h>
#define USE_USBCON

/*******************************************************************************
* ROS NodeHandle
*******************************************************************************/
ros::NodeHandle nh;

/*******************************************************************************
* Subscriber
*******************************************************************************/
ros::Subscriber<geometry_msgs::Twist> cmd_vel_sub("cmd_vel", commandVelocityCallback);

/*******************************************************************************
* Publisher
*******************************************************************************/
// Bumpers, cliffs, buttons, encoders, battery of Turtlebot3
turtlebot3_msgs::SensorState sensor_state_msg;
ros::Publisher sensor_state_pub("sensor_state", &sensor_state_msg);

//sonar sensors
sensor_msgs::Range range_msg1;
ros::Publisher pub_range1( "sonar_one_range", &range_msg1);
sensor_msgs::Range range_msg2;
ros::Publisher pub_range2( "sonar_two_range", &range_msg2);
sensor_msgs::Range range_msg3;
ros::Publisher pub_range3( "sonar_three_range", &range_msg3);
sensor_msgs::Range range_msg4;
ros::Publisher pub_range4( "sonar_four_range", &range_msg4);
sensor_msgs::Range range_msg5;
ros::Publisher pub_range5( "sonar_five_range", &range_msg5);
sensor_msgs::Range range_msg6;
ros::Publisher pub_range6( "sonar_six_range", &range_msg6);
sensor_msgs::Range range_msg7;
ros::Publisher pub_range7( "sonar_seven_range", &range_msg7);
sensor_msgs::Range range_msg8;
ros::Publisher pub_range8( "sonar_eight_range", &range_msg8);


// IMU of Turtlebot3
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu", &imu_msg);

// Command velocity of Turtlebot3 using RC100 remote controller
geometry_msgs::Twist cmd_vel_rc100_msg;
ros::Publisher cmd_vel_rc100_pub("cmd_vel_rc100", &cmd_vel_rc100_msg);

nav_msgs::Odometry odom;
ros::Publisher odom_pub("odom", &odom);

sensor_msgs::JointState joint_states;
ros::Publisher joint_states_pub("joint_states", &joint_states);

/*******************************************************************************
* Transform Broadcaster
*******************************************************************************/
// TF of Turtlebot3
geometry_msgs::TransformStamped tfs_msg;
geometry_msgs::TransformStamped odom_tf;
tf::TransformBroadcaster tfbroadcaster;

/*******************************************************************************
* SoftwareTimer of Turtlebot3
*******************************************************************************/
static uint32_t tTime[4];

/*******************************************************************************
* Declaration for motor
*******************************************************************************/
//Turtlebot3MotorDriver motor_driver;
bool init_encoder_[2]  = {false, false};
int32_t last_diff_tick_[2];
int32_t last_tick_[2];
double last_rad_[2];
double last_velocity_[2];
double goal_linear_velocity  = 0.0;
double goal_angular_velocity = 0.0;

/*******************************************************************************
* Declaration for odom/imu
*******************************************************************************/
bool init_orientation = false;
bool robot_moving = true;
bool init_imu_orientation = false;

/*******************************************************************************
* Declaration for IMU
*******************************************************************************/
cIMU imu;

/*******************************************************************************
* Declaration for RC100 remote controller
*******************************************************************************/
RC100 remote_controller;
double const_cmd_vel    = 0.2;

/*******************************************************************************
* Declaration for test drive
*******************************************************************************/
bool start_move = false;
bool start_rotate = false;
int32_t last_left_encoder  = 0;
int32_t last_right_encoder = 0;

/*******************************************************************************
* Declaration for SLAM and navigation
*******************************************************************************/
unsigned long prev_update_time;
float odom_pose[3];
char *joint_states_name[] = {"wheel_left_joint", "wheel_right_joint"};
float joint_states_pos[2] = {0.0, 0.0};
float joint_states_vel[2] = {0.0, 0.0};
float joint_states_eff[2] = {0.0, 0.0};

/*******************************************************************************
* Declaration for LED
*******************************************************************************/
#define LED_TXD         0
#define LED_RXD         1
#define LED_LOW_BATTERY 2
#define LED_ROS_CONNECT 3

/*******************************************************************************
* Declaration for Battery
*******************************************************************************/
#define BATTERY_POWER_OFF             0
#define BATTERY_POWER_STARTUP         1
#define BATTERY_POWER_NORMAL          2
#define BATTERY_POWER_CHECK           3
#define BATTERY_POWER_WARNNING        4

static bool    setup_end       = false;
static uint8_t battery_voltage = 0;
static float   battery_valtage_raw = 0;
static uint8_t battery_state   = BATTERY_POWER_OFF;

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

#define BDPIN_LED_USER_3        24
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
PololuQik2s12v10 qik(12);
//PololuQik2s12v10 qik(2, 3, 4);
// **************** FOR ROBOT BASE ******************
// Motor 0 is the left and Motor 1 is the right
// **************************************************

//******Initializes built-in buzzer*******
#define BDPIN_BUZZER 31

int beep = NOTE_C4;
int noteDuration = 200;
//****************************************

//*******sonar array parameters********************
int trigPin = 50;
int echoPin = 51;

long duration;
double distance;
boolean obstacle = false;
const double DISTANCE_TO_STOP_CM = 70;
long range_time;
char frameid1[] = "sonar_one_range";
char frameid2[] = "sonar_two_range";
char frameid3[] = "sonar_three_range";
char frameid4[] = "sonar_four_range";
char frameid5[] = "sonar_five_range";
char frameid6[] = "sonar_six_range";
char frameid7[] = "sonar_seven_range";
char frameid8[] = "sonar_eight_range";
//**************************************************

//******rotation speed*******
const double ROTATION_VELOCITY = 35.0;
//****************************************
//**********functions for sonar array****************

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
    distance = duration*0.034/2; //cm
}

void printSensorData(int pin1, int pin2, double dist){
  // Prints the distance on the Serial Monitor
  Serial.print("Sensor9 Distance: ");
  Serial.print(distance);
  Serial.println(" [cm] ");
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

boolean obstacleDetected(){
    if(distance <= DISTANCE_TO_STOP_CM){
      obstacle = true;
      Serial.println("Object detected");
    }
    else{ //obstacle remains false by default
      Serial.println("Clear");
      obstacle = false;
    }

    return obstacle;
  }
void sonarSweep(){
  for(int i = 0; i <= 12; i=i+2){ //excludes the two sensors at the bottom
    readSensorData(i);
    printSensorData(i, distance);
    if(obstacleDetected())
      break;
  }
  
  readSensorData(10, 11);
  printSensorData(10, 11, distance);
  }

void resetObstacle(){
  obstacle = false;
}

///************end of functions for sonar array*********************

/*******************************************************************************
* Setup function
*******************************************************************************/
void setup(){

  Serial.begin(115200);

 //tests buzzer that is to be used for obstacle avoidance. buzzer should beep twice then turn off
//*************************************************
tone(BDPIN_BUZZER, beep, noteDuration);
delay(noteDuration / 2);
tone(BDPIN_BUZZER, beep, noteDuration);
noTone(BDPIN_BUZZER);
//*************************************************

//**************initializes sonar array************
for(int i = 0; i <= 16; i=i+2){
    initializeSensors(i);
  }
  initializeSensors(10, 11);
//*************************************************

  qik.init();
  
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
  
  // Initialize ROS node handle, advertise and subscribe the topics
  nh.initNode();
  //nh.getHardware()->setBaud(115200);
  nh.subscribe(cmd_vel_sub);
  nh.advertise(sensor_state_pub);
  //sonar sensors
  nh.advertise(pub_range1);
  nh.advertise(pub_range2);
  nh.advertise(pub_range3);
  nh.advertise(pub_range4);
  nh.advertise(pub_range5);
  nh.advertise(pub_range6);
  nh.advertise(pub_range7);
  nh.advertise(pub_range8);
  
  nh.advertise(imu_pub);
 // nh.advertise(cmd_vel_rc100_pub);
  nh.advertise(odom_pub);
  nh.advertise(joint_states_pub);
  tfbroadcaster.init(nh);

  nh.loginfo("Connected to OpenCR board!");

  // Setting for Dynamixel motors
 // motor_driver.init();

 // Setting up Sonar Sensors
  range_msg1.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg1.header.frame_id =  frameid1;
  range_msg1.field_of_view = 0.21;  //now 12 deg, was 18 degrees in rad
  range_msg1.min_range = 0.09; //in meters, was 0.09m
  range_msg1.max_range = 1.0; //was 2m

  range_msg2.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg2.header.frame_id =  frameid2;
  range_msg2.field_of_view = 0.21;  // 18 degrees in rad
  range_msg2.min_range = 0.09; //in meters
  range_msg2.max_range = 1.0;

  range_msg3.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg3.header.frame_id =  frameid3;
  range_msg3.field_of_view = 0.21;  // 18 degrees in rad
  range_msg3.min_range = 0.09; //in meters
  range_msg3.max_range = 1.0; //

  range_msg4.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg4.header.frame_id =  frameid4;
  range_msg4.field_of_view = 0.21;  // 18 degrees in rad
  range_msg4.min_range = 0.09; //in meters
  range_msg4.max_range = 1.0;

  range_msg5.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg5.header.frame_id =  frameid5;
  range_msg5.field_of_view = 0.21;  // 18 degrees in rad
  range_msg5.min_range = 0.09; //in meters
  range_msg5.max_range = 1.0;

  range_msg6.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg6.header.frame_id =  frameid6;
  range_msg6.field_of_view = 0.21;  // 18 degrees in rad
  range_msg6.min_range = 0.09; //in meters
  range_msg6.max_range = 1.0;

  range_msg7.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg7.header.frame_id =  frameid7;
  range_msg7.field_of_view = 0.21;  // 18 degrees in rad
  range_msg7.min_range = 0.09; //in meters
  range_msg7.max_range = 1.0;

  range_msg8.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg8.header.frame_id =  frameid8;
  range_msg8.field_of_view = 0.21;  // 18 degrees in rad
  range_msg8.min_range = 0.09; //in meters
  range_msg8.max_range = 1.0;

  

  // Setting for IMU
  imu.begin();

  // Setting for ROBOTIS RC100 remote controller and cmd_vel
  ///remote_controller.begin(1);  // 57600bps baudrate for RC100 control

  cmd_vel_rc100_msg.linear.x  = 0.0;
  cmd_vel_rc100_msg.angular.z = 0.0;

  // Setting for SLAM and navigation (odometry, joint states, TF)
  odom_pose[0] = 0.0;
  odom_pose[1] = 0.0;
  odom_pose[2] = 0.0;

  joint_states.header.frame_id = "base_footprint";
  joint_states.name            = joint_states_name;

  joint_states.name_length     = 2;
  joint_states.position_length = 2;
  joint_states.velocity_length = 2;
  joint_states.effort_length   = 2;

  prev_update_time = millis();

  pinMode(13, OUTPUT);
  //Serial2.begin(115200);
  //SerialBT2.begin(57600);

  setup_end = true;
  //SimpleTimer timer;
  //int timerID;

  
  
}

/*******************************************************************************
* Loop function
*******************************************************************************/
void loop()
{
  //resetObstacle(); //every loop iteration, set reset obstacle to false. If an obstacle is detected in the following lines it obstacle will become true
  /*
  SimpleTimer timer;
  int timerID = timer.setInterval(1000, sonarSweep); //Call sonarSweep every 1 second
  */
  //sonarSweep();

 // receiveRemoteControlData();

//sonar arrays on the side are 10,11 & 14,15
//sonar arrays next to side are 50,51 & 16,17


  if ((millis()-tTime[0]) >= (1000 / CONTROL_MOTOR_SPEED_PERIOD)){
    //if(!obstacle){
    controlMotorSpeed();
    tTime[0] = millis();
 // qik.getErrors();
 }
 if(millis() >= range_time ){
      //first pin trig, second echo
      readSensorData(10,11);//sonar 1
      //int r =0;  
      range_msg1.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg1.header.stamp = nh.now();
      pub_range1.publish(&range_msg1);
      //range_time =  millis() + 7; //50; probably don't need to update everytime
      
      readSensorData(50,51);//sonar 2
      range_msg2.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg2.header.stamp = nh.now();
      pub_range2.publish(&range_msg2);
      //range_time =  millis() + 7; //50;
      
      readSensorData(52,53);//sonar 3
      range_msg3.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg3.header.stamp = nh.now();
      pub_range3.publish(&range_msg3);
      //range_time =  millis() + 7; //50;
      
      readSensorData(54,55);//sonar 4
      range_msg4.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg4.header.stamp = nh.now();
      pub_range4.publish(&range_msg4);
      //range_time =  millis() + 7; //50;
      
      readSensorData(56,57);//sonar 5
      range_msg5.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg5.header.stamp = nh.now();
      pub_range5.publish(&range_msg5);
      //range_time =  millis() + 7; //50;
      
      readSensorData(58,59);//sonar 6
      range_msg6.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg6.header.stamp = nh.now();
      pub_range6.publish(&range_msg6);
      //range_time =  millis() + 7; //50;
      
      readSensorData(60,61);//sonar 7
      range_msg7.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg7.header.stamp = nh.now();
      pub_range7.publish(&range_msg7);
      //range_time =  millis() + 7; //50;
      
      readSensorData(62,63);//sonar 8
      range_msg8.range = distance / 100; //divide by 100 to convert cm -> m
      range_msg8.header.stamp = nh.now();
      pub_range8.publish(&range_msg8);
      range_time =  millis() + 200; //update frquency = 5hz or + 200
      
}
    /*else{
      stopMotor();
      tone(BDPIN_BUZZER, beep, noteDuration);
      delay(noteDuration / 2);
      noTone(BDPIN_BUZZER);
      */
/*    Need some function to keep track of time so robot does not rotate infinitely
      while(obstacle){
        rotateClockwise();
      }
      */
//    }
//  }

  if ((millis()-tTime[1]) >= (1000 / CMD_VEL_PUBLISH_PERIOD))
  {
   // cmd_vel_rc100_pub.publish(&cmd_vel_rc100_msg);
    tTime[1] = millis();
  }

  if ((millis()-tTime[2]) >= (1000 / DRIVE_INFORMATION_PUBLISH_PERIOD))
  {
    publishSensorStateMsg();
    publishDriveInformation();
    tTime[2] = millis();
  }

  if ((millis()-tTime[3]) >= (1000 / IMU_PUBLISH_PERIOD))
  {
    publishImuMsg();
    tTime[3] = millis();
  }

  // Check push button pressed for simple test drive
  checkPushButtonState();

  // Update the IMU unit
  imu.update();

  // Start Gyro Calibration after ROS connection
  updateGyroCali();

  // Show LED status
  showLedStatus();

  // Update Voltage
  updateVoltageCheck();

  // Call all the callbacks waiting to be called at that point in time
  nh.spinOnce();
  //Serial.print("left ");
  //Serial.println(encoderValueM0);
  //Serial.print("right ");
  //Serial.println(encoderValueM1);
  
}

//**********************end of main loop****************************************

/*******************************************************************************
* Callback function for cmd_vel msg
*******************************************************************************/
void commandVelocityCallback(const geometry_msgs::Twist& cmd_vel_msg)
{
  goal_linear_velocity  = cmd_vel_msg.linear.x;
  goal_angular_velocity = cmd_vel_msg.angular.z;
}

/*******************************************************************************
* Publish msgs (IMU data: angular velocity, linear acceleration, orientation)
*******************************************************************************/
void publishImuMsg(void)
{
  static geometry_msgs::Quaternion last_imu_orientation;
  static geometry_msgs::Quaternion last_tf_orientation;
  
  imu_msg.header.stamp    = nh.now();
  imu_msg.header.frame_id = "imu_link";

  imu_msg.angular_velocity.x = imu.SEN.gyroADC[0];
  imu_msg.angular_velocity.y = imu.SEN.gyroADC[1];
  imu_msg.angular_velocity.z = imu.SEN.gyroADC[2];
  imu_msg.angular_velocity_covariance[0] = 0.02;
  imu_msg.angular_velocity_covariance[1] = 0;
  imu_msg.angular_velocity_covariance[2] = 0;
  imu_msg.angular_velocity_covariance[3] = 0;
  imu_msg.angular_velocity_covariance[4] = 0.02;
  imu_msg.angular_velocity_covariance[5] = 0;
  imu_msg.angular_velocity_covariance[6] = 0;
  imu_msg.angular_velocity_covariance[7] = 0;
  imu_msg.angular_velocity_covariance[8] = 0.02;

  imu_msg.linear_acceleration.x = imu.SEN.accADC[0];
  imu_msg.linear_acceleration.y = imu.SEN.accADC[1];
  imu_msg.linear_acceleration.z = imu.SEN.accADC[2];
  imu_msg.linear_acceleration_covariance[0] = 0.04;
  imu_msg.linear_acceleration_covariance[1] = 0;
  imu_msg.linear_acceleration_covariance[2] = 0;
  imu_msg.linear_acceleration_covariance[3] = 0;
  imu_msg.linear_acceleration_covariance[4] = 0.04;
  imu_msg.linear_acceleration_covariance[5] = 0;
  imu_msg.linear_acceleration_covariance[6] = 0;
  imu_msg.linear_acceleration_covariance[7] = 0;
  imu_msg.linear_acceleration_covariance[8] = 0.04;

  if(!init_imu_orientation)
  {
    imu_msg.orientation.w = imu.quat[0];
    imu_msg.orientation.x = imu.quat[1];
    imu_msg.orientation.y = imu.quat[2];
    imu_msg.orientation.z = imu.quat[3]; 

    tfs_msg.transform.rotation.w = imu.quat[0];
    tfs_msg.transform.rotation.x = imu.quat[1];
    tfs_msg.transform.rotation.y = imu.quat[2];
    tfs_msg.transform.rotation.z = imu.quat[3];
    
    init_imu_orientation=true;
  }

  if(!robot_moving)
  {//only really need to keep track of w and z as those are changing steadily more so
    imu_msg.orientation.w = last_imu_orientation.w;
    imu_msg.orientation.x = last_imu_orientation.x;
    imu_msg.orientation.y = last_imu_orientation.y;
    imu_msg.orientation.z = last_imu_orientation.z;
  }
  else
  {
    imu_msg.orientation.w = imu.quat[0];
    imu_msg.orientation.x = imu.quat[1];
    imu_msg.orientation.y = imu.quat[2];
    imu_msg.orientation.z = imu.quat[3];
  }
  //Serial.println(imu_msg.orientation.w,6);
  //Serial.println(imu_msg.orientation.z,6);
  //Serial.println(imu_msg.orientation.x,6);
  //Serial.println(imu_msg.orientation.y,6);
  //set w back to one if delta_theta is small

  imu_msg.orientation_covariance[0] = 0.0025;
  imu_msg.orientation_covariance[1] = 0;
  imu_msg.orientation_covariance[2] = 0;
  imu_msg.orientation_covariance[3] = 0;
  imu_msg.orientation_covariance[4] = 0.0025;
  imu_msg.orientation_covariance[5] = 0;
  imu_msg.orientation_covariance[6] = 0;
  imu_msg.orientation_covariance[7] = 0;
  imu_msg.orientation_covariance[8] = 0.0025;

  imu_pub.publish(&imu_msg);

  tfs_msg.header.stamp    = nh.now();
  tfs_msg.header.frame_id = "base_link";
  tfs_msg.child_frame_id  = "imu_link";

  if(!robot_moving)
  {//only really need to keep track of w and z as those are changing steadily more so
    tfs_msg.transform.rotation.w = last_tf_orientation.w;
    tfs_msg.transform.rotation.x = last_tf_orientation.x;
    tfs_msg.transform.rotation.y = last_tf_orientation.y;
    tfs_msg.transform.rotation.z = last_tf_orientation.z;
  }
  else
  {
    tfs_msg.transform.rotation.w = imu.quat[0];
    tfs_msg.transform.rotation.x = imu.quat[1];
    tfs_msg.transform.rotation.y = imu.quat[2];
    tfs_msg.transform.rotation.z = imu.quat[3];
  }

  tfs_msg.transform.translation.x = -0.032;
  tfs_msg.transform.translation.y = 0.0;
  tfs_msg.transform.translation.z = 0.068;

  last_imu_orientation.w = imu_msg.orientation.w;
  last_imu_orientation.x = imu_msg.orientation.x;
  last_imu_orientation.y = imu_msg.orientation.y;
  last_imu_orientation.z = imu_msg.orientation.z;
  
  last_tf_orientation.w = tfs_msg.transform.rotation.w;
  last_tf_orientation.x = tfs_msg.transform.rotation.x;
  last_tf_orientation.y = tfs_msg.transform.rotation.y;
  last_tf_orientation.z = tfs_msg.transform.rotation.z;

  //Serial.println(tfs_msg.transform.rotation.w,6);
  //Serial.println(tfs_msg.transform.rotation.z,6);
  //Serial.println(tfs_msg.transform.rotation.x,6);
  //Serial.println(tfs_msg.transform.rotation.y,6);

  tfbroadcaster.sendTransform(tfs_msg);
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



/*******************************************************************************
* Publish msgs (sensor_state: bumpers, cliffs, buttons, encoders, battery)
*******************************************************************************/
void publishSensorStateMsg(void)
{
  bool dxl_comm_result = false;

  int32_t current_tick;

  sensor_state_msg.stamp = nh.now();
  sensor_state_msg.battery = checkVoltage();

  //dxl_comm_result = motor_driver.readEncoder(sensor_state_msg.left_encoder, sensor_state_msg.right_encoder);

  // **************** FOR ROBOT BASE ******************
  // Motor 0 is the left and Motor 1 is the right
  // **************************************************

  sensor_state_msg.left_encoder = encoderValueM0; //robot thought it was going opposite directionu
  sensor_state_msg.right_encoder = encoderValueM1;
  
  //if (dxl_comm_result == true)
  //{
    sensor_state_pub.publish(&sensor_state_msg);
  //}
  //else
  //{
  //  return;
  //}

  current_tick = sensor_state_msg.left_encoder;

  if (!init_encoder_[LEFT])
  {
    last_tick_[LEFT] = current_tick;
    init_encoder_[LEFT] = true;
  }

  last_diff_tick_[LEFT] = current_tick - last_tick_[LEFT];
  last_tick_[LEFT] = current_tick;
  last_rad_[LEFT] += TICK2RAD * (double)last_diff_tick_[LEFT]; //update TICK2RAD value

  current_tick = sensor_state_msg.right_encoder;

  if (!init_encoder_[RIGHT])
  {
    last_tick_[RIGHT] = current_tick;
    init_encoder_[RIGHT] = true;
  }

  last_diff_tick_[RIGHT] = current_tick - last_tick_[RIGHT];
  last_tick_[RIGHT] = current_tick;
  last_rad_[RIGHT] += TICK2RAD * (double)last_diff_tick_[RIGHT];
}

/*******************************************************************************
* Publish msgs (odometry, joint states, tf)
*******************************************************************************/
void publishDriveInformation(void)
{
  unsigned long time_now = millis();
  unsigned long step_time = time_now - prev_update_time;
  prev_update_time = time_now;
  ros::Time stamp_now = nh.now();

  // odom
  updateOdometry((double)(step_time * 0.001));
  odom.header.stamp = stamp_now;
  odom_pub.publish(&odom);

  // joint_states
  updateJoint();
  joint_states.header.stamp = stamp_now;
  joint_states_pub.publish(&joint_states);

  // tf
  updateTF(odom_tf);
  tfbroadcaster.sendTransform(odom_tf);
}

/*******************************************************************************
* Calculate the odometry
*******************************************************************************/
bool updateOdometry(double diff_time)
{
  double odom_vel[3];

  double wheel_l, wheel_r;      // rotation value of wheel [rad]
  double delta_s, delta_theta;
  static double last_theta = 0.0;
  static geometry_msgs::Quaternion last_orientation; //make static
  double v, w;                  // v = translational velocity [m/s], w = rotational velocity [rad/s]
  double step_time;

  wheel_l = wheel_r = 0.0;
  delta_s = delta_theta = 0.0;
  v = w = 0.0;
  step_time = 0.0;

  step_time = diff_time;

  if (step_time == 0)
    return false;

  wheel_l = TICK2RAD * (double)last_diff_tick_[LEFT];
  wheel_r = TICK2RAD * (double)last_diff_tick_[RIGHT];

  if (isnan(wheel_l))
    wheel_l = 0.0;

  if (isnan(wheel_r))
    wheel_r = 0.0;

  delta_s     = WHEEL_RADIUS * (wheel_r + wheel_l) / 2.0;
  delta_theta = atan2f(imu.quat[1]*imu.quat[2] + imu.quat[0]*imu.quat[3],
                       0.5f - imu.quat[2]*imu.quat[2] - imu.quat[3]*imu.quat[3]) - last_theta;

  //move update after delta_theta correction
  /*v = delta_s / step_time;
  w = delta_theta / step_time;*/
  

  last_velocity_[LEFT]  = wheel_l / step_time;
  last_velocity_[RIGHT] = wheel_r / step_time;

  Serial.print("i");
  Serial.println(imu.quat[3],7);

  Serial.print(" delta_theta before = ");
  Serial.println(delta_theta,6);
  //.00009 is too small, 0.0002 is too small, saw 0.000696 once, was 0.0003, 0.00045
  if(abs(delta_theta) <= 0.0017)//maybe also compare to previous delta_theta to get rid of outliers
  {
    //Serial.println("yes");
    delta_theta = 0;
    robot_moving = false;
  }

   v = delta_s / step_time;
  w = delta_theta / step_time;

  // compute odometric pose
  odom_pose[0] += delta_s * cos(odom_pose[2] + (delta_theta / 2.0));
  odom_pose[1] += delta_s * sin(odom_pose[2] + (delta_theta / 2.0));
  odom_pose[2] += delta_theta;//find delta_theta when robot isn't moving to make dead zone
  Serial.print(" delta_theta = ");
  Serial.println(delta_theta,6);
  Serial.print("odom_pose[2] = ");
  Serial.println(odom_pose[2],6);

  // compute odometric instantaneouse velocity
  odom_vel[0] = v;
  odom_vel[1] = 0.0;
  odom_vel[2] = w;

  odom.pose.pose.position.x = odom_pose[0];
  odom.pose.pose.position.y = odom_pose[1];
  odom.pose.pose.position.z = 0;
  if(!init_orientation)
  { 
   last_orientation = tf::createQuaternionFromYaw(odom_pose[2]);
   init_orientation = true; 
  }
  if(abs(delta_theta) <= 0.0011)
  {
    //Serial.println("yes");
    odom.pose.pose.orientation = last_orientation;
  }
  else
  {
    robot_moving = true;
    odom.pose.pose.orientation = tf::createQuaternionFromYaw(odom_pose[2]);
  }
  //Serial.print("orientation = ");
  //Serial.println(odom.pose.pose.orientation);

  // We should update the twist of the odometry
  odom.twist.twist.linear.x  = odom_vel[0];
  odom.twist.twist.angular.z = odom_vel[2];

  last_orientation = odom.pose.pose.orientation; //store last orientation

  last_theta = atan2f(imu.quat[1]*imu.quat[2] + imu.quat[0]*imu.quat[3],
                      0.5f - imu.quat[2]*imu.quat[2] - imu.quat[3]*imu.quat[3]);

  return true;
}

/*******************************************************************************
* Calculate the joint states
*******************************************************************************/
void updateJoint(void)
{
  joint_states_pos[LEFT]  = last_rad_[LEFT];
  joint_states_pos[RIGHT] = last_rad_[RIGHT];

  joint_states_vel[LEFT]  = last_velocity_[LEFT];
  joint_states_vel[RIGHT] = last_velocity_[RIGHT];

  joint_states.position = joint_states_pos;
  joint_states.velocity = joint_states_vel;
}

/*******************************************************************************
* Calculate the TF
*******************************************************************************/
void updateTF(geometry_msgs::TransformStamped& odom_tf)
{
  odom.header.frame_id = "odom";
  odom_tf.header = odom.header;
  odom_tf.child_frame_id = "base_footprint";
  odom_tf.transform.translation.x = odom.pose.pose.position.x;
  odom_tf.transform.translation.y = odom.pose.pose.position.y;
  odom_tf.transform.translation.z = odom.pose.pose.position.z;
  odom_tf.transform.rotation = odom.pose.pose.orientation;
}


/*******************************************************************************
* Receive remocon (RC100) data
*******************************************************************************/
void receiveRemoteControlData(void)
{
  int received_data = 0;

  if (remote_controller.available())
  {
    received_data = remote_controller.readData();

    if (received_data & RC100_BTN_U)
    {
      cmd_vel_rc100_msg.linear.x += VELOCITY_LINEAR_X * SCALE_VELOCITY_LINEAR_X;
    }
    else if (received_data & RC100_BTN_D)
    {
      cmd_vel_rc100_msg.linear.x -= VELOCITY_LINEAR_X * SCALE_VELOCITY_LINEAR_X;
    }

    if (received_data & RC100_BTN_L)
    {
      cmd_vel_rc100_msg.angular.z += VELOCITY_ANGULAR_Z * SCALE_VELOCITY_ANGULAR_Z;
    }
    else if (received_data & RC100_BTN_R)
    {
      cmd_vel_rc100_msg.angular.z -= VELOCITY_ANGULAR_Z * SCALE_VELOCITY_ANGULAR_Z;
    }

    if (received_data & RC100_BTN_6)
    {
      cmd_vel_rc100_msg.linear.x  = const_cmd_vel;
      cmd_vel_rc100_msg.angular.z = 0.0;
    }
    else if (received_data & RC100_BTN_5)
    {
      cmd_vel_rc100_msg.linear.x  = 0.0;
      cmd_vel_rc100_msg.angular.z = 0.0;
    }

    if (cmd_vel_rc100_msg.linear.x > MAX_LINEAR_VELOCITY)
    {
      cmd_vel_rc100_msg.linear.x = MAX_LINEAR_VELOCITY;
    }

    if (cmd_vel_rc100_msg.angular.z > MAX_ANGULAR_VELOCITY)
    {
      cmd_vel_rc100_msg.angular.z = MAX_ANGULAR_VELOCITY;
    }

    goal_linear_velocity  = cmd_vel_rc100_msg.linear.x;
    goal_angular_velocity = cmd_vel_rc100_msg.angular.z;
  }
}

/******************************************************************************
* Rotate robot clockwise
*******************************************************************************/
void rotateClockwise(void){
  
  if(!(last_velocity_[RIGHT] == 0 && last_velocity_[LEFT] == 0)){
    stopMotor();
  }
  while(obstacle){  
    resetObstacle();
    qik.setM0Speed(ROTATION_VELOCITY);
    qik.setM1Speed(-ROTATION_VELOCITY);
    sonarSweep(); 
  } 

  stopMotor();
}

/******************************************************************************
* Rotate robot counterclockwise
*******************************************************************************/
void rotateCounterClockwise(void){
  
  if(!(last_velocity_[RIGHT] == 0 && last_velocity_[LEFT] == 0)){
    stopMotor();
  }
  while(obstacle){
    resetObstacle();
    qik.setM0Speed(-ROTATION_VELOCITY);
    qik.setM1Speed(ROTATION_VELOCITY);
    sonarSweep();
  }
}

/******************************************************************************
* Slow down robot until stop
*******************************************************************************/
void stopMotor(void){
  //use code from function below to 

  //gradually slow down motors
  for(int i = 0; i < 3; i++){
    qik.setM0Speed(last_velocity_[LEFT] / 2);
    qik.setM1Speed(last_velocity_[RIGHT] / 2);
    delay(125);
  }

  qik.setM0Speed(0.0);
  qik.setM1Speed(0.0);
}

/*******************************************************************************
* Control motor speed
*******************************************************************************/
void controlMotorSpeed(void)
{
  bool dxl_comm_result = false;

  double wheel_speed_cmd[2];
  double lin_vel1;
  double lin_vel2;

  wheel_speed_cmd[LEFT]  = goal_linear_velocity - (goal_angular_velocity * WHEEL_SEPARATION / 2);
  wheel_speed_cmd[RIGHT] = -1 * (goal_linear_velocity + (goal_angular_velocity * WHEEL_SEPARATION / 2));

  lin_vel1 = wheel_speed_cmd[LEFT] * VELOCITY_CONSTANT_VALUE;
  if (lin_vel1 > LIMIT_X_MAX_VELOCITY)
  {
    lin_vel1 =  LIMIT_X_MAX_VELOCITY;
  }
  else if (lin_vel1 < -LIMIT_X_MAX_VELOCITY)
  {
    lin_vel1 = -LIMIT_X_MAX_VELOCITY;
  }

  lin_vel2 = wheel_speed_cmd[RIGHT] * VELOCITY_CONSTANT_VALUE;
  if (lin_vel2 > LIMIT_X_MAX_VELOCITY)
  {
    lin_vel2 =  LIMIT_X_MAX_VELOCITY;
  }
  else if (lin_vel2 < -LIMIT_X_MAX_VELOCITY)
  {
    lin_vel2 = -LIMIT_X_MAX_VELOCITY;
  }

  
  qik.setM0Speed(lin_vel1);
  qik.setM1Speed(lin_vel2);
  //dxl_comm_result = motor_driver.speedControl((int64_t)lin_vel1, (int64_t)lin_vel2);
  //if (dxl_comm_result == false)
  //  return;
}

/*******************************************************************************
* Get Button Press (Push button 1, Push button 2)
*******************************************************************************/
uint8_t getButtonPress(void)
{
  uint8_t button_state = 0;
  static uint32_t t_time[2];
  static uint8_t button_state_num[2] = {0, };

  for (int button_num = 0; button_num < 2; button_num++)
  {
    switch (button_state_num[button_num])
    {
     case WAIT_FOR_BUTTON_PRESS:
       if (getPushButton() & (1 << button_num))
       {
         t_time[button_num] = millis();
         button_state_num[button_num] = WAIT_SECOND;
       }
       break;

     case WAIT_SECOND:
       if ((millis()-t_time[button_num]) >= 1000)
       {
         if (getPushButton() & (1 << button_num))
         {
           button_state_num[button_num] = CHECK_BUTTON_RELEASED;
           button_state |= (1 << button_num);
         }
         else
         {
           button_state_num[button_num] = WAIT_FOR_BUTTON_PRESS;
         }
       }
       break;

     case CHECK_BUTTON_RELEASED:
       if (!(getPushButton() & (1 << button_num)))
         button_state_num[button_num] = WAIT_FOR_BUTTON_PRESS;
       break;

     default :
       button_state_num[button_num] = WAIT_FOR_BUTTON_PRESS;
       break;
    }
  }

  return button_state;
}

/*******************************************************************************
* Turtlebot3 test drive using RC100 remote controller
*******************************************************************************/
void testDrive(void)
{
  int32_t current_tick = sensor_state_msg.right_encoder;
  double diff_encoder = 0.0;

  if (start_move)
  {
    diff_encoder = TEST_DISTANCE / (0.207 / 4096); // (Circumference of Wheel) / (The number of tick per revolution)

    if (abs(last_right_encoder - current_tick) <= diff_encoder)
    {
      goal_linear_velocity  = 0.05 * SCALE_VELOCITY_LINEAR_X;
    }
    else
    {
      goal_linear_velocity  = 0.0;
      start_move = false;
    }
  }
  else if (start_rotate)
  {
    diff_encoder = (TEST_RADIAN * TURNING_RADIUS) / (0.207 / 4096);

    if (abs(last_right_encoder - current_tick) <= diff_encoder)
    {
      goal_angular_velocity= -0.7 * SCALE_VELOCITY_ANGULAR_Z;
    }
    else
    {
      goal_angular_velocity  = 0.0;
      start_rotate = false;
    }
  }
}

/*******************************************************************************
* Check Push Button State
*******************************************************************************/
void checkPushButtonState()
{
  uint8_t button_state = getButtonPress();

  if (button_state & (1<<0))
  {
    start_move = true;
    last_left_encoder = sensor_state_msg.left_encoder;
    last_right_encoder = sensor_state_msg.right_encoder;
  }

  if (button_state & (1<<1))
  {
    start_rotate = true;
    last_left_encoder = sensor_state_msg.left_encoder;
    last_right_encoder = sensor_state_msg.right_encoder;
  }

  testDrive();
}

/*******************************************************************************
* Check voltage
*******************************************************************************/
float checkVoltage(void)
{
  float vol_value;

  vol_value = getPowerInVoltage();

  return vol_value;
}

/*******************************************************************************
* Show LED status
*******************************************************************************/
void showLedStatus(void)
{
  static uint32_t t_time = millis();

  if ((millis()-t_time) >= 500)
  {
    t_time = millis();
    digitalWrite(13, !digitalRead(13));
  }

  if (getPowerInVoltage() < 11.1)
  {
    setLedOn(LED_LOW_BATTERY);
  }
  else
  {
    setLedOff(LED_LOW_BATTERY);
  }

  if (nh.connected())
  {
    setLedOn(LED_ROS_CONNECT);
  }
  else
  {
    setLedOff(LED_ROS_CONNECT);
  }

  updateRxTxLed();
}

void updateRxTxLed(void)
{
  static uint32_t rx_led_update_time;
  static uint32_t tx_led_update_time;
  static uint32_t rx_cnt;
  static uint32_t tx_cnt;

  if ((millis()-tx_led_update_time) > 50)
  {
    tx_led_update_time = millis();

    if (tx_cnt != Serial.getTxCnt())
    {
      setLedToggle(LED_TXD);
    }
    else
    {
      setLedOff(LED_TXD);
    }

    tx_cnt = Serial.getTxCnt();
  }

  if ((millis()-rx_led_update_time) > 50)
  {
    rx_led_update_time = millis();

    if (rx_cnt != Serial.getRxCnt())
    {
      setLedToggle(LED_RXD);
    }
    else
    {
      setLedOff(LED_RXD);
    }

    rx_cnt = Serial.getRxCnt();
  }
}

/*******************************************************************************
* Start Gyro Calibration
*******************************************************************************/
void updateGyroCali(void)
{
  static bool gyro_cali = false;
  uint32_t pre_time;
  uint32_t t_time;

  char log_msg[50];

  if (nh.connected())
  {
    if (gyro_cali == false)
    {
      sprintf(log_msg, "Start Calibration of Gyro");
      nh.loginfo(log_msg);

      imu.SEN.gyro_cali_start();

      t_time   = millis();
      pre_time = millis();
      while(!imu.SEN.gyro_cali_get_done())
      {
        imu.update();

        if (millis()-pre_time > 5000)
        {
          break;
        }
        if (millis()-t_time > 100)
        {
          t_time = millis();
          setLedToggle(LED_ROS_CONNECT);
        }
      }
      gyro_cali = true;

      sprintf(log_msg, "Calibrattion End");
      nh.loginfo(log_msg);
    }
  }
  else
  {
    gyro_cali = false;
  }
}

/*******************************************************************************
* updateVoltageCheck
*******************************************************************************/
void updateVoltageCheck(void)
{
  static bool startup = false;
  static int vol_index = 0;
  static int prev_state = 0;
  static int alram_state = 0;
  static int check_index = 0;

  int i;
  float vol_sum;
  float vol_value;

  static uint32_t process_time[8] = {0,};
  static float    vol_value_tbl[10] = {0,};

  float voltage_ref       = 11.0 + 0.0;
  float voltage_ref_warn  = 11.0 + 0.0;


  if (startup == false)
  {
    startup = true;
    for (i=0; i<8; i++)
    {
      process_time[i] = millis();
    }
  }

  if (millis()-process_time[0] > 100)
  {
    process_time[0] = millis();

    vol_value_tbl[vol_index] = getPowerInVoltage();

    vol_index++;
    vol_index %= 10;

    vol_sum = 0;
    for(i=0; i<10; i++)
    {
        vol_sum += vol_value_tbl[i];
    }
    vol_value = vol_sum/10;
    battery_valtage_raw = vol_value;

    //Serial.println(vol_value);

    battery_voltage = vol_value;
  }


  if(millis()-process_time[1] > 1000)
  {
    process_time[1] = millis();

    //Serial.println(battery_state);

    switch(battery_state)
    {
      case BATTERY_POWER_OFF:
        if (setup_end == true)
        {
          alram_state = 0;
          if(battery_valtage_raw > 5.0)
          {
            check_index   = 0;
            prev_state    = battery_state;
            battery_state = BATTERY_POWER_STARTUP;
          }
          else
          {
            noTone(BDPIN_BUZZER);
          }
        }
        break;

      case BATTERY_POWER_STARTUP:
        if(battery_valtage_raw > voltage_ref)
        {
          check_index   = 0;
          prev_state    = battery_state;
          battery_state = BATTERY_POWER_NORMAL;
          setPowerOn();
        }

        if(check_index < 5)
        {
          check_index++;
        }
        else
        {
          if (battery_valtage_raw > 5.0)
          {
            prev_state    = battery_state;
            battery_state = BATTERY_POWER_CHECK;
          }
          else
          {
            prev_state    = battery_state;
            battery_state = BATTERY_POWER_OFF;
          }
        }
        break;

      case BATTERY_POWER_NORMAL:
        alram_state = 0;
        if(battery_valtage_raw < voltage_ref)
        {
          prev_state    = battery_state;
          battery_state = BATTERY_POWER_CHECK;
          check_index   = 0;
        }
        break;

      case BATTERY_POWER_CHECK:
        if(check_index < 5)
        {
          check_index++;
        }
        else
        {
          if(battery_valtage_raw < voltage_ref_warn)
          {
            setPowerOff();
            prev_state    = battery_state;
            battery_state = BATTERY_POWER_WARNNING;
          }
        }
        if(battery_valtage_raw >= voltage_ref)
        {
          setPowerOn();
          prev_state    = battery_state;
          battery_state = BATTERY_POWER_NORMAL;
        }
        break;

      case BATTERY_POWER_WARNNING:
        alram_state ^= 1;
        if(alram_state)
        {
          tone(BDPIN_BUZZER, 1000, 500);
        }

        if(battery_valtage_raw > voltage_ref)
        {
          setPowerOn();
          prev_state    = battery_state;
          battery_state = BATTERY_POWER_NORMAL;
        }
        else
        {
          setPowerOff();
        }

        if(battery_valtage_raw < 5.0)
        {
          setPowerOff();
          prev_state    = battery_state;
          battery_state = BATTERY_POWER_OFF;
        }
        break;

      default:
        break;
    }
  }
}

void setPowerOn()
{
  digitalWrite(BDPIN_DXL_PWR_EN, HIGH);
}

void setPowerOff()
{
  digitalWrite(BDPIN_DXL_PWR_EN, LOW);
}
