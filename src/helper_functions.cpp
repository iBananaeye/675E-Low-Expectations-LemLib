#include "lemlib/api.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"
// #include "string"
#include "queue"

void intaker(double v) {
    intake.move_velocity(v);
    // conveyor.move_velocity(v);
}

void intaker_wait(double v,int time) {
    intake.move_velocity(v);
    // conveyor.move_velocity(v);

    pros::delay(time);
    
    intake.move_velocity(0);
    // conveyor.move_velocity(0);
}

//---------

void wait(int time){
  pros::delay(time);
}

//---------
int offset = 0;
void wall_staker(int pos, int arm_vel)
{
    // arm.move_absolute(pos-offset, arm_vel);
}

void setWallStakePos(int pos)
{
    offset = pos;
}

// void doink()
// {
//     doinker.set_value(true);
// }

void undoink()
{
    doinker.set_value(false);
}

//---------
enum colorSensed : int {RED,BLUE, OTHER};
int team = RED; //default


//Color sensors returns a hue in degrees on a color wheel
//Red is at the top of the wheel blue is around 6 to 9 o clock

//Red is 330-30
//Blue is 150-250 depending on shade

const double blueThreshold = 70;
const double lowRedThreshold = 25; 
const double highRedThreshold = 330;

int getRingColor()
{
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

//sets the team to whatever ring is over the color sensor
void setTeam() 
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
        if(state == true && autoClamp.get_value() == 0)
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

void forceClamp()
{
    clamp.set_value(true);
    isClamped = true;
}


bool getClampState()
{
    return isClamped;
}

void stopWhenClamped(bool forceClampWhenDone, bool cancelAllQueuedMotions)
{
    while(chassis.isInMotion())
    {
        if(getClampState() == true)
        {
            cancelAllQueuedMotions ? chassis.cancelAllMotions() : chassis.cancelMotion();
            break;
        }
        pros::delay(20);
    }
    if(forceClampWhenDone)
    {
        forceClamp();
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

    if(fabs(x-curX) < xThreshold && fabs(y-curY) < yThreshold)
    {
        return true;
    }
    return false;
}

double convertAngle(double target, double angle)
{
    if(angle >= target + 180)
    {
        return angle - 360;
        
    }
    else if(target >= angle + 180)
    {
        return angle + 360;
    }
    return angle;
}

double degreesToRadians(double angleInDegrees)
{
    return angleInDegrees * M_PI / 180.0; //angle in radians
}

void moveRelative(double dist, int timeout, lemlib::MoveToPointParams params, bool async)
{
    if(dist < 0)
    {
        params.forwards = false;
    }
    lemlib::Pose pose = chassis.getPose();
    double heading = degreesToRadians(pose.theta);
    double deltaX = dist * sin(heading) * params.forwards ? 1 : -1;
    double deltaY = dist * cos(heading) * params.forwards ? 1 : -1;

    chassis.moveToPoint(pose.x + deltaX, pose.y + deltaY, timeout, params, async);
}

void turnRelative(double deltaAngle, int timeout, lemlib::TurnToHeadingParams params, bool async)
{
    chassis.turnToHeading(chassis.getPose().theta + deltaAngle, timeout, params, async);
}








std::queue<printMessage> printQueue;
pros::Mutex screenMutex;


void printToController(printMessage printedMessage, int waitTimeInMs, bool finishWaiting)
{
    if(waitTimeInMs != 0)
    {
        int waitedTime = 0;
        while(waitedTime < waitTimeInMs)
        {
            if(screenMutex.try_lock())
            {
                printQueue.push(printedMessage);
                screenMutex.unlock();
                if(finishWaiting) wait(waitTimeInMs-waitedTime);
                return;
            }
            wait((int)(waitTimeInMs/10.0));
            waitedTime += waitTimeInMs/10.0;
        }
    }
    else
    {
        if(screenMutex.try_lock())
        {
            printQueue.push(printedMessage);
            screenMutex.unlock();
        }
    }
}

void screenHandler()
{
    while(true)
    {
        if(!printQueue.empty())
        {
            while(!screenMutex.try_lock()) 
            {
                wait(20);
            }
            printMessage message = printQueue.front();
            printQueue.pop();
            screenMutex.unlock();
            master.print(message.rowNum, message.colNum,"%s", (message.text + (message.text.length() < 16 ? std::string(16 - message.text.length(), ' ') : "")).c_str());
            wait(250);
        }
        else
        {
            wait(30);
        }
    }
}

double convertDirectGearRatio(double input)
{
    return input * 24.0/84.0 * 72.0/60.0;
}