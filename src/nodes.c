/***************************************
 $Header: /home/amb/routino/src/RCS/nodes.c,v 1.44 2010/07/26 18:17:20 amb Exp $

 Node data type functions.

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


#include <sys/types.h>
#include <stdlib.h>
#include <math.h>

#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "files.h"
#include "profiles.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes* LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 Nodes *nodes;

 nodes=(Nodes*)malloc(sizeof(Nodes));

#if !SLIM

 nodes->data=MapFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 nodes->file=*((NodesFile*)nodes->data);

 /* Set the pointers in the Nodes structure. */

 nodes->offsets=(index_t*)(nodes->data+sizeof(NodesFile));
 nodes->nodes  =(Node*   )(nodes->data+sizeof(NodesFile)+(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t));

#else

 nodes->fd=ReOpenFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 ReadFile(nodes->fd,&nodes->file,sizeof(NodesFile));

 nodes->nodesoffset=sizeof(NodesFile)+(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t);

 nodes->incache[0]=NO_NODE;
 nodes->incache[1]=NO_NODE;
 nodes->incache[2]=NO_NODE;

#endif

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest node given its latitude, longitude and optionally profile.

  index_t FindClosestNode Returns the closest node.

  Nodes* nodes The set of nodes to search.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look.

  Profile *profile The profile of the mode of transport (or NULL).

  distance_t *bestdist Returns the distance to the best node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestNode(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn=NO_NODE;
 distance_t bestd=INF_DISTANCE;

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    int latb,lonb,llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          llbin=lonb*nodes->file.latbins+latb;

          /* Check if this grid square has any hope of being close enough */

          if(delta>0)
            {
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb));
             double lat2=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb+1));
             double lon2=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb+1));

             if(latb==latbin)
               {
                distance_t dist1=Distance(latitude,lon1,latitude,longitude);
                distance_t dist2=Distance(latitude,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else if(lonb==lonbin)
               {
                distance_t dist1=Distance(lat1,longitude,latitude,longitude);
                distance_t dist2=Distance(lat2,longitude,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else
               {
                distance_t dist1=Distance(lat1,lon1,latitude,longitude);
                distance_t dist2=Distance(lat2,lon1,latitude,longitude);
                distance_t dist3=Distance(lat2,lon2,latitude,longitude);
                distance_t dist4=Distance(lat1,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance && dist3>distance && dist4>distance)
                   continue;
               }
            }

          /* Check every node in this grid square. */

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *node=LookupNode(nodes,i,1);
             double lat=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(node->latoffset));
             double lon=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(node->lonoffset));

             distance_t dist=Distance(lat,lon,latitude,longitude);

             if(dist<distance)
               {
                if(profile)
                  {
                   Segment *segment;

                   /* Decide if this is node is valid for the profile */

                   segment=FirstSegment(segments,nodes,i);

                   do
                     {
                      Way *way=LookupWay(ways,segment->way,1);

                      if(way->allow&profile->allow)
                         break;

                      segment=NextSegment(segments,segment,i);
                     }
                   while(segment);

                   if(!segment)
                      continue;
                  }

                bestn=i; bestd=distance=dist;
               }
            }

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 return(bestn);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest segment to a latitude, longitude and optionally profile.

  index_t FindClosestSegment Returns the closest segment index.

  Nodes* nodes The set of nodes to search.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look.

  Profile *profile The profile of the mode of transport (or NULL).

  distance_t *bestdist Returns the distance to the best segment.

  index_t *bestnode1 Returns the best node at one end.

  index_t *bestnode2 Returns the best node at the other end.

  distance_t *bestdist1 Returns the distance to the best node at one end.

  distance_t *bestdist2 Returns the distance to the best node at the other end.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestSegment(Nodes* nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                           distance_t distance,Profile *profile, distance_t *bestdist,
                           index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn1=NO_NODE,bestn2=NO_NODE;
 distance_t bestd=INF_DISTANCE,bestd1=INF_DISTANCE,bestd2=INF_DISTANCE;
 index_t    bests=NO_SEGMENT;

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    int latb,lonb,llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          llbin=lonb*nodes->file.latbins+latb;

          /* Check if this grid square has any hope of being close enough */

          if(delta>0)
            {
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb));
             double lat2=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb+1));
             double lon2=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb+1));

             if(latb==latbin)
               {
                distance_t dist1=Distance(latitude,lon1,latitude,longitude);
                distance_t dist2=Distance(latitude,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else if(lonb==lonbin)
               {
                distance_t dist1=Distance(lat1,longitude,latitude,longitude);
                distance_t dist2=Distance(lat2,longitude,latitude,longitude);

                if(dist1>distance && dist2>distance)
                   continue;
               }
             else
               {
                distance_t dist1=Distance(lat1,lon1,latitude,longitude);
                distance_t dist2=Distance(lat2,lon1,latitude,longitude);
                distance_t dist3=Distance(lat2,lon2,latitude,longitude);
                distance_t dist4=Distance(lat1,lon2,latitude,longitude);

                if(dist1>distance && dist2>distance && dist3>distance && dist4>distance)
                   continue;
               }
            }

          /* Check every node in this grid square. */

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *node=LookupNode(nodes,i,1);
             double lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(node->latoffset));
             double lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(node->lonoffset));
             distance_t dist1;

             dist1=Distance(lat1,lon1,latitude,longitude);

             if(dist1<distance)
               {
                Segment *segment;

                /* Check each segment for closeness and if valid for the profile */

                segment=FirstSegment(segments,nodes,i);

                do
                  {
                   if(IsNormalSegment(segment))
                     {
                      Way *way=NULL;

                      if(profile)
                         way=LookupWay(ways,segment->way,1);

                      if(!profile || way->allow&profile->allow)
                        {
                         distance_t dist2,dist3;
                         double lat2,lon2,dist3a,dist3b,distp;

                         GetLatLong(nodes,OtherNode(segment,i),&lat2,&lon2);

                         dist2=Distance(lat2,lon2,latitude,longitude);

                         dist3=Distance(lat1,lon1,lat2,lon2);

                         /* Use law of cosines (assume flat Earth) */

                         dist3a=((double)dist1*(double)dist1-(double)dist2*(double)dist2+(double)dist3*(double)dist3)/(2.0*(double)dist3);
                         dist3b=(double)dist3-dist3a;

                         if(dist3a>=0 && dist3b>=0)
                            distp=sqrt((double)dist1*(double)dist1-dist3a*dist3a);
                         else if(dist3a>0)
                           {
                            distp=dist2;
                            dist3a=dist3;
                            dist3b=0;
                           }
                         else /* if(dist3b>0) */
                           {
                            distp=dist1;
                            dist3a=0;
                            dist3b=dist3;
                           }

                         if(distp<(double)bestd)
                           {
                            bests=IndexSegment(segments,segment);
                            bestn1=i;
                            bestn2=OtherNode(segment,i);
                            bestd1=(distance_t)dist3a;
                            bestd2=(distance_t)dist3b;
                            bestd=(distance_t)distp;
                           }
                        }
                     }

                   segment=NextSegment(segments,segment,i);
                  }
                while(segment);
               }

            } /* dist1 < distance */

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 *bestnode1=bestn1;
 *bestnode2=bestn2;
 *bestdist1=bestd1;
 *bestdist2=bestd2;

 return(bests);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the latitude and longitude associated with a node.

  Nodes *nodes The set of nodes.

  index_t index The node index.

  double *latitude Returns the latitude.

  double *longitude Returns the logitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetLatLong(Nodes *nodes,index_t index,double *latitude,double *longitude)
{
 Node *node=LookupNode(nodes,index,2);
 int latbin=-1,lonbin=-1;
 int start,end,mid;
 index_t offset;

 /* Binary search - search key closest below is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an inexact match is wanted we must set end=mid-1
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 /* Search for longitude */

 start=0;
 end=nodes->file.lonbins-1;

 do
   {
    mid=(start+end)/2;                  /* Choose mid point */

    offset=LookupNodeOffset(nodes,nodes->file.latbins*mid);

    if(offset<index)                    /* Mid point is too low */
       start=mid;
    else if(offset>index)               /* Mid point is too high */
       end=mid-1;
    else                                /* Mid point is correct */
      {lonbin=mid;break;}
   }
 while((end-start)>1);

 if(lonbin==-1)
   {
    offset=LookupNodeOffset(nodes,nodes->file.latbins*end);

    if(offset>index)
       lonbin=start;
    else
       lonbin=end;
   }

 while(lonbin<nodes->file.lonbins && 
       LookupNodeOffset(nodes,lonbin*nodes->file.latbins)==LookupNodeOffset(nodes,(lonbin+1)*nodes->file.latbins))
    lonbin++;

 /* Search for latitude */

 start=0;
 end=nodes->file.latbins-1;

 do
   {
    mid=(start+end)/2;                  /* Choose mid point */

    offset=LookupNodeOffset(nodes,lonbin*nodes->file.latbins+mid);

    if(offset<index)                    /* Mid point is too low */
       start=mid;
    else if(offset>index)               /* Mid point is too high */
       end=mid-1;
    else                                /* Mid point is correct */
      {latbin=mid;break;}
   }
 while((end-start)>1);

 if(latbin==-1)
   {
    offset=LookupNodeOffset(nodes,lonbin*nodes->file.latbins+end);

    if(offset>index)
       latbin=start;
    else
       latbin=end;
   }

 while(latbin<nodes->file.latbins &&
       LookupNodeOffset(nodes,lonbin*nodes->file.latbins+latbin)==LookupNodeOffset(nodes,lonbin*nodes->file.latbins+latbin+1))
    latbin++;

 /* Return the values */

 *latitude =latlong_to_radians(bin_to_latlong(nodes->file.latzero+latbin)+off_to_latlong(node->latoffset));
 *longitude=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonbin)+off_to_latlong(node->lonoffset));
}
