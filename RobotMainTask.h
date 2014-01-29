#ifndef SHS_2605_TESTCODE_2_2014_H
#define SHS_2605_TESTCODE_2_2014_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"

#include "SubSystems/MecanumDrive.h"

#include "CANJagServer/CANJaguarServer.h"
#include "CANJagServer/AsynchCANJaguar.h"

#include "Filters/ExponentialFilter.h"

#define SQRT_2 1.4142

class RobotMainTask : public IterativeRobot
{
public:

	RobotMainTask ();
	~RobotMainTask ();

	void DisabledInit ();

	void TeleopInit ();
	void TeleopPeriodic ();

	void AutonomousInit ();
	void AutonomousPeriodic ();
	void AutonomousRoutine ();

	void TestInit ();

	static int AutonomousTaskStub ( RobotMainTask * ThisObj );

private:

	Task * AutonomousTask;

	Joystick * TranslateStick;
	Joystick * RotateStick;

	ExponentialFilter * JoyFilter;

	CANJaguarServer * JagServer;

	CANJagConfigInfo WheelJagConfig;

	AsynchCANJaguar * WheelFL;
	AsynchCANJaguar * WheelFR;
	AsynchCANJaguar * WheelRL;
	AsynchCANJaguar * WheelRR;

	MecanumDrive * Drive;

};

START_ROBOT_CLASS ( RobotMainTask );

#endif
