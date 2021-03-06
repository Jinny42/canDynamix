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

/* Authors: Taehoon Lim (Darby) */

#include "candynamix_gazebo/candynamix_gazebo.h"

CanDynamix_Gazebo::CanDynamix_Gazebo()
  : nh_priv_("~")
{
  //Init gazebo ros candynamix node
  ROS_INFO("CanDynamix Simulation Node Init");
  ROS_ASSERT(init());
}

CanDynamix_Gazebo::~CanDynamix_Gazebo()
{
  updatecommandVelocity(0.0, 0.0);
  ros::shutdown();
}

/*******************************************************************************
* Init function
*******************************************************************************/
bool CanDynamix_Gazebo::init()
{
  // initialize ROS parameter
  nh_.param("is_debug", is_debug_, is_debug_);
  
/*
  std::string robot_model = nh_.param<std::string>("tb3_model", "");

  if (!robot_model.compare("burger"))
  {
    turning_radius_ = 0.08;
    rotate_angle_ = 50.0 * DEG2RAD;
    front_distance_limit_ = 0.7;
    side_distance_limit_  = 0.4;
  }
  else if (!robot_model.compare("waffle"))
  {
    turning_radius_ = 0.1435;
    rotate_angle_ = 40.0 * DEG2RAD;
    front_distance_limit_ = 0.7;
    side_distance_limit_  = 0.6;
  }
*/

    turning_radius_ = 0.08;
    rotate_angle_ = 50.0 * DEG2RAD;
    front_distance_limit_ = 0.7;
    side_distance_limit_  = 0.4;

 // ROS_INFO("robot_model : %s", robot_model.c_str());
  ROS_INFO("turning_radius_ : %lf", turning_radius_);
  ROS_INFO("front_distance_limit_ = %lf", front_distance_limit_);
  ROS_INFO("side_distance_limit_ = %lf", side_distance_limit_);

  // initialize variables
  right_joint_encoder_ = 0.0;
  priv_right_joint_encoder_ = 0.0;
  // initialize publishers
  cmd_vel_pub_   = nh_.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

  // initialize subscribers
  //laser_scan_sub_  = nh_.subscribe("/scan", 10, &CanDynamix_Gazebo::laserScanMsgCallBack, this);
  joint_state_sub_ = nh_.subscribe("/joint_states", 10, &CanDynamix_Gazebo::jointStateMsgCallBack, this);

  return true;
}

void CanDynamix_Gazebo::jointStateMsgCallBack(const sensor_msgs::JointState::ConstPtr &msg)
{
  right_joint_encoder_ = msg->position.at(0);
}

/*
void CanDynamix_Gazebo::laserScanMsgCallBack(const sensor_msgs::LaserScan::ConstPtr &msg)
{
  uint16_t scan_angle[3] = {0, 30, 330};

  for (int num = 0; num < 3; num++)
  {
    if (std::isinf(msg->ranges.at(scan_angle[num])))
    {
      direction_vector_[num] = msg->range_max;
    }
    else
    {
      direction_vector_[num] = msg->ranges.at(scan_angle[num]);
    }
  }
}
*/

void CanDynamix_Gazebo::updatecommandVelocity(double linear, double angular)
{
  geometry_msgs::Twist cmd_vel;

  cmd_vel.linear.x  = linear;
  cmd_vel.angular.z = angular;

  cmd_vel_pub_.publish(cmd_vel);
}

/*******************************************************************************
* Control Loop function
*******************************************************************************/
bool CanDynamix_Gazebo::controlLoop()
{
  static uint8_t candynamix_state_num = 0;
  double wheel_radius = 0.033;
  double candynamix_rotation = 0.0;

  candynamix_rotation = (rotate_angle_ * turning_radius_ / wheel_radius);

  switch(candynamix_state_num)
  {
    case GET_TB3_DIRECTION:
      if (direction_vector_[CENTER] > front_distance_limit_)
      {
        candynamix_state_num = TB3_DRIVE_FORWARD;
      }

      if (direction_vector_[CENTER] < front_distance_limit_ || direction_vector_[LEFT] < side_distance_limit_)
      {
        priv_right_joint_encoder_ = right_joint_encoder_ - candynamix_rotation;
        candynamix_state_num = TB3_RIGHT_TURN;
      }
      else if (direction_vector_[RIGHT] < side_distance_limit_)
      {
        priv_right_joint_encoder_ = right_joint_encoder_ + candynamix_rotation;
        candynamix_state_num = TB3_LEFT_TURN;
      }
      break;

    case TB3_DRIVE_FORWARD:
      updatecommandVelocity(LINEAR_VELOCITY, 0.0);
      candynamix_state_num = GET_TB3_DIRECTION;
      break;

    case TB3_RIGHT_TURN:
      if (priv_right_joint_encoder_ == 0.0)
      {
        candynamix_state_num = GET_TB3_DIRECTION;
      }
      else
      {
        if (fabs(priv_right_joint_encoder_ - right_joint_encoder_) < 0.1)
        {
          candynamix_state_num = GET_TB3_DIRECTION;
        }
        else
        {
          updatecommandVelocity(0.0, -1 * ANGULAR_VELOCITY);
        }
      }
      break;

    case TB3_LEFT_TURN:
      if (priv_right_joint_encoder_ == 0.0)
      {
        candynamix_state_num = GET_TB3_DIRECTION;
      }
      else
      {
        if (fabs(priv_right_joint_encoder_ - right_joint_encoder_) < 0.1)
        {
          candynamix_state_num = GET_TB3_DIRECTION;
        }
        else
        {
          updatecommandVelocity(0.0, ANGULAR_VELOCITY);
        }
      }
      break;

    default:
      candynamix_state_num = GET_TB3_DIRECTION;
      break;
  }

  return true;
}

/*******************************************************************************
* Main function
*******************************************************************************/
int main(int argc, char* argv[])
{
  ros::init(argc, argv, "candynamix_gazebo");
  CanDynamix_Gazebo CanDynamix_Gazebo;

  ros::Rate loop_rate(125);

  while (ros::ok())
  {
    CanDynamix_Gazebo.controlLoop();
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
