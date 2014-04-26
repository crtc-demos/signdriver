#!/usr/bin/perl

my @g = ();
my @r = ();

for ( my $x = 0; $x < 128; $x++ ) { 
	for ( my $y = 0; $y < 16; $y++ ) { 

		my $dl = 0;
		$dl += ( $x ) * ( $x );
		$dl += ( $y - 7.5 ) * ( $y - 7.5 );
		$dl = sqrt($dl) / 150;

		if ( $dl > 1 ) { $dl = 1; } 
		if ( $dl < 0 ) { $dl < 0; } 

		my $dr = 0;
		$dr += ( $x - 127 ) * ( $x - 127 );
		$dr += ( $y - 7.5 ) * ( $y - 7.5 );
		$dr = sqrt($dr) / 150;

		if ( $dr > 1 ) { $dr = 1; } 
		if ( $dr < 0 ) { $dr < 0; } 

		$g[$x][$y] = 1 - $dl;
		$r[$x][$y] = 1 - $dr;
		}
	}

for ( my $y = 0; $y < 16; $y++ ) { 
	for ( my $x = 0; $x < 128; $x++ ) { 

		my $old = $g[$x][$y];
		my $new = 0;
		if ( $g[$x][$y] > 0.5 ) { $new = 1; } 
		$g[$x][$y] = $new;
		$qe = $old - $new;
		$g[$x+1][$y] = $g[$x+1][$y] + $qe * 7/16;
		$g[$x-1][$y+1] = $g[$x-1][$y+1] + $qe * 3/16;
		$g[$x][$y+1] = $g[$x][$y+1] + $qe * 5/16;
		$g[$x+1][$y+1] = $g[$x+1][$y+1] + $qe * 1/16;


		$old = $r[$x][$y];
		$new = 0;
		if ( $r[$x][$y] > 0.5 ) { $new = 1; } 
		$r[$x][$y] = $new;
		$qe = $old - $new;
		$r[$x+1][$y] = $r[$x+1][$y] + $qe * 7/16;
		$r[$x-1][$y+1] = $r[$x-1][$y+1] + $qe * 3/16;
		$r[$x][$y+1] = $r[$x][$y+1] + $qe * 5/16;
		$r[$x+1][$y+1] = $r[$x+1][$y+1] + $qe * 1/16;


		}
	}

for ( my $oy = 0; $oy < 32; $oy++ ) { 
	my $l = "";
	my $y = $oy % 16;
	my $m = 1;
	if ( $oy >= 24 ) { $m = 0; } 
	for ( my $x = 0; $x < 128; $x += 4 ) { 
		my $hd = 0;
		$hd += $m * $g[$x+0][$y] * 8;
		$hd += $m * $g[$x+1][$y] * 4;
		$hd += $m * $g[$x+2][$y] * 2;
		$hd += $m * $g[$x+3][$y] * 1;
		$l .= sprintf("%X", $hd);
		}
	for ( my $x = 0; $x < 128; $x += 4 ) { 
		my $hd = 0;
		$hd += $m * $r[$x+0][$y] * 8;
		$hd += $m * $r[$x+1][$y] * 4;
		$hd += $m * $r[$x+2][$y] * 2;
		$hd += $m * $r[$x+3][$y] * 1;
		$l .= sprintf("%X", $hd);
		}
	print $l;
	$l = "";
	}
print "\n";
