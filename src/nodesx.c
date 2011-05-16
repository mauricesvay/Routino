/***************************************
 $Header: /home/amb/routino/src/RCS/nodesx.c,v 1.76 2010/11/13 14:22:28 amb Exp $

 Extented Node data type functions.

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"

#include "types.h"

#include "files.h"
#include "logging.h"
#include "functions.h"


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/*+ A temporary file-local variable for use by the sort functions. +*/
static NodesX *sortnodesx;

/* Functions */

static int sort_by_id(NodeX *a,NodeX *b);
static int index_by_id(NodeX *nodex,index_t index);

static int sort_by_lat_long(NodeX *a,NodeX *b);
static int index_by_lat_long(NodeX *nodex,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list (create a new file or open an existing one).

  NodesX *NewNodeList Returns the node list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(int append)
{
 NodesX *nodesx;

 nodesx=(NodesX*)calloc(1,sizeof(NodesX));

 assert(nodesx); /* Check calloc() worked */

 nodesx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(nodesx->filename,"%s/nodesx.input.tmp",option_tmpdirname);
 else
    sprintf(nodesx->filename,"%s/nodesx.%p.tmp",option_tmpdirname,nodesx);

#if SLIM
 nodesx->nfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 sprintf(nodesx->nfilename,"%s/nodes.%p.tmp",option_tmpdirname,nodesx);
#endif

 if(append)
   {
    off_t size;

    nodesx->fd=OpenFileAppend(nodesx->filename);

    size=SizeFile(nodesx->filename);

    nodesx->xnumber=size/sizeof(NodeX);
   }
 else
    nodesx->fd=OpenFileNew(nodesx->filename);

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a node list.

  NodesX *nodesx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeNodeList(NodesX *nodesx,int keep)
{
 if(!keep)
    DeleteFile(nodesx->filename);

 free(nodesx->filename);

 if(nodesx->idata)
    free(nodesx->idata);

#if !SLIM
 if(nodesx->ndata)
    free(nodesx->ndata);
#endif

#if SLIM
 DeleteFile(nodesx->nfilename);

 free(nodesx->nfilename);
#endif

 if(nodesx->super)
    free(nodesx->super);

 if(nodesx->offsets)
    free(nodesx->offsets);

 free(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single node to an unsorted node list.

  NodesX* nodesx The set of nodes to process.

  node_t id The node identification.

  double latitude The latitude of the node.

  double longitude The longitude of the node.

  allow_t allow The allowed traffic types through the node.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendNode(NodesX* nodesx,node_t id,double latitude,double longitude,allow_t allow)
{
 NodeX nodex;

 nodex.id=id;
 nodex.latitude =radians_to_latlong(latitude);
 nodex.longitude=radians_to_latlong(longitude);
 nodex.allow=allow;

 WriteFile(nodesx->fd,&nodex,sizeof(NodeX));

 nodesx->xnumber++;

 assert(nodesx->xnumber<NODE_FAKE); /* NODE_FAKE marks the high-water mark for real nodes. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list (i.e. create the sortable indexes).

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX* nodesx)
{
 int fd;

 /* Print the start message */

 printf_first("Sorting Nodes");

 /* Close the files and re-open them (finished appending) */

 CloseFile(nodesx->fd);
 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Allocate the array of indexes */

 nodesx->idata=(node_t*)malloc(nodesx->xnumber*sizeof(node_t));

 assert(nodesx->idata); /* Check malloc() worked */

 /* Sort by node indexes */

 sortnodesx=nodesx;

 filesort_fixed(nodesx->fd,fd,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))index_by_id);

 /* Close the files and re-open them */

 CloseFile(nodesx->fd);
 CloseFile(fd);

 nodesx->fd=ReOpenFile(nodesx->filename);

 /* Print the final message */

 printf_last("Sorted Nodes: Nodes=%d Duplicates=%d",nodesx->xnumber,nodesx->xnumber-nodesx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  NodeX *a The first extended node.

  NodeX *b The second extended node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(NodeX *a,NodeX *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Index the nodes after sorting.

  int index_by_id Return 1 if the value is to be kept, otherwise zero.

  NodeX *nodex The extended node.

  index_t index The index of this node in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int index_by_id(NodeX *nodex,index_t index)
{
 if(index==0 || sortnodesx->idata[index-1]!=nodex->id)
   {
    sortnodesx->idata[index]=nodex->id;

    sortnodesx->number++;

    return(1);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list geographically.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeListGeographically(NodesX* nodesx)
{
 int fd;

 /* Print the start message */

 printf_first("Sorting Nodes Geographically");

 /* Allocate the memory for the geographical offsets array */

 nodesx->offsets=(index_t*)malloc((nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 nodesx->latlonbin=0;

 /* Close the files and re-open them */

 CloseFile(nodesx->fd);
 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Sort geographically */

 sortnodesx=nodesx;

 filesort_fixed(nodesx->fd,fd,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_lat_long,(int (*)(void*,index_t))index_by_lat_long);

 /* Close the files and re-open them */

 CloseFile(nodesx->fd);
 CloseFile(fd);

 nodesx->fd=ReOpenFile(nodesx->filename);

 /* Finish off the indexing */

 for(;nodesx->latlonbin<=(nodesx->latbins*nodesx->lonbins);nodesx->latlonbin++)
    nodesx->offsets[nodesx->latlonbin]=nodesx->number;

 /* Print the final message */

 printf_last("Sorted Nodes Geographically");
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into latitude and longitude order.

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  NodeX *a The first extended node.

  NodeX *b The second extended node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX *a,NodeX *b)
{
 ll_bin_t a_lon=latlong_to_bin(a->longitude);
 ll_bin_t b_lon=latlong_to_bin(b->longitude);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    ll_bin_t a_lat=latlong_to_bin(a->latitude);
    ll_bin_t b_lat=latlong_to_bin(b->latitude);

    if(a_lat<b_lat)
       return(-1);
    else if(a_lat>b_lat)
       return(1);
    else
      {
#ifdef REGRESSION_TESTING
       // Need this for regression testing because filesort_heapsort() is not order
       // preserving like qsort() is (or was when tested).

       index_t a_id=a->id;
       index_t b_id=b->id;

       if(a_id<b_id)
          return(-1);
       else if(a_id>b_id)
          return(1);
       else
#endif
          return(0);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Index the nodes after sorting.

  int index_by_lat_long Return 1 if the value is to be kept, otherwise zero.

  NodeX *nodex The extended node.

  index_t index The index of this node in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int index_by_lat_long(NodeX *nodex,index_t index)
{
 /* Work out the offsets */

 ll_bin_t latbin=latlong_to_bin(nodex->latitude )-sortnodesx->latzero;
 ll_bin_t lonbin=latlong_to_bin(nodex->longitude)-sortnodesx->lonzero;
 int llbin=lonbin*sortnodesx->latbins+latbin;

 for(;sortnodesx->latlonbin<=llbin;sortnodesx->latlonbin++)
    sortnodesx->offsets[sortnodesx->latlonbin]=index;

 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node index.

  index_t IndexNodeX Returns the index of the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexNodeX(NodesX* nodesx,node_t id)
{
 int start=0;
 int end=nodesx->number-1;
 int mid;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                        /* There are no nodes */
    return(NO_NODE);
 else if(id<nodesx->idata[start])     /* Check key is not before start */
    return(NO_NODE);
 else if(id>nodesx->idata[end])       /* Check key is not after end */
    return(NO_NODE);
 else
   {
    do
      {
       mid=(start+end)/2;             /* Choose mid point */

       if(nodesx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesx->idata[mid]>id) /* Mid point is too high */
          end=mid-1;
       else                           /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(nodesx->idata[start]==id)      /* Start is correct */
       return(start);

    if(nodesx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_NODE);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove any nodes that are not part of a highway.

  NodesX *nodesx The complete node list.

  SegmentsX *segmentsx The list of segments.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 NodeX nodex;
 int total=0,highway=0,nothighway=0;
 ll_bin_t lat_min_bin,lat_max_bin,lon_min_bin,lon_max_bin;
 latlong_t lat_min,lat_max,lon_min,lon_max;
 int fd;

 /* Print the start message */

 printf_first("Checking: Nodes=0");

 /* While we are here we can work out the range of data */

 lat_min=radians_to_latlong( 2);
 lat_max=radians_to_latlong(-2);
 lon_min=radians_to_latlong( 4);
 lon_max=radians_to_latlong(-4);

 /* Modify the on-disk image */

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);
 SeekFile(nodesx->fd,0);

 while(!ReadFile(nodesx->fd,&nodex,sizeof(NodeX)))
   {
    if(IndexFirstSegmentX(segmentsx,nodex.id)==NO_SEGMENT)
       nothighway++;
    else
      {
       nodex.id=highway;

       WriteFile(fd,&nodex,sizeof(NodeX));

       nodesx->idata[highway]=nodesx->idata[total];
       highway++;

       if(nodex.latitude<lat_min)
          lat_min=nodex.latitude;
       if(nodex.latitude>lat_max)
          lat_max=nodex.latitude;
       if(nodex.longitude<lon_min)
          lon_min=nodex.longitude;
       if(nodex.longitude>lon_max)
          lon_max=nodex.longitude;
      }

    total++;

    if(!(total%10000))
       printf_middle("Checking: Nodes=%d Highway=%d not-Highway=%d",total,highway,nothighway);
   }

 /* Close the files and re-open them */

 CloseFile(nodesx->fd);
 CloseFile(fd);

 nodesx->fd=ReOpenFile(nodesx->filename);

 nodesx->number=highway;

 /* Work out the number of bins */

 lat_min_bin=latlong_to_bin(lat_min);
 lon_min_bin=latlong_to_bin(lon_min);
 lat_max_bin=latlong_to_bin(lat_max);
 lon_max_bin=latlong_to_bin(lon_max);

 nodesx->latzero=lat_min_bin;
 nodesx->lonzero=lon_min_bin;

 nodesx->latbins=(lat_max_bin-lat_min_bin)+1;
 nodesx->lonbins=(lon_max_bin-lon_min_bin)+1;

 /* Allocate and clear the super-node markers */

 nodesx->super=(uint8_t*)calloc(nodesx->number,sizeof(uint8_t));

 assert(nodesx->super); /* Check calloc() worked */

 /* Print the final message */

 printf_last("Checked: Nodes=%d Highway=%d not-Highway=%d",total,highway,nothighway);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the real node data.

  NodesX *nodesx The set of nodes to use.

  int iteration The final super-node iteration.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateRealNodes(NodesX *nodesx,int iteration)
{
 index_t i;

 /* Print the start message */

 printf_first("Creating Real Nodes: Nodes=0");

 /* Map into memory */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
#endif

 /* Allocate the memory (or open the file) */

#if !SLIM
 nodesx->ndata=(Node*)malloc(nodesx->number*sizeof(Node));

 assert(nodesx->ndata); /* Check malloc() worked */
#else
 nodesx->nfd=OpenFileNew(nodesx->nfilename);
#endif

 /* Loop through and allocate. */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX *nodex=LookupNodeX(nodesx,i,1);
    Node  *node =LookupNodeXNode(nodesx,nodex->id,1);

    node->latoffset=latlong_to_off(nodex->latitude);
    node->lonoffset=latlong_to_off(nodex->longitude);
    node->firstseg=NO_SEGMENT;
    node->allow=nodex->allow;
    node->flags=0;

    if(nodesx->super[nodex->id]==iteration)
       node->flags|=NODE_SUPER;

#if SLIM
    PutBackNodeXNode(nodesx,nodex->id,1);
#endif

    if(!((i+1)%10000))
       printf_middle("Creating Real Nodes: Nodes=%d",i+1);
   }

 /* Free the unneeded memory */

 free(nodesx->super);
 nodesx->super=NULL;

 /* Unmap from memory */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
#endif

 /* Print the final message */

 printf_last("Creating Real Nodes: Nodes=%d",nodesx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the segment indexes to the nodes.

  NodesX *nodesx The list of nodes to process.

  SegmentsX* segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 index_t i;

 if(nodesx->number==0 || segmentsx->number==0)
    return;

 /* Print the start message */

 printf_first("Indexing Segments: Segments=0");

 /* Map into memory */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
 segmentsx->xdata=MapFile(segmentsx->filename);
#endif

 /* Index the nodes */

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,i,1);
    node_t id1=segmentx->node1;
    node_t id2=segmentx->node2;
    Node *node1=LookupNodeXNode(nodesx,id1,1);
    Node *node2=LookupNodeXNode(nodesx,id2,2);

    /* Check node1 */

    if(node1->firstseg==NO_SEGMENT)
      {
       node1->firstseg=i;

#if SLIM
       PutBackNodeXNode(nodesx,id1,1);
#endif
      }
    else
      {
       index_t index=node1->firstseg;

       do
         {
          segmentx=LookupSegmentX(segmentsx,index,1);

          if(segmentx->node1==id1)
            {
             index++;

             if(index>=segmentsx->number)
                break;

             segmentx=LookupSegmentX(segmentsx,index,1);

             if(segmentx->node1!=id1)
                break;
            }
          else
            {
             Segment *segment=LookupSegmentXSegment(segmentsx,index,1);

             if(segment->next2==NO_NODE)
               {
                segment->next2=i;

#if SLIM
                PutBackSegmentXSegment(segmentsx,index,1);
#endif

                break;
               }
             else
                index=segment->next2;
            }
         }
       while(1);
      }

    /* Check node2 */

    if(node2->firstseg==NO_SEGMENT)
      {
       node2->firstseg=i;

#if SLIM
       PutBackNodeXNode(nodesx,id2,2);
#endif
      }
    else
      {
       index_t index=node2->firstseg;

       do
         {
          segmentx=LookupSegmentX(segmentsx,index,1);

          if(segmentx->node1==id2)
            {
             index++;

             if(index>=segmentsx->number)
                break;

             segmentx=LookupSegmentX(segmentsx,index,1);

             if(segmentx->node1!=id2)
                break;
            }
          else
            {
             Segment *segment=LookupSegmentXSegment(segmentsx,index,1);

             if(segment->next2==NO_NODE)
               {
                segment->next2=i;

#if SLIM
                PutBackSegmentXSegment(segmentsx,index,1);
#endif

                break;
               }
             else
                index=segment->next2;
            }
         }
       while(1);
      }

    if(!((i+1)%10000))
       printf_middle("Indexing Segments: Segments=%d",i+1);
   }

 /* Unmap from memory */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
 segmentsx->xdata=UnmapFile(segmentsx->filename);
#endif

 /* Print the final message */

 printf_last("Indexed Segments: Segments=%d ",segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  NodesX* nodesx The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesX* nodesx,const char *filename)
{
 index_t i;
 int fd;
 NodesFile nodesfile={0};
 int super_number=0;

 /* Print the start message */

 printf_first("Writing Nodes: Nodes=0");

 /* Map into memory */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
#endif

 /* Write out the nodes data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(NodesFile));
 WriteFile(fd,nodesx->offsets,(nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 for(i=0;i<nodesx->number;i++)
   {
    NodeX *nodex=LookupNodeX(nodesx,i,1);
    Node *node=LookupNodeXNode(nodesx,nodex->id,1);

    if(node->flags&NODE_SUPER)
       super_number++;

    WriteFile(fd,node,sizeof(Node));

    if(!((i+1)%10000))
       printf_middle("Writing Nodes: Nodes=%d",i+1);
   }

 /* Write out the header structure */

 nodesfile.number=nodesx->number;
 nodesfile.snumber=super_number;

 nodesfile.latbins=nodesx->latbins;
 nodesfile.lonbins=nodesx->lonbins;

 nodesfile.latzero=nodesx->latzero;
 nodesfile.lonzero=nodesx->lonzero;

 SeekFile(fd,0);
 WriteFile(fd,&nodesfile,sizeof(NodesFile));

 CloseFile(fd);

 /* Unmap from memory */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
#endif

 /* Print the final message */

 printf_last("Wrote Nodes: Nodes=%d",nodesx->number);
}
