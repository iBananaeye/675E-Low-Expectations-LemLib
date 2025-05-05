#pragma once
#include "api.h"
#include "lemlib/api.hpp"
#include "pros/adi.hpp"
#include "pros/motors.hpp"

enum Port
{
     /* ------ DRIVETRAIN ------ */
    LEFT_BACK_WHEEL_PORT = 11,
    LEFT_MIDDLE_WHEEL_PORT = 1,
    LEFT_FRONT_WHEEL_PORT = 4,

    RIGHT_BACK_WHEEL_PORT = -19,
    RIGHT_MIDDLE_WHEEL_PORT = -16,
    RIGHT_FRONT_WHEEL_PORT = -2,

    /* ------ MOTORS ------ */
    INTAKE_PORT = -3,
    CONVEYOR_PORT = 10,
    WALL_STAKE_PORT = 21,
    // RIGHT_WALL_STAKE_PORT = -10,

    /* ------ SENSORS ------ */
    COLOR_SENSOR_PORT = 6,
    IMU_PORT = 20,
    ROTATION_SENSOR_PORT = 8,
    PERPENDICULAR_TRACKING_ROTATION_PORT = 2,
    // PARALLEL_TRACKING_ROTATION_PORT = 11,

    /* ------ ADI IN/OUT ------ */
    CLAMP_PORT = 'B',
    SORTER_PORT = 'F', 
    DOINKER_PORT = 'G',
    LEFT_AUTOCLAMP_LIMIT_SWITCH_PORT = 'H',
    RIGHT_AUTOCLAMP_LIMIT_SWITCH_PORT = 'A'
    // INTAKE_LIFTER_PORT = 'H'
};

//All declared in config.cpp except chassis
extern pros::Controller master;
extern lemlib::Chassis chassis; //declared at the top of main.cpp

extern pros::Motor intake;
extern pros::Motor conveyor;
extern pros::Motor wallStake;

extern pros::Imu imu;
extern pros::Optical light;
extern pros::Rotation rotation;
extern pros::Rotation perpendicularRotation;
// extern pros::Rotation parallelRotation;

extern pros::adi::DigitalOut clamp;
extern pros::adi::DigitalOut sorter;
extern pros::adi::DigitalOut doinker;
extern pros::adi::DigitalIn leftAutoClamp;
extern pros::adi::DigitalIn rightAutoClamp;
// extern pros::adi::DigitalOut intakeLifter;

struct deviceInfo
{
    int port;
    pros::DeviceType deviceType;
    std::string deviceName;
    pros::Device* device;
};
extern deviceInfo devices[13];