#include "RobotMainTask.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

// TODO: Add some sort of synchonization for MecanumDrive stuff, sort out how to do vision.

RobotMainTask :: RobotMainTask ():
	WheelJagConfig ()
{

	JagServer = new CANJaguarServer ();
	JagServer -> Start ();

	WheelJagConfig.Mode = CANJaguar :: kSpeed;
	WheelJagConfig.P = 0.1200;
	WheelJagConfig.I = 0.0090;
	WheelJagConfig.D = 0.0015;
	WheelJagConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	WheelJagConfig.EncoderLinesPerRev = 360;
	WheelJagConfig.NeutralAction = CANJaguar :: kNeutralMode_Brake;

	WheelFL = new AsynchCANJaguar ( JagServer, 4, WheelJagConfig );
	WheelFR = new AsynchCANJaguar ( JagServer, 3, WheelJagConfig );
	WheelRL = new AsynchCANJaguar ( JagServer, 6, WheelJagConfig );
	WheelRR = new AsynchCANJaguar ( JagServer, 7, WheelJagConfig );

	Drive = new MecanumDrive ( WheelFL, WheelFR, WheelRL, WheelRR );

	Drive -> SetMotorScale ( 500 );
	Drive -> SetPreScale ( 1, 1 );

	Drive -> SetInverted ( false, true, false, true );
	Drive -> SetSineInversion ( false );

	Drive -> Disable ();

	WheelFL -> Enable ();
	WheelFR -> Enable ();
	WheelRL -> Enable ();
	WheelRR -> Enable ();

	AutonomousTask = new Task ( "SHS_Autononmous", (FUNCPTR) & AutonomousTaskStub );

	TranslateStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );

	JoyFilter = new ExponentialFilter ( 2.0 );

};

RobotMainTask :: ~RobotMainTask ()
{

	delete WheelFL;
	delete WheelFR;
	delete WheelRL;
	delete WheelRR;

	delete JagServer;

};

// --------------------------------------------- Disabled --------------------------------------------- //

void RobotMainTask :: DisabledInit ()
{

	printf ( "Operating Mode: DISABLED\n" );

	AutonomousTask -> Stop ();
	Drive -> Disable ();

};

// ---------------------------------------------- Teleop ---------------------------------------------- //

void RobotMainTask :: TeleopInit ()
{

	printf ( "Operating Mode: TELEOP\n" );

	Drive -> Enable ();

};

void RobotMainTask :: TeleopPeriodic ()
{

	Drive -> SetTranslation ( JoyFilter -> Compute ( TranslateStick -> GetX () ), - JoyFilter -> Compute ( TranslateStick -> GetY () ) );
	Drive -> SetRotation ( JoyFilter -> Compute ( RotateStick -> GetX () ) );
	Drive -> PushTransform ();

};

// -------------------------------------------- Autonomous -------------------------------------------- //

void RobotMainTask :: AutonomousInit ()
{

	printf ( "Operating Mode: AUTONOMOUS\n" );

	Drive -> Enable ();
	AutonomousTask -> Start ( (uint32_t) this );

};

void RobotMainTask :: AutonomousPeriodic ()
{
};

void RobotMainTask :: AutonomousRoutine ()
{

	JagServer -> ClearSendError ();

	Wait ( 0.01 );

	Drive -> SetTranslation ( 0, 0.5 );
	Drive -> PushTransform ();

	if ( JagServer -> CheckSendError () )
		printf ( "Jaguar Send Error!!\n");

	Wait ( 1.0 );

	Drive -> SetTranslation ( 0, 0 );
	Drive -> SetRotation ( 0.5 );
	Drive -> PushTransform ();

	Wait ( 0.5 );

	Drive -> SetTranslation ( 0, 0.5 );
	Drive -> SetRotation ( 0 );
	Drive -> PushTransform ();

	Wait ( 1.0 );

	Drive -> SetTranslation ( 0, 0 );
	Drive -> SetRotation ( 0 );
	Drive -> PushTransform ();

};

int RobotMainTask :: AutonomousTaskStub ( RobotMainTask * MainObj )
{

	MainObj -> AutonomousRoutine ();

	return 0;

};
