#ifndef SHS_2605_CANJAGUAR_SERVER_H
#define SHS_2605_CANJAGUAR_SERVER_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"
#include "Util/JaguarUtils.h"
#include "Util/Vector.h"

#define CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME 5000

typedef int32_t CAN_ID;

class CANJaguarServer
{
public:

	CANJaguarServer ();
	~CANJaguarServer ();

	bool Start ();
	void Stop ();

	void IsRunning ();

	void AddJag ( CAN_ID ID, CANJagConfigInfo Info );
	void RemoveJag ( CAN_ID ID );

	void DisableJag ( CAN_ID ID );
	void EnableJag ( CAN_ID ID );

	void ConfigJag ( CAN_ID, CANJagConfigInfo );

	void SetJag ( CAN_ID ID, double Speed, uint8_t SyncGroup = 0 );
	double GetJag ( CAN_ID ID );

	bool CheckSendError ();
	void ClearSendError ();

	void UpdateJagSyncGroup ( uint8_t SyncGroup );

	void RunLoop ();

	enum CANJagServerSendMessageType
	{

		SEND_MESSAGE_NOP = 0,
		SEND_MESSAGE_JAG_DISABLE,
		SEND_MESSAGE_JAG_ENABLE,
		SEND_MESSAGE_JAG_GET,
		SEND_MESSAGE_JAG_SET,
		SEND_MESSAGE_JAG_ADD,
		SEND_MESSAGE_JAG_REMOVE,
		SEND_MESSAGE_JAG_CONFIG,
		SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP,

	};

	typedef struct CANJagServerMessage
	{

		uint32_t Command;
		void * Data;

	} CanJagServerMessage;

	typedef struct ServerCANJagInfo
	{

		CAN_ID ID;
		CANJaguar * Jag;
		CANJagConfigInfo Info;

	} ServerCanJagInfo;

	typedef struct SetCANJagMessage
	{

		CAN_ID ID;
		double Speed;
		uint8_t SyncGroup;

	} SetJagMessage;

	typedef struct ConfigCANJagMessage
	{

		CAN_ID ID;
		CANJagConfigInfo Config;

	} ConfigCANJagMessage;

	typedef ConfigCANJagMessage AddCANJagMessage;

	typedef struct GetCANJagMessage
	{

		CAN_ID ID;
		double Speed;

	};

private:

	bool Running;
	bool SendError;

	Task * ServerTask;

	MSG_Q_ID MessageSendQueue;
	MSG_Q_ID MessageReceiveQueue;
	SEM_ID ResponseSemaphore;

	uint32_t RunLoopCounter;

	Vector <ServerCanJagInfo> * Jags;

	static void _StartServerTask ( CANJaguarServer * Server );

};

#endif
