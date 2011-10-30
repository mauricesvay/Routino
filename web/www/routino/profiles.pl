################################################################################
########################### Routino default profile ############################
################################################################################

$routino={ # contains all default Routino options (generated using "--help-profile-perl").

  # Default transport type
  transport => 'motorcar',

  # Transport types
  transports => { foot => 1, horse => 2, wheelchair => 3, bicycle => 4, moped => 5, motorbike => 6, motorcar => 7, goods => 8, hgv => 9, psv => 10 },

  # Highway types
  highways => { motorway => 1, trunk => 2, primary => 3, secondary => 4, tertiary => 5, unclassified => 6, residential => 7, service => 8, track => 9, cycleway => 10, path => 11, steps => 12, ferry => 13 },

  # Property types
  properties => { paved => 1, multilane => 2, bridge => 3, tunnel => 4, footroute => 5, bicycleroute => 6 },

  # Restriction types
  restrictions => { oneway => 1, turns => 2, weight => 3, height => 4, width => 5, length => 6 },

  # Allowed highways
  profile_highway => {
      motorway => { foot =>   0,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>   0,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100 },
         trunk => { foot =>  40,  horse =>  25,  wheelchair =>  40,  bicycle =>  30,  moped =>  90,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100 },
       primary => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  70,  moped => 100,  motorbike =>  90,  motorcar =>  90,  goods =>  90,  hgv =>  90,  psv =>  90 },
     secondary => { foot =>  60,  horse =>  50,  wheelchair =>  60,  bicycle =>  80,  moped =>  90,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80 },
      tertiary => { foot =>  70,  horse =>  75,  wheelchair =>  70,  bicycle =>  90,  moped =>  80,  motorbike =>  70,  motorcar =>  70,  goods =>  70,  hgv =>  70,  psv =>  70 },
  unclassified => { foot =>  80,  horse =>  75,  wheelchair =>  80,  bicycle =>  90,  moped =>  70,  motorbike =>  60,  motorcar =>  60,  goods =>  60,  hgv =>  60,  psv =>  60 },
   residential => { foot =>  90,  horse =>  75,  wheelchair =>  90,  bicycle =>  90,  moped =>  60,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50 },
       service => { foot =>  90,  horse =>  75,  wheelchair =>  90,  bicycle =>  90,  moped =>  80,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80 },
         track => { foot =>  95,  horse => 100,  wheelchair =>  95,  bicycle =>  90,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
      cycleway => { foot =>  95,  horse =>  90,  wheelchair =>  95,  bicycle => 100,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
          path => { foot => 100,  horse => 100,  wheelchair => 100,  bicycle =>  90,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
         steps => { foot =>  80,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
         ferry => { foot =>  20,  horse =>  20,  wheelchair =>  20,  bicycle =>  20,  moped =>  20,  motorbike =>  20,  motorcar =>  20,  goods =>  20,  hgv =>  20,  psv =>  20 }
     },

  # Speed limits
  profile_speed => {
      motorway => { foot =>   0,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>  48,  motorbike => 112,  motorcar => 112,  goods =>  96,  hgv =>  89,  psv =>  89 },
         trunk => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  96,  motorcar =>  96,  goods =>  96,  hgv =>  80,  psv =>  80 },
       primary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  96,  motorcar =>  96,  goods =>  96,  hgv =>  80,  psv =>  80 },
     secondary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  88,  motorcar =>  88,  goods =>  88,  hgv =>  80,  psv =>  80 },
      tertiary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80 },
  unclassified => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  64,  motorcar =>  64,  goods =>  64,  hgv =>  64,  psv =>  64 },
   residential => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  48,  motorcar =>  48,  goods =>  48,  hgv =>  48,  psv =>  48 },
       service => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  32,  motorbike =>  32,  motorcar =>  32,  goods =>  32,  hgv =>  32,  psv =>  32 },
         track => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  16,  motorbike =>  16,  motorcar =>  16,  goods =>  16,  hgv =>  16,  psv =>  16 },
      cycleway => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
          path => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
         steps => { foot =>   4,  horse =>   0,  wheelchair =>   4,  bicycle =>   0,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0 },
         ferry => { foot =>  10,  horse =>  10,  wheelchair =>  10,  bicycle =>  10,  moped =>  10,  motorbike =>  10,  motorcar =>  10,  goods =>  10,  hgv =>  10,  psv =>  10 }
     },

  # Highway properties
  profile_property => {
         paved => { foot =>  50,  horse =>  20,  wheelchair =>  90,  bicycle =>  50,  moped => 100,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100 },
     multilane => { foot =>  25,  horse =>  25,  wheelchair =>  25,  bicycle =>  25,  moped =>  35,  motorbike =>  60,  motorcar =>  60,  goods =>  60,  hgv =>  60,  psv =>  60 },
        bridge => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  50,  moped =>  50,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50 },
        tunnel => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  50,  moped =>  50,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50 },
     footroute => { foot =>  55,  horse =>  50,  wheelchair =>  55,  bicycle =>  50,  moped =>  50,  motorbike =>  50,  motorcar =>  45,  goods =>  45,  hgv =>  45,  psv =>  45 },
  bicycleroute => { foot =>  55,  horse =>  50,  wheelchair =>  55,  bicycle =>  60,  moped =>  50,  motorbike =>  50,  motorcar =>  45,  goods =>  45,  hgv =>  45,  psv =>  45 }
     },

  # Restrictions
  profile_restrictions => {
          oneway => { foot =>    0,  horse =>    1,  wheelchair =>    0,  bicycle =>    1,  moped =>    1,  motorbike =>    1,  motorcar =>    1,  goods =>    1,  hgv =>    1,  psv =>    1 },
           turns => { foot =>    0,  horse =>    1,  wheelchair =>    0,  bicycle =>    1,  moped =>    1,  motorbike =>    1,  motorcar =>    1,  goods =>    1,  hgv =>    1,  psv =>    1 },
          weight => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  5.0,  hgv => 10.0,  psv => 15.0 },
          height => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  2.5,  hgv =>  3.0,  psv =>  3.0 },
           width => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  2.0,  hgv =>  2.5,  psv =>  2.5 },
          length => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  5.0,  hgv =>  6.0,  psv =>  6.0 }
     }

}; # end of routino variable

1;
