#ifndef SHS_2605_VECTOR_H
#define SHS_2605_VECTOR_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include <stdlib.h>
#include <stdint.h>

#define VECTOR_DEFAULT_SIZE 16
#define VECTOR_EXPAND_CAP 2048

template <class T>
class Vector
{
public:
	
	Vector ( int32_t InitSize = - 1 )
	{
		
		if ( InitSize <= 0 )
			InitSize = VECTOR_DEFAULT_SIZE;
		
		Data = (T *) malloc ( sizeof ( T ) * InitSize );
		
		if ( Data == NULL )
			throw "NULL allocation.";
			
		Size = InitSize;
		
		Length = 0;
		
	};
	
	~Vector ()
	{
		
		free ( Data );
		Data = 0;
		
	};
	
	T & operator[] ( unsigned int x )
	{
		
		if ( x >= Length )
			throw "Error: element out of bounds.";
		return Data [ x ];
		
	};

	void FillTo ( uint32_t index, T fillValue )
	{

		expandFor ( index );

		for ( uint32_t i = Length; i < index + 1; i ++ )
			Data [ i ] = fillValue;

	};
	
	void Push ( T x )
	{
		
		expandFor ( Length + 1 );
		
		Data [ Length ] = x;
		Length ++;
		
	};
	
	T Pop ()
	{
		
		Length --;
		return Data [ Length ];
		
	};
	
	T Peek ()
	{
		
		return Data [ Length - 1 ];
		
	};
	
	void Remove ( uint32_t start, uint32_t length )
	{
		
		if ( start >= Length )
			return;
		if ( start + length > Length )
			length = Length - start;
		
		for ( unsigned int i = start; i < Length - length; i ++ )
			Data [ i ] = Data [ i + length ];
		
		Length -= length;
		
	};
	
	int32_t IndexOf ( T x, uint32_t start = 0 )
	{
		
		for (; start < Length; start ++ )
			if ( Data [ start ] == x )
				return start;
		
		return -1;
		
	};
	
	uint32_t GetLength ()
	{
		
		return Length;
		
	}
	
private:
	
	void expandFor ( uint32_t postlength )
	{
		
		if ( postlength <= Size )
			return;
		
		uint32_t newSize;
		
		if ( Size * 2 >= postlength )
			newSize = Size * 2;
		else
			newSize = postlength + Size;
		
		Size = newSize;
		
		Data = (T *) realloc ( (void *) Data, Size * sizeof ( T ) );
		
		if ( Data == NULL )
			throw "NULL allocation.";
		
	};
	
	T * Data;
	uint32_t Size;
	uint32_t Length;
	
};

#endif
