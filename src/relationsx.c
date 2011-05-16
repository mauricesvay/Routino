/***************************************
 $Header: /home/amb/routino/src/RCS/relationsx.c,v 1.10 2010/11/13 14:57:30 amb Exp $

 Extended Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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

#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "logging.h"
#include "functions.h"


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new relation list (create a new file or open an existing one).

  RelationsX *NewRelationList Returns the relation list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

RelationsX *NewRelationList(int append)
{
 RelationsX *relationsx;

 relationsx=(RelationsX*)calloc(1,sizeof(RelationsX));

 assert(relationsx); /* Check calloc() worked */

 relationsx->rfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->rfilename,"%s/relationsx.route.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->rfilename,"%s/relationsx.route.%p.tmp",option_tmpdirname,relationsx);

 if(append)
   {
    off_t size,position=0;

    relationsx->rfd=OpenFileAppend(relationsx->rfilename);

    size=SizeFile(relationsx->rfilename);

    while(position<size)
      {
       FILESORT_VARINT relationsize;

       SeekFile(relationsx->rfd,position);
       ReadFile(relationsx->rfd,&relationsize,FILESORT_VARSIZE);

       relationsx->rxnumber++;
       position+=relationsize+FILESORT_VARSIZE;
      }

    SeekFile(relationsx->rfd,size);
   }
 else
    relationsx->rfd=OpenFileNew(relationsx->rfilename);

 return(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a relation list.

  RelationsX *relationsx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeRelationList(RelationsX *relationsx,int keep)
{
 if(!keep)
    DeleteFile(relationsx->rfilename);

 free(relationsx->rfilename);

 free(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted route relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  allow_t routes The types of routes that this relation is for.

  way_t *ways The array of ways that are members of the relation.

  int nways The number of ways that are members of the relation.

  relation_t *relations The array of relations that are members of the relation.

  int nrelations The number of relations that are members of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendRouteRelation(RelationsX* relationsx,relation_t id,allow_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations)
{
 RouteRelX relationx;
 FILESORT_VARINT size;
 way_t zeroway=0;
 relation_t zerorelation=0;

 relationx.id=id;
 relationx.routes=routes;

 size=sizeof(RouteRelX)+(nways+1)*sizeof(way_t)+(nrelations+1)*sizeof(relation_t);

 WriteFile(relationsx->rfd,&size,FILESORT_VARSIZE);
 WriteFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

 WriteFile(relationsx->rfd,ways    ,nways*sizeof(way_t));
 WriteFile(relationsx->rfd,&zeroway,      sizeof(way_t));

 WriteFile(relationsx->rfd,relations    ,nrelations*sizeof(relation_t));
 WriteFile(relationsx->rfd,&zerorelation,           sizeof(relation_t));

 relationsx->rxnumber++;

 assert(!(relationsx->rxnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortRelationList(RelationsX* relationsx)
{
 /* Don't need to sort route relations */
}


/*++++++++++++++++++++++++++++++++++++++
  Process the route relations and apply the information to the ways.

  RelationsX *relationsx The set of relations to process.

  WaysX *waysx The set of ways to update.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx)
{
 RouteRelX *unmatched=NULL,*lastunmatched=NULL;
 int nunmatched=0,lastnunmatched=0,iteration=0;

 if(waysx->number==0)
    return;

 /* Map into memory */

#if !SLIM
 waysx->xdata=MapFileWriteable(waysx->filename);
#endif

 /* Re-open the ways file read/write */

#if SLIM
 CloseFile(waysx->fd);
 waysx->fd=ReOpenFileWriteable(waysx->filename);
#endif

 /* Open the file and read through it */

 relationsx->rfd=ReOpenFile(relationsx->rfilename);

 do
   {
    int ways=0,relations=0;
    int i;

    SeekFile(relationsx->rfd,0);

    /* Print the start message */

    printf_first("Processing Route Relations: Iteration=%d Relations=0 Modified Ways=0",iteration);

    for(i=0;i<relationsx->rxnumber;i++)
      {
       FILESORT_VARINT size;
       RouteRelX relationx;
       way_t wayid;
       relation_t relationid;
       allow_t routes=Allow_None;

       /* Read each route relation */

       ReadFile(relationsx->rfd,&size,FILESORT_VARSIZE);
       ReadFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

       /* Decide what type of route it is */

       if(iteration==0)
         {
          relations++;
          routes=relationx.routes;
         }
       else
         {
          int j;

          for(j=0;j<lastnunmatched;j++)
             if(lastunmatched[j].id==relationx.id)
               {
                relations++;

                if((lastunmatched[j].routes|relationx.routes)==relationx.routes)
                   routes=0; /* Nothing new to add */
                else
                   routes=lastunmatched[j].routes;
                break;
               }
         }

       /* Loop through the ways */

       do
         {
          ReadFile(relationsx->rfd,&wayid,sizeof(way_t));

          /* Update the ways that are listed for the relation */

          if(wayid && routes)
            {
             index_t way=IndexWayX(waysx,wayid);

             if(way!=NO_WAY)
               {
                WayX *wayx=LookupWayX(waysx,way,1);

                if(routes&Allow_Foot)
                   wayx->way.props|=Properties_FootRoute;

                if(routes&Allow_Bicycle)
                   wayx->way.props|=Properties_BicycleRoute;

#if SLIM
                PutBackWayX(waysx,way,1);
#endif

                ways++;
               }
            }
         }
       while(wayid);

       /* Loop through the relations */

       do
         {
          ReadFile(relationsx->rfd,&relationid,sizeof(relation_t));

          /* Add the relations that are listed for this relation to the list for next time */

          if(relationid && routes && relationid!=relationx.id)
            {
             if(nunmatched%256==0)
                unmatched=(RouteRelX*)realloc((void*)unmatched,(nunmatched+256)*sizeof(RouteRelX));

             unmatched[nunmatched].id=relationid;
             unmatched[nunmatched].routes=routes;

             nunmatched++;
            }
         }
       while(relationid);

       if(!((i+1)%10000))
          printf_middle("Processing Route Relations: Iteration=%d Relations=%d Modified Ways=%d",iteration,relations,ways);
      }

    if(lastunmatched)
       free(lastunmatched);

    lastunmatched=unmatched;
    lastnunmatched=nunmatched;

    unmatched=NULL;
    nunmatched=0;

    /* Print the final message */

    printf_last("Processed Route Relations: Iteration=%d Relations=%d Modified Ways=%d",iteration,relations,ways);
   }
 while(lastnunmatched && ++iteration<5);

 if(lastunmatched)
    free(lastunmatched);

 CloseFile(relationsx->rfd);

 /* Unmap from memory */

#if !SLIM
 waysx->xdata=UnmapFile(waysx->filename);
#endif

 /* Re-open the ways file read only */

#if SLIM
 CloseFile(waysx->fd);
 waysx->fd=ReOpenFile(waysx->filename);
#endif
}
