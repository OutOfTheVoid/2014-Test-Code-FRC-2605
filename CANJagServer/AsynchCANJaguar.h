#ifndef SHS_2605_ASYNCH_CANJAGUAR_H
#define SHS_2605_ASYNCH_CANJAGUAR_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"

#include "CANJaguarServer.h"
#include "src/Util/JaguarUtils.h"

class AsynchCANJaguar : public SpeedController, public ErrorBase
{
public:

	AsynchCANJaguar ( CANJaguarServer * Server, CAN_ID ID, CANJagConfigInfo Config );
	~AsynchCANJaguar ();

	virtual void Disable ();
	virtual void Enable ( double EncoderInitialPosition = 0.0 );

	virtual void Set ( float Speed, uint8_t SyncGroup = 0 );
	virtual float Get ();

	float GetBusVoltage ();
	float GetOutputVoltage ();
	float GetOutputCurrent ();

	void Configure ( CANJagConfigInfo Config );

	virtual void PIDWrite ( float Speed );

	static void UpdateSyncGroup ( CANJaguarServer * Server, uint8_t SyncGroup );

private:

	CANJaguarServer * Server;
	CAN_ID ID;

	CANJaguar :: ControlMode LastControlMode;

};

#endif
