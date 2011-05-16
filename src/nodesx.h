/***************************************
 $Header: /home/amb/routino/src/RCS/nodesx.h,v 1.30 2010/08/02 18:44:54 amb Exp $

 A header file for the extended nodes.

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


#ifndef NODESX_H
#define NODESX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "nodes.h"

#include "typesx.h"

#include "files.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _NodeX
{
 node_t    id;                  /*+ The node identifier. +*/

 latlong_t latitude;            /*+ The node latitude. +*/
 latlong_t longitude;           /*+ The node longitude. +*/

 allow_t   allow;               /*+ The node allowed traffic. +*/
};

/*+ A structure containing a set of nodes (memory format). +*/
struct _NodesX
{
 char     *filename;            /*+ The name of the temporary file. +*/
 int       fd;                  /*+ The file descriptor of the temporary file. +*/

 index_t   xnumber;             /*+ The number of unsorted extended nodes. +*/

#if !SLIM

 NodeX    *xdata;               /*+ The extended node data (sorted). +*/

#else

 NodeX     xcached[2];          /*+ Two cached nodes read from the file in slim mode. +*/

#endif

 index_t   number;              /*+ How many entries are still useful? +*/

 node_t   *idata;               /*+ The extended node IDs (sorted by ID). +*/

 uint8_t  *super;               /*+ A marker for super nodes (same order sorted nodes). +*/

#if !SLIM

 Node     *ndata;               /*+ The actual nodes (same order as geographically sorted nodes). +*/

#else

 char     *nfilename;           /*+ The name of the temporary file for nodes in slim mode. +*/
 int       nfd;                 /*+ The file descriptor of the temporary file. +*/

 Node      ncached[2];          /*+ Two cached nodes read from the file in slim mode. +*/

#endif

 index_t   latbins;             /*+ The number of bins containing latitude. +*/
 index_t   lonbins;             /*+ The number of bins containing longitude. +*/

 ll_bin_t  latzero;             /*+ The bin number of the furthest south bin. +*/
 ll_bin_t  lonzero;             /*+ The bin number of the furthest west bin. +*/

 index_t   latlonbin;           /*+ A temporary index into the offsets array. +*/

 index_t  *offsets;             /*+ An array of offset to the first node in each bin. +*/
};


/* Functions */

NodesX *NewNodeList(int append);
void FreeNodeList(NodesX *nodesx,int keep);

void SaveNodeList(NodesX *nodesx,const char *filename);

index_t IndexNodeX(NodesX* nodesx,node_t id);

void AppendNode(NodesX* nodesx,node_t id,double latitude,double longitude,allow_t allow);

void SortNodeList(NodesX *nodesx);

void SortNodeListGeographically(NodesX* nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void CreateRealNodes(NodesX *nodesx,int iteration);

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx);


/* Macros / inline functions */

#if !SLIM

#define LookupNodeX(nodesx,index,position)      &(nodesx)->xdata[index]
  
#define LookupNodeXNode(nodesx,index,position)  &(nodesx)->ndata[index]

#else

static NodeX *LookupNodeX(NodesX* nodesx,index_t index,int position);

static Node *LookupNodeXNode(NodesX* nodesx,index_t index,int position);

static void PutBackNodeXNode(NodesX* nodesx,index_t index,int position);


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended node.

  NodeX *LookupNodeX Returns a pointer to the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  index_t index The node index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline NodeX *LookupNodeX(NodesX* nodesx,index_t index,int position)
{
 SeekFile(nodesx->fd,(off_t)index*sizeof(NodeX));

 ReadFile(nodesx->fd,&nodesx->xcached[position-1],sizeof(NodeX));

 return(&nodesx->xcached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular extended node's normal node.

  Node *LookupNodeXNode Returns a pointer to the node with the specified id.

  NodesX* nodesx The set of nodes to process.

  index_t index The node index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Node *LookupNodeXNode(NodesX* nodesx,index_t index,int position)
{
 SeekFile(nodesx->nfd,(off_t)index*sizeof(Node));

 ReadFile(nodesx->nfd,&nodesx->ncached[position-1],sizeof(Node));

 return(&nodesx->ncached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Put back an extended node's normal node.

  NodesX* nodesx The set of nodes to process.

  index_t index The node index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void PutBackNodeXNode(NodesX* nodesx,index_t index,int position)
{
 SeekFile(nodesx->nfd,(off_t)index*sizeof(Node));

 WriteFile(nodesx->nfd,&nodesx->ncached[position-1],sizeof(Node));
}

#endif /* SLIM */


#endif /* NODESX_H */
