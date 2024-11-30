#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"

void intaker(double v) {
    intake.move_velocity(-v);
    conveyor.move_velocity(v);
}

void intaker_wait(double v,int time) {
    intake.move_velocity(-v);
    conveyor.move_velocity(v);

    pros::delay(time);
    
    intake.move_velocity(0);
    conveyor.move_velocity(0);
}

//---------

void wait(int time){
  pros::delay(time);
}

//---------

void wall_staker(int pos, int arm_vel)
{
    arm.move_absolute(pos, arm_vel);
}


void doink()
{
    doinker.set_value(true);
}

void undoink()
{
    doinker.set_value(false);
}

//---------
enum{RED,BLUE, OTHER};
int team = BLUE; //default


int getRingColor()
{
    //Color sensors returns a hue in degrees on a color wheel
    //Red is at the top of the wheel blue is around 6 to 9 o clock

    //Red is 330-30
    //Blue is 150-250 depending on shade

    const double blueThreshold = 70;
    const double lowRedThreshold = 25; 
    const double highRedThreshold = 330;

    double hue = light.get_hue();

    if(hue != 0 && (hue < lowRedThreshold || hue > highRedThreshold)) 
    {
        return RED;
    }
    else if(hue != 0 && hue > blueThreshold) 
    {
        return BLUE;
    }
    return OTHER;
}


void setTeam() //sets the team to whatever ring is over the color sensor
{
    int col = getRingColor(); //defaults to red
    if(col != OTHER)
    {
        team = col;
    }
}


void setTeam(int allianceColor)
{
    team = allianceColor;
}

int getTeam()
{
    return team;
}

//This task is included here to ensure isClamped is consistent with other clamping methods
bool isClamped = false;
bool autoClampIsActive = true;
const int minTimeBetweenClamps = 1000; // in milliseconds
void autonAutoClamp()
{  
    clamp.set_value(false);
    bool state = false;
    wait(300);
    while(true)
    {
        if(autoClampIsActive && state == false && autoClamp.get_value() == 1)
        {
            clamp.set_value(true);
            wait(250);
            state = true;
            isClamped = true;
        }
        if(autoClamp.get_value() == 0)
        {
            state = false;
            wait(minTimeBetweenClamps);
        }
        wait(20);
    }
}

void toggleAutonAutoClamp()
{
    autoClampIsActive = !autoClampIsActive;
}

void unclamp()
{
    clamp.set_value(false);
    isClamped = false;
}

//clamp was already taken
void forceClamp()
{
    clamp.set_value(true);
    isClamped = true;
}


bool getClampState()
{
    return isClamped;
}

void stopWhenClamped()
{
    while(chassis.isInMotion())
    {
        if(getClampState() == true)
        {
            chassis.cancelAllMotions();
            break;
        }
        pros::delay(20);
    }
}

// Unclamps, repositions, and reclamps to get a better angle of the clamp
// This turning approach simply didn't work
// void reClamp()
// {
//     wait(1000);
//     double theta = chassis.getPose().theta;
//     double curX = chassis.getPose().x;
//     double curY = chassis.getPose().y;
//     chassis.turnToHeading(theta + 25, 1000);
//     chassis.waitUntilDone();
//     unclamp();
//     wait(300);
//     chassis.turnToHeading(theta, 1000, {.earlyExitRange = 3});
//     chassis.swingToHeading(theta+25, lemlib::DriveSide::LEFT, 1000);
//     chassis.moveToPoint(curX - 3, curY - 3, 2000, {.forwards = false});
//     stopWhenClamped();
//     if(getClampState() == false) forceClamp();
// }

bool isNearPos(double x, double y, double xThreshold, double yThreshold)
{
    double curX = chassis.getPose().x;
    double curY = chassis.getPose().y;

    if(abs(x-curX) < xThreshold && abs(y-curY) < yThreshold)
    {
        return true;
    }
    return false;
}