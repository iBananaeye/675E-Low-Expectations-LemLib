#include "skills_auton.hpp"
#include "helper_functions.hpp"
#include "main.h"

const int DRIVE_SPEED = 110; // This is 110/127 (around 87% of max speed). When this is 87%, the robot is correcting by making one side
                            // faster and one side slower, giving better heading correction.
const int TURN_SPEED = 65;
const int SWING_SPEED = 90;
const int INTAKE_SPEED = 550;
enum : int{RED,BLUE};

/*
HOW TO TUNE PID

Step 1 - kP
You'll start with kP. Set it to some number, and run your example autonomous. The robot will either undershoot the target (kP too low), or the robot is oscillate around the target (kP too high).

To tune a PD controller, you want the kP to oscillate a little bit, usually a bounce or two.

Step 2 - kD
After finding a kP that oscillates a little, we can tune kD. Increase kD until the oscillation goes away. This movement should look more "snappy" then just a P loop.

Step 3 - Repeat
Repeat Steps 1 and Steps 2 until kD cannot remove oscillation from the movement. Then go back to the last values that worked.

Step 4 - kI
Sometimes you need a little extra power to get your robot all the way there. Integral can be a dangerous variable because it grows exponentially. The fourth parameter is what the error needs to be for I to start. For turns, we found around 15 degrees is good.

Increase kI until any minor disturbances are accounted for. You might need to adjust kD while tuning kI.
*/


//------------------ SKILLS ---------------------
// Path 2
void skills(){
  bool clamp_state = false;
}

//------------------ LEMLIB MATCH AUTONS ---------------------

//Grabs close mogol, scores pre-load and 1 ring, tries to clear corner
//Unfinished, still needs to touch ladder
void bottom_red_new()
{
  pros::Task autAutoClampT(autonAutoClamp);
  pros::Task autSorterT(sorts);
  setTeam(RED);
  chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);

  chassis.setPose(30.5, -9.5, 0);

  //Grabs close mogol
  chassis.moveToPose(51,-46, -38, 3000, {.forwards = false, .minSpeed = 40});
  stopWhenClamped(); //ends motion early if clamped
  if(getClampState() == false) 
  {
    forceClamp();
  }

  //Scores preload and nearby ring
  intaker(INTAKE_SPEED);
  wait(300);
  chassis.turnToHeading(-90, 1000, {.earlyExitRange = 5});
  chassis.moveToPoint(21,-44, 4000, {.earlyExitRange = 3});

  //Moves to corner and clears with doinker
  chassis.moveToPose(7.5,-15, -5, 3000);
  chassis.waitUntilDone();
  doink();
  wait(300);
  chassis.turnToHeading(45, 1000);
  chassis.turnToHeading(-10, 1000);
  chassis.waitUntilDone();
  undoink();

  //Moves to ladder
  chassis.moveToPose(36, -18, -180, 4000, {.forwards = false});
  chassis.moveToPoint(72,-48, 4000);

  autAutoClampT.remove();
  autSorterT.remove();
}




//Rushes mid mogol, scores 1 ring on it, grabs second mogol and scores 1 ring
//Was never finished in favor of different routing, still needs to clear corner and touch ladder
void bottom_red_goal_rush()
{
  pros::Task autAutoClampT(autonAutoClamp);
  pros::Task autSorterT(sorts);
  setTeam(RED);
  chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);

  chassis.setPose(7.5,-13, 0);

  //Rushes middle mogol
  chassis.moveToPoint(10, -36, 4000, {.forwards = false, .minSpeed = 80, .earlyExitRange = 10});
  chassis.moveToPose(24, -72,-25, 4000, {.forwards = false, .minSpeed = 50});
  stopWhenClamped();
  if(getClampState() == false)
  {
    forceClamp();
  }
  chassis.turnToHeading(45, 1000, {.earlyExitRange = 5});

  //Scores preload, begins moving toward second mogol, and drops first mogol
  intaker(INTAKE_SPEED);
  wait(500);
  intaker(0);
  chassis.moveToPoint(36,-60,2000);
  chassis.waitUntilDone();
  unclamp();
  wait(300);
  chassis.turnToHeading(-150,1000, {.earlyExitRange = 5});

  //Grabs the second mogol
  chassis.moveToPoint(53,-46,3000, {.forwards = false});
  stopWhenClamped();
  if(getClampState() == false)
  {
    forceClamp();
  }

  //Scores nearby ring onto the second mogol
  intaker(INTAKE_SPEED);
  chassis.turnToHeading(-90, 1000, {.earlyExitRange = 5});
  chassis.moveToPoint(21, -48, 4000);

  autAutoClampT.remove();
  autSorterT.remove();
}




//Rushes mid mogol, scores preload on it,  clears corner, puts mogol in corner, grabs second mogol, scores 1 ring, touches ladder
//Unfinished, second mogol grab is inconsistent, sometimes clamps a ring, doesn't do anything after that
//After clearing corner the odometry has drifted substantially from reality, but manually correcting it seems to break turning
void red_goal_rush()
{
  pros::Task autAutoClampT(autonAutoClamp);
  pros::Task autSorterT(sorts);
  chassis.setBrakeMode(pros::E_MOTOR_BRAKE_HOLD);
  setTeam(RED);

  chassis.setPose(7.5,-13, 0);

  // Rushes middle mogol and scores preload onto it
  chassis.moveToPoint(10, -36, 4000, {.forwards = false, .minSpeed = 80, .earlyExitRange = 10});
  chassis.moveToPose(23, -66,-25, 4000, {.forwards = false, .minSpeed = 50});
  stopWhenClamped();
  if(getClampState() == false)
  {
    forceClamp();
  }
  intaker(INTAKE_SPEED);

  //Moves to and clears positive corner with doinker
  chassis.moveToPoint(14, -36, 4000, { .minSpeed = 80, .earlyExitRange = 10});
  chassis.moveToPose(7.5, -13, -14.5, 1500);
  chassis.waitUntilDone();
  doink();
  wait(500);
  chassis.turnToHeading(160, 2000, {.direction = lemlib::AngularDirection::CW_CLOCKWISE});

  //Backs the mogol into the positive corner and unclamps
  chassis.moveToPoint(2, 2, 1000, {.forwards=false});
  chassis.waitUntilDone();
  unclamp();
  wait(300);

  //Exits corner and positions to clamp the second mogol without hitting other rings
  chassis.moveToPoint(64, -21, 3000, {.minSpeed = 80, .earlyExitRange = 5});
  chassis.turnToHeading(82, 1000);
  chassis.waitUntilDone();
  wait(1000);

  //Robot consistently collects error, this is an attempt to manually compensate for the error
  chassis.setPose(68,-33, chassis.getPose().theta);
  chassis.calibrate();
  imu.set_heading(chassis.getPose().theta);

  //Clamps the second mogol
  chassis.moveToPoint(45, -40, 3000, {.forwards = false});
  stopWhenClamped();
  if(getClampState() == false)
  {
    forceClamp();
  }

  //Score a ring onto the mogol
  intaker(INTAKE_SPEED);
  // For whatever reason this turn just does not work, the robot just bugs out refuses to turn
  chassis.turnToHeading(0, 2000); //{.minSpeed = 80, .earlyExitRange = 7}
  // chassis.moveToPoint(20, -48, 3000);
  // chassis.waitUntilDone();
  // chassis.swingToHeading(135, lemlib::DriveSide::RIGHT, 2000, {.direction = lemlib::AngularDirection::CCW_COUNTERCLOCKWISE});
  // chassis.moveToPoint(72, -48, 4000);
  // chassis.waitUntilDone();

}




//Uses a path built from jerrypath.io
ASSET(simple_txt);
ASSET(midgoal_rushRLL_txt);

//Rushes mid goal and moves away
void red_follow()
{
  pros::Task autAutoClampT(autonAutoClamp);
  pros::Task autSorterT(sorts);
  setTeam(RED);
  chassis.turnToHeading(0,1000);
  chassis.waitUntilDone();


  chassis.setPose(-53, -62, 270);
  chassis.follow(simple_txt,15, 20000, false);
  
  //Waits until the robot reaches a certain point before unclamping, intake is active while clamped
  while(isNearPos(-9.5, -50.682, 3.5, 3.5) == false)
  {
    if(getClampState() == true)
    {
      intaker(INTAKE_SPEED);
    }
    else
    {
      intaker(0);
    }
    wait(20);
  }
  unclamp();

  while(isNearPos(-63.988, -64.769, 1.5, 3.5) == false)
  {
    wait(20);
  }
  unclamp();

  autAutoClampT.remove();
  autSorterT.remove();
}