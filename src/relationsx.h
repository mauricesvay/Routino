/***************************************
 $Header: /home/amb/routino/src/RCS/relationsx.h,v 1.2 2010/09/25 18:47:32 amb Exp $

 A header file for the extended Relations structure.

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


#ifndef RELATIONSX_H
#define RELATIONSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "typesx.h"


/* Data structures */


/*+ An extended structure containing a single route relation. +*/
struct _RouteRelX
{
 relation_t id;                 /*+ The relation identifier. +*/

 allow_t    routes;             /*+ The types of route that this relation belongs to. +*/
};


/*+ A structure containing a set of relations. +*/
struct _RelationsX
{
 /* Route relations */

 char      *rfilename;         /*+ The name of the temporary file (for the RouteRelX). +*/
 int        rfd;               /*+ The file descriptor of the temporary file (for the RouteRelX). +*/

 index_t    rxnumber;          /*+ The number of unsorted extended route relations. +*/
};


/* Functions */


RelationsX *NewRelationList(int append);
void FreeRelationList(RelationsX *relationsx,int keep);

void AppendRouteRelation(RelationsX* relationsx,relation_t id,allow_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations);

void SortRelationList(RelationsX *relationsx);

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx);


#endif /* RELATIONSX_H */
