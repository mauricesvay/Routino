/***************************************
 $Header: /home/amb/routino/src/RCS/results.h,v 1.18 2010/07/23 14:32:16 amb Exp $

 A header file for the results.

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


#ifndef RESULTS_H
#define RESULTS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"


/* Constants */

/*+ A result is not currently queued. +*/
#define NOT_QUEUED (uint32_t)(~0)


/* Data structures */

/*+ The result for a node. +*/
typedef struct _Result
{
 index_t   node;                /*+ The node for which this result applies. +*/
 index_t   segment;             /*+ The segment for the path to here (from prev). +*/

 index_t   prev;                /*+ The previous node following the best path. +*/
 index_t   next;                /*+ The next node following the best path. +*/

 score_t   score;               /*+ The best actual weighted distance or duration score from the start to the node. +*/

 score_t   sortby;              /*+ The best possible weighted distance or duration score from the start to the finish. +*/
 uint32_t  queued;              /*+ The position of this result in the queue. +*/
}
 Result;

/*+ A list of results. +*/
typedef struct _Results
{
 uint32_t  nbins;               /*+ The number of bins. +*/
 uint32_t  mask;                /*+ A bit mask to select the bottom 'nbins' bits. +*/

 uint32_t  alloced;             /*+ The amount of space allocated for results
                                    (the length of the number and pointers arrays and
                                     1/nbins times the amount in the real results). +*/
 uint32_t  number;              /*+ The total number of occupied results. +*/

 uint32_t *count;               /*+ An array of nbins counters of results in each array. +*/
 Result ***point;               /*+ An array of nbins arrays of pointers to actual results. +*/

 Result  **data;                /*+ An array of arrays containing the actual results
                                    (don't need to realloc the array of data when adding more,
                                    only realloc the array that points to the array of data).
                                    Most importantly pointers into the real data don't change
                                    as more space is allocated (since realloc is not being used). +*/

 index_t start;                 /*+ The start node. +*/
 index_t finish;                /*+ The finish node. +*/
}
 Results;


/* Forward definitions for opaque type */

typedef struct _Queue Queue;


/* Results Functions */

Results *NewResultsList(int nbins);
void FreeResultsList(Results *results);

Result *InsertResult(Results *results,index_t node);
void ZeroResult(Result *result);

Result *FindResult(Results *results,index_t node);

Result *FirstResult(Results *results);
Result *NextResult(Results *results,Result *result);


/* Queue Functions */

Queue *NewQueueList(void);
void FreeQueueList(Queue *queue);

void InsertInQueue(Queue *queue,Result *result);
Result *PopFromQueue(Queue *queue);


#endif /* RESULTS_H */
