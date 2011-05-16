/***************************************
 $Header: /home/amb/routino/src/RCS/types.h,v 1.48 2010/09/17 17:43:41 amb Exp $

 Type definitions

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


#ifndef TYPES_H
#define TYPES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>
#include <math.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/* Constants and macros for handling them */

/*+ An undefined node index. +*/
#define NO_NODE        (~(index_t)0)

/*+ An undefined segment index. +*/
#define NO_SEGMENT     (~(index_t)0)

/*+ An undefined way index. +*/
#define NO_WAY         (~(index_t)0)


/*+ The lowest number allowed for a fake node. +*/
#define NODE_FAKE      ((index_t)0xffff0000)

/*+ The lowest number allowed for a fake segment. +*/
#define SEGMENT_FAKE   ((index_t)0xffff0000)


/*+ The latitude and longitude conversion factor from floating point (radians) to integer. +*/
#define LAT_LONG_SCALE (1024*65536)

/*+ The latitude and longitude integer range within each bin. +*/
#define LAT_LONG_BIN   65536

/*+ A flag to mark a node as a super-node. +*/
#define NODE_SUPER     ((uint16_t)0x8000)


/*+ A flag to mark a segment as one-way from node1 to node2. +*/
#define ONEWAY_1TO2    ((distance_t)0x80000000)

/*+ A flag to mark a segment as one-way node2 to node1. +*/
#define ONEWAY_2TO1    ((distance_t)0x40000000)

/*+ A flag to mark a segment as a super-segment. +*/
#define SEGMENT_SUPER  ((distance_t)0x20000000)

/*+ A flag to mark a segment as a normal segment. +*/
#define SEGMENT_NORMAL ((distance_t)0x10000000)

/*+ The real distance ignoring the ONEWAY_* and SEGMENT_* flags. +*/
#define DISTANCE(xx)   ((distance_t)((xx)&(~(ONEWAY_1TO2|ONEWAY_2TO1|SEGMENT_SUPER|SEGMENT_NORMAL))))

/*+ The distance flags selecting only the ONEWAY_* and SEGMENT_* flags. +*/
#define DISTFLAG(xx)   ((distance_t)((xx)&(ONEWAY_1TO2|ONEWAY_2TO1|SEGMENT_SUPER|SEGMENT_NORMAL)))


/*+ A very large almost infinite distance. +*/
#define INF_DISTANCE   DISTANCE(~0)

/*+ A very large almost infinite score. +*/
#define INF_SCORE      (score_t)1E30


/* Simple Types */


/*+ A node, segment or way index. +*/
typedef uint32_t index_t;


/*+ A node latitude or longitude. +*/
typedef int32_t  latlong_t;

/*+ A node latitude or longitude bin number. +*/
typedef int16_t  ll_bin_t;

/*+ A node latitude or longitude offset. +*/
typedef uint16_t ll_off_t;


/*+ Conversion from a latlong (integer latitude or longitude) to a bin number. +*/
#define latlong_to_bin(xxx) (ll_bin_t)((latlong_t)((xxx)&~(LAT_LONG_BIN-1))/LAT_LONG_BIN)

/*+ Conversion from a bin number to a latlong (integer latitude or longitude). +*/
#define bin_to_latlong(xxx) ((latlong_t)(xxx)*LAT_LONG_BIN)

/*+ Conversion from a latlong (integer latitude or longitude) to a bin offset. +*/
#define latlong_to_off(xxx) (ll_off_t)((latlong_t)(xxx)&(LAT_LONG_BIN-1))

/*+ Conversion from a bin offset to a latlong (integer latitude or longitude). +*/
#define off_to_latlong(xxx) ((latlong_t)(xxx))


/*+ Conversion from a latitude or longitude in radians to a latlong (integer latitude or longitude). +*/
#define radians_to_latlong(xxx) ((latlong_t)floor((xxx)*LAT_LONG_SCALE))

/*+ Conversion from a latlong (integer latitude or longitude) to a latitude or longitude in radians. +*/
#define latlong_to_radians(xxx) ((double)(xxx)/LAT_LONG_SCALE)


/*+ Conversion from radians to degrees. +*/
#define radians_to_degrees(xxx) ((xxx)*(180.0/M_PI))

/*+ Conversion from degrees to radians. +*/
#define degrees_to_radians(xxx) ((xxx)*(M_PI/180.0))


/*+ A distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A duration, measured in 1/10th seconds. +*/
typedef uint32_t duration_t;

/*+ A routing optimisation score. +*/
typedef float score_t;


/*+ Conversion from distance_t to kilometres. +*/
#define distance_to_km(xx) ((double)(xx)/1000.0)

/*+ Conversion from metres to distance_t. +*/
#define km_to_distance(xx) ((distance_t)((double)(xx)*1000.0))

/*+ Conversion from duration_t to minutes. +*/
#define duration_to_minutes(xx) ((double)(xx)/600.0)

/*+ Conversion from duration_t to hours. +*/
#define duration_to_hours(xx)   ((double)(xx)/36000.0)

/*+ Conversion from hours to duration_t. +*/
#define hours_to_duration(xx)   ((duration_t)((double)(xx)*36000.0))

/*+ Conversion from distance_t and speed_t to duration_t. +*/
#define distance_speed_to_duration(xx,yy) ((duration_t)(((double)(xx)/(double)(yy))*(36000.0/1000.0)))


/*+ The type of a way. +*/
typedef uint8_t waytype_t;

/*+ The different types of a way. +*/
typedef enum _Highway
 {
  Way_Motorway    = 1,
  Way_Trunk       = 2,
  Way_Primary     = 3,
  Way_Secondary   = 4,
  Way_Tertiary    = 5,
  Way_Unclassified= 6,
  Way_Residential = 7,
  Way_Service     = 8,
  Way_Track       = 9,
  Way_Cycleway    =10,
  Way_Path        =11,
  Way_Steps       =12,
  Way_Ferry       =13,

  Way_Count       =14,       /* One more than the number of highway types. */

  Way_OneWay      =32,
  Way_Roundabout  =64
 }
 Highway;

#define HIGHWAY(xx) ((xx)&0x1f)


/*+ The different methods of transport. +*/
typedef enum _Transport
 {
  Transport_None       =  0,

  Transport_Foot       =  1,
  Transport_Horse      =  2,
  Transport_Wheelchair =  3,
  Transport_Bicycle    =  4,
  Transport_Moped      =  5,
  Transport_Motorbike  =  6,
  Transport_Motorcar   =  7,
  Transport_Goods      =  8,
  Transport_HGV        =  9,
  Transport_PSV        = 10,

  Transport_Count      = 11     /*+ One more than the number of transport types. +*/
 }
 Transport;


/*+ The allowed traffic on a way. +*/
typedef uint16_t allow_t;

#define ALLOWED(xx)  (1<<((xx)-1))

/*+ The different allowed traffic on a way. +*/
typedef enum _Allowed
 {
  Allow_None       = 0,

  Allow_Foot       = ALLOWED(Transport_Foot      ),
  Allow_Horse      = ALLOWED(Transport_Horse     ),
  Allow_Wheelchair = ALLOWED(Transport_Wheelchair),
  Allow_Bicycle    = ALLOWED(Transport_Bicycle   ),
  Allow_Moped      = ALLOWED(Transport_Moped     ),
  Allow_Motorbike  = ALLOWED(Transport_Motorbike ),
  Allow_Motorcar   = ALLOWED(Transport_Motorcar  ),
  Allow_Goods      = ALLOWED(Transport_Goods     ),
  Allow_HGV        = ALLOWED(Transport_HGV       ),
  Allow_PSV        = ALLOWED(Transport_PSV       ),

  Allow_ALL        = 65535
 }
 Allowed;


/*+ The individual properties of a highway. +*/
typedef enum _Property
 {
  Property_None         = 0,

  Property_Paved        = 1,
  Property_Multilane    = 2,
  Property_Bridge       = 3,
  Property_Tunnel       = 4,
  Property_FootRoute    = 5,
  Property_BicycleRoute = 6,

  Property_Count        = 7       /* One more than the number of property types. */
 }
 Property;


/*+ The combined set of properties of a way. +*/
typedef uint8_t wayprop_t;

#define PROPERTIES(xx)  (1<<((xx)-1))

/*+ The different properties of a way. +*/
typedef enum _Properties
 {
  Properties_None         = 0,

  Properties_Paved        = PROPERTIES(Property_Paved),
  Properties_Multilane    = PROPERTIES(Property_Multilane),
  Properties_Bridge       = PROPERTIES(Property_Bridge),
  Properties_Tunnel       = PROPERTIES(Property_Tunnel),
  Properties_FootRoute    = PROPERTIES(Property_FootRoute),
  Properties_BicycleRoute = PROPERTIES(Property_BicycleRoute),

  Properties_ALL          = 255
 }
 Properties;


/*+ The speed limit of a way, measured in km/hour. +*/
typedef uint8_t speed_t;

/*+ The maximum weight of a way, measured in 0.2 tonnes. +*/
typedef uint8_t weight_t;

/*+ The maximum height of a way, measured in 0.1 metres. +*/
typedef uint8_t height_t;

/*+ The maximum width of a way, measured in 0.1 metres. +*/
typedef uint8_t width_t;

/*+ The maximum length of a way, measured in 0.1 metres. +*/
typedef uint8_t length_t;


/*+ Conversion of km/hr to speed_t. +*/
#define kph_to_speed(xxx)      (speed_t)(xxx)

/*+ Conversion of speed_t to km/hr. +*/
#define speed_to_kph(xxx)      (int)(xxx)

/*+ Conversion of tonnes to weight_t. +*/
#define tonnes_to_weight(xxx)  (weight_t)((xxx)*5)

/*+ Conversion of weight_t to tonnes. +*/
#define weight_to_tonnes(xxx)  ((double)(xxx)/5.0)

/*+ Conversion of metres to height_t. +*/
#define metres_to_height(xxx)  (height_t)((xxx)*10)

/*+ Conversion of height_t to metres. +*/
#define height_to_metres(xxx)  ((double)(xxx)/10.0)

/*+ Conversion of metres to width_t. +*/
#define metres_to_width(xxx)   (width_t)((xxx)*10)

/*+ Conversion of width_t to metres. +*/
#define width_to_metres(xxx)   ((double)(xxx)/10.0)

/*+ Conversion of metres to length_t. +*/
#define metres_to_length(xxx)  (length_t)((xxx)*10)

/*+ Conversion of length_t to metres. +*/
#define length_to_metres(xxx)  ((double)(xxx)/10.0)


/* Data structures */

typedef struct _Node Node;

typedef struct _Nodes Nodes;

typedef struct _Segment Segment;

typedef struct _Segments Segments;

typedef struct _Way Way;

typedef struct _Ways Ways;


/* Functions */

Highway HighwayType(const char *highway);
Transport TransportType(const char *transport);
Property PropertyType(const char *property);

const char *HighwayName(Highway highway);
const char *TransportName(Transport transport);
const char *PropertyName(Property property);

const char *AllowedNameList(allow_t allowed);
const char *PropertiesNameList(wayprop_t properties);

const char *HighwayList(void);
const char *TransportList(void);
const char *PropertyList(void);


#endif /* TYPES_H */
