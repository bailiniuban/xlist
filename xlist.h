#ifndef XLIST_H
#define XLIST_H

#include <stdio.h>
#include <memory.h>

#define XLISTPAGESIZE			1024		//Don't modify, because some bit-operations base on 1024. eg.<<10,>>10

typedef int INT_T;
typedef unsigned UINT_T;
typedef long LONG_T;
typedef double DOUBLE_T;
typedef char CHAR_T;
typedef void VOID_T;
typedef void* OBJECT_T;
typedef int BOOL_T;

struct _XLIST_V1_T;
struct _XLIST_PAGE_V1_T;

typedef struct _XLIST_V1_T XLIST_T;
typedef struct _XLIST_PAGE_V1_T XLIST_PAGE_T;

typedef BOOL_T (*PAGECHECK)(XLIST_PAGE_T *);
typedef LONG_T (*PAGEEVENT)(XLIST_PAGE_T *);

//Structs
struct _XLIST_V1_T
{
	XLIST_PAGE_T		**Pages;
	LONG_T				PageCount;
	LONG_T				Count;
	BOOL_T				Locked;
	BOOL_T				Loaded;
	char				*FileName;	//整体存储在文件
	LONG_T				Sorted;		//-1:大小；0:无序；1:小大>>>n(不等于-1、0、1时):从已排序的队列中曾删除过n-1个数
	INT_T				(*Compare)(VOID_T *obj, VOID_T *obj2);
};

struct _XLIST_PAGE_V1_T
{//Page 还可以通过Items的某个元素向下延伸XLIST_PAGE_V1_T;
	XLIST_PAGE_T		*Prev;
	XLIST_PAGE_T		*Next;
	XLIST_PAGE_T		*Parent;
	XLIST_PAGE_T		*Child;		//最后一个的Child指向NULL；
	VOID_T				**Items;
	LONG_T				ItemCount;
	LONG_T				Count;
	BOOL_T				Locked;
	BOOL_T				Loaded;		//1:已载入内存，未载入内存，Items are loaded or not.
	LONG_T				Sorted;		//本页是否已经排序；
	LONG_T				Features;	//其他特征标志。
	char				*FileName;	//该页存贮文件
};

//xlist_page functions' declare
static BOOL_T xlist_page_isfull(XLIST_PAGE_T *Me);
static BOOL_T xlist_page_isempty(XLIST_PAGE_T *Me);
static BOOL_T xlist_page_hasemptyplace(XLIST_PAGE_T *Me);
static VOID_T *xlist_page_add(XLIST_PAGE_T *Me, VOID_T *obj);
static XLIST_PAGE_T *xlist_page_newbrother(XLIST_PAGE_T *Me);		//2015.11.16
static XLIST_PAGE_T *xlist_page_newchild(XLIST_PAGE_T *Me);			//2015.11.16
static XLIST_PAGE_T *xlist_page_appendchild(XLIST_PAGE_T *Me);		//2015.11.16

static XLIST_PAGE_T *xlist_page_create(void);
static VOID_T xlist_page_destroy(XLIST_PAGE_T *Me);

//xlist_page events:
static LONG_T xlist_page_event_onadd(XLIST_PAGE_T *Me);

//xlist functions' declare
static XLIST_T *xlist_create(void);
static XLIST_T *xlist_create_withpages(LONG_T n);
static VOID_T xlist_destroy(XLIST_T **Me);

static VOID_T *xlist_add(XLIST_T *Me, VOID_T *obj);
static XLIST_PAGE_T *xlist_newpage(XLIST_T *Me);
static XLIST_PAGE_T *xlist_findipage(XLIST_T *Me);
static XLIST_PAGE_T *xlist_newchild(XLIST_T *Me, PAGECHECK fcheck);	//2015.11.16

static BOOL_T xlist_alwaystrue(XLIST_PAGE_T *Me);
static BOOL_T xlist_alwaysfalse(XLIST_PAGE_T *Me);
static BOOL_T xlist_isfull(XLIST_T *Me);

static LONG_T xlist_foreach(XLIST_PAGE_T *Me, PAGECHECK fcheck, PAGEEVENT fexec);
static LONG_T xlist_foreachparent(XLIST_PAGE_T *Me, PAGECHECK fcheck, PAGEEVENT fexec);
static XLIST_PAGE_T *xlist_broadcast(XLIST_PAGE_T *Me, PAGECHECK func);		//广播，并尝试找到一个响应者
static LONG_T xlist_inform(XLIST_PAGE_T *Me,LONG_T features, PAGECHECK fcheck, PAGEEVENT fexec);	//

#endif
