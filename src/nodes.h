/***************************************
 $Header: /home/amb/routino/src/RCS/nodes.h,v 1.37 2010/08/03 18:28:30 amb Exp $

 A header file for the nodes.

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


#ifndef NODES_H
#define NODES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "files.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single node. +*/
struct _Node
{
 index_t    firstseg;           /*+ The index of the first segment. +*/

 ll_off_t   latoffset;          /*+ The node latitude offset within its bin. +*/
 ll_off_t   lonoffset;          /*+ The node longitude offset within its bin. +*/

 allow_t    allow;              /*+ The types of transport that are allowed through the node. +*/
 uint16_t   flags;              /*+ Flags containing extra information (super-node, turn restriction). +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _NodesFile
{
 index_t  number;               /*+ How many nodes in total? +*/
 index_t  snumber;              /*+ How many super-nodes? +*/

 index_t  latbins;              /*+ The number of bins containing latitude. +*/
 index_t  lonbins;              /*+ The number of bins containing longitude. +*/

 ll_bin_t latzero;              /*+ The bin number of the furthest south bin. +*/
 ll_bin_t lonzero;              /*+ The bin number of the furthest west bin. +*/
}
 NodesFile;


/*+ A structure containing a set of nodes (and pointers to mmap file). +*/
struct _Nodes
{
 NodesFile file;                /*+ The header data from the file. +*/

#if !SLIM

 void     *data;                /*+ The memory mapped data. +*/

 index_t  *offsets;             /*+ An array of offsets to the first node in each bin. +*/

 Node     *nodes;               /*+ An array of nodes. +*/

#else

 int       fd;                  /*+ The file descriptor for the file. +*/
 off_t     nodesoffset;         /*+ The offset of the nodes within the file. +*/

 Node      cached[3];           /*+ The cached nodes. +*/
 index_t   incache[3];          /*+ The indexes of the cached nodes. +*/

#endif
};


/* Functions */

Nodes *LoadNodeList(const char *filename);

index_t FindClosestNode(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist);

index_t FindClosestSegment(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                           distance_t distance,Profile *profile, distance_t *bestdist,
                           index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2);

void GetLatLong(Nodes *nodes,index_t index,double *latitude,double *longitude);


/* Macros and inline functions */

#if !SLIM

/*+ Return a Node pointer given a set of nodes and an index. +*/
#define LookupNode(xxx,yyy,zzz)     (&(xxx)->nodes[yyy])

/*+ Return a Segment points given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy,zzz)   LookupSegment((xxx),(yyy)->nodes[zzz].firstseg,1)

/*+ Return true if this is a super-node. +*/
#define IsSuperNode(xxx,yyy)        (((xxx)->nodes[yyy].flags)&NODE_SUPER)

/*+ Return the offset of a geographical region given a set of nodes and an index. +*/
#define LookupNodeOffset(xxx,yyy)   ((xxx)->offsets[yyy])

#else

static Node *LookupNode(Nodes *nodes,index_t index,int position);

#define FirstSegment(xxx,yyy,zzz)   LookupSegment((xxx),FirstSegment_internal(yyy,zzz),1)

static index_t FirstSegment_internal(Nodes *nodes,index_t index);

static int IsSuperNode(Nodes *nodes,index_t index);

static index_t LookupNodeOffset(Nodes *nodes,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Find the Node information for a particular node.

  Node *LookupNode Returns a pointer to the cached node information.

  Nodes *nodes The nodes structure to use.

  index_t index The index of the node.

  int position The position in the cache to store the value.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Node *LookupNode(Nodes *nodes,index_t index,int position)
{
 SeekFile(nodes->fd,nodes->nodesoffset+(off_t)index*sizeof(Node));

 ReadFile(nodes->fd,&nodes->cached[position-1],sizeof(Node));

 nodes->incache[position-1]=index;

 return(&nodes->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the index of the first segment of a node (called by FirstSegment() macro).

  index_t FirstSegment_internal Returns the index of the first segment.

  Nodes *nodes The nodes structure to use.

  index_t index The index of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t FirstSegment_internal(Nodes *nodes,index_t index)
{
 if(nodes->incache[0]==index)
    return(nodes->cached[0].firstseg);
 else if(nodes->incache[1]==index)
    return(nodes->cached[1].firstseg);
 else if(nodes->incache[2]==index)
    return(nodes->cached[2].firstseg);
 else
   {
    Node *node=LookupNode(nodes,index,3);

    return(node->firstseg);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Decide if a node is a super-node.

  int IsSuperNode Return true if it is a supernode.

  Nodes *nodes The nodes structure to use.

  index_t index The index of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int IsSuperNode(Nodes *nodes,index_t index)
{
 if(nodes->incache[0]==index)
    return(nodes->cached[0].flags&NODE_SUPER);
 else if(nodes->incache[1]==index)
    return(nodes->cached[1].flags&NODE_SUPER);
 else if(nodes->incache[2]==index)
    return(nodes->cached[2].flags&NODE_SUPER);
 else
   {
    Node *node=LookupNode(nodes,index,3);

    return(node->flags&NODE_SUPER);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Find the offset of nodes in a geographical region.

  index_t LookupNodeOffset Returns the value of the index offset.

  Nodes *nodes The nodes structure to use.

  index_t index The index of the offset.
  ++++++++++++++++++++++++++++++++++++++*/

static inline index_t LookupNodeOffset(Nodes *nodes,index_t index)
{
 index_t offset;

 SeekFile(nodes->fd,sizeof(NodesFile)+(off_t)index*sizeof(index_t));

 ReadFile(nodes->fd,&offset,sizeof(index_t));

 return(offset);
}

#endif


#endif /* NODES_H */
