#include "lemlib/api.hpp" // IWYU pragma: keep
#include "helper_functions.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"
#include "string"


const int ON = 1;
const int OFF = 0;

const int vel = 520;
const int arm_vel = 500; 
const int dMechVel = vel/1.6;

bool clamp_state = false;

enum : int {RED, BLUE, OTHER};
const int sorterWaitTime = 325; //time before piston is retracted



void intakes() {
    while (true) {
        //anti-jam
        correctIntakeJam(vel);
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            intaker(vel); //Intakes
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            intaker(-vel); //Outakes
        } else {
            intaker(0);
        }
        pros::delay(10); 

        //direct mech macro
        if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X))
        {
            intaker(dMechVel);
            while(getRingColor() != getTeam())
            {
                
                if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) break;
                if(intake.get_torque() > 0.3 && fabs(intake.get_actual_velocity()) < 30 && fabs(intake.get_target_velocity()- dMechVel) <5 && getRingColor() != getTeam())
                {
                    double curPos = intake.get_position();
                    intaker((intake.get_target_velocity()<0 ? 1 : -1)*dMechVel);
                    while(fabs(intake.get_position() - curPos) < 380)
                    {
                        if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) break;
                        wait(15);
                    }
                    
                }
                wait(10);
                if((intake.get_target_velocity()<0))
                {
                    intaker(dMechVel);
                }
                
            }
            double curPos = intake.get_position();
            intaker(-vel);
            while(intake.get_position() - curPos > -10)
            {
                wait(10);
                if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) || master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) break;
            } 
            intaker(0);
        }
    }
}


void clamps() {
    clamp.set_value(clamp_state); // retracted

    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_A)) {
            clamp.set_value(!clamp_state);
            clamp_state = !clamp_state;
            wait(460);
        }
        pros::delay(10);
    }
}

bool manualSort = false; // defaults to manual
void sorts()
{
    int team = getTeam(); // defaults to red
    int color = team;
    bool sorterState = false;
    

    light.set_integration_time(25); //25 Millisecond refresh rate
    //The sensor can go down to 3, but brain only accepts every 20ms. Anything faster than this leds to nonsense being sensed
    
    printToController(printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manualSort ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B")), 300, true);

    sorter.set_value(OFF); //defaults to off
    while(true)
    {
        wait(15);
        color = getRingColor();
        printMessage info = printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manualSort ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B"));
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT)) // Alternates from auto to manual mode
        {
            manualSort = !manualSort;
            sorter.set_value(OFF);
            printToController(printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manualSort ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B")), 300, true);
        }
        if(!manualSort && (color != OTHER) && light.get_proximity() == 255)
        {
            switch (color == team)
            {
                case true:
                    sorter.set_value(OFF);
                    sorterState = OFF;
                    printToController(info);
                    break;
                case false:
                    sorter.set_value(ON);
                    double curPos = intake.get_position();
                    printToController(info, sorterWaitTime, true);
                    while(intake.get_position() - curPos < 275) //prevents wasting air if opposing rings is held over sensor
                    {
                        if((light.get_hue() != 0 && (getRingColor() == team || getRingColor() == OTHER)) || master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
                        {
                            break;
                        }
                        wait(10);
                    }
                    sorter.set_value(OFF);
                    break;
            }
        }
        if(manualSort)
        {
            if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) //manually control like clamp
            {
                sorter.set_value(!sorterState);
                sorterState = !sorterState;
                wait(300);
            }
        }
    }
}

void doinks()
{
    bool doinkerState = OFF;
    doinker.set_value(OFF);
    while(true)
    {
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_B))
        {
            doinker.set_value(!doinkerState);
            doinkerState = !doinkerState;
            wait(350);
        }
        pros::delay(20);
    }
}

bool hasClamped = false;
bool autoClampActive = true;
void autoClamps()
{  
    clamp.set_value(false);
    bool state = false;
    printToController(printMessage(1,0,std::string(autoClampActive ? "AutClam-" : "ManClam-") + (hasClamped ? "Clamped" : "Unclamped")), 300, true);
    while(true)
    {
        if(autoClampActive && state == false && leftAutoClamp.get_value() == ON && rightAutoClamp.get_value() == ON)
        {
            clamp.set_value(true);
            state = true;
            hasClamped = !hasClamped;
            printToController(printMessage(1,0,std::string("AutClam-Clamped")), 450, true, true, ".");
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A))
        {
            clamp.set_value(!hasClamped);
            hasClamped = !hasClamped;
            printToController(printMessage(1,0,std::string(autoClampActive ? "AutClam-" : "ManClam-") + (hasClamped ? "Clamped" : "Unclamped")), 400, true);
        }
        if(leftAutoClamp.get_value() == OFF && rightAutoClamp.get_value() == OFF)
        {
            state = false;
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y))
        {
            autoClampActive = !autoClampActive;
            printToController(printMessage(1,0,std::string(autoClampActive ? "AutClam-" : "ManClam-") + (hasClamped ? "Clamped" : "Unclamped")), 300, true);
        }
        wait(20);

    }
}




enum directWallScorePositions : signed int
{
    DOWN_POSITION = 0,
    SCORE_POSITION = 125,
    OUT_POSITION = 90
};

// PID Constants
    double kP = 2;  //4
    double kI = 0; 
    double kD = 0; //0.575
    double gain = 2.2;
    double armThreshold = 5;
    double armTarget = DOWN_POSITION;
    
void directWallScore()
{
    const int waitTime = 20;
    const int maxVelocity = 200;
    
    
    double error = (armTarget - convertAngle(armTarget, rotation.get_angle()/100.0)); //Target-currentAngle in degrees;
    double previousError = error;
    int previousTimeInMs = 0;
   
    int deltaTimeInMs;
    double proportional;
    double integral;
    double derivative;
    
    double correctionVelocity;
    int signed signedMaxVelocity;

    const int numVars = 5;
    double* varsPID[numVars] = {&kP, &kI, &kD, &gain, &armThreshold};
    char* varsPIDNames[numVars] = {(char*)"kP", (char*)"kI", (char*)"kD", (char*)"Gain", (char*)"Thold"}; 
    int index = 0;

    wallStake.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    rotation.set_data_rate(10);
  
  
    wait(300);
    while(true)
    {
        
        // ----------PID----------//
        error = armTarget-convertAngle(armTarget, rotation.get_angle()/ 100.0); //degrees
        deltaTimeInMs = pros::millis() - previousTimeInMs;

        proportional = error; //More error equals faster motion 
        if(fabs(error) > armThreshold) integral += error * deltaTimeInMs / 1000.0; //Gets accumulation of error (Effectively ramps up force if error is not going away)
        derivative = (error - previousError)/(deltaTimeInMs/1000.0); //Positive only when error is getting worse, negative when getting better (accounts for momentum)

        correctionVelocity = gain * (kP * proportional + kI * integral + kD * derivative);
        if(fabs(error) > armThreshold)
        {
            signedMaxVelocity = correctionVelocity * maxVelocity / fabs(correctionVelocity);

            wallStake.move_velocity((int) (fabs(correctionVelocity) < maxVelocity ? correctionVelocity : signedMaxVelocity));
        }
        else
        {
            wallStake.move_velocity(0);
        }
        previousError = error;
        previousTimeInMs += deltaTimeInMs;


        //----------OP-Control----------//
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
        {
            armTarget = DOWN_POSITION;
            kP = 2;
            kD = 0;
            armThreshold = 0.5;
            gain = 2;
            integral = 0;
            kI = 0;
            // wallStake.move_velocity(200);
        }
        else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
        {
            armTarget = SCORE_POSITION;
            kP = 3.2;
            kD = 0.1
            ;
            armThreshold = 1.5;
            gain = 1.75;
            kI= 0.00;
            // wallStake.move_velocity(-200);
        }
        // else{
        //     wallStake.move_velocity(0);
        // }



        // else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT))
        // {
        //     armTarget = OUT_POSITION;
        //     kP = 2;
        //     kD = 0;
        //     armThreshold = 1.5;
        //     gain = 1.5;
        //     kI= 0;
        // }

        // :( this is way too complicated i feel sorry for whoever has to read my absurd use of pointers and ternary operators
        //----------Live-PID-Tuner----------//
        // else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT))
        // {
        //     leftWallStake.move_velocity(0);
        //     rightWallStake.move_velocity(0);
        //     while(true)
        //     {
        //         if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP) && master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
        //         {
        //             break;
        //         }
        //         else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP))
        //         {
        //             (*varsPID[index]) = (*varsPID[index]) + master.get_digital(pros::E_CONTROLLER_DIGITAL_X) ? 1 : 0.1;
        //         }
        //         else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
        //         {
        //             (*varsPID[index]) = (*varsPID[index]) - ((*varsPID[index]) == 0) ? 0 : master.get_digital(pros::E_CONTROLLER_DIGITAL_X) ? 1 : 0.1;
        //         }
        //         else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
        //         {
        //             index -= index == 0 ? 0 : 1;
        //         }
        //         else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT))
        //         {
        //             index += index == numVars ? 0 : 1;
        //         }
        //         master.print(0,0, "%s:%.1lf E:%.1lf V:%d     ", *varsPIDNames[index], (*varsPID[index]), error, (int)correctionVelocity);
        //         wait(250);
        //         // printToController(printMessage(0,0, std::string(varsPIDNames[index]) + ":" + std::to_string(*varsPID[index]) + " E:" + std::to_string(error) + " V:" + std::to_string((int)correctionVelocity)), 250, true);
        //     }
        //     integral = 0;
        // }
        wait(waitTime);
    }

    
}


void setDirect()
{
    armTarget = SCORE_POSITION;
    kP = 2;
    kD = 0.2;
    armThreshold = 1.5;
    gain = 1.75;
    kI= 0.05;
}





// deprecated ladybrown code below
// void wall_score() {
//     arm.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
//     arm.tare_position();
//     int armPosition = DOWN_POSITION;
//     while (true) 
//     {
//         if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1) )
//         {
//             if(armPosition == DOWN_POSITION)
//             {
//                 armPosition = LOAD_POSITION;
//                 arm.move_absolute(LOAD_POSITION, arm_vel);
//             }
//             else
//             {
//                 armPosition = DOWN_POSITION;
//                 arm.move_absolute(DOWN_POSITION, arm_vel);
//             }
//             wait(250);
//         }
//         else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
//         {
//             if(armPosition == LOAD_POSITION)
//             {
//                 armPosition = SCORE_POSITION;
//                 arm.move_absolute(SCORE_POSITION, arm_vel);
//             }
//             else if(armPosition == SCORE_POSITION)
//             {
//                 armPosition = INSERT_POSITION;
//                 arm.move_absolute(INSERT_POSITION, arm_vel);
//             }
//             else if(armPosition == INSERT_POSITION)
//             {
//                 armPosition = LOAD_POSITION;
//                 arm.move_absolute(LOAD_POSITION, arm_vel);
//             }
//             else
//             {
//                 arm.move_absolute(SCORE_POSITION, arm_vel);
//             }
//             wait(300);
//         }
//         // Outputs encoder to screen to find wall stake positions, uncomment when drastic changes are made to wall mech
//         // master.print(0,0, "Pos: %.1lf", arm.get_position());
//         // wait(250);
//         pros::delay(20);
//     }
// }

// void lifts() {
//     intakeLifter.set_value(false); // retracted
//     bool state = false;

//     while (true) {
//         if (master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) {
//             intakeLifter.set_value(!state);
//             state = !state;
//             wait(460);
//         }
//         pros::delay(10);
//     }
// }

// void intakesConveyorSorter() {
//     int team = getTeam(); // defaults to red
//     int color = team;
//     bool sorterState = false;
//     bool manual = true; // defaults to automatic
//     light.set_integration_time(20);
//     printToController(printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manual ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B")),300, true);
//     // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B"); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'
//     while (true) {
//         color = getRingColor();
//         printMessage info = printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manual ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B"));
//         if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) // Alternates from auto to manual mode
//         {
//             manual = !manual;
//             printToController(info, 300, true);
//             // wait(300);
//             // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
//         }
//         if(!manual && master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) && !master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)&& color != 2 && light.get_proximity() > 215 &&color !=team && intake.get_actual_velocity() > 10)
//         {
//             // intaker(-vel);
//             // wait(1000);
//             // intaker(0);
//             // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B"); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'
//             // wait(220);
//             printToController(info, 190, true);
//             intaker(-vel);
//             double curPos = intake.get_position();
//             while(intake.get_position() - curPos > -100)
//             {
//                 wait(10);
//             }
//             intaker(0);
//         }
//         if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
//             intaker(vel); //Intakes
//         } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
//             intaker(-vel); //Outakes
//         } else {
//             intaker(0);
//         }
//         pros::delay(10); 
//     }
// }

void  updateDriverScreenInfo()
{
    printToController(printMessage(0,0,std::string("T") + (getTeam() == RED ? "R" : "B") + (manualSort ? "Man" : "Aut") + (getRingColor() == RED ? "R" : getRingColor()==OTHER ? "U" : "B")), 20);
    printToController(printMessage(1,0,std::string(autoClampActive ? "AutClam-" : "ManClam-") + (hasClamped ? "Clamped" : "Unclamped")), 20, false);
}