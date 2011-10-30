#!/usr/bin/perl

die "Usage: $0 [-v] < <error-log-file>\n" if($#ARGV>0);

$verbose=0;
$verbose=1 if($#ARGV==0 && $ARGV[0] eq "-v");

# Read in each line from the error log and store them

%errors=();
%errorids=();

while(<STDIN>)
  {
   s%\r*\n%%;

   undef $errorid;

   if(m%Node ([0-9]+)%)           # Generic node
     {
      $errorid=$1;
      s%Node [0-9]+%Node <node-id>%g;
     }

   if(m%Way ([0-9]+)%)            # Generic way
     {
      $errorid=$1;
      s%Way [0-9]+%Way <way-id>%g;
     }

   if(m%Relation ([0-9]+)%)       # Generic relation
     {
      $errorid=$1;
      s%Relation [0-9]+%Relation <relation-id>%g;
     }

   if(m%Nodes [0-9]+ and [0-9]+%i) # Special case nodes
     {
      s%Nodes [0-9]+ and [0-9]+%Nodes <node-id1> and <node-id2>%gi;
     }

   $errors{$_}++;

   if($verbose && defined $errorid)
     {
      if(defined $errorids{$_})
        {
         $errorids{$_}.=",$errorid";
        }
      else
        {
         $errorids{$_}="$errorid";
        }
     }
  }

# Print out the results

foreach $error (sort { $errors{$b} <=> $errors{$a} } (keys %errors))
  {
   printf "%9d : $error\n",$errors{$error};

   if($verbose && defined $errorids{$error})
     {
      print "            $errorids{$error}\n";
     }
  }
