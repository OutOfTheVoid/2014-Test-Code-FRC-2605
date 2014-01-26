#include "AsynchCANJaguar.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

AsynchCANJaguar :: AsynchCANJaguar ( CANJaguarServer * Server, CAN_ID ID, CANJagConfigInfo Config )
{

	this -> Server = Server;
	this -> ID = ID;

	Server -> AddJag ( ID, Config );

};

AsynchCANJaguar :: ~AsynchCANJaguar ()
{
	
	Server -> RemoveJag ( ID );

}

void AsynchCANJaguar :: Enable ()
{

	Server -> EnableJag ( ID );

};

void AsynchCANJaguar :: Disable ()
{

	Server -> DisableJag ( ID );

};

void AsynchCANJaguar :: Set ( float Speed, uint8_t SyncGroup )
{

	Server -> SetJag ( ID, Speed );

};

float AsynchCANJaguar :: Get ()
{

	return Server -> GetJag ( ID );

};

void AsynchCANJaguar :: Configure ( CANJagConfigInfo Config )
{

	Server -> ConfigJag ( ID, Config );

};

void AsynchCANJaguar :: PIDWrite ( float Speed )
{

	Set ( Speed );

};

void AsynchCANJaguar :: UpdateSyncGroup ( CANJaguarServer * Server, uint8_t SyncGroup )
{

	Server -> UpdateJagSyncGroup ( SyncGroup );

};
