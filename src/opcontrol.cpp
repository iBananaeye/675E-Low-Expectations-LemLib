#include "helper_functions.hpp"
#include "main.h"
#include "pros/adi.h"
#include "pros/misc.h"
#include "pros/motors.h"

const int ON = 1;
const int OFF = 0;

const int vel = 600;
const int arm_vel = 500; 

bool clamp_state = false;

//Wall Stake Positions in encoder ticks (Negative is up)
const int DOWN_POSITION = -15; //Not 0 to make sure the motors don't fry themselves going through metal
const int LOAD_POSITION = -115;
const int SCORE_POSITION = -555; 
const int INSERT_POSITION = -630;

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

void intakesConSorter() {
    int team = getTeam(); // defaults to red
    int color = team;
    bool sorterState = false;
    bool manual = false; // defaults to automatic

    light.set_integration_time(20);
    while (true) {
        color = getRingColor();
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1) && !master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)&& color != 2 && light.get_proximity() > 215 &&color !=team)
        {
            intaker(-vel);
            wait(1000);
            intaker(0);
            // wait(75);
            // intaker(-vel);
            // wait(250);
            // intaker(0);
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

void wall_score() {
    arm.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    arm.tare_position();
    int armPosition = DOWN_POSITION;
    while (true) 
    {
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1) )
        {
            if(armPosition == DOWN_POSITION)
            {
                armPosition = LOAD_POSITION;
                arm.move_absolute(LOAD_POSITION, arm_vel);
            }
            else
            {
                armPosition = DOWN_POSITION;
                arm.move_absolute(DOWN_POSITION, arm_vel);
            }
            wait(250);
        }
        else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
        {
            if(armPosition == LOAD_POSITION)
            {
                armPosition = SCORE_POSITION;
                arm.move_absolute(SCORE_POSITION, arm_vel);
            }
            else if(armPosition == SCORE_POSITION)
            {
                armPosition = INSERT_POSITION;
                arm.move_absolute(INSERT_POSITION, arm_vel);
            }
            else if(armPosition == INSERT_POSITION)
            {
                armPosition = LOAD_POSITION;
                arm.move_absolute(LOAD_POSITION, arm_vel);
            }
            else
            {
                arm.move_absolute(SCORE_POSITION, arm_vel);
            }
            wait(300);
        }
        // Outputs encoder to screen to find wall stake positions, uncomment when drastic changes are made to wall mech
        // master.print(0,0, "Pos: %.1lf", arm.get_position());
        // wait(250);
        pros::delay(20);
    }
}

void sorts()
{
    int team = getTeam(); // defaults to red
    int color = team;
    bool sorterState = false;
    bool manual = false; // defaults to automatic

    light.set_integration_time(25); //25 Millisecond refresh rate
    //The sensor can go down to 3, but brain only accepts every 20ms. Anything faster than this leds to nonsense being sensed
    
    wait(300);
    master.print(0,0,"T%s-%s %s", 
            team == RED ? "R" : "B" ,
            manual ? "Man" : "Aut", 
            color == RED ? "R" : color==OTHER ? "U" : "B"
        ); //Prints 'T[Alliance Color Initial]-[Whether in auto or manual mode] [Last seen color]'

    sorter.set_value(OFF); //defaults to off
    while(true)
    {
        wait(10);
        color = getRingColor();
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y)) // Alternates from auto to manual mode
        {
            manual = !manual;
            wait(300);
            sorter.set_value(OFF);
            master.print(0,0,"T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
        }
        if(!manual && (color != OTHER) && light.get_proximity() == 255)
        {
            switch (color == team)
            {
                case true:
                    sorter.set_value(OFF);
                    sorterState = OFF;
                    master.print(0,0,"T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
                    break;
                case false:
                    sorter.set_value(ON);
                    master.print(0,0,"T%s-%s %s ", team == RED ? "R" : "B" , manual ? "Man" : "Aut", color == RED ? "R" : color==OTHER ?"U" : "B");
                    while(true) //prevents wasting air if opposing rings is held over sensor
                    {
                        wait(sorterWaitTime);
                        if((getRingColor() == team || getRingColor() == OTHER))
                        {
                            break;
                        }
                    }
                    sorter.set_value(OFF);
                    break;
            }
        }
        if(manual)
        {
            if(master.get_digital(pros::E_CONTROLLER_DIGITAL_X)) //manually control like clamp
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
            wait(200);
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
    wait(300);
    while(true)
    {
        if(active && state == false && autoClamp.get_value() == ON)
        {
            clamp.set_value(true);
            wait(250);
            state = true;
            isClamped = true;
        }
        if(master.get_digital(pros::E_CONTROLLER_DIGITAL_A))
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
            wait(300);
        }
        wait(20);
    }
}



//ignore everything below here, i would comment it out but
//I would need to comment stuff out across 3 files and that is no fun

// void debugTurn() //help find turn angles
// {
//     master.clear();
//     while(true)
//     {
//         if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT))
//         {
//             imu.tare_yaw();
//             wait(200);
//         }
//         master.print(1,0, "Angle: %.1lf  ", imu.get_yaw());
//         wait(300);
//     }
// }


// Has not been fixed since lemlib replacement
// int inches = 0;
// void debugDrive() //Lets you move a certain amount from controller input, not used anywhere 
// {
//     chassis.set_drive_brake(pros::E_MOTOR_BRAKE_BRAKE);
//     master.clear();
//     while(true)
//     {
//         int target = 0;
//         while(master.get_digital(pros::E_CONTROLLER_DIGITAL_RIGHT) == 0)
//         {
//             if(master.get_digital(pros::E_CONTROLLER_DIGITAL_LEFT) == 1 && master.get_digital(pros::E_CONTROLLER_DIGITAL_UP) == 1)
//             {
//                 inches = 0;
//             }
//             else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_UP))
//             {
//                 target += 1;
//             }
//             else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
//             {
//                 target -= 1;
//             }
//             master.print(2,0, "Dist: %d, Go: %d     ", inches, target);
//             wait(200);
//         }
//         for(pros::Motor m : chassis.left_motors)
//         {
//             if(target < 0)
//             {
//                 m.move_velocity(-40);
//             }
//             else
//             {
//                 m.move_velocity(40);
//             }
            
//         }
//         for(pros::Motor m : chassis.right_motors)
//         {
//             if(target < 0)
//             {
//                 m.move_velocity(-42);
//             }
//             else
//             {
//                 m.move_velocity(42);
//             }
//         }
//         chassis.left_motors[1].tare_position();
//         double factor = 48;
//         int offset = 20;
//         if(target < 0)
//         {
//             factor = 51.5;
//         }
//         else if(target == 0)
//         {
//             offset = 0;
//         }
//         while((abs(chassis.left_motors[1].get_position()) < abs(target * factor - offset)))
//         {
//             wait(100);
//         }
//         for(pros::Motor m : chassis.left_motors)
//         {
//             m.move_velocity(0);
//         }
//         for(pros::Motor m : chassis.right_motors)
//         {
//             m.move_velocity(0);
//         }
//         inches += target;
//         wait(200);
//         master.clear_line(1);
//     }
// }