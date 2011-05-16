/***************************************
 $Header: /home/amb/routino/src/RCS/functions.h,v 1.58 2010/09/25 13:54:18 amb Exp $

 Header file for function prototypes

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


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include "types.h"

#include "profiles.h"
#include "results.h"


/*+ The number of waypoints allowed to be specified. +*/
#define NWAYPOINTS 99


/* In fakes.c */

/*+ Return true if this is a fake node. +*/
#define IsFakeNode(xxx)    ((xxx)>=NODE_FAKE)

/*+ Return true if this is a fake segment. +*/
#define IsFakeSegment(xxx) ((xxx)>=SEGMENT_FAKE)

index_t CreateFakes(Nodes *nodes,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2);

void GetFakeLatLong(index_t node, double *latitude,double *longitude);

Segment *FirstFakeSegment(index_t node);
Segment *NextFakeSegment(Segment *segment,index_t node);
Segment *ExtraFakeSegment(index_t node,index_t fakenode);

Segment *LookupFakeSegment(index_t index);
index_t IndexFakeSegment(Segment *segment);


/* In optimiser.c */

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);
Results *FindMiddleRoute(Nodes *supernodes,Segments *supersegments,Ways *superways,Results *begin,Results *end,Profile *profile);

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);

void FixForwardRoute(Results *results,index_t finish);


/* In output.c */

void PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);


/* In sorting.c */

/*+ The type, size and alignment of variable to store the variable length +*/
#define FILESORT_VARINT   unsigned short
#define FILESORT_VARSIZE  sizeof(FILESORT_VARINT)
#define FILESORT_VARALIGN sizeof(void*)

void filesort_fixed(int fd_in,int fd_out,size_t itemsize,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void filesort_vary(int fd_in,int fd_out,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void filesort_heapsort(void **datap,size_t nitems,int(*compare)(const void*, const void*));


#endif /* FUNCTIONS_H */
