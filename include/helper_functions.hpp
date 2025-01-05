#pragma once
#include "api.h"

void intaker(double v);
void intaker_wait(double v, int time);

void wait(int time);

/**
* @brief Moves the wall stake mechanism to an absolute position in encoder ticks at a specific velocity
* @param Position
*   The position the wall stake mech will move to. Use the Wall enum (DOWN, LOAD, SCORE, INSERT), for the same positions as opcontrol
* @param ArmVelocity
*   The maximum velocity of the wall stake mechanism in rpm (0-600)
*/
void wall_staker(int pos, int arm_vel);

/**
* @brief Sets the starting position of the wall stake mech
* @param Position
* The offset that will be applied to the wall stake. Use the Wall enum (DOWN, LOAD, SCORE, INSERT), for the same positions as opcontrol
*/
void setWallStakePos(int pos);

/**
 * @brief Extend the doinker piston
 */
void doink();

/**
 * @brief Retract the doinker piston
 */
void undoink();

/**
*  @brief Determines whether the color sensor sees a red ring, blue ring, or no ring
* \returns a integer representing the color the Optical Sensors reads (0-Red, 1-Blue, 2-Other)
*/
int getRingColor();

/**
* @brief Automatically sets the robot's team color while the color sorter is active with the Optical sensor
*/
void setTeam();

/**
* @brief Manually sets the robot's team color while the color sorter is active
* @param AllianceColor
*   The Color of the Robots Alliance (0 for Red, 1 for Blue)
*/
void setTeam(int t);

/**
* \return an integer representing the robot's alliance color (0 for Red, 1 for Blue)
*/
int getTeam();


void autonAutoClamp();
void toggleAutonAutoClamp();
void unclamp();
void forceClamp();

/**
* \returns a boolean representing whether the robot is currently clamping (true) or not (false)
*/
bool getClampState();

/**
* @brief Halts code progress until the robot has clamped or motion has ended
* @param forceClampWhenDone If true, the robot will clamp when the motion stops
* @param cancelAllQueuedMotions If true, chassis.cancelAllMotions() will be ran instead of chassis.cancelMotion()
*/
void stopWhenClamped(bool forceClampWhenDone = false, bool cancelAllQueuedMotions = false);

void reClamp();

/** 
* @brief Uses LemLib's odometry to determine whether the robot is close to a specific point 
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

/**
 * @brief Converts the inputted angle to an angle that allows the Wall Mechanism's PID to effectively describe the error
 * @param target
 *  The target that the arm is attempting to reach
 * @param angle
 *  The angle that is going to be converted to an corresponding angle within 180 degrees of the target
 * \returns A double with an angle (-180,360) that corresponds to the input but within 180 degrees of target (Angle - 360)
 */
double convertAngle(double target, double angle);

/**
 * @brief Converts an angle from degrees to radians
 * @param angleInDegrees 
 *  An angle in degrees that will be converted to radians
 * \returns A double of the inputted angle in radians
 */
double degreesToRadians(double angleInDegrees);

void moveRelative(double dist, int timeout, lemlib::MoveToPointParams params = {}, bool async = true);

void turnRelative(double deltaAngle, int timeout, lemlib::TurnToHeadingParams params = {}, bool async = true);

struct printMessage
{
    int rowNum;
    int colNum;
    std::string text;
};

void printToController(printMessage printedMessage, int waitTimeInMs = 0, bool finishWaiting = false);

void screenHandler();

