#include "lemlib/api.hpp" // IWYU pragma: keep
#include "main.h"
#include "pros/motors.h"
#include "skills_auton.hpp"

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
  lemlib::Omniwheel::NEW_275, //Wheel diameter (May be different than what the name suggests)
  450, //RPM of drive
  2 //Horizontal drift, 2 is recommend value for majority omni drive
};
//Fix wheel dia and dist to track
lemlib::TrackingWheel perpendicularWheel(
  &perpendicularRotation, //Tracking wheel rotation sensor
  lemlib::Omniwheel::OLD_275, //Omniwheel used
  1, //Distance to tracking center
  1 //Gear ratio
);

// lemlib::TrackingWheel parallelWheel(
//   &parallelRotation, //Tracking wheel rotation sensor
//   lemlib::Omniwheel::NEW_2, //Omniwheel used
//   1, //Distance to tracking center
//   1 //Gear ratio
// );

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
  
  // pros::Task screen_task([&]() {
  //       chassis.setPose(0,0,0);
  //       while (true) {
  //           // print robot location to the brain screen
  //           master.print(0,0, "X:%.1lf Y:%.1lf",chassis.getPose().x, chassis.getPose().y);
  //           pros::delay(300);
  //           master.print(1, 0, "R: %.1lf  ", chassis.getPose().theta);
  //           wait(300);
  //       }
  //   });
  pros::Task screenHandlerAT(screenHandler);
 pros::Task deviceMonitorAT(deviceMonitor);

  // skills();
  // red_goal_rush();
  // bottom_red_new();
  // bottom_red_goal_rush();
  // red_follow();
  // skills3();
  // top_blue_simple();
  // chassis.moveToPoint(0,12,1000, {.maxSpeed = 40});
  // chassis.waitUntilDone();
  // wait(2000);
  // chassis.moveToPoint(-24,36, 3000, {.maxSpeed = 40});
  // chassis.waitUntilDone();
  // screen_task.remove();
  // red_mogoRush();
   red_hard();
  // skillsreal();
  // blue_simple_bottom();
  // blue_hard();
  screenHandlerAT.remove();
  deviceMonitorAT.remove();
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
    setTeam(RED); // Included for redundancy, should have been set in the auton, no input takes color sensor input for team, RED or BLUE are valid inputs
    
    pros::Task intakeT(intakes);
    // pros::Task intakeConSortT(intakesConveyorSorter); // Uses the conveyor to color sort
    // pros::Task clampT(clamps); //Purely manually controlled clamp
    pros::Task autoclampT(autoClamps);
    pros::Task directWallScoreT(directWallScore);
    pros::Task sorterT(sorts); //Uses a piston to sort
    pros::Task doinkerT(doinks);
    
    // pros::Task liftsintake(lifts);
    pros::Task screenHandlerT(screenHandler);
    pros::Task deviceMonitorT(deviceMonitor);

    //Lemlib arcade drive

    // pros::Task screen_task([&]() {
    //   wait(5000);
    //     while (true) {
    //         // print robot location to the brain screen
    //         master.print(0,0, "%.3lf  ",intake.get_target_velocity());
    //         wait(150);
    //         master.print(1,0,"%.3lf",intake.get_torque());
    //         wait(150);
    //     }
    // });

    // pros::Task screen_task([&]() {
    //     chassis.setPose(0,0,0);
    //     while (true) {
    //         // print robot location to the brain screen
    //         master.print(0,0, "X:%.1lf Y:%.1lf",chassis.getPose().x, chassis.getPose().y);
    //         pros::delay(300);
    //         master.print(1, 0, "R: %.1lf  ", chassis.getPose().theta);
    //         wait(300);
    //     }
    // });

    //sdcard record
    // FILE* file = fopen("/usd/data.bin", "wb");
    // int stickData[2000];
    // int startTime = pros::millis();
    // int i = 0;
    // while((int)(pros::millis-startTime) < 10000)
    // {
    //   int leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
    //   int rightX = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
    //   stickData[i++] = leftY;
    //   stickData[i++] = rightX;

    //   chassis.arcade(leftY, rightX);

    //   pros::delay(20);
    // }
    // chassis.arcade(0, 0);
    // int size = sizeof(stickData) / sizeof(stickData[0]);
    // if(file != nullptr)
    // {
    //   fwrite(stickData, sizeof(int), size, file);
    //   fclose(file);
    // }
    // wait(5000);
    // int readStickData[2000];
    // FILE* file2 = fopen("/usd/data.bin", "rb");
    // if (file2 != NULL) {
    //     fread(readStickData, sizeof(int), 2000, file2); 
    //     fclose(file2);
    // }
    // int x = 0;
    // while(x < 1999)
    // {
    //   int leftY = stickData[x++];
    //   int rightX = stickData[x++];

    //   chassis.arcade(leftY, rightX);

    //   pros::delay(20);
    // }

    //sdcardless record
    // int stickData[2000];
    // int i =0;
    // while(i<1999)
    // {
    //   int leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
    //   int rightX = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
    //   stickData[i++] = leftY;
    //   stickData[i++] = rightX;

    //   chassis.arcade(leftY, rightX);

    //   pros::delay(20);
    // }
    // chassis.arcade(0, 0);
    // while(!master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
    // {
    //   wait(50);
    // }
    // wait(1000);
    // i = 0;
    // while(i < 1999)
    // {
    //   int leftY = stickData[i++];
    //   int rightX = stickData[i++];

    //   chassis.arcade(leftY, rightX);
    //   pros::delay(20);
    // }
    // chassis.arcade(0, 0);

    while(true)
    {
      int leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
      int rightX = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

      chassis.arcade(leftY, rightX);

      pros::delay(20);
    }
}




//To-do
//toggle color sort with ring over sensor