#ifndef SHS_2605_ASYNCH_CANJAGUAR_H
#define SHS_2605_ASYNCH_CANJAGUAR_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"

#include "CANJaguarServer.h"
#include "Util/JaguarUtils.h"

class AsynchCANJaguar : public SpeedController
{
public:

	AsynchCANJaguar ( CANJaguarServer * Server, CAN_ID ID, CANJagConfigInfo Config );
	~AsynchCANJaguar ();

	void Disable ();
	void Enable ();

	void Set ( float Speed, uint8_t SyncGroup = 0 );
	float Get ();

	void Configure ( CANJagConfigInfo Config );

	void PIDWrite ( float Speed );

	static void UpdateSyncGroup ( CANJaguarServer * Server, uint8_t SyncGroup );

private:

	CANJaguarServer * Server;
	CAN_ID ID;

};

#endif
