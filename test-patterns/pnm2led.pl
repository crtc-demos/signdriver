#!/usr/bin/perl

my $filename = $ARGV[0];
if ( $filename eq "" ) { die "No filename."; } 

open(B, $filename);
my $dummy = <B>;
$dummy = <B>;
$dummy = <B>;
my $max = <B>; chomp($max);
my @g = ();
for ( my $y = 0; $y < 16; $y++ ) {
	for ( my $x = 0; $x < 128; $x++ ) {
		for ( my $c = 0; $c < 3; $c++ ) { 
			my $line = <B>; chomp($line);
			if ( $line =~ /^\s*$/ ) { $c--; next; } 
			if ( $c == 0 ) { $r[127-$x][15-$y] = $line; }
			if ( $c == 1 ) { $g[127-$x][15-$y] = $line; }
			}
		}
	}
close(B);

for ( my $y = 0; $y < 16; $y++ ) {
	for ( my $x = 0; $x < 128; $x++ ) {
		$r[$x][$y] /= $max;
		$g[$x][$y] /= $max;
		}
	}

# Dither it!
for ( my $y = 0; $y < 16; $y++ ) {
	for ( my $x = 0; $x < 128; $x++ ) {
		my $oldr = $r[$x][$y];
		my $newr = 0;
		if ( $oldr >= 0.25  ) { $newr = 0.5; } 
		if ( $oldr >= 0.75  ) { $newr = 1; } 
		$r[$x][$y] = $newr;
		my $qer = $oldr - $newr;

		$r[$x+1][$y] = $r[$x+1][$y] + $qer * 7/16;
		$r[$x-1][$y+1] = $r[$x-1][$y+1] + $qer * 3/16;
		$r[$x][$y+1] = $r[$x][$y+1] + $qer * 5/16;
		$r[$x+1][$y+1] = $r[$x+1][$y+1] + $qer * 1/16;

		my $oldg = $g[$x][$y];
		my $newg = 0;
		if ( $oldg >= 0.25  ) { $newg = 0.5; } 
		if ( $oldg >= 0.75  ) { $newg = 1; } 
		$g[$x][$y] = $newg;
		my $qeg = $oldg - $newg;

		$g[$x+1][$y] = $g[$x+1][$y] + $qeg * 7/16;
		$g[$x-1][$y+1] = $g[$x-1][$y+1] + $qeg * 3/16;
		$g[$x][$y+1] = $g[$x][$y+1] + $qeg * 5/16;
		$g[$x+1][$y+1] = $g[$x+1][$y+1] + $qeg * 1/16;
		}
	}

for ( my $s = 0; $s < 2; $s++ ) { 
	for ( my $y = 0; $y < 16; $y++ ) {
		my $rl = "";
		my $gl = "";
		for ( my $x1 = 0; $x1 < 128; $x1 += 4 ) {
			my $thisbitr = 0;
			my $thisbitg = 0;
			for ( my $x2 = 0; $x2 < 4; $x2++ ) {

				my $x = $x1 + $x2;

				$rq = 0;
				if ( $r[$x][$y] >= 0.25 ) { $rq = 0.5; } 
				if ( $r[$x][$y] >= 0.75 ) { $rq = 1; } 

				$gq = 0;
				if ( $g[$x][$y] >= 0.25 ) { $gq = 0.5; } 
				if ( $g[$x][$y] >= 0.75 ) { $gq = 1; } 

				my $theta = $s * 0.5 + 0.25;
				
				if ( $rq > $theta ) { 
					$thisbitr ^= 1 << $x2;
					}
				if ( $gq > $theta ) { 
					$thisbitg ^= 1 << $x2;
					}

				}
			$rl .= sprintf("%X", $thisbitr);
			$gl .= sprintf("%X", $thisbitg);
			}
		print $gl.$rl;
		}
	}
