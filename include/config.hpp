#pragma once
#include "api.h"
#include "lemlib/api.hpp"
#include "pros/adi.hpp"
#include "pros/motors.hpp"

//Left Drive: -6, -11, -15
//Right Drive: 17, 20, 7                             
//Conveyor: 9
//Intake: 2
//Wall Stake: 8
//Color Sensor: 10
//Radio: 1
//IMU: 18
//Clamp: B
//AutoClamp Limit Switch: D
//Doinker: H
//Sorter: F
enum Port : signed int
{
    /* ------ DRIVETRAIN ------ */
    LEFT_BACK_WHEEL_PORT = -6,
    LEFT_MIDDLE_WHEEL_PORT = -11,
    LEFT_FRONT_WHEEL_PORT = -15,

    RIGHT_BACK_WHEEL_PORT = 17,
    RIGHT_MIDDLE_WHEEL_PORT = 20,
    RIGHT_FRONT_WHEEL_PORT = 7,

    /* ------ MOTORS ------ */
    INTAKE_PORT = 2,
    CONVEYOR_PORT = 9,
    WALL_STAKE_PORT = 8,

    /* ------ SENSORS ------ */
    COLOR_SENSOR_PORT = 10,
    IMU_PORT = 18,

    /* ------ ADI IN/OUT ------ */
    CLAMP_PORT = 'G',
    SORTER_PORT = 'F', 
    DOINKER_PORT = 'H',
    AUTOCLAMP_PORT = 'D'
};

//All declared in config.cpp except chassis
extern pros::Controller master;
extern lemlib::Chassis chassis; //declared at the top of main.cpp

extern pros::Motor intake;
extern pros::Motor conveyor;
extern pros::Motor arm;

extern pros::Imu imu;
extern pros::Optical light;

extern pros::adi::DigitalOut clamp;
extern pros::adi::DigitalOut sorter;
extern pros::adi::DigitalOut doinker;
extern pros::adi::DigitalIn autoClamp;