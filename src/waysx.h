/***************************************
 $Header: /home/amb/routino/src/RCS/waysx.h,v 1.28 2010/09/25 18:47:32 amb Exp $

 A header file for the extended Ways structure.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#ifndef WAYSX_H
#define WAYSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "typesx.h"
#include "ways.h"

#include "files.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
struct _WayX
{
 way_t    id;                   /*+ The way identifier. +*/

 index_t  prop;                 /*+ The index of the properties of the way in the compacted list. +*/

 Way      way;                  /*+ The real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 char    *filename;             /*+ The name of the temporary file (for the WaysX). +*/
 int      fd;                   /*+ The file descriptor of the temporary file (for the WaysX). +*/

 index_t  xnumber;              /*+ The number of unsorted extended ways. +*/

#if !SLIM

 WayX    *xdata;                /*+ The extended data for the Ways (sorted). +*/

#else

 WayX     xcached[2];           /*+ Two cached ways read from the file in slim mode. +*/

#endif

 index_t  number;               /*+ How many entries are still useful? +*/

 index_t  cnumber;              /*+ How many entries are there after compacting? +*/

 index_t *idata;                /*+ The index of the extended data for the Ways (sorted by ID). +*/

 char    *nfilename;            /*+ The name of the temporary file (for the names). +*/

 uint32_t nlength;              /*+ How long is the string of name entries? +*/
};


/* Functions */


WaysX *NewWayList(int append);
void FreeWayList(WaysX *waysx,int keep);

void SaveWayList(WaysX *waysx,const char *filename);

index_t IndexWayX(WaysX* waysx,way_t id);

void AppendWay(WaysX* waysx,way_t id,Way *way,const char *name);

void SortWayList(WaysX *waysx);

void CompactWayList(WaysX *waysx);


/* Macros / inline functions */

#if !SLIM

#define LookupWayX(waysx,index,position)  &(waysx)->xdata[index]
  
#else

static WayX *LookupWayX(WaysX* waysx,index_t index,int position);


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended way.

  WayX *LookupWayX Returns a pointer to the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  index_t index The way index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline WayX *LookupWayX(WaysX* waysx,index_t index,int position)
{
 SeekFile(waysx->fd,(off_t)index*sizeof(WayX));

 ReadFile(waysx->fd,&waysx->xcached[position-1],sizeof(WayX));

 return(&waysx->xcached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Put back an extended way.

  WaysX* waysx The set of ways to process.

  index_t index The way index to put back.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void PutBackWayX(WaysX* waysx,index_t index,int position)
{
 SeekFile(waysx->fd,(off_t)index*sizeof(WayX));

 WriteFile(waysx->fd,&waysx->xcached[position-1],sizeof(WayX));
}

#endif /* SLIM */


#endif /* WAYSX_H */
