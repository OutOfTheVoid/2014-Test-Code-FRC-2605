#ifndef SHS_2605_SHOOOTERBELTS_H
#define SHS_2605_SHOOOTERBELTS_H

#include "WPILib.h"
#include "Sensors/AnalogSensor.h"

typedef struct
{
	
	SpeedController * Motor;
	bool Inverted;
	
} BeltMotor;

class ShooterBelts
{
public:

	ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR, SpeedController * ArmSL, SpeedController * ArmSR, AnalogSensor * AngleL, AnalogSensor * AngleR );
	~ShooterBelts ();

	void SetMotors ( SpeedController * BeltBL, SpeedController * BeltBR, SpeedController * ArmSL, SpeedController * ArmRS );
	void SetSensors ( AnalogSensor * AngleL, AnalogSensor * AngleR );

	void SetInverted ( bool BL, bool BR, bool SL, bool SR );

	void CalibrateArmLeftOut ();
	void CalibrateArmLeftIn ();

	void CalibrateArmRightOut ();
	void CalibrateArmRightIn ();

	bool Enable ();
	void Disable ();
	
	bool GetEnabled ();

private:

	bool Enabled;

	double S, AL, AR, AS;

	BeltMotor RollerL, RollerR, ArmL, ArmR;

	AnalogSensor * PotL, * PotR;

};

#endif
