#include <SoftwareSerial.h>
#include <PololuQik.h>

#include "turtlebot3_core_config_test.h"

/*******************************************************************************
* ROS NodeHandle
*******************************************************************************/
ros::NodeHandle nh;

/*******************************************************************************
* Subscriber
*******************************************************************************/
ros::Subscriber<geometry_msgs::Twist> cmd_vel_sub("cmd_vel", commandVelocityCallback);

/*******************************************************************************/

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

/*******************************************************************************
* SoftwareTimer of Turtlebot3
*******************************************************************************/
static uint32_t tTime[4];

/*******************************************************************************
* Declaration for motor
*******************************************************************************/
double goal_linear_velocity  = 0.0;
double goal_angular_velocity = 0.0;



void setup() {
  //Serial.begin(9600);
  qik.init();
  // Initialize ROS node handle, advertise and subscribe the topics
  nh.initNode();
  //nh.getHardware()->setBaud(115200);
  nh.subscribe(cmd_vel_sub);

  nh.loginfo("Connected to OpenCR board!");

}

void loop() {
  if ((millis()-tTime[0]) >= (1000 / CONTROL_MOTOR_SPEED_PERIOD))
  {
    controlMotorSpeed();
    tTime[0] = millis();
  }

   // Call all the callbacks waiting to be called at that point in time
  nh.spinOnce();
  
}
  /*******************************************************************************
  * Callback function for cmd_vel msg
  *******************************************************************************/
  void commandVelocityCallback(const geometry_msgs::Twist& cmd_vel_msg)
  {
    goal_linear_velocity  = cmd_vel_msg.linear.x;
    goal_angular_velocity = cmd_vel_msg.angular.z;
  }

  /*******************************************************************************
  * Control motor speed
  *******************************************************************************/
  void controlMotorSpeed(void)
  {
    
    qik.setM0Speed((goal_linear_velocity*50) + ((50 * goal_angular_velocity * WHEEL_SEPARATION) / 2));
    qik.setM1Speed((goal_linear_velocity*-50) - ((-50 * goal_angular_velocity * WHEEL_SEPARATION) / 2));

    
    /*
    bool dxl_comm_result = false;
    double wheel_speed_cmd[2];
    double lin_vel1;
    double lin_vel2;
  
    wheel_speed_cmd[LEFT]  = goal_linear_velocity - (goal_angular_velocity * WHEEL_SEPARATION / 2);
    wheel_speed_cmd[RIGHT] = goal_linear_velocity + (goal_angular_velocity * WHEEL_SEPARATION / 2);
  
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
  
    dxl_comm_result = motor_driver.speedControl((int64_t)lin_vel1, (int64_t)lin_vel2);
    if (dxl_comm_result == false)
      return;
    */
  }


