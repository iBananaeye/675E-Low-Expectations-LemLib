#include "lemlib/api.hpp" // IWYU pragma: keep
#include "helper_functions.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"
#include "string"


const int ON = 1;
const int OFF = 0;

const int vel = 600;
const int arm_vel = 500; 

bool clamp_state = false;

//Wall Stake Positions in encoder ticks (Negative is up)
// const int DOWN_POSITION = -15; //Not 0 to make sure the motors don't fry themselves going through metal
// const int LOAD_POSITION = -115;
// const int SCORE_POSITION = -555; 
// const int INSERT_POSITION = -630;

enum : int {RED, BLUE, OTHER};
const int sorterWaitTime = 275; //time before piston is retracted

void intakes() {
    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            intaker(vel); //Intakes
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            intaker(-vel); //Outakes
        } else {
            intaker(0);
        }
        pros::delay(10); 
    }
}

void intakesConveyorSorter() {
    int team = getTeam(); // defaults to red
    int color = team;
    bool sorterState = false;
    bool manual = false; // defaults to automatic

    light.set_integration_time(20);
    master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B"); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'
    while (true) {
        color = getRingColor();
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) // Alternates from auto to manual mode
        {
            manual = !manual;
            wait(300);
            master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
        }
        if(!manual && master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) && !master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)&& color != 2 && light.get_proximity() > 215 &&color !=team && intake.get_actual_velocity() > 10)
        {
            // intaker(-vel);
            // wait(1000);
            // intaker(0);
            master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B"); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'
            wait(220);
            intaker(-vel);
            wait(275);
            intaker(0);
            
        }
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            intaker(vel); //Intakes
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            intaker(-vel); //Outakes
        } else {
            intaker(0);
        }
        pros::delay(10); 
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

void sorts()
{
    // int team = getTeam(); // defaults to red
    // int color = team;
    // bool sorterState = false;
    // bool manual = true; // defaults to manual

    // light.set_integration_time(25); //25 Millisecond refresh rate
    // //The sensor can go down to 3, but brain only accepts every 20ms. Anything faster than this leds to nonsense being sensed
    
    // // wait(300);
    // // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B"); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'
    // printToController(printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manual ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B")), 300, true);

    // sorter.set_value(OFF); //defaults to off
    // while(true)
    // {
    //     wait(15);
    //     color = getRingColor();
    //     printMessage info = printMessage(0,0,std::string("T") + (team == RED ? "R" : "B") + (manual ? "Man" : "Aut") + (color == RED ? "R" : color==OTHER ? "U" : "B"));
    //     if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)) // Alternates from auto to manual mode
    //     {
    //         manual = !manual;
    //         sorter.set_value(OFF);
    //         // wait(300);
    //         // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
    //         printToController(info, 300, true);
    //     }
    //     if(!manual && (color != OTHER) && light.get_proximity() == 255)
    //     {
    //         switch (color == team)
    //         {
    //             case true:
    //                 sorter.set_value(OFF);
    //                 sorterState = OFF;
    //                 // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
    //                 printToController(info);
    //                 break;
    //             case false:
    //                 sorter.set_value(ON);
    //                 // master.print(0,0, "T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
    //                 // wait(sorterWaitTime);
    //                 printToController(info, sorterWaitTime, true);
    //                 while(true) //prevents wasting air if opposing rings is held over sensor
    //                 {
    //                     if((getRingColor() == team || getRingColor() == OTHER) || master.get_digital(pros::E_CONTROLLER_DIGITAL_Y))
    //                     {
    //                         break;
    //                     }
    //                     wait(sorterWaitTime);
    //                 }
    //                 sorter.set_value(OFF);
    //                 break;
    //         }
    //     }
    //     if(manual)
    //     {
    //         if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) //manually control like clamp
    //         {
    //             sorter.set_value(!sorterState);
    //             sorterState = !sorterState;
    //             wait(300);
    //         }
    //     }
    // }
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

void autoClamps()
{  
    clamp.set_value(false);
    bool state = false;
    bool isClamped = false;
    bool active = true;
    // wait(500);
    // master.print(2,0, "%s-%s", active ? "Aut" : "Man", isClamped ? "Clamped" : "Unclamped");
    printToController(printMessage(2,0,std::string(active ? "AutClamp-" : "ManClamp-") + (isClamped ? "Clamped" : "Unclamped")), 300, true);
    while(true)
    {
        if(active && state == false && autoClamp.get_value() == ON)
        {
            clamp.set_value(true);
            wait(250);
            state = true;
            isClamped = true;
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y))
        {
            clamp.set_value(!isClamped);
            isClamped = !isClamped;
            wait(400);
        }
        if(autoClamp.get_value() == OFF)
        {
            state = false;
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
        {
            active = !active;
            // wait(300);
            // master.print(2,0, "%s-%s", active ? "Aut" : "Man", isClamped ? "Clamped" : "Unclamped");
            printToController(printMessage(2,0,std::string(active ? "AutClamp-" : "ManClamp-") + (isClamped ? "Clamped" : "Unclamped")), 300, true);
        }
        wait(20);
    }
}




enum directWallScorePositions : signed int
{
    DOWN_POSITION = 360,
    SCORE_POSITION = 290,
    OUT_POSITION = 335
};

void directWallScore()
{
    // PID Constants
    double kP = 2;  //4
    double kI = 0; 
    double kD = 0; //0.575
    double gain = 2;
    double armThreshold = 2;
    
    const int waitTime = 20;
    const int maxVelocity = 80;
    
    double armTarget = DOWN_POSITION;
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

    leftWallStake.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    rightWallStake.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    rotation.set_data_rate(10);
    // rotation.reset_position();
    // rotation.reverse();

    wait(300);
    while(true)
    {
        
        //----------PID----------//
        error = armTarget-convertAngle(armTarget, rotation.get_angle()/ 100.0); //degrees
        deltaTimeInMs = pros::millis() - previousTimeInMs;

        proportional = error; //More error equals faster motion 
        if(fabs(error) > armThreshold) integral += error * deltaTimeInMs / 1000.0; //Gets accumulation of error (Effectively ramps up force if error is not going away)
        derivative = (error - previousError)/(deltaTimeInMs/1000.0); //Positive only when error is getting worse, negative when getting better (accounts for momentum)

        correctionVelocity = gain * (kP * proportional + kI * integral + kD * derivative);
        if(fabs(error) > armThreshold)
        {
            signedMaxVelocity = correctionVelocity * maxVelocity / fabs(correctionVelocity);

            leftWallStake.move_velocity((int) (fabs(correctionVelocity) < maxVelocity ? correctionVelocity : signedMaxVelocity));
            rightWallStake.move_velocity((int) (fabs(correctionVelocity) < maxVelocity ? correctionVelocity : signedMaxVelocity));
        }
        else
        {
            leftWallStake.move_velocity(0);
            rightWallStake.move_velocity(0);
        }
        previousError = error;
        previousTimeInMs += deltaTimeInMs;


        //----------OP-Control----------//
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
        {
            armTarget = DOWN_POSITION;
            kP = 2;
            kD = 0;
            armThreshold = 5;
            gain = 2;
            integral = 0;
            kI = 0;
        }
        else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
        {
            armTarget = SCORE_POSITION;
            kP = 2;
            kD = 0;
            armThreshold = 1.5;
            gain = 1.5;
            kI= 0.05;
        }
        else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT))
        {
            armTarget = OUT_POSITION;
            kP = 2;
            kD = 0;
            armThreshold = 1.5;
            gain = 1.5;
            kI= 0;
        }

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