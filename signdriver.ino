/*   oo   oo  * signdriver
 *  o    o    *
 *  ooo  o    * Arduino code to drive a 128x16 LED banner display
 *            *
 *  ooo   oo  *
 *   o   o    *
 *   o   ooo  */

#include  <SPI.h>

#define PIN_L 2
#define PIN_EN 3
#define PIN_D 4
#define PIN_C 5
#define PIN_B 6
#define PIN_A 7
#define PIN_R1 11
#define PIN_S 13

#define DELAY_MS 2
#define DELAY_SCAN 1

void setup() {
  //pinMode(PIN_S, OUTPUT);
  //pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_EN, OUTPUT);
  pinMode(PIN_L, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_D, OUTPUT);
    
  digitalWrite(PIN_L, LOW);
  digitalWrite(PIN_S, HIGH);

  /*pinMode (13, OUTPUT);
  digitalWrite (13, HIGH);*/
  
  SPI.setBitOrder(LSBFIRST);
  // CPOL=1 (base value of clock is 1), CPHA=1 (data propagated on falling edge), 
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  
  Serial.begin(115200);
}

static unsigned char rows[2][16][32];
static int cur_screen = 0;
static int cur_row = 0;
static int cur_nybble = 0;
static int cur_byte = 0;
static int disp_screen = 0;
static int power_switch = 0x00;
static int cycles = 4;

int decodechar (char c)
{
  return
    (c >= '0' && c <= '9') ? c - '0' :
    (c >= 'A' && c <= 'F') ? c - 'A' + 10 : 
    (c >= 'a' && c <= 'f') ? c - 'a' + 10 : 
    -1;
}

int scroll_up (int x0, int y0, int x1, int y1)
{ 
  if ( x0 > x1 ) { return -1; } 
  if ( y0 > y1 ) { return -1; } 
  if ( x0 < 0 ) { x0 = 0; } 
  if ( x0 > 15 ) { x0 = 15; } 
  if ( x1 < 0 ) { x1 = 0; } 
  if ( x1 > 15 ) { x1 = 15; } 
  if ( y0 < 0 ) { y0 = 0; } 
  if ( y0 > 15 ) { y0 = 15; } 
  if ( y1 < 0 ) { y1 = 0; } 
  if ( y1 > 15 ) { y1 = 15; } 
  
  for ( int s = 0; s < 2; s++ )
  { 
    for ( int y = y0 + 1; y <= y1; y++ )
    { 
      for ( int x = x0; x <= x1; x++ )
      {
        rows[s][y-1][x] = rows[s][y][x];
        rows[s][y-1][x+16] = rows[s][y][x+16];
      }
    }
    for ( int x = x0; x <= x1; x++ )
    {
      rows[s][y1][x] = 0;
      rows[s][y1][x+16] = 0;
    }
  }
}

int scroll_down (int x0, int y0, int x1, int y1)
{ 
  if ( x0 > x1 ) { return -1; } 
  if ( y0 > y1 ) { return -1; } 
  if ( x0 < 0 ) { x0 = 0; } 
  if ( x0 > 15 ) { x0 = 15; } 
  if ( x1 < 0 ) { x1 = 0; } 
  if ( x1 > 15 ) { x1 = 15; } 
  if ( y0 < 0 ) { y0 = 0; } 
  if ( y0 > 15 ) { y0 = 15; } 
  if ( y1 < 0 ) { y1 = 0; } 
  if ( y1 > 15 ) { y1 = 15; } 
  
  for ( int s = 0; s < 2; s++ )
  { 
    for ( int y = y1 - 1; y >= y0; y-- )
    { 
      for ( int x = x0; x <= x1; x++ )
      {
        rows[s][y+1][x] = rows[s][y][x];
        rows[s][y+1][x+16] = rows[s][y][x+16];
      }
    }
    for ( int x = x0; x <= x1; x++ )
    {
      rows[s][y0][x] = 0;
      rows[s][y0][x+16] = 0;
    }
  }
}

int scroll_right (int x0, int y0, int x1, int y1)
{ 
  if ( x0 > x1 ) { return -1; } 
  if ( y0 > y1 ) { return -1; } 
  if ( x0 < 0 ) { x0 = 0; } 
  if ( x0 > 15 ) { x0 = 15; } 
  if ( x1 < 0 ) { x1 = 0; } 
  if ( x1 > 15 ) { x1 = 15; } 
  if ( y0 < 0 ) { y0 = 0; } 
  if ( y0 > 15 ) { y0 = 15; } 
  if ( y1 < 0 ) { y1 = 0; } 
  if ( y1 > 15 ) { y1 = 15; } 
  
  for ( int s = 0; s < 2; s++ )
  { 
    for ( int y = y0; y <= y1; y++ )
    { 
      int last_lsb = 0;
      for ( int x = x0; x <= x1; x++ )
      {
        int t = rows[s][y][x] >> 7;        
        rows[s][y][x] <<= 1;
        rows[s][y][x] |= last_lsb;
        last_lsb = t;
      }
      last_lsb = 0;
      for ( int x = x0+16; x <= x1+16; x++ )
      {
        int t = rows[s][y][x] >> 7;        
        rows[s][y][x] <<= 1;
        rows[s][y][x] |= last_lsb;
        last_lsb = t;
      }
    }
  }
}

int scroll_left (int x0, int y0, int x1, int y1)
{ 
  if ( x0 > x1 ) { return -1; } 
  if ( y0 > y1 ) { return -1; } 
  if ( x0 < 0 ) { x0 = 0; } 
  if ( x0 > 15 ) { x0 = 15; } 
  if ( x1 < 0 ) { x1 = 0; } 
  if ( x1 > 15 ) { x1 = 15; } 
  if ( y0 < 0 ) { y0 = 0; } 
  if ( y0 > 15 ) { y0 = 15; } 
  if ( y1 < 0 ) { y1 = 0; } 
  if ( y1 > 15 ) { y1 = 15; } 
  
  for ( int s = 0; s < 2; s++ )
  { 
    for ( int y = y0; y <= y1; y++ )
    { 
      int last_msb = 0;
      for ( int x = x1; x >= x0; x-- )
      {
        int t = ( rows[s][y][x] & 1 ) << 7;        
        rows[s][y][x] >>= 1;
        rows[s][y][x] |= last_msb;
        last_msb = t;
      }
      last_msb = 0;
      for ( int x = x1+16; x >= x0+16; x-- )
      {
        int t = ( rows[s][y][x] & 1  ) << 7;        
        rows[s][y][x] >>= 1;
        rows[s][y][x] |= last_msb;
        last_msb = t;
      }
    }
  }
}

void loop() {
  
  while (Serial.available() > 0)
  {
    int x, incomingByte = Serial.read();
    if ( incomingByte == 'R' || incomingByte == 'r' )  /* 'reset' */
    {
      cur_screen = 0;
      cur_row = 0;
      cur_nybble = 0;
      cur_byte = 0;
      power_switch = 0x00;
      continue;
    }
    if ( incomingByte == 'S' || incomingByte == 's' )  /* 'show' */
    {
      power_switch = 0xff;
      continue;
    }
    if ( incomingByte == 'Q' || incomingByte == 'q' )  /* increase half-bright brightness */
    {
      cycles <<= 1; if ( cycles == 0 ) { cycles = 1; } 
      continue;
    }
    if ( incomingByte == 'W' || incomingByte == 'w' )  /* decrease half-bright brightness */
    {
      cycles >>= 1; if ( cycles == 0 ) { cycles = 1; } 
      continue;
    }
    if ( incomingByte == 'H' || incomingByte == 'h' )  /* 'hide' */
    {
      power_switch = 0x00;
      continue;
    }
    if ( incomingByte == 'U' || incomingByte == 'u' )  /* scroll 'up' */
    {
      scroll_left(2, 1, 9, 13); continue;
    }    
    if ( incomingByte == 'V' || incomingByte == 'v' )  /* scroll 'down' */
    {
      scroll_left(0, 0, 15, 15); continue;
    }
    x = decodechar (incomingByte);
    if (x == -1)
      continue;
    if (cur_nybble == 0)
      rows[cur_screen][cur_row][cur_byte] = x;
    else
      rows[cur_screen][cur_row][cur_byte] |= x << 4;
    cur_nybble++;
    if (cur_nybble == 2)
    {
      cur_nybble = 0;
      cur_byte++;
      if (cur_byte == 32)
      {
        cur_byte = 0;
        cur_row++;
        if (cur_row == 16 )
        {
          cur_row = 0;
            if ( cur_screen == 1 )
            {
              power_switch = 0xff;
            }
          cur_screen = 1-cur_screen;
        }
      }
    }
  }
  
  disp_screen = (disp_screen+1) % cycles;
  for ( int y = 0; y < 16  ; y++ )
  {
    for (int byt = 0; byt < 32; byt++)
    {
      SPI.transfer ((rows[disp_screen != 0][y][byt] & power_switch ) ^ 0xff );
    }

    digitalWrite(PIN_EN, HIGH);

    digitalWrite (PIN_A, (y & 1) ? HIGH : LOW);
    digitalWrite (PIN_B, (y & 2) ? HIGH : LOW);
    digitalWrite (PIN_C, (y & 4) ? HIGH : LOW);
    digitalWrite (PIN_D, (y & 8) ? HIGH : LOW);

    digitalWrite(PIN_L, HIGH);
    delayMicroseconds(DELAY_SCAN);
    digitalWrite(PIN_L, LOW);      
 
    digitalWrite(PIN_EN, LOW);
  }
}
