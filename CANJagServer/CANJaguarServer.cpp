#include "CANJaguarServer.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

/**
* Constructor
*
* @param DoBrownOutCheck Whether or not to peridoically check if any Jaguars on the List have browned-out.
* @param ParseTimeout How many system ticks to lock the message loop while no messages are queued before moving on to Brown-out detection or re-trying.
* @param CommandTimeout How many system ticks to lock a command waiting on the message queue to have space.
*/
CANJaguarServer :: CANJaguarServer ( bool DoBrownOutCheck, double BrownOutCheckInterval, uint32_t ParseTimeout, uint32_t CommandTimeout )
{

	JagCheckInterval = BrownOutCheckInterval;
	CheckJags = DoBrownOutCheck;
	ParseWait = ParseTimeout;
	CommandWait = CommandTimeout;

	Running = false;

	//Jaguar Check Interval Timer
	JagCheckTimer = new Timer ();

	// Server task. _StartServerTask calls this -> RunLoop.
	ServerTask = new Task ( "CANJaguarServer_Task", (FUNCPTR) & _StartServerTask, CANJAGSERVER_PRIORITY, CANJAGSERVER_STACKSIZE );

	// Array for Server Jaguar Information structures.
	Jags = new Vector <ServerCanJagInfo> ();

};

/**
* Destructor
*/
CANJaguarServer :: ~CANJaguarServer ()
{

	Stop ();

	delete ServerTask;
	delete Jags;
	delete JagCheckTimer;

};

/**
* Set the message loop receive timout.
*
* @param ParseTimeout How many system ticks to lock the message loop while no messages are queued before moving on to Brown-out detection or re-trying.
*/
void CANJaguarServer :: SetParseMessageTimeout ( uint32_t ParseTimeout )
{

	ParseWait = ParseTimeout;

};

/**
* Set the command send timeout.
*
* CommandTimeout How many system ticks to lock a command waiting on the message queue to have space.
*/
void CANJaguarServer :: SetCommandMessageTimeout ( uint32_t CommandTimeout )
{

	CommandWait = CommandTimeout;

};

/**
* Enable or disable brown-Out betection.
*
* @param DoBrownOutCheck Whether or not to enable brown-out checking.
*/
void CANJaguarServer :: SetBrownOutCheckEnabled ( bool DoBrownOutCheck )
{

	CheckJags = DoBrownOutCheck;

};

/**
* Set the time interval between server messages to check for brown-outs.
*
* @param Interval Interval time in seconds.
*/
void CANJaguarServer :: SetJagCheckInterval ( double Interval )
{

	JagCheckInterval = Interval;

};

/**
* Start the server. 
*
* You need to call this before anything that passes messages.
*/
bool CANJaguarServer :: Start ()
{

	// Message Send Queue - A cross-thread command queue to direct the server thread.
	MessageSendQueue = msgQCreate ( CANJAGSERVER_MESSAGEQUEUE_LENGTH, sizeof ( CanJagServerMessage * ), MSG_Q_FIFO );

	// Handle error
	if ( MessageSendQueue == NULL )
		return false;

	// Message Response Queue - A cross-thread response queue to get the server thread's responses.
	MessageReceiveQueue = msgQCreate ( 10, sizeof ( CanJagServerMessage * ), MSG_Q_FIFO );

	// Handle Error
	if ( MessageReceiveQueue == NULL )
	{

		msgQDelete ( MessageSendQueue );
		MessageSendQueue = NULL;

	}

	// Response Semaphore - Mutex primitive used to ensure that commands which require an immediate response do not encounter a semi-race condition on the response.
	ResponseSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	// Handle error
	if ( ResponseSemaphore == NULL )
	{

		msgQDelete ( MessageSendQueue );
		msgQDelete ( MessageReceiveQueue );

		MessageSendQueue = NULL;
		MessageReceiveQueue = NULL;

		return false;

	}

	// Start Task, Handle error.
	if ( ! ServerTask -> Start ( (uint32_t) this ) )
	{

		msgQDelete ( MessageSendQueue );
		msgQDelete ( MessageReceiveQueue );
		semDelete ( ResponseSemaphore );

		MessageSendQueue = NULL;
		MessageReceiveQueue = NULL;
		ResponseSemaphore = NULL;

		return false;

	}

	// Success!
	Running = true;
	SendError = false;

	return true;

};

/**
* Stop the server. 
*
* (Invalidates all jaguars. Do not allow a wrapper class to persist beyond this call.)
*/
void CANJaguarServer :: Stop ()
{

	// Make sure no command call is waiting on a response. Protects the commanding thread from a deadlock.
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	ServerTask -> Stop ();

	semGive ( ResponseSemaphore );

	// Are there still messages in the queue?
	bool DeQueueSuccessful = true;

	// Get rid of them.
	while  ( DeQueueSuccessful )
	{

		CANJagServerMessage * Message;

		DeQueueSuccessful = ( msgQReceive ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), 0 ) != ERROR );

		if ( Message != NULL )
		{

			// Correctly de-allocate messages.
			switch ( Message -> Command )
			{

				case SEND_MESSAGE_NOP:
				case SEND_MESSAGE_JAG_DISABLE:
				case SEND_MESSAGE_JAG_ENABLE:
				case SEND_MESSAGE_JAG_REMOVE:

					delete Message;

					break;

				case SEND_MESSAGE_JAG_SET:

					{
						
						SetCANJagMessage * SJMessage = reinterpret_cast <SetCANJagMessage *> ( Message -> Data );

						if ( SJMessage != NULL )
							delete SJMessage;

					}

					delete Message;
					break;

				case SEND_MESSAGE_JAG_ADD:

					{

						AddCANJagMessage * AJMessage = reinterpret_cast <AddCANJagMessage *> ( Message -> Data );

						if ( AJMessage != NULL )
							delete AJMessage;
					
					}
						
					delete Message;
					break;

				case SEND_MESSAGE_JAG_CONFIG:
					
					{

						ConfigCANJagMessage * CJMessage = reinterpret_cast <ConfigCANJagMessage *> ( Message -> Data );

						if ( CJMessage != NULL )
							delete CJMessage;

					}

					delete Message;
					break;

				default:

					delete Message;
					break;

			}


		}

	}

	// Destroy queues and semaphore

	msgQDelete ( MessageSendQueue );
	msgQDelete ( MessageReceiveQueue );
	semDelete ( ResponseSemaphore );

	MessageSendQueue = NULL;
	MessageReceiveQueue = NULL;
	ResponseSemaphore = NULL;

	Running = false;

};

/**
* Determine whether an error occured during the last send.
*/
bool CANJaguarServer :: CheckSendError ()
{

	return SendError;

}

/**
* Clear the SendError flag.
*/
void CANJaguarServer :: ClearSendError ()
{

	SendError = false;

};

/** 
* Disable a jaguar.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: DisableJag ( CAN_ID ID )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_DISABLE;
	Message -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

};

/**
* Enable a jaguar.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: EnableJag ( CAN_ID ID )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_ENABLE;
	Message -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Calls Set() on a Jaguar. 
*
* @param ID Controller ID on the CAN-Bus.
* @param Speed What speed to set the controller to.
* @param SyncGroup The SyncGroup to add this Set () to.
*/
void CANJaguarServer :: SetJag ( CAN_ID ID, double Speed, uint8_t SyncGroup )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();

	SetCANJagMessage * SJMessage = new SetCANJagMessage ();

	SJMessage -> ID = ID;
	SJMessage -> Speed = Speed;
	SJMessage -> SyncGroup = SyncGroup;

	Message -> Command = SEND_MESSAGE_JAG_SET;
	Message -> Data = reinterpret_cast <uint32_t> ( SJMessage );

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Adds a Jaguar to the Server's list.
*
* @param ID Controller ID on the CAN-Bus.
* @param Speed What speed to set the controller to.
* @param SyncGroup The SyncGroup to add this Set () to.
*/
void CANJaguarServer :: AddJag ( CAN_ID ID, CANJagConfigInfo Configuration )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	AddCANJagMessage * AJMessage = new AddCANJagMessage ();

	AJMessage -> ID = ID;
	AJMessage -> Config = Configuration;

	Message -> Command = SEND_MESSAGE_JAG_ADD;
	Message -> Data = reinterpret_cast <uint32_t> ( AJMessage );

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Configures a Jaguar.
*
* @param ID Controller ID on the CAN-Bus.
* @param Configuration Configuration Information.
*/
void CANJaguarServer :: ConfigJag ( CAN_ID ID, CANJagConfigInfo Configuration )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	ConfigCANJagMessage * CJMessage = new AddCANJagMessage ();

	CJMessage -> ID = ID;
	CJMessage -> Config = Configuration;

	Message -> Command = SEND_MESSAGE_JAG_CONFIG;
	Message -> Data = reinterpret_cast <uint32_t> ( CJMessage );

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Gets the speed value of a Jaguar.
*
* @param ID Controller ID on the CAN-Bus.
*/
double CANJaguarServer :: GetJag ( CAN_ID ID )
{

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();
	
	SendMessage -> Command = SEND_MESSAGE_JAG_GET;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
		return 0;

	CANJagServerMessage * ReceiveMessage;

	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
		return 0;

	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	GetCANJagMessage * GMessage = (GetCANJagMessage *) ReceiveMessage -> Data;

	if ( GMessage != NULL )
	{

		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET )
		{

			if ( GMessage -> ID == ID )
			{

				double val = GMessage -> Speed;

				delete GMessage;
				delete ReceiveMessage;

				return val;

			}
			
			delete GMessage;
			delete ReceiveMessage;

			return 0;

		}

	}
	
	delete ReceiveMessage;
	return 0;

};

/**
* Updates the SyncGroup of Jaguars.
*
* @param SyncGroup The mask of sync groups to update pending Set () operations on.
*/
void CANJaguarServer :: UpdateJagSyncGroup ( uint8_t SyncGroup )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP;
	Message -> Data = static_cast <uint32_t> ( SyncGroup );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

};

/**
* Removes a Jaguar from the Server.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: RemoveJag ( CAN_ID ID )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_REMOVE;
	Message -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

void CANJaguarServer :: RunLoop ()
{

	if ( MessageSendQueue == NULL )
		return;

	if ( Jags == NULL )
		return;

	uint32_t JagLoopCounter = 0;
	CANJagServerMessage * Message;

	CAN_ID ID;
	bool Conflict = false;

	double PreJagCheckTime = Timer :: GetPPCTimestamp ();

	while ( true )
	{

		if ( msgQReceive ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), ParseWait ) != ERROR )
		{

			if ( Message != NULL )
			{

				switch ( Message -> Command )
				{

					// No-Op
					case SEND_MESSAGE_NOP:

						delete Message;

						break;

					// Disable Jaguar
					case SEND_MESSAGE_JAG_DISABLE:

						ID = static_cast <CAN_ID> ( Message -> Data );

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								JagInfo.Jag -> Set ( 0 );
								JagInfo.Jag -> DisableControl ();

								break;

							}	

						}

						delete Message;

						break;

					// Enable Jaguar
					case SEND_MESSAGE_JAG_ENABLE:

						ID = static_cast <CAN_ID> ( Message -> Data );

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								JagInfo.Jag -> EnableControl ();

								break;

							}

					}

						delete Message;

						break;

				// Set Jaguar
					case SEND_MESSAGE_JAG_SET:

						SetCANJagMessage * SJMessage = reinterpret_cast <SetCANJagMessage *> ( Message -> Data );

						if ( SJMessage == NULL )
						{

							delete Message;
							return;

						}

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == SJMessage -> ID )
							{

								JagInfo.Jag -> Set ( SJMessage -> Speed, SJMessage -> SyncGroup );

								break;

							}

						}

						delete SJMessage;
						delete Message;

						break;

					// Add Jaguar
					case SEND_MESSAGE_JAG_ADD:

						AddCANJagMessage * AJMessage = reinterpret_cast <AddCANJagMessage *> ( Message -> Data );

						if ( AJMessage == NULL )
						{

							delete Message;
							return;

						}

						Conflict = false;

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							if ( ( * Jags ) [ i ].ID == AJMessage -> ID )
							{

								delete AJMessage;
								delete Message;

								Conflict = true;

								break;

							}

						}

						if ( Conflict )
							break;

						{

							ServerCANJagInfo NewJag;

							NewJag.ID = AJMessage -> ID;
							NewJag.Jag = new CANJaguar ( AJMessage -> ID );
							NewJag.Info = AJMessage -> Config;

							ConfigCANJaguar ( NewJag.Jag, NewJag.Info );

							Jags -> Push ( NewJag );

						}

						delete AJMessage;
						delete Message;

						break;

					// Config Jaguar
					case SEND_MESSAGE_JAG_CONFIG:

						ConfigCANJagMessage * CJMessage = reinterpret_cast <ConfigCANJagMessage *> ( Message -> Data );

						if ( CJMessage == NULL )
						{

							delete Message;
							break;

						}

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == SJMessage -> ID )
							{

								ConfigCANJaguar ( JagInfo.Jag, CJMessage -> Config );

								( * Jags ) [ i ].Info = CJMessage -> Config;

								break;

							}

						}

						delete Message;
						delete CJMessage;

						break;

					// Get Jaguar Speed
					case SEND_MESSAGE_JAG_GET:

						GetCANJagMessage * GJMessage = reinterpret_cast <GetCANJagMessage *> ( Message -> Data );
						CANJagServerMessage * SendMessage;

						if ( GJMessage == NULL )
						{

							SendMessage = new CanJagServerMessage ();
						
							SendMessage -> Command = 0xFFFFFFFF;
							SendMessage -> Data = 0;

							msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

							delete Message;

							break;

						}

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == GJMessage -> ID )
							{

								GetCANJagMessage * JVMessage = new GetCANJagMessage ();

								JVMessage -> ID = JagInfo.ID;
								JVMessage -> Speed = JagInfo.Jag -> Get ();

								SendMessage -> Command = SEND_MESSAGE_JAG_GET;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( JVMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

							}

						}

						delete Message;
						delete GJMessage;

						break;

				// Remove Jaguar
					case SEND_MESSAGE_JAG_REMOVE:

						ID = static_cast <uint32_t> ( Message -> Data );

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								Jags -> Remove ( i, 1 );

								JagInfo.Jag -> DisableControl ();
								delete JagInfo.Jag;

								delete Message;
								break;

							}

						}

						delete Message;

						break;

					case SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP:

						uint8_t Group = static_cast <uint8_t> ( Message -> Data );
						CANJaguar :: UpdateSyncGroup ( Group );

						delete Message;

						break;

					default:

						delete Message;

						break;

				}

			}

		}

		double CheckTime = Timer :: GetPPCTimestamp ();
		double CheckTimeDelta = CheckTime - PreJagCheckTime;

		if ( CheckJags && ( JagCheckInterval <= CheckTimeDelta ) )
		{

			PreJagCheckTime = CheckTime;

			if ( JagLoopCounter >= Jags -> GetLength () )
				JagLoopCounter = 0;

			if ( Jags -> GetLength () != 0 )
				CheckCANJaguar ( ( * Jags ) [ JagLoopCounter ].Jag, ( * Jags ) [ JagLoopCounter ].Info );

			JagLoopCounter ++;

		}

	}

};	

void CANJaguarServer :: _StartServerTask ( CANJaguarServer * Server )
{

	Server -> RunLoop ();

};
