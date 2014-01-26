#ifndef SHS_2605_MECANUM_DRIVE_H
#define SHS_2605_MECANUM_DRIVE_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"
#include <math.h>

#define PI_Div_4 0.78539816339

typedef struct
{
	
	SpeedController * Motor;
	bool Inverted;
	
} MecMotor;

class MecanumDrive
{
public:
	
	//Constructor. Takes four speed controllers
	MecanumDrive ( SpeedController * fl, SpeedController * fr, SpeedController * rl, SpeedController * rr );
	~MecanumDrive ();
	
	//Change motors
	void SetMotors ( SpeedController * fl, SpeedController * fr, SpeedController * rl, SpeedController * rr );
	
	//Invert motors
	void SetInverted ( bool fl, bool fr, bool rl, bool rr );
	
	//Enable or disable drive
	bool Enable ();
	void Disable ();
	
	//Get the enabled state
	bool GetEnabled ();
	
	//Set/Get motor output multiplier
	void SetMotorScale ( double s );
	double GetMotorScale ();
	
	//Get/Set prescale constants
	void SetPreScale ( double translation, double rotation );
	double GetPreScaleTranslation ();
	double GetPreScaleRotation ();
	
	//Set speeds
	void SetTranslation ( double x, double y );
	void SetRotation ( double r );
	
	//Enable/Disable sin/cos switch. Can fix problem if inversion doesn't fix problems.
	void SetSineInversion ( bool si );

	//Print out debug using printf. Shows IO and enabled status.
	void DebugValues ();
	
	//Push transform to controllers
	void PushTransform ();
	
private:
	
	MecMotor MFL, MFR, MRL, MRR;
	double tx, ty, tr, scale, prescale_r, prescale_t;
	bool enabled, sineInverted;
	
};

#endif
