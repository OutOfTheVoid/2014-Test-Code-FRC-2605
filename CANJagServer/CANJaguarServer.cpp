#include "CANJaguarServer.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

/**
* Constructor
*/
CANJaguarServer :: CANJaguarServer ()
{

	Running = false;

	// Server task. _StartServerTask calls this -> RunLoop.
	ServerTask = new Task ( "CANJaguarServer_Task", (FUNCPTR) & _StartServerTask );

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

};

/**
* Start the server. 
*
* You need to call this before anything that passes messages.
*/
bool CANJaguarServer :: Start ()
{

	// Message Send Queue - A cross-thread command queue to direct the server thread.
	MessageSendQueue = msgQCreate ( 256, sizeof ( CanJagServerMessage * ), MSG_Q_FIFO );

	// Handle error
	if ( MessageSendQueue == NULL )
		return false;

	// Message Response Queue - A cross-thread response queue to get the server thread's responses.
	MessageReceiveQueue = msgQCreate ( 256, sizeof ( CanJagServerMessage * ), MSG_Q_FIFO );

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

	// Make sure no command call is waiting on a response. Protects the caller from a deadlock.
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
						
						SetCANJagMessage * SJMessage = (SetCANJagMessage *) Message -> Data;

						if ( SJMessage != NULL )
							delete SJMessage;

					}

					delete Message;
					break;

				case SEND_MESSAGE_JAG_ADD:

					{

						AddCANJagMessage * AJMessage = (AddCANJagMessage *) Message -> Data;

						if ( AJMessage != NULL )
							delete AJMessage;
					
					}
						
					delete Message;
					break;

				case SEND_MESSAGE_JAG_CONFIG:
					
					{

						ConfigCANJagMessage * CJMessage = (ConfigCANJagMessage *) Message -> Data;

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
	Message -> Data = (void *) ID;

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
	Message -> Data = (void *) ID;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME, MSG_PRI_NORMAL ) == ERROR );

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
	Message -> Data = (void *) SJMessage;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME, MSG_PRI_NORMAL ) == ERROR );

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
	Message -> Data = (void *) AJMessage;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME, MSG_PRI_NORMAL ) == ERROR );

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
	Message -> Data = (void *) CJMessage;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME, MSG_PRI_NORMAL ) == ERROR );

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
	SendMessage -> Data = (void *) ID;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & SendMessage, sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
		return 0;

	CANJagServerMessage * ReceiveMessage;

	if ( msgQReceive ( MessageReceiveQueue, (char *) & ReceiveMessage, sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
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
	Message -> Data = (void *) (uint32_t) SyncGroup;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

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
	Message -> Data = (void *) ID;

	SendError = ( msgQSend ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME, MSG_PRI_NORMAL ) == ERROR );

};

void CANJaguarServer :: RunLoop ()
{

	if ( MessageSendQueue == NULL )
		return;

	if ( Jags == NULL )
		return;

	uint32_t RunLoopCounter = 0;
	uint32_t JagLoopCounter = 0;
	CANJagServerMessage * Message; 

	while ( true )
	{
		
		RunLoopCounter ++;

		if ( msgQReceive ( MessageSendQueue, (char *) & Message, sizeof ( CANJagServerMessage * ), CANJAGSERVER_NON_IMPERATIVE_WAIT_TIME ) != ERROR )
		{

			CAN_ID ID;
			bool Conflict = false;

			switch ( Message -> Command )
			{

				// No-Op
				case SEND_MESSAGE_NOP:

					delete Message;

					return;

				// Disable Jaguar
				case SEND_MESSAGE_JAG_DISABLE:

					ID = (uint32_t) Message -> Data;

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

				// Enable Jaguar
				case SEND_MESSAGE_JAG_ENABLE:

					ID = (uint32_t) Message -> Data;

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

				// Set Jaguar
				case SEND_MESSAGE_JAG_SET:

					SetCANJagMessage * SJMessage = (SetCANJagMessage *) Message -> Data;

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

					AddCANJagMessage * AJMessage = (AddCANJagMessage *) Message -> Data;

					if ( AJMessage == NULL )
					{

						delete Message;
						return;

					}

					Conflict = false;

					for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
					{

						if ( ( * Jags ) [ i ].ID == SJMessage -> ID )
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

					ConfigCANJagMessage * CJMessage = (ConfigCANJagMessage *) Message -> Data;

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

					GetCANJagMessage * GJMessage = (GetCANJagMessage *) Message -> Data;
					CANJagServerMessage * SendMessage;

					if ( GJMessage == NULL )
					{

						SendMessage = new CanJagServerMessage ();
						
						SendMessage -> Command = 0xFFFFFFFF;
						SendMessage -> Data = NULL;

						msgQSend ( MessageReceiveQueue, (char *) & SendMessage, sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

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
							SendMessage -> Data = (void *) JVMessage;

							msgQSend ( MessageReceiveQueue, (char *) & SendMessage, sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

						}

					}

					delete Message;
					delete GJMessage;

					break;

				// Remove Jaguar
				case SEND_MESSAGE_JAG_REMOVE:

					ID = (uint32_t) Message -> Data;

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

					uint8_t Group = (uint8_t) (uint32_t) Message -> Data;
					CANJaguar :: UpdateSyncGroup ( Group );

					delete Message;

					break;

				default:

					delete Message;

					break;

			}

		}

		if ( RunLoopCounter & 0xF == 0 )
		{

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
