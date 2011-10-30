/***************************************
 A header file for the extended Ways structure.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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
 way_t    id;                   /*+ The way identifier; the OSM value. +*/

 index_t  prop;                 /*+ The index of the properties of the way in the compacted list. +*/

 Way      way;                  /*+ The real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 char    *filename;             /*+ The name of the temporary file (for the WaysX). +*/
 int      fd;                   /*+ The file descriptor of the temporary file (for the WaysX). +*/

 index_t  number;               /*+ The number of extended ways still being considered. +*/

#if !SLIM

 WayX    *data;                 /*+ The extended ways data (when mapped into memory). +*/

#else

 WayX     cached[2];            /*+ Two cached ways read from the file in slim mode. +*/

#endif

 index_t  cnumber;              /*+ The number of entries after compacting. +*/

 way_t   *idata;                /*+ The extended way IDs (sorted by ID). +*/

 char    *nfilename;            /*+ The name of the temporary file (for the names). +*/
 int      nfd;                  /*+ The file descriptor of the temporary file (for the names). +*/

 uint32_t nlength;              /*+ The length of the string of name entries. +*/
};


/* Functions in waysx.c */


WaysX *NewWayList(int append);
void FreeWayList(WaysX *waysx,int keep);

void SaveWayList(WaysX *waysx,const char *filename);

index_t IndexWayX(WaysX *waysx,way_t id);

void AppendWay(WaysX *waysx,way_t id,Way *way,const char *name);

void SortWayList(WaysX *waysx);

void CompactWayList(WaysX *waysx);


/* Macros / inline functions */

#if !SLIM

#define LookupWayX(waysx,index,position)  &(waysx)->data[index]
  
#define PutBackWayX(waysx,index,position) /* nop */

#else

static WayX *LookupWayX(WaysX *waysx,index_t index,int position);

static void PutBackWayX(WaysX *waysx,index_t index,int position);


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended way with the specified id from the file on disk.

  WayX *LookupWayX Returns a pointer to a cached copy of the extended way.

  WaysX *waysx The set of ways to use.

  index_t index The way index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline WayX *LookupWayX(WaysX *waysx,index_t index,int position)
{
 SeekFile(waysx->fd,(off_t)index*sizeof(WayX));

 ReadFile(waysx->fd,&waysx->cached[position-1],sizeof(WayX));

 return(&waysx->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Put back an extended way's data into the file on disk.

  WaysX *waysx The set of ways to use.

  index_t index The way index to put back.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void PutBackWayX(WaysX *waysx,index_t index,int position)
{
 SeekFile(waysx->fd,(off_t)index*sizeof(WayX));

 WriteFile(waysx->fd,&waysx->cached[position-1],sizeof(WayX));
}

#endif /* SLIM */


#endif /* WAYSX_H */
