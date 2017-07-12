#!/usr/bin/perl
if($#ARGV != 1){
    print "ERROR : Usage : perl miss_reduction.pl <baseline> <dut>\n";
    exit;
}

our $baseline_stats_file = shift;
our $dut_stats_file = shift;

my $baseline_misses = compute_num_misses($baseline_stats_file);
my $dut_misses = compute_num_misses($dut_stats_file);

chomp($baseline_misses);
chomp($dut_misses);
#print "$baseline_misses, $dut_misses \n";
  
$miss_reduction = 100*($baseline_misses - $dut_misses)/$baseline_misses;
print "$miss_reduction \n";

sub compute_num_misses
{
    $stats_file = $_[0];   
    my $num_misses = 0;
    foreach (qx[cat $stats_file 2>/dev/null]) {
        $line = $_;
        if ($line =~ m/LLC LOAD[\s\t]+ACCESS:[\s\t]+([\d]+)[\s\t]+HIT:[\s\t]+([\d]+)[\s\t]+MISS:[\s\t]+([\d]+)/) 
        {
            $num_misses = $num_misses + $3;
        }
        
        if ($line =~ m/LLC RFO[\s\t]+ACCESS:[\s\t]+([\d]+)[\s\t]+HIT:[\s\t]+([\d]+)[\s\t]+MISS:[\s\t]+([\d]+)/) 
        {
            $num_misses = $num_misses + $3;
        }
    }
    unless ( defined($num_misses) ) {
        print "ERROR problem with $stats_file\n";
        return $num_misses;
    }
    return $num_misses;
}

