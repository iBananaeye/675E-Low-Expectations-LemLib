#include "main.h"
#include "pros/adi.hpp"
#include "pros/motors.h"
#include "pros/motors.hpp"


pros::Controller master(pros::E_CONTROLLER_MASTER);

pros::Motor intake(INTAKE_PORT, pros::v5::MotorGears::blue, pros::v5::MotorEncoderUnits::degrees);
// pros::Motor conveyor(CONVEYOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorEncoderUnits::degrees);
pros::Motor leftWallStake(LEFT_WALL_STAKE_PORT,pros::v5::MotorGears::green,pros::v5::MotorEncoderUnits::degrees);
pros::Motor rightWallStake(RIGHT_WALL_STAKE_PORT,pros::v5::MotorGears::green,pros::v5::MotorEncoderUnits::degrees);

pros::Optical light(COLOR_SENSOR_PORT);
pros::Imu imu(IMU_PORT);
pros::Rotation rotation(ROTATION_SENSOR_PORT);
pros::Rotation perpendicularRotation(PERPENDICULAR_TRACKING_ROTATION_PORT);
// pros::Rotation parallelRotation(PARALLEL_TRACKING_ROTATION_PORT);

pros::adi::DigitalOut clamp(CLAMP_PORT);
pros::adi::DigitalOut sorter(SORTER_PORT);
pros::adi::DigitalOut doinker(DOINKER_PORT);
pros::adi::DigitalIn leftAutoClamp(LEFT_AUTOCLAMP_LIMIT_SWITCH_PORT);
pros::adi::DigitalIn rightAutoClamp(RIGHT_AUTOCLAMP_LIMIT_SWITCH_PORT);
// pros::adi::DigitalOut intakeLifter(INTAKE_LIFTER_PORT);

//Right out of the way
//b doinker
//y clamp
//x sorter off

deviceInfo devices[11] = {
    deviceInfo(abs(LEFT_BACK_WHEEL_PORT), pros::DeviceType::motor, "LB"),
    deviceInfo(abs(LEFT_MIDDLE_WHEEL_PORT), pros::DeviceType::motor, "LM"),
    deviceInfo(abs(LEFT_FRONT_WHEEL_PORT), pros::DeviceType::motor, "LF"),
    deviceInfo(abs(RIGHT_BACK_WHEEL_PORT), pros::DeviceType::motor, "RB"),
    deviceInfo(abs(RIGHT_MIDDLE_WHEEL_PORT), pros::DeviceType::motor, "RM"),
    deviceInfo(abs(RIGHT_FRONT_WHEEL_PORT), pros::DeviceType::motor, "RF"),
    deviceInfo(abs(INTAKE_PORT), pros::DeviceType::motor, "Intake"),
    deviceInfo(abs(LEFT_WALL_STAKE_PORT), pros::DeviceType::motor, "Wall"),
    deviceInfo(abs(COLOR_SENSOR_PORT), pros::DeviceType::optical, "Color"),
    deviceInfo(abs(ROTATION_SENSOR_PORT), pros::DeviceType::rotation, "DMech"),
    deviceInfo(abs(IMU_PORT), pros::DeviceType::imu, "IMU")
};