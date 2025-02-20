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
enum Port
{
     /* ------ DRIVETRAIN ------ */
    LEFT_BACK_WHEEL_PORT = -11,
    LEFT_MIDDLE_WHEEL_PORT = -1,
    LEFT_FRONT_WHEEL_PORT = -15,

    RIGHT_BACK_WHEEL_PORT = 18,
    RIGHT_MIDDLE_WHEEL_PORT = 16,
    RIGHT_FRONT_WHEEL_PORT = 5,

    /* ------ MOTORS ------ */
    INTAKE_PORT = -7,
    LEFT_WALL_STAKE_PORT = 4,
    RIGHT_WALL_STAKE_PORT = -10,

    /* ------ SENSORS ------ */
    COLOR_SENSOR_PORT = 6,
    IMU_PORT = 14,
    ROTATION_SENSOR_PORT = 8,
    PERPENDICULAR_TRACKING_ROTATION_PORT = 16,
    PARALLEL_TRACKING_ROTATION_PORT = 11,

    /* ------ ADI IN/OUT ------ */
    CLAMP_PORT = 'B',
    SORTER_PORT = 'F', 
    DOINKER_PORT = 'G',
    LEFT_AUTOCLAMP_LIMIT_SWITCH_PORT = 'H',
    RIGHT_AUTOCLAMP_LIMIT_SWITCH_PORT = 'A'
    // INTAKE_LIFTER_PORT = 'H'
};//18,11

// int usedPorts[] = {
//     LEFT_BACK_WHEEL_PORT,
//     LEFT_MIDDLE_WHEEL_PORT,
//     LEFT_FRONT_WHEEL_PORT,
//     RIGHT_BACK_WHEEL_PORT,
//     RIGHT_MIDDLE_WHEEL_PORT,
//     RIGHT_FRONT_WHEEL_PORT,
//     INTAKE_PORT,
//     LEFT_WALL_STAKE_PORT,
//     RIGHT_WALL_STAKE_PORT,
//     COLOR_SENSOR_PORT,
//     IMU_PORT,
//     ROTATION_SENSOR_PORT,
//     CLAMP_PORT,
//     SORTER_PORT , 
//     AUTOCLAMP_LIMIT_SWITCH_PORT
// };


//All declared in config.cpp except chassis
extern pros::Controller master;
extern lemlib::Chassis chassis; //declared at the top of main.cpp

extern pros::Motor intake;
extern pros::Motor leftWallStake;
extern pros::Motor rightWallStake;

extern pros::Imu imu;
extern pros::Optical light;
extern pros::Rotation rotation;
extern pros::Rotation perpendicularRotation;
extern pros::Rotation parallelRotation;

extern pros::adi::DigitalOut clamp;
extern pros::adi::DigitalOut sorter;
extern pros::adi::DigitalOut doinker;
extern pros::adi::DigitalIn leftAutoClamp;
extern pros::adi::DigitalIn rightAutoClamp;
extern pros::adi::DigitalOut intakeLifter;

struct deviceInfo
{
    int port;
    pros::DeviceType deviceType;
    std::string deviceName;
    pros::Device* device;
};
extern deviceInfo devices[11];