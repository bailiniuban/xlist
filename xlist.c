#include "xlist.h"

//-----------------------------------------------------------------------------
//xlist functions' define:
//xlist_create: Create XList
static XLIST_T *xlist_create(void)
{
	XLIST_T *r = (XLIST_T *)calloc(1, sizeof(XLIST_T));
	if(xlist_newpage(r)!=NULL)
		r->Loaded = 1;
	return r;
}

static XLIST_T *xlist_create_withpages(LONG_T n)
{//
	XLIST_T *r = (XLIST_T *)calloc(1, sizeof(XLIST_T));
	XLIST_PAGE_T **pgs = (XLIST_PAGE_T **)calloc(n, sizeof(XLIST_PAGE_T *));
	if(pgs!=NULL)
	{
		r->Pages = pgs;
		r->PageCount = n;
		r->Loaded = 0;
	}
	return r;
}

static VOID_T xlist_destroy(XLIST_T **Me)
{
	if(Me==NULL) return;
	if(*Me==NULL) return;
	XLIST_T *xl = *Me;
	LONG_T i;
	for(i=0;i<xl->PageCount;i++)
	{
		xlist_page_destroy(xl->Pages[i]);
	}
	free(xl->Pages);
	free(*Me);
	*Me = NULL;
	return;
}

static BOOL_T xlist_alwaystrue(XLIST_PAGE_T *Me)
{
	return 1;
}

static BOOL_T xlist_alwaysfalse(XLIST_PAGE_T *Me)
{
	return 0;
}

static XLIST_PAGE_T *xlist_newpage(XLIST_T *Me)
{
	if(Me==NULL) return NULL;
	
	XLIST_PAGE_T **pgs = (XLIST_PAGE_T **)calloc(Me->PageCount+1, sizeof(XLIST_PAGE_T *));
	if(pgs==NULL) return NULL;
	pgs[Me->PageCount] = xlist_page_create();
	if(pgs[Me->PageCount]==NULL) return NULL;
	pgs[Me->PageCount]->Prev = pgs[Me->PageCount];
	pgs[Me->PageCount]->Next = pgs[Me->PageCount];
	
	if(Me->Pages==NULL)
	{
		Me->Pages = pgs;
	}
	else
	{
		memcpy(pgs, Me->Pages, sizeof(XLIST_PAGE_T *)*Me->PageCount);
		free(Me->Pages);
		Me->Pages = pgs;
	}
	
	Me->PageCount++;
	return pgs[Me->PageCount-1];
}

static XLIST_PAGE_T *xlist_newchild(XLIST_T *Me, PAGECHECK fcheck)
{
	if(Me==NULL) return NULL;
	XLIST_PAGE_T *pg = NULL;
	LONG_T i;
	if(fcheck == NULL)
	{ 
		if(Me->PageCount==0 || Me->Pages==NULL)
		{
			pg = xlist_newpage(Me);
		}
		else
		{
			pg = xlist_page_appendchild(Me->Pages[Me->PageCount-1]);
		}
	}
	else
	{
		for(i=0;i<Me->PageCount;i++)
		{
			if(fcheck(Me->Pages[i]))
			{
				pg = xlist_page_appendchild(Me->Pages[i]);
				break;
			}
		}
	}
	return pg;
}

//xlist_add: Add a object into the xlist
static VOID_T *xlist_add(XLIST_T *Me, VOID_T *obj)
{
	if(Me==NULL) return NULL;
	
	XLIST_PAGE_T *pg = NULL;
	
	if(xlist_isfull(Me)||(Me->PageCount==0 && Me->Pages==NULL))
	{
		pg = xlist_newpage(Me);
	}
	else
	{	//找到第一个有空位置的Page
		pg = xlist_findipage(Me);
	}
	
	VOID_T *r = xlist_page_add(pg, obj);
	if(r!=NULL) Me->Count ++;
	return r;
}

static XLIST_PAGE_T *xlist_broadcast(XLIST_PAGE_T *Me, PAGECHECK func)
{
	if(Me==NULL) return NULL;
	
	XLIST_PAGE_T *r, *pg, *cur;
	pg = Me;
	r = NULL;
	do
	{
		cur = pg;
		do
		{
			if(func(cur)) return pg;
			cur = cur->Child;
			r = xlist_broadcast(cur, func);
			if(r!=NULL) return r;
		} while(cur!=NULL);
		pg = pg->Next;
	} while(pg!=Me);
	return NULL;
}

static LONG_T xlist_foreach(XLIST_PAGE_T *Me, PAGECHECK fcheck, PAGEEVENT fexec)
{
	if(Me==NULL) return 0;
	if(fcheck==NULL||fexec==NULL) return 0;
	
	LONG_T r = 0;
	XLIST_PAGE_T *pg, *cur;
	pg=Me;
	do
	{
		printf("hi\n");
		cur = pg;
		do
		{
			if(fcheck(cur))
				r += fexec(cur);
			cur = cur->Child;
			r += xlist_foreach(cur, fcheck, fexec);
		} while(cur!=NULL);
		pg = pg->Next;
	} while(pg!=Me);
	return r;
}

static LONG_T xlist_foreachparent(XLIST_PAGE_T *Me, PAGECHECK fcheck, PAGEEVENT fexec)
{
	if(Me==NULL) return 0;
	if(fcheck==NULL||fexec==NULL) return 0;
	
	LONG_T r = 0;
	XLIST_PAGE_T *cur = Me;
	do
	{
		if(fcheck(cur))
			r += fexec(cur);
		cur = cur->Parent;
	} while(cur!=NULL);
	return r;
}

static LONG_T xlist_inform(XLIST_PAGE_T *Me,LONG_T features, PAGECHECK fcheck, PAGEEVENT fexec)
{//返回值：事件完成后影响到的PAGE个数
	//features:0,仅自己，>0自己这层及子孙；<0：父所有的直系父亲
	if(Me==NULL) return 0;
	if(fcheck==NULL || fexec==NULL) return 0;
	if(features==0)
	{
		if(fcheck(Me)) return fexec(Me);
	}
	else
	{
		if(features>0)
		{
			return xlist_foreach(Me, fcheck, fexec);
		}
		else
		{
			return xlist_foreachparent(Me, fcheck, fexec);
		}
	}
	return 0;
}

static XLIST_PAGE_T *xlist_findipage(XLIST_T *Me)
{
	if(Me==NULL) return NULL;
	if(Me->Pages==NULL) return NULL;
	
	XLIST_PAGE_T *pg = NULL;
	LONG_T i;
	for(i=0;i<Me->PageCount;i++)
	{
		pg=xlist_broadcast(Me->Pages[i],xlist_page_hasemptyplace);
		if(pg!=NULL)
		{
			return pg;
		} 
	}
	return NULL;
}

static BOOL_T xlist_isfull(XLIST_T *Me)
{
	if(Me==NULL) return 0;
	if(Me->Count==0) return 0;
	return (Me->Count & (XLISTPAGESIZE-1)) ? 0 : 1;
}

static BOOL_T xlist_isempty(XLIST_T *Me)
{
	if(Me==NULL) return 1;
	return (Me->Count==0) ? 1 : 0;
}
//-----------------------------------------------------------------------------
//xlist_page functions' define:
static XLIST_PAGE_T *xlist_page_create(void)
{
	XLIST_PAGE_T *r = (XLIST_PAGE_T *)calloc(1, sizeof(XLIST_PAGE_T));
	if(r!=NULL)
	{
		r->Items = (VOID_T **)calloc(XLISTPAGESIZE, sizeof(VOID_T *));
		if(r->Items!=NULL)
			r->Loaded = 1;
	}
	
	return r;
}

static LONG_T xlist_page_event_onadd(XLIST_PAGE_T *Me)
{
	Me->Count ++;
	return 1;
}

static VOID_T *xlist_page_add(XLIST_PAGE_T *Me, VOID_T *obj)
{
	if(Me==NULL) return NULL;
	if(xlist_page_isfull(Me))
	{
		//新增一个page.
		return NULL;
	}
	if(Me->ItemCount==XLISTPAGESIZE)
	{
		return NULL;
	}
	
	VOID_T *r=NULL;
	int i;
	for(i=0;i<XLISTPAGESIZE;i++)
	{
		if(Me->Items[i]==NULL)
		{
			Me->Items[i]=obj;
			Me->ItemCount ++;
			
			//Parents' Count++
			xlist_inform(Me, -1, xlist_alwaystrue, xlist_page_event_onadd);
			
			r = obj;
			break;
		}
	}
	return r;
}

static XLIST_PAGE_T *xlist_page_newbrother(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return NULL;
	XLIST_PAGE_T *r = xlist_page_create();
	if(r==NULL) return NULL;
	r->Prev = Me;
	r->Next = Me->Next;
	Me->Next = r;
	r->Next->Prev = r;
	return r;
}

static XLIST_PAGE_T *xlist_page_newchild(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return NULL;
	XLIST_PAGE_T *r = xlist_page_create();
	if(r==NULL) return NULL;
	r->Prev = r;
	r->Next = r;
	r->Child = Me->Child;
	Me->Child = r;
	return r;
}

static XLIST_PAGE_T *xlist_page_appendchild(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return NULL;
	XLIST_PAGE_T *cur;
	cur = Me;
	while(cur->Child!=NULL)
	{
		cur = cur->Child;
	}
	
	return xlist_page_newchild(cur);
}

static BOOL_T xlist_page_isfull(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return 0;
	if(Me->Count==0) return 0;
	return (Me->Count & (XLISTPAGESIZE - 1)) ? 0 : 1;	//有子page
}

static BOOL_T xlist_page_hasemptyplace(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return 0;
	return (Me->ItemCount==XLISTPAGESIZE) ? 0 : 1;
}

static BOOL_T xlist_page_isempty(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return 1;
	return (Me->Count==0) ? 1 : 0;
}

static VOID_T xlist_page_destroy(XLIST_PAGE_T *Me)
{
	if(Me==NULL) return;
	//删除子
	xlist_page_destroy(Me->Child);
	//删除兄弟
	XLIST_PAGE_T *pg = Me->Next;
	XLIST_PAGE_T *cur;
	Me->Next = NULL;
	
	if(pg==Me)
	{
		free(pg->FileName)
		free(pg);
	}
	else
	{
		while(pg!=NULL)
		{
			cur = pg->Next;
			xlist_page_destroy(pg->Child);
			free(pg->FileName);
			free(pg);
			pg = cur;
		}
	}
}
