/***************************************
 $Header: /home/amb/routino/src/RCS/fakes.c,v 1.2 2010/08/04 16:44:51 amb Exp $

 Fake node and segment generation.

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


#include "types.h"
#include "nodes.h"
#include "segments.h"

#include "functions.h"


/*+ The minimum distance along a segment from a node to insert a fake node. (in km). +*/
#define MINSEGMENT 0.005


/*+ A set of fake segments to allow start/finish in the middle of a segment. +*/
static Segment fake_segments[2*NWAYPOINTS];

/*+ A set of fake node latitudes and longitudes. +*/
static double fake_lon[NWAYPOINTS+1],fake_lat[NWAYPOINTS+1];


/*++++++++++++++++++++++++++++++++++++++
  Create a pair of fake segments corresponding to the given segment split in two.

  index_t CreateFakes Returns the fake node index (or a real one in special cases).

  Nodes *nodes The set of nodes to use.

  int point Which of the waypoints is this.

  Segment *segment The segment to split.

  index_t node1 The first node at the end of this segment.

  index_t node2 The second node at the end of this segment.

  distance_t dist1 The distance to the first node.

  distance_t dist2 The distance to the second node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t CreateFakes(Nodes *nodes,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2)
{
 index_t fakenode;
 double lat1,lon1,lat2,lon2;

 /* Check if we are actually close enough to an existing node */

 if(dist1<km_to_distance(MINSEGMENT) && dist2>km_to_distance(MINSEGMENT))
    return(node1);

 if(dist2<km_to_distance(MINSEGMENT) && dist1>km_to_distance(MINSEGMENT))
    return(node2);

 if(dist1<km_to_distance(MINSEGMENT) && dist2<km_to_distance(MINSEGMENT))
   {
    if(dist1<dist2)
       return(node1);
    else
       return(node2);
   }

 /* Create the fake node */

 fakenode=NODE_FAKE+point;

 GetLatLong(nodes,node1,&lat1,&lon1);
 GetLatLong(nodes,node2,&lat2,&lon2);

 if(lat1>3 && lat2<-3)
    lat2+=2*M_PI;
 else if(lat1<-3 && lat2>3)
    lat1+=2*M_PI;

 fake_lat[point]=lat1+(lat2-lat1)*(double)dist1/(double)(dist1+dist2);
 fake_lon[point]=lon1+(lon2-lon1)*(double)dist1/(double)(dist1+dist2);

 if(fake_lat[point]>M_PI) fake_lat[point]-=2*M_PI;

 /* Create the first fake segment */

 fake_segments[2*point-2]=*segment;

 if(segment->node1==node1)
    fake_segments[2*point-2].node1=fakenode;
 else
    fake_segments[2*point-2].node2=fakenode;

 fake_segments[2*point-2].distance=DISTANCE(dist1)|DISTFLAG(segment->distance);

 /* Create the second fake segment */

 fake_segments[2*point-1]=*segment;

 if(segment->node1==node2)
    fake_segments[2*point-1].node1=fakenode;
 else
    fake_segments[2*point-1].node2=fakenode;

 fake_segments[2*point-1].distance=DISTANCE(dist2)|DISTFLAG(segment->distance);

 return(fakenode);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup the latitude and longitude of a fake node.

  index_t fakenode The node to lookup.

  double *latitude Returns the latitude

  double *longitude Returns the longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetFakeLatLong(index_t fakenode, double *latitude,double *longitude)
{
 index_t realnode=fakenode-NODE_FAKE;

 *latitude =fake_lat[realnode];
 *longitude=fake_lon[realnode];
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the first fake segment associated to a fake node.

  Segment *FirstFakeSegment Returns the first fake segment.

  index_t fakenode The node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FirstFakeSegment(index_t fakenode)
{
 index_t realnode=fakenode-NODE_FAKE;

 return(&fake_segments[2*realnode-2]);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the next (there can only be two) fake segment associated to a fake node.

  Segment *NextFakeSegment Returns the second fake segment.

  Segment *segment The first fake segment.

  index_t fakenode The node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextFakeSegment(Segment *segment,index_t fakenode)
{
 index_t realnode=fakenode-NODE_FAKE;

 if(segment==&fake_segments[2*realnode-2])
    return(&fake_segments[2*realnode-1]);
 else
    return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the fake segment between a node and a fake node.

  Segment *ExtraFakeSegment Returns a segment between the two specified nodes if it exists.

  index_t node The real node.

  index_t fakenode The fake node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *ExtraFakeSegment(index_t node,index_t fakenode)
{
 index_t realnode=fakenode-NODE_FAKE;

 if(fake_segments[2*realnode-2].node1==node || fake_segments[2*realnode-2].node2==node)
    return(&fake_segments[2*realnode-2]);

 if(fake_segments[2*realnode-1].node1==node || fake_segments[2*realnode-1].node2==node)
    return(&fake_segments[2*realnode-1]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a fake segment given its index.

  Segment *LookupFakeSegment Returns a pointer to the segment.

  index_t fakesegment The index of the fake segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *LookupFakeSegment(index_t fakesegment)
{
 index_t realsegment=fakesegment-SEGMENT_FAKE;

 return(&fake_segments[realsegment]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the fake index of a fake segment.

  index_t IndexFakeSegment Returns the fake segment.

  Segment *segment The segment to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexFakeSegment(Segment *segment)
{
 index_t realsegment=segment-&fake_segments[0];

 return(realsegment+SEGMENT_FAKE);
}
