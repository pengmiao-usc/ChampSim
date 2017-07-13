#!/usr/bin/perl
if($#ARGV != 1){
    print "ERROR : Usage : perl speedup.pl <baseline> <dut>\n";
    exit;
}

our $baseline_stats_file = shift;
our $dut_stats_file = shift;

my $baseline_cpi = compute_cpi($baseline_stats_file);
my $dut_cpi = compute_cpi($dut_stats_file);

chomp($baseline_cpi);
chomp($dut_cpi);
#print "$baseline_cpi, $dut_cpi \n";
  
$speedup = 100*($baseline_cpi - $dut_cpi)/$baseline_cpi;
#$speedup = 0.00001 if $speedup < 0.00001;

print "$speedup \n";

sub compute_cpi
{
    $stats_file = $_[0];   
    my $cpi = 0.0;

    foreach (qx[cat $stats_file 2>/dev/null]) {
        $line = $_;

        if ($line =~ m/CPU 0 cumulative IPC: [\d\.]+ instructions: [\d]+ cycles: ([\d]+)/) {
            $cpi = 0.0 + $1;
            last;
        }
    }
    unless ( defined($cpi) ) {
        print "ERROR problem with $stats_file\n";
        return $cpi;
    }
    return $cpi;
}
