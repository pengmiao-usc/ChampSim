#!/usr/bin/perl
if($#ARGV != 2){
    print "ERROR : Usage : perl arithmean.pl <new val> <old mean> <num elements>\n";
    exit;
}
$new_val=shift;
$mean=shift;
$num_elements=shift;
$weight=(1/$num_elements);

$mean += $new_val * $weight;
printf "%.6f\n", ($mean);
