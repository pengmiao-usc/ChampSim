#!/usr/bin/perl
if($#ARGV != 2){
    print "ERROR : Usage : perl geomean.pl <new val> <old mean> <num elements>\n";
    exit;
}
$new_val=shift;
$mean=shift;
$num_elements=shift;
$weight=(1/$num_elements);
#if($new_val < 0.000001){
#    $new_val = 0.000001;
#}
$mean *= (100+$new_val) ** $weight;
printf "%.6f\n", ($mean);
