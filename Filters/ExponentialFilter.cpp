#include "ExponentialFilter.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

ExponentialFilter :: ExponentialFilter ( double expt )
{

	SetExponent ( expt );

};

ExponentialFilter :: ~ExponentialFilter ()
{
};

void ExponentialFilter :: SetExponent ( double expt )
{

	exponent = expt;

};

double ExponentialFilter :: Compute ( double value )
{

	bool sign = ( value < 0 );
	return fabs ( pow ( value, exponent ) ) * ( sign ? - 1.0f : 1.0f );

};
