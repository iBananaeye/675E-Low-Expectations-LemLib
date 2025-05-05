#include "lemlib/api.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"
#include "string"
#include "queue"

void intaker(double v) {
    intake.move_velocity(v);
    conveyor.move_velocity(v);
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
    // double deltaX = dist * sin(heading) * (params.forwards ? 1 : -1);
    // double deltaY = dist * cos(heading) * (params.forwards ? 1 : -1);
    double deltaX = dist * sin(heading);
    double deltaY = dist * cos(heading);

    chassis.moveToPoint(pose.x + deltaX, pose.y + deltaY, timeout, params, async);
}

void turnRelative(double deltaAngle, int timeout, lemlib::TurnToHeadingParams params, bool async)
{
    chassis.turnToHeading(chassis.getPose().theta + deltaAngle, timeout, params, async);
}








std::queue<printMessage> printQueue;
std::queue<printMessage> driverInfoQueue;
std::queue<printMessage> autonInfoQueue;
pros::Mutex screenMutex;
int screen = 0;
// const int numScreens = 5;
const int maxQueueLength = 30;

void printToController(printMessage printedMessage, int waitTimeInMs, bool finishWaiting, bool rumble, std::string rumblePattern)
{
    if(waitTimeInMs != 0)
    {
        int waitedTime = 0;
        while(waitedTime < waitTimeInMs)
        {
            if(screenMutex.try_lock())
            {
                printQueue.push(printedMessage);
                if(rumble) printQueue.push(printMessage(-1,-1, rumblePattern));
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
    int selectedAutonIndex = 0;
    wait(1000);
    while(true)
    {
        wait(30);
        if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT))
        {
            screen++;
            if(screen==numScreens)
            {
                screen =0;
                updateDriverScreenInfo();
            }
            switch(screen)
            {
                case 1:
                    printToController(printMessage(0,0,std::string("AutonInfo"),1));
                    printToController(printMessage(1,0,std::string("Name: ") + (autonList[selectedAutonIndex].autonName),1));
                    printToController(printMessage(2,0,std::string("Color: ") + (autonList[selectedAutonIndex].allianceColor),1));
                    break;
                case 2:
                    master.print(0,0,"Odometry         ");
                    break;
                case 3:
                    master.print(0,0,"LBDebugger        ");
                    break;
                case 4:
                    master.print(0,0,"AntiJamDebugger   ");
                    break;
            }
            wait(95);
        }
        if(screen >1)
        {
            switch(screen)
            {
                case 2:
                    master.print(1,0, "X:%.1lf Y:%.1lf         ",chassis.getPose().x, chassis.getPose().y);
                    wait(125);
                    master.print(2, 0, "R: %.1lf          ", chassis.getPose().theta);
                    wait(125);
                    break;
                case 3:
                    master.print(1,0, "LBPos: %.1lf         ",wallStake.get_position());
                    wait(125);
                    master.print(2, 0, "RotAng: %.1lf        ", rotation.get_angle()/100.0);
                    wait(125);
                    break;
                case 4:
                    master.print(1,0, "TarVel: %d         ",intake.get_target_velocity());
                    wait(125);
                    master.print(2,0,"Torque: %.3lf            ",intake.get_torque());
                    wait(125);
                    break;
            }
        }
        else
        {
            if(!printQueue.empty())
            {
                while(!screenMutex.try_lock()) 
                {
                    wait(20);
                }
                printMessage message = printQueue.front();
                printQueue.pop();
                if(screen != message.screenNum)
                {
                    printQueue.push(message);
                    screenMutex.unlock();
                    continue;
                }
                screenMutex.unlock();
                if(message.rowNum == -1 || message.colNum == -1) master.rumble(message.text.c_str());
                else master.print(message.rowNum, message.colNum,"%s", (message.text + (message.text.length() < 16 ? std::string(16 - message.text.length(), ' ') : "")).c_str());
                wait(95);
            }
        }
    }
}

double vel = 0;
bool feedDirect = false;

void setIntake(double intakevel, bool feedDirectbool)
{
    vel = intakevel;
    feedDirect = feedDirectbool;
    intaker(vel);
}

void intakeAntiJam() 
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

void deviceMonitor()
{
    bool isMuted = false;
    bool hasPrinted = false;
    while(true)
    {
        printf("testmain1\n");
        std::vector<int> usedPorts;
        for(deviceInfo device : devices)
        {
            if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
            {
                isMuted = !isMuted;
                printToController(printMessage(2,0, std::string("Muted")),1000,true);
            }

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
                printToController(printMessage(2,0, info),2000,true, !isMuted, "-");
                hasPrinted = true;
            }
            else
            {
                if(deviceChecker.get_plugged_type() != device.deviceType)
                {
                    std::string info = std::string("MisMatchTypeP") + std::to_string(device.port);
                    printToController(printMessage(2,0,info), 2000, true, !isMuted, ".");
                    hasPrinted = true;
                }
                usedPorts.push_back(device.port);
            }
            wait(20);
        }
        for(int i = 1; i<22; i++)
        {
            if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
            {
                isMuted = !isMuted;
                printToController(printMessage(2,0, isMuted ? std::string("Muted") : std::string("Unmuted")),1000,true);
            }
            if(std::find(usedPorts.begin(), usedPorts.end(), i) == usedPorts.end())
            {
                pros::Device device(i);
                if(!device.is_installed() && device.get_plugged_type() != pros::DeviceType::radio)
                {
                    std::string info = std::string("UnknownDeviceP") + std::to_string(i);
                    printToController(printMessage(2,0,info), 2000, false, !isMuted, ".");
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


void recordDriveNoSDCard(int seconds)
{
    int arrayLength = seconds * 50 *2;
    int stickData[arrayLength];
    int i =0;
    double startTime = pros::millis()/1000.0;
    double timeOffset = 0;
    int exitLength = arrayLength;
    while(i<arrayLength-1)
    {
      int leftY = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
      int rightX = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
      stickData[i++] = leftY;
      stickData[i++] = rightX;

      chassis.arcade(leftY, rightX);

      
      if((i+1) % 50)
      {
        printToController(printMessage(2,0,std::string("Rec:") + std::to_string((int)(seconds-pros::millis()/1000-startTime-timeOffset)) + "SecLeft"));
      }
      if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT))
      {
        double pauseStart = pros::millis();
        chassis.arcade(0, 0);
        printToController(printMessage(2,0,std::string("Rec:Paused")));
        wait(1000);
        while(true)
        {
            wait(50);
            if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT) || master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) break;
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) 
        {
            exitLength = i;
            break;
        }
        timeOffset = (pros::millis() - pauseStart)/1000.0;
      }
      pros::delay(20);
    }
    chassis.arcade(0, 0);
    wait(1000);
    printToController(printMessage(2,0,std::string("PlayBackReady")));
    while(!master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
    {
      wait(50);
    }
    wait(1000);
    i = 0;
    printToController(printMessage(2,0,std::string("Playback:Ongoing")));
    while(i < exitLength-1)
    {
        int leftY = stickData[i++];
        int rightX = stickData[i++];

        chassis.arcade(leftY, rightX);
        if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT))
        {
            double pauseStart = pros::millis();
            chassis.arcade(0, 0);
            printToController(printMessage(2,0,std::string("Playback:Paused")));
            wait(1000);
            while(true)
            {
                wait(50);
                if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT) || master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) break;
            }
            printToController(printMessage(2,0,std::string("Playback:Ongoing")));
        }
        pros::delay(20.2);
    }
    chassis.arcade(0, 0);
}




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

