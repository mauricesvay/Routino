#!/usr/bin/perl
#
# Routino data visualiser custom link CGI
#
# Part of the Routino routing software.
#
# This file Copyright 2008,2009 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Use the perl CGI module
use CGI ':cgi';

# Create the query and get the parameters

$query=new CGI;

@rawparams=$query->param;

# Legal CGI parameters with regexp validity check

%legalparams=(
              "lon"  => "[-0-9.]+",
              "lat"  => "[-0-9.]+",
              "zoom" => "[0-9]+"
             );

# Validate the CGI parameters, ignore invalid ones

foreach $key (@rawparams)
  {
   foreach $test (keys (%legalparams))
     {
      if($key =~ m%^$test$%)
        {
         $value=$query->param($key);

         if($value =~ m%^$legalparams{$test}$%)
           {
            $cgiparams{$key}=$value;
            last;
           }
        }
     }
  }

# Open template file and output it

open(TEMPLATE,"<visualiser.html");

# Parse the template and fill in the parameters

print header('text/html');

while(<TEMPLATE>)
  {
   if(m%^<BODY.+>%)
     {
      s/'lat'/$cgiparams{'lat'}/   if(defined $cgiparams{'lat'});
      s/'lon'/$cgiparams{'lon'}/   if(defined $cgiparams{'lon'});
      s/'zoom'/$cgiparams{'zoom'}/ if(defined $cgiparams{'zoom'});
      print;
     }
   else
     {
      print;
     }
  }

close(TEMPLATE);
