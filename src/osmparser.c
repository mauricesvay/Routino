/***************************************
 $Header: /home/amb/routino/src/RCS/osmparser.c,v 1.73 2010/11/13 14:22:28 amb Exp $

 OSM XML file parser (either JOSM or planet)

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "typesx.h"
#include "functionsx.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"

#include "xmlparse.h"
#include "tagging.h"

#include "logging.h"


/* Macros */

#define ISTRUE(xx) (!strcmp(xx,"true") || !strcmp(xx,"yes") || !strcmp(xx,"1"))


/* Local variables */

static long nnodes=0,nways=0,nrelations=0;
static TagList *current_tags=NULL;

static node_t *way_nodes=NULL;
static int     way_nnodes=0;

static node_t     *relation_nodes=NULL;
static int         relation_nnodes=0;
static way_t      *relation_ways=NULL;
static int         relation_nways=0;
static relation_t *relation_relations=NULL;
static int         relation_nrelations=0;

static NodesX     *nodes;
static SegmentsX  *segments;
static WaysX      *ways;
static RelationsX *relations;


/* Local functions */

static void process_node_tags(TagList *tags,node_t id,double latitude,double longitude);
static void process_way_tags(TagList *tags,way_t id);
static void process_relation_tags(TagList *tags,relation_t id);


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
//static int osmType_function(const char *_tag_,int _type_);
static int relationType_function(const char *_tag_,int _type_,const char *id);
static int wayType_function(const char *_tag_,int _type_,const char *id);
static int memberType_function(const char *_tag_,int _type_,const char *type,const char *ref,const char *role);
static int ndType_function(const char *_tag_,int _type_,const char *ref);
static int nodeType_function(const char *_tag_,int _type_,const char *id,const char *lat,const char *lon);
static int tagType_function(const char *_tag_,int _type_,const char *k,const char *v);
//static int boundType_function(const char *_tag_,int _type_);
//static int boundsType_function(const char *_tag_,int _type_);


/* The XML tag definitions */

/*+ The boundsType type tag. +*/
static xmltag boundsType_tag=
              {"bounds",
               0, {NULL},
               NULL,
               {NULL}};

/*+ The boundType type tag. +*/
static xmltag boundType_tag=
              {"bound",
               0, {NULL},
               NULL,
               {NULL}};

/*+ The tagType type tag. +*/
static xmltag tagType_tag=
              {"tag",
               2, {"k","v"},
               tagType_function,
               {NULL}};

/*+ The nodeType type tag. +*/
static xmltag nodeType_tag=
              {"node",
               3, {"id","lat","lon"},
               nodeType_function,
               {&tagType_tag,NULL}};

/*+ The ndType type tag. +*/
static xmltag ndType_tag=
              {"nd",
               1, {"ref"},
               ndType_function,
               {NULL}};

/*+ The memberType type tag. +*/
static xmltag memberType_tag=
              {"member",
               3, {"type","ref","role"},
               memberType_function,
               {NULL}};

/*+ The wayType type tag. +*/
static xmltag wayType_tag=
              {"way",
               1, {"id"},
               wayType_function,
               {&ndType_tag,&tagType_tag,NULL}};

/*+ The relationType type tag. +*/
static xmltag relationType_tag=
              {"relation",
               1, {"id"},
               relationType_function,
               {&memberType_tag,&tagType_tag,NULL}};

/*+ The osmType type tag. +*/
static xmltag osmType_tag=
              {"osm",
               0, {NULL},
               NULL,
               {&boundsType_tag,&boundType_tag,&nodeType_tag,&wayType_tag,&relationType_tag,NULL}};

/*+ The xmlDeclaration type tag. +*/
static xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xmlDeclaration_tag,&osmType_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the boundsType XSD type is seen

  int boundsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int boundsType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the boundType XSD type is seen

  int boundType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int boundType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the tagType XSD type is seen

  int tagType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int tagType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START && current_tags)
   {
    XMLPARSE_ASSERT_STRING(_tag_,k);
    XMLPARSE_ASSERT_STRING(_tag_,v);

    AppendTag(current_tags,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the nodeType XSD type is seen

  int nodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).

  const char *lat The contents of the 'lat' attribute (or NULL if not defined).

  const char *lon The contents of the 'lon' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int nodeType_function(const char *_tag_,int _type_,const char *id,const char *lat,const char *lon)
{
 static node_t node_id;
 static double latitude,longitude;

 if(_type_&XMLPARSE_TAG_START)
   {
    nnodes++;

    if(!(nnodes%1000))
       printf_middle("Reading: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld",ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    /* Handle the node information */

    XMLPARSE_ASSERT_STRING(_tag_,id); node_id=atoll(id); /* need long long conversion */
    XMLPARSE_ASSERT_FLOATING(_tag_,lat,latitude);
    XMLPARSE_ASSERT_FLOATING(_tag_,lon,longitude);
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&NodeRules,current_tags);

    process_node_tags(result,node_id,latitude,longitude);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the ndType XSD type is seen

  int ndType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *ref The contents of the 'ref' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int ndType_function(const char *_tag_,int _type_,const char *ref)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    node_t node_id;

    XMLPARSE_ASSERT_STRING(_tag_,ref); node_id=atoll(ref); /* need long long conversion */

    if(way_nnodes && (way_nnodes%256)==0)
       way_nodes=(node_t*)realloc((void*)way_nodes,(way_nnodes+256)*sizeof(node_t));

    way_nodes[way_nnodes++]=node_id;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the memberType XSD type is seen

  int memberType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *ref The contents of the 'ref' attribute (or NULL if not defined).

  const char *role The contents of the 'role' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int memberType_function(const char *_tag_,int _type_,const char *type,const char *ref,const char *role)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_STRING(_tag_,ref);

    if(!strcmp(type,"node"))
      {
       node_t node_id=atoll(ref); /* need long long conversion */

       if(relation_nnodes && (relation_nnodes%256)==0)
          relation_nodes=(node_t*)realloc((void*)relation_nodes,(relation_nnodes+256)*sizeof(node_t));

       relation_nodes[relation_nnodes++]=node_id;
      }
    else if(!strcmp(type,"way"))
      {
       way_t way_id=atoll(ref); /* need long long conversion */

       if(relation_nways && (relation_nways%256)==0)
          relation_ways=(way_t*)realloc((void*)relation_ways,(relation_nways+256)*sizeof(way_t));

       relation_ways[relation_nways++]=way_id;
      }
    else if(!strcmp(type,"relation"))
      {
       relation_t relation_id=atoll(ref); /* need long long conversion */

       if(relation_nrelations && (relation_nrelations%256)==0)
          relation_relations=(relation_t*)realloc((void*)relation_relations,(relation_nrelations+256)*sizeof(relation_t));

       relation_relations[relation_nrelations++]=relation_id;
      }
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the wayType XSD type is seen

  int wayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int wayType_function(const char *_tag_,int _type_,const char *id)
{
 static way_t way_id;

 if(_type_&XMLPARSE_TAG_START)
   {
    nways++;

    if(!(nways%1000))
       printf_middle("Reading: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld",ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    way_nnodes=0;

    /* Handle the way information */

    XMLPARSE_ASSERT_STRING(_tag_,id); way_id=atoll(id); /* need long long conversion */
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&WayRules,current_tags);

    process_way_tags(result,way_id);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the relationType XSD type is seen

  int relationType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int relationType_function(const char *_tag_,int _type_,const char *id)
{
 static relation_t relation_id;

 if(_type_&XMLPARSE_TAG_START)
   {
    nrelations++;

    if(!(nrelations%1000))
       printf_middle("Reading: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld",ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    relation_nnodes=relation_nways=relation_nrelations=0;

    /* Handle the relation information */

    XMLPARSE_ASSERT_STRING(_tag_,id); relation_id=atoll(id); /* need long long conversion */
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&RelationRules,current_tags);

    process_relation_tags(result,relation_id);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the osmType XSD type is seen

  int osmType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int osmType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the XML declaration is seen

  int xmlDeclaration_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseOSM Returns 0 if OK or something else in case of an error.

  FILE *file The file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseOSM(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables. */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 way_nodes=(node_t*)malloc(256*sizeof(node_t));

 relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 nnodes=0,nways=0,nrelations=0;

 printf_first("Reading: Lines=0 Nodes=0 Ways=0 Relations=0");

 retval=ParseXML(file,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_IGNORE);

 printf_last("Read: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld",ParseXML_LineNumber(),nnodes,nways,nrelations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a node.

  TagList *tags The list of node tags.

  node_t id The id of the node.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_node_tags(TagList *tags,node_t id,double latitude,double longitude)
{
 allow_t allow=Allow_ALL;

 int i;

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycle"))
          if(!ISTRUE(v))
             allow&=~Allow_Bicycle;

       break;

      case 'f':
       if(!strcmp(k,"foot"))
          if(!ISTRUE(v))
             allow&=~Allow_Foot;

       break;

      case 'g':
       if(!strcmp(k,"goods"))
          if(!ISTRUE(v))
             allow&=~Allow_Goods;

       break;

      case 'h':
       if(!strcmp(k,"horse"))
          if(!ISTRUE(v))
             allow&=~Allow_Horse;

       if(!strcmp(k,"hgv"))
          if(!ISTRUE(v))
             allow&=~Allow_HGV;

       break;

      case 'm':
       if(!strcmp(k,"moped"))
          if(!ISTRUE(v))
             allow&=~Allow_Moped;

       if(!strcmp(k,"motorbike"))
          if(!ISTRUE(v))
             allow&=~Allow_Motorbike;

       if(!strcmp(k,"motorcar"))
          if(!ISTRUE(v))
             allow&=~Allow_Motorcar;

       break;

      case 'p':
       if(!strcmp(k,"psv"))
          if(!ISTRUE(v))
             allow&=~Allow_PSV;

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
          if(!ISTRUE(v))
             allow&=~Allow_Wheelchair;

       break;

      default:
       ;
      }
   }

 /* Create the node */

 AppendNode(nodes,id,degrees_to_radians(latitude),degrees_to_radians(longitude),allow);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a way.

  TagList *tags The list of way tags.

  way_t id The id of the way.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_way_tags(TagList *tags,way_t id)
{
 Way   way={0};
 int   oneway=0,roundabout=0;
 char *name=NULL,*ref=NULL;

 int i;

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycle"))
          if(ISTRUE(v))
             way.allow|= Allow_Bicycle;

       if(!strcmp(k,"bicycleroute"))
          if(ISTRUE(v))
             way.props|=Properties_BicycleRoute;

       if(!strcmp(k,"bridge"))
          if(ISTRUE(v))
             way.props|=Properties_Bridge;

       break;

      case 'f':
       if(!strcmp(k,"foot"))
          if(ISTRUE(v))
             way.allow|= Allow_Foot;

       if(!strcmp(k,"footroute"))
          if(ISTRUE(v))
             way.props|=Properties_FootRoute;

       break;

      case 'g':
       if(!strcmp(k,"goods"))
          if(ISTRUE(v))
             way.allow|=Allow_Goods;

       break;

      case 'h':
       if(!strcmp(k,"highway"))
          way.type=HighwayType(v);

       if(!strcmp(k,"horse"))
          if(ISTRUE(v))
             way.allow|=Allow_Horse;

       if(!strcmp(k,"hgv"))
          if(ISTRUE(v))
             way.allow|=Allow_HGV;

       break;

      case 'j':
       if(!strcmp(k,"junction") && !strcmp(v,"roundabout"))
          roundabout=1;

       break;

      case 'm':
       if(!strcmp(k,"maxspeed"))
         {
          if(strstr(v,"mph"))
             way.speed=kph_to_speed(1.609*atof(v));
          else
             way.speed=kph_to_speed(atof(v));
         }

       if(!strcmp(k,"maxweight"))
         {
          if(strstr(v,"kg"))
             way.weight=tonnes_to_weight(atof(v)/1000);
          else
             way.weight=tonnes_to_weight(atof(v));
         }

       if(!strcmp(k,"maxheight"))
         {
          if(strchr(v,'\''))
            {
             int feet,inches;

             if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                way.height=metres_to_height((feet+(double)inches/12.0)*0.254);
             else if(sscanf(v,"%d'",&feet)==1)
                way.height=metres_to_height((feet+(double)inches/12.0)*0.254);
            }
          else if(strstr(v,"ft") || strstr(v,"feet"))
             way.height=metres_to_height(atof(v)*0.254);
          else
             way.height=metres_to_height(atof(v));
         }

       if(!strcmp(k,"maxwidth"))
         {
          if(strchr(v,'\''))
            {
             int feet,inches;

             if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                way.width=metres_to_height((feet+(double)inches/12.0)*0.254);
             else if(sscanf(v,"%d'",&feet)==1)
                way.width=metres_to_height((feet+(double)inches/12.0)*0.254);
            }
          else if(strstr(v,"ft") || strstr(v,"feet"))
             way.width=metres_to_width(atof(v)*0.254);
          else
             way.width=metres_to_width(atof(v));
         }

       if(!strcmp(k,"maxlength"))
         {
          if(strchr(v,'\''))
            {
             int feet,inches;

             if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                way.length=metres_to_height((feet+(double)inches/12.0)*0.254);
             else if(sscanf(v,"%d'",&feet)==1)
                way.length=metres_to_height((feet+(double)inches/12.0)*0.254);
            }
          else if(strstr(v,"ft") || strstr(v,"feet"))
             way.length=metres_to_length(atof(v)*0.254);
          else
             way.length=metres_to_length(atof(v));
         }

       if(!strcmp(k,"moped"))
          if(ISTRUE(v))
             way.allow|=Allow_Moped;

       if(!strcmp(k,"motorbike"))
          if(ISTRUE(v))
             way.allow|=Allow_Motorbike;

       if(!strcmp(k,"motorcar"))
          if(ISTRUE(v))
             way.allow|=Allow_Motorcar;

       if(!strcmp(k,"multilane"))
          if(ISTRUE(v))
             way.props|=Properties_Multilane;

       break;

      case 'n':
       if(!strcmp(k,"name"))
          name=v;

       break;

      case 'o':
       if(!strcmp(k,"oneway"))
         {
          if(ISTRUE(v))
             oneway=1;
          else if(!strcmp(v,"-1"))
             oneway=-1;
         }

       break;

      case 'p':
       if(!strcmp(k,"paved"))
          if(ISTRUE(v))
             way.props|=Properties_Paved;

       if(!strcmp(k,"psv"))
          if(ISTRUE(v))
             way.allow|=Allow_PSV;

       break;

      case 'r':
       if(!strcmp(k,"ref"))
          ref=v;

       break;

      case 't':
       if(!strcmp(k,"tunnel"))
          if(ISTRUE(v))
             way.props|=Properties_Tunnel;

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
          if(ISTRUE(v))
             way.allow|=Allow_Wheelchair;

       break;

      default:
       ;
      }
   }

 /* Create the way */

 if(way.type>0 && way.type<Way_Count)
   {
    if(way.allow)
      {
       char *refname;

       if(oneway)
          way.type|=Way_OneWay;

       if(roundabout)
          way.type|=Way_Roundabout;

       if(ref && name)
         {
          refname=(char*)malloc(strlen(ref)+strlen(name)+4);
          sprintf(refname,"%s (%s)",name,ref);
         }
       else if(ref && !name)
          refname=ref;
       else if(!ref && name)
          refname=name;
       else /* if(!ref && !name) */
          refname="";

       AppendWay(ways,id,&way,refname);

       if(ref && name)
          free(refname);

       for(i=1;i<way_nnodes;i++)
         {
          node_t from=way_nodes[i-1];
          node_t to  =way_nodes[i];

          if(oneway>0)
            {
             AppendSegment(segments,id,from,to,ONEWAY_1TO2);
             AppendSegment(segments,id,to,from,ONEWAY_2TO1);
            }
          else if(oneway<0)
            {
             AppendSegment(segments,id,from,to,ONEWAY_2TO1);
             AppendSegment(segments,id,to,from,ONEWAY_1TO2);
            }
          else
            {
             AppendSegment(segments,id,from,to,0);
             AppendSegment(segments,id,to,from,0);
            }
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a relation.

  TagList *tags The list of relation tags.

  relation_t id The id of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_relation_tags(TagList *tags,relation_t id)
{
 allow_t routes=Allow_None;
 int i;

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycleroute"))
          if(ISTRUE(v))
             routes|=Allow_Bicycle;

       break;

      case 'f':
       if(!strcmp(k,"footroute"))
          if(ISTRUE(v))
             routes|=Allow_Foot;

       break;

      default:
       ;
      }
   }

 /* Create the route relation (must store all relations that have ways or
    relations even if they are not routes because they might be referenced by
    other relations that are routes) */

 if(relation_nways || relation_nrelations)
    AppendRouteRelation(relations,id,routes,
                        relation_ways,relation_nways,
                        relation_relations,relation_nrelations);
}
