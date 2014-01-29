#include "JaguarUtils.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

CANJagConfigInfo :: CANJagConfigInfo ()
{
	
	Mode = CANJaguar :: kVoltage;
	NeutralAction = CANJaguar :: kNeutralMode_Coast;
	
	MaxVoltage = 12;
	Safety = false;
	
};

void ConfigCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf )
{
	
	Jag -> DisableControl ();
	
	switch ( Conf.Mode )
	{
		
	case CANJaguar :: kPercentVbus:
		Jag -> ChangeControlMode ( CANJaguar :: kPercentVbus );
		break;
		
	case CANJaguar :: kVoltage:
		Jag -> ChangeControlMode ( CANJaguar :: kVoltage );
		break;
		
	case CANJaguar :: kCurrent:
		Jag -> ChangeControlMode ( CANJaguar :: kCurrent );
		break;
		
	case CANJaguar :: kPosition:
		Jag -> ChangeControlMode ( CANJaguar :: kPosition );
		Jag -> SetPositionReference ( Conf.PosRef );
		if ( Conf.PosRef == 0 )
			Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
		else
			Jag -> ConfigPotentiometerTurns ( Conf.PotentiometerTurnsPerRev );
		Jag -> SetPID ( Conf.P, Conf.I, Conf.D );
		break;
		
	case CANJaguar :: kSpeed:
		Jag -> ChangeControlMode ( CANJaguar :: kSpeed );
		Jag -> SetSpeedReference ( Conf.SpeedRef );
		Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
		Jag -> SetPID ( Conf.P, Conf.I, Conf.D );
		break;
		
	default:
		Jag -> ChangeControlMode ( CANJaguar :: kVoltage );
		break;
		
	}
	
	Jag -> ConfigMaxOutputVoltage ( Conf.MaxVoltage );
	Jag -> ConfigNeutralMode ( Conf.NeutralAction );
	Jag -> SetSafetyEnabled ( Conf.Safety );
	Jag -> EnableControl ();
	
};

void CheckCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf )
{
	
	if ( Jag -> GetControlMode () != Conf.Mode )
	{

		printf ( "CAN_JAGUAR RECONFIGURATION\n" );
		ConfigCANJaguar ( Jag, Conf );


	}
	
};
