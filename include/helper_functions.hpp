#pragma once
#include "api.h"

void intaker(double v);
void intaker_wait(double v, int time);

void wait(int time);

enum Wall : int
{
    DOWN = -15, //Not 0 to make sure the motors don't fry themselves going through metal
    LOAD = -115,
    SCORE = -555, 
    INSERT = -630
};

/*
* Moves the wall stake mechanism to an absolute position in encoder ticks at a specific velocity
* @param Position
*   The position the wall stake mech will move to. Use the Wall enum (DOWN, LOAD, SCORE, INSERT), for the same positions as opcontrol
* @param ArmVelocity
*   The maximum velocity of the wall stake mechanism in rpm (0-600)
*/
void wall_staker(int pos, int arm_vel);

void doink();
void undoink();

/*
*  Determines whether the color sensor sees a red ring, blue ring, or no ring
* \returns a integer representing the color the Optical Sensors reads (0-Red, 1-Blue, 2-Other)
*/
int getRingColor();

/*
* Automatically ets the robot's team color while the color sorter is active with the Optical sensor
*/
void setTeam();

/*
* Manually sets the robot's team color while the color sorter is active
* @param AllianceColor
*   The Color of the Robots Alliance (0 for Red, 1 for Blue)
*/
void setTeam(int t);

/*
* \return an integer representing the robot's alliance color (0 for Red, 1 for Blue)
*/
int getTeam();


void autonAutoClamp();
void toggleAutonAutoClamp();
void unclamp();
void forceClamp();

/*
* \returns a boolean representing whether the robot is currently clamping (true) or not (false)
*/
bool getClampState();

/*
* Halts code progress until the robot has clamped or motion has ended
*
* Cancels all queued motions if the robot clamped during the motion
*/
void stopWhenClamped();

void reClamp();

/*
* Uses LemLib's odometry to determine whether the robot is close to a specific point 
* @param x
*   The target x value that the robot's x is being compared to
* @param y
*   The target y value that the robot's y is being compared to
* @param xThreshold
*   The absolute range about the target x in which the robot is considered near. Set this to a very high value to ignore this axis
* @param yThreshold
*   The absolute range about the target y in which the robot is considered near. Set this to a very high value to ignore this axis
* \returns a boolean representing whether the robot is within (true) an acceptable distance of a specific point or not (false)
*/
bool isNearPos(double x, double y, double xThreshold, double yThreshold);