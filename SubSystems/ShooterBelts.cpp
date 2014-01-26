#include "ShooterBelts.h"

/**
* Constructor
*
* @param RollerBL Left belt roller speed controller.
* @param RollerBR Right belt roller speed controller.
* @param ArmSL Left arm servo speed controller.
* @param ArmSR Right arm servo speed controller.
* @param AngleL Left arm angle measurement sensor.
* @param AngleR Right arm angle measurement sensor.
*/
ShooterBelts :: ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR, SpeedController * ArmSL, SpeedController * ArmSR, AnalogSensor * AngleL, AnalogSensor * AngleR )
{

	RollerL.Motor = RollerBL;
	RollerR.Motor = RollerBR;
	RollerL.Inverted = false;
	RollerR.Inverted = false;

	ArmL.Motor = ArmSL;
	ArmR.Motor = ArmSL;
	ArmL.Inverted = false;
	ArmR.Inverted = false;

	Enabled = false;

};

/**
* Destructor.
*/
ShooterBelts :: ~ShooterBelts ()
{



};

/**
* Set the motor controllers for the shooiter belt system.
*
* @param BeltL The left belt speed controller. 
* @param BeltR The right belt speed controller.
* @param ArmL The left arm speed controller.
* @param ArmR The right arm speed controller.
*/
void ShooterBelts :: SetMotors ( SpeedController * BeltBL, SpeedController * BeltBR, SpeedController * ArmSL, SpeedController * ArmSR )
{



};

void ShooterBelts :: SetSensors ( AnalogSensor * AngleL, AnalogSensor * AngleR )
{



};

void ShooterBelts :: SetInverted ( bool BL, bool BR, bool SL, bool SR );

void ShooterBelts :: CalibrateArmLeftOut ();
void ShooterBelts :: CalibrateArmLeftIn ();

void ShooterBelts :: CalibrateArmRightOut ();
void ShooterBelts :: CalibrateArmRightIn ();

bool ShooterBelts :: Enable ();
void ShooterBelts :: Disable ();
	
bool ShooterBelts :: GetEnabled ();