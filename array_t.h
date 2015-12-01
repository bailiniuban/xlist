#ifndef ARRAY_T_H
#define ARRAY_T_H

#include <stddef.h>
#include <memory.h>

#include "xtypes.h"

#define ARRAY_DEFAULT_LENGTH			256

typedef _ARRAY_T ARRAY_T;


struct _ARRAY_T
{
	size_t			MaxLength;			//预定义的最大长度，确保不越界
	size_t			Length;				//占用位置
	OBJECT_T		*Items;
};

//-----------------------------------------------------------------------------
ARRAY_T *array_create(size_t len);
VOID_T array_clearall(ARRAY_T *Me);
VOID_T array_destroy(ARRAY_T *Me);
BOOL_T array_setitem(ARRAY_T *Me, size_t idx, OBJECT_T obj);
OBJECT_T array_getitem(ARRAY_T *Me, size_t idx);

//-----------------------------------------------------------------------------
ARRAY_T *array_create(size_t len)
{
	if(len==0) len=ARRAY_DEFAULT_LENGTH;
	ARRAY_T *r = calloc(1, size_t(ARRAY_T));
	if(r==NULL)	return NULL;
	r->Items = calloc(len, size_t(OBJECT_T));
	if(r->Items==NULL)
	{
		free(r);
		return NULL;
	}
	r->MaxLength = len;
	return r;
}

VOID_T array_clearall(ARRAY_T *Me)
{
	if(Me==NULL) return;
	if(Me->Length==0) return;
	size_t i;
	for(i=0;i<Me->MaxLength;i++)
	{
		if(Me->Items[i]!=NULL)
		{
			free(Me->Items[i]);
			Me->Items[i]=NULL;
		}
	}
	Me->Length = 0;
}

VOID_T array_destroy(ARRAY_T *Me)
{	//Need clearall at first.
	if(Me==NULL) return;
	array_clearall(Me);
	free(Me);
}

BOOL_T array_setitem(ARRAY_T *Me, size_t idx, OBJECT_T obj)
{
	if(Me==NULL) return 0;
	
	if(idx>=Me->MaxLength) return 0;
	
	if(obj==NULL)
	{
		if(Me->Items[idx]!=NULL)
		{
			free(Me->Items[idx]);
			Me->Items[idx]=NULL;
			Me->Length--;
		}
	}
	else
	{	
		if(Me->Items[idx]!=NULL)
		{
			free(Me->Items[idx]);
			Me->Items[idx]=obj;
		}
		else
		{
			Me->Items[idx]=obj;
			Me->Length++;
		}
	}
	return 1;
}

OBJECT_T array_getitem(ARRAY_T *Me, size_t idx)
{
	if(Me==NULL) return NULL;
	
	if(idx>=Me->MaxLength) return NULL; 

	return Me->Items[idx];
}

#endif
