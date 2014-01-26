#include "MecanumDrive.h"
#include <math.h>

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

MecanumDrive :: MecanumDrive ( SpeedController * fl, SpeedController * fr, SpeedController * rl, SpeedController * rr )
{
	
	MFL.Motor = fl;
	MFR.Motor = fr;
	MRL.Motor = rl;
	MRR.Motor = rr;
	
	MFL.Inverted = false;
	MFR.Inverted = false;
	MRL.Inverted = false;
	MRR.Inverted = false;
	
	tx = 0;
	ty = 0;
	tr = 0;
	
	prescale_t = 1;
	prescale_r = 1;
	
	scale = 1;
	
	sineInverted = false;

	enabled = false;
	
};

MecanumDrive :: ~MecanumDrive () {};

void MecanumDrive :: SetMotors ( SpeedController * fl, SpeedController * fr, SpeedController * rl, SpeedController * rr )
{
	
	if ( enabled )
		return;

	MFL.Motor = fl;
	MFR.Motor = fr;
	MRL.Motor = rl;
	MRR.Motor = rr;
	
};

void MecanumDrive :: SetInverted ( bool fl, bool fr, bool rl, bool rr )
{
	
	if ( enabled )
		return;

	MFL.Inverted = fl;
	MFR.Inverted = fr;
	MRL.Inverted = rl;
	MRR.Inverted = rr;
	
};

bool MecanumDrive :: Enable ()
{
	
	if ( MFL.Motor == NULL || MFR.Motor == NULL || MRL.Motor == NULL || MRR.Motor == NULL )
		return false;

	enabled = true;
	return true;
	
};

void MecanumDrive :: Disable ()
{
	
	enabled = false;
	
	tx = 0;
	ty = 0;
	tr = 0;
	
	PushTransform ();
	
};

bool MecanumDrive :: GetEnabled ()
{
	
	return enabled;
	
};

void MecanumDrive :: PushTransform ()
{
	
	double ForceAngle;
	double ForceMagnitude;
	double SinCalc;
	double CosCalc;
	
	if ( ! enabled )
	{
		
		MFL.Motor -> Set ( 0 );
		MFR.Motor -> Set ( 0 );
		MRL.Motor -> Set ( 0 );
		MRR.Motor -> Set ( 0 );
		
		return;
	
	}
		
	ForceMagnitude = sqrt ( tx * tx + ty * ty );
	
	ForceAngle = atan2 ( tx, ty );
	ForceAngle += PI_Div_4;
	
	SinCalc = sin ( ForceAngle ) * ForceMagnitude;
	CosCalc = cos ( ForceAngle ) * ForceMagnitude;

	MFL.Motor -> Set ( ( ( sineInverted ? CosCalc : SinCalc ) + tr ) * ( MFL.Inverted ? - scale : scale ) );
	MFR.Motor -> Set ( ( ( CosCalc ) - tr ) * ( MFR.Inverted ? - scale : scale ) );
	MRL.Motor -> Set ( ( ( sineInverted ? SinCalc : CosCalc ) + tr ) * ( MRL.Inverted ? - scale : scale ) );
	MRR.Motor -> Set ( ( ( SinCalc ) - tr ) * ( MRR.Inverted ? - scale : scale ) );
	
};

void MecanumDrive :: DebugValues ()
{
	
	double ForceAngle;
	double ForceMagnitude;
	double SinCalc;
	double CosCalc;
	
	double flw, frw, rlw, rrw;
	
	ForceMagnitude = sqrt ( tx * tx + ty * ty );
	
	ForceAngle = atan2 ( tx, ty );
	ForceAngle += PI_Div_4;
	
	SinCalc = sin ( ForceAngle ) * ForceMagnitude;
	CosCalc = cos ( ForceAngle ) * ForceMagnitude;
	
	flw = ( ( sineInverted ? CosCalc : SinCalc ) + tr ) * ( MFL.Inverted ? - scale : scale );
	frw = ( ( CosCalc ) - tr ) * ( MFR.Inverted ? - scale : scale );
	rlw = ( ( sineInverted ? SinCalc : CosCalc ) + tr ) * ( MRL.Inverted ? - scale : scale );
	rrw = ( ( SinCalc ) - tr ) * ( MRR.Inverted ? - scale : scale );
	
	printf ( "[ Mecanum Drive Debug ]\n%s\nInput X: %4.4f\nInput Y: %4.4f\nInput R: %4.4f\n[%+4.4f]---[%+4.4f]\n   |         |   \n   |         |   \n   |         |   \n   |         |   \n   |         |   \n   |         |   \n[%+4.4f]---[%+4.4f]\n", ( enabled ? "Enabled." : "Disabled." ), tx, ty, tr, flw, frw, rlw, rrw );
	
};

void MecanumDrive :: SetMotorScale ( double s )
{
	
	scale = s;
	
};

double MecanumDrive :: GetMotorScale ()
{
	
	return scale;
	
};

void MecanumDrive :: SetPreScale ( double translation, double rotation )
{

	prescale_t = translation;
	prescale_r = rotation;
	
};

double MecanumDrive :: GetPreScaleRotation ()
{
	
	return prescale_r;
	
};

double MecanumDrive :: GetPreScaleTranslation ()
{
	
	return prescale_t;
	
};

void MecanumDrive :: SetTranslation ( double x, double y )
{
	
	tx = x * prescale_t;
	ty = y * prescale_t;
	
};

void MecanumDrive :: SetRotation ( double r )
{
	
	tr = r * prescale_r;
	
};

void MecanumDrive :: SetSineInversion ( bool si = false )
{
	
	sineInverted = !si;
	
};
