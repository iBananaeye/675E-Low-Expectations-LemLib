#include "main.h"
#include "pros/adi.hpp"
#include "pros/motors.h"
#include "pros/motors.hpp"


pros::Controller master(pros::E_CONTROLLER_MASTER);

pros::Motor intake(INTAKE_PORT, pros::v5::MotorGears::blue, pros::v5::MotorEncoderUnits::degrees);
// pros::Motor conveyor(CONVEYOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorEncoderUnits::degrees);
pros::Motor arm(WALL_STAKE_PORT, pros::v5::MotorGears::blue, pros::v5::MotorEncoderUnits::degrees);

pros::Optical light(COLOR_SENSOR_PORT);
pros::Imu imu(IMU_PORT);
pros::Rotation rotation(ROTATION_SENSOR_PORT);
pros::Rotation perpendicularRotation(PERPENDICULAR_TRACKING_ROTATION_PORT);
pros::Rotation parallelRotation(PARALLEL_TRACKING_ROTATION_PORT);

pros::adi::DigitalOut clamp(CLAMP_PORT);
pros::adi::DigitalOut sorter(SORTER_PORT);
pros::adi::DigitalOut doinker(DOINKER_PORT);
pros::adi::DigitalIn autoClamp(AUTOCLAMP_LIMIT_SWITCH_PORT);