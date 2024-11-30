#include "main.h"
#include "pros/motors.h"
#include "skills_auton.hpp"
#include "lemlib/api.hpp" // IWYU pragma: keep

pros::MotorGroup leftMotors({
  Port::LEFT_BACK_WHEEL_PORT,
  Port::LEFT_MIDDLE_WHEEL_PORT,
  Port::LEFT_FRONT_WHEEL_PORT
}
);

pros::MotorGroup rightMotors({
  Port::RIGHT_BACK_WHEEL_PORT,
  Port::RIGHT_MIDDLE_WHEEL_PORT,
  Port::RIGHT_FRONT_WHEEL_PORT
});

lemlib::Drivetrain drivetrain{
  &leftMotors, //List of left motors
  &rightMotors, //List of right motors
  12.6, //Distance between left and right side of the drive
  lemlib::Omniwheel::NEW_325, //Wheel diameter (May be different than what the name suggests)
  360, //RPM of drive
  2 //Horizontal drift, 2 is recommend value for majority omni drive
};

lemlib::OdomSensors odomSensors{ //Odometry method, use nullptr if you dont have
  nullptr, //First vertical tracking wheel
  nullptr, //Second vertical tracking wheel
  nullptr, //First horizontal tracking wheel
  nullptr, //Second horizontal tracking wheel
  &imu //imu, declared in config.cpp
};

/* To tune PID do the following
Mock auton to turn/drive a certain amount, if it ossicilates about the target, increase kD
If it doesn't oscilate increase kP
Repeat until no amount of kD stops the ossicilation
Return to the previous kP and kD value
kI rarely needs tuning as it is steady state error, for v5rc the chance this is non-zero is very low

Ensure that tthe movements have long enough timeouts to observe any osciltion
For turning, osciliation will be the robot constantly turning back and forth while overshooting the intended point
For driving, osciliation will be the robot following a winding zig-zag-ish path instead of traveling in a smooth motion
*/

lemlib::ControllerSettings lateralController{ //PID for translational/lateral movements
  10, //kP - Proportional Gain
  0, //kI - Proportional Integral Gain
  5, //kD - Proportional Derivative Gain
  3, //AntiWindup
  1, //SmallError - Smaller Error range in inches
  100, //SmallErrorTimeout
  3, //LargeError - Larger Error range in inches
  500, //LargeError Timeout
  20, //Slew - Maximum acceleration
};
lemlib::ControllerSettings angularController{ //PID for rotational/angular/turning movements
  4, //kP - Proportional Gain
  0, //kI - Proportional Integral Gain
  22, //kD - Proportional Derivative Gain 
  0, //AntiWindup 0
  1, //SmallError - Smaller Error range in degrees 1
  100, //SmallErrorTimeout 100
  3, //LargeError - Larger Error range in degrees 3
  500, //LargeError Timeout 500
  0, //Slew - Maximum acceleration
};


lemlib::Chassis chassis(
  drivetrain,
  lateralController,
  angularController,
  odomSensors
);



/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 */
void initialize() 
{
	pros::delay(500); // Stop the user from doing anything while legacy ports configure.

  //Lemlib intialization
  pros::lcd::initialize();
  chassis.calibrate();
  
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() 
{
  chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 */

void autonomous() {
  //As of 11/27/2024 the autons don't seem to work if the following 3 lines are not included
  chassis.curvature(0, 0);
  chassis.turnToHeading(0,1000);
  chassis.waitUntilDone();
  
  pros::Task screen_task([&]() {
        chassis.setPose(0,0,0);
        while (true) {
            // print robot location to the brain screen
            master.print(1,0, "X: %.1lf Y: %.1lf",chassis.getPose().x, chassis.getPose().y); // y
            // delay to save resources
            pros::delay(300);
            master.print(2, 0, "R: %.1lf  ", chassis.getPose().theta);
            wait(300);
        }
    });

  red_follow();
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 */

void opcontrol() { 
    chassis.setBrakeMode(pros::E_MOTOR_BRAKE_COAST);
    
    enum teamColors : int {RED,BLUE};
    //Included for redundancy, should have been set in the auton
    setTeam(); //no input takes color sensor input for team, RED or BLUE are valid inputs
    
    pros::Task intakeT(intakes);
    // pros::Task intakeConSortT(intakesConSorter); // Uses the conveyor to color sort
    //pros::Task clampT(clamps); //Purely manually controlled clamp
    pros::Task autoclampT(autoClamps);
    pros::Task wall_scoreT(wall_score);
    pros::Task sorterT(sorts);
    pros::Task doinkerT(doinks);
    
    // pros::Task debugTurnT(debugTurn);
    // pros::Task debugDriveT(debugDrive);

    //Lemlib arcade drive
    while(true)
    {
      //Get joystick values
      int leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
      int rightX = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

      chassis.arcade(leftY, rightX);
      pros::delay(10);
    }
  	pros::delay(10); 
}