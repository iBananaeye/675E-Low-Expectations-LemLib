#include "lemlib/api.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"
#include "string"
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

void doink()
{
    doinker.set_value(true);
}

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

const double blueThreshold = 125;
const double lowRedThreshold = 30; 
const double highRedThreshold = 325;

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
    wait(150);
    while(true)
    {
        if(autoClampIsActive && state == false && (leftAutoClamp.get_value() == 1) && (rightAutoClamp.get_value() == 1))
        {
            clamp.set_value(true);
            wait(250);
            state = true;
            isClamped = true;
        }
        if(state == true && leftAutoClamp.get_value() == 0 && rightAutoClamp.get_value() == 0)
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
    if((angle >= target + 180) && rotation.get_position() / 100.0 <500)
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
    double deltaX = dist * sin(heading) * (params.forwards ? 1 : -1);
    double deltaY = dist * cos(heading) * (params.forwards ? 1 : -1);
    // double deltaX = dist * sin(heading);
    // double deltaY = dist * cos(heading);

    chassis.moveToPoint(pose.x + deltaX, pose.y + deltaY, timeout, params, async);
}

void turnRelative(double deltaAngle, int timeout, lemlib::TurnToHeadingParams params, bool async)
{
    chassis.turnToHeading(chassis.getPose().theta + deltaAngle, timeout, params, async);
}








std::queue<printMessage> printQueue;
pros::Mutex screenMutex;


void printToController(printMessage printedMessage, int waitTimeInMs, bool finishWaiting, bool rumble)
{
    if(waitTimeInMs != 0)
    {
        int waitedTime = 0;
        while(waitedTime < waitTimeInMs)
        {
            if(screenMutex.try_lock())
            {
                printQueue.push(printedMessage);
                if(rumble) printQueue.push(printMessage(-1,-1, std::string("rumble")));
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
    wait(1000);
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
            if(message.rowNum == -1 || message.colNum == -1) master.rumble("-");
            else master.print(message.rowNum, message.colNum,"%s", (message.text + (message.text.length() < 16 ? std::string(16 - message.text.length(), ' ') : "")).c_str());
            wait(125);
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

// void deviceMonitor()
// {
//     pros::Device device(1);
//     device.get_all_devices();
//     for(int portNumber : usedPorts)
//     {
//         std::vector<pros::Device> vec[3]={};
//         if(!device.is_installed())
//         {
//             device.get_plugged_type();
//         }
//     }
// }


double vel = 0;
bool feedDirect = false;

void setIntake(double intakevel, bool feedDirectbool)
{
    vel = intakevel;
    feedDirect = feedDirectbool;
    intaker(vel);
}

void intakeAntiJam() //lucasleo64
{
    bool running = false;
    light.set_integration_time(20);
    while(true)
    {
        while(true)
        {        
            if(intake.get_torque() > 0.25 && fabs(intake.get_actual_velocity()) < 50 && fabs(intake.get_target_velocity()) > 0)
            {
                double curPos = intake.get_position();
                intaker((intake.get_target_velocity()<0 ? 1 : -1)* 375);
                while(fabs(intake.get_position() - curPos) < 380)
                {
                    wait(15);
                }
                running = false;
            }
            if(!running)
            {
                intaker(vel);
                running = true;
            }
            if(feedDirect && (getRingColor() == getTeam())) {break;}
            wait(10);
        }
        if(feedDirect)
        {
            double curPos = intake.get_position();
            intaker(-vel);
            while(intake.get_position() - curPos > -10)
            {
                wait(10);
            } 
            intaker(0);
        }
        while(feedDirect)
        {
            if(getRingColor() != getTeam()) break;
            wait(100);
        }
    }
}

void autIntaker(double vel)
{
    
}

void deviceMonitor()
{
    bool hasPrinted = false;
    while(true)
    {
        std::vector<int> usedPorts;
        for(deviceInfo device : devices)
        {
            pros::Device deviceChecker(device.port);
            wait(50);
            std::string deviceType;
            switch(device.deviceType)
            {
                case pros::DeviceType::motor: {
                    deviceType = std::string("Motor");
                    break;}
                case pros::DeviceType::optical:{
                    deviceType = std::string("Optical");
                    break;}
                case pros::DeviceType::rotation: {
                    deviceType = std::string("Rotation");
                    break;}
                case pros::DeviceType::imu: {
                    deviceType = std::string("IMU");
                    break;}
                case pros::DeviceType::radio: {
                    deviceType = std::string("Radio");
                    break;}
                default:
                    deviceType = std::string("NA");
            }

            if((deviceChecker.is_installed()))
            {
                std::string info = std::string("DC") + std::to_string(device.port) + "-" + device.deviceName + "-" + deviceType;
                printToController(printMessage(2,0, info),2000,true, true);
                hasPrinted = true;
            }
            else
            {
                if(deviceChecker.get_plugged_type() != device.deviceType)
                {
                    std::string info = std::string("MisMatchTypeP") + std::to_string(device.port);
                    printToController(printMessage(2,0,info), 2000, true, true);
                    hasPrinted = true;
                }
                usedPorts.push_back(device.port);
            }
            wait(20);
        }
        for(int i = 1; i<22; i++)
        {
            if(std::find(usedPorts.begin(), usedPorts.end(), i) == usedPorts.end())
            {
                pros::Device device(i);
                if(!device.is_installed() && device.get_plugged_type() != pros::DeviceType::radio)
                {
                    std::string info = std::string("UnknownDeviceP") + std::to_string(i);
                    printToController(printMessage(2,0,info), 2000, false, true);
                    hasPrinted = true;
                }
            }
            wait(10);
        }
        if(!hasPrinted)
        {
            printToController(printMessage(2,0,std::string("")), 100);
        }
        wait(5000);
        hasPrinted=false;
    }
}

bool correctIntakeJam(double intakeVel)
{
    if(intake.get_torque() > 0.27 && fabs(intake.get_actual_velocity()) < 30 && fabs(intake.get_target_velocity()- intakeVel) < 5 && getRingColor() != getTeam())
    {
        double curPos = intake.get_position();
        intaker((intake.get_target_velocity()<0 ? 1 : -1)*intakeVel);
        while(fabs(intake.get_position() - curPos) < 380)
        {
            if((intake.get_target_velocity() != intakeVel ? !master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) : !master.get_digital(pros::E_CONTROLLER_DIGITAL_R2))) 
                break;
            wait(15);
        }
        return true;
    }
    return false;
}