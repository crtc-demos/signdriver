signdriver
==========

Arduino code to drive a 128x16 LED banner display

The sign is made up of two 64x16 displays, A connected through B, with the red output line of the A conneted to the green input line of B.

```

 .---------------------------. 
 |  _______      _______     |
 | |   A   |<-G-|   B   |<-G-'
 `-|_______|<-R-|_______|<-R------Arduino
 
```
