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
#define OFFCYCLES 7

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

int decodechar (char c)
{
  return
    (c >= '0' && c <= '9') ? c - '0' :
    (c >= 'A' && c <= 'F') ? c - 'A' + 10 : 
    (c >= 'a' && c <= 'f') ? c - 'a' + 10 : 
    -1;
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
    if ( incomingByte == 'H' || incomingByte == 'h' )  /* 'hide' */
    {
      power_switch = 0x00;
      continue;
    }
    if ( incomingByte == 'U' || incomingByte == 'u' )  /* scroll 'up' */
    {
      for ( int s = 0; s < 2; s++ )
      { 
        for ( int y = 1; y < 16; y++ )
        { 
          for ( int x = 0; x < 32; x++ )
          {
            rows[s][y-1][x] = rows[s][y][x];
          }
        }
        for ( int x = 0; x < 32; x++ )
        {
          rows[s][15][x] = 0;
        }
      }
    }    
    if ( incomingByte == 'V' || incomingByte == 'v' )  /* scroll 'down' */
    {
      for ( int s = 0; s < 2; s++ )
      { 
        for ( int y = 14; y >= 0; y-- )
        { 
          for ( int x = 0; x < 32; x++ )
          {
            rows[s][y+1][x] = rows[s][y][x];
          }
        }
        for ( int x = 0; x < 32; x++ )
        {
          rows[s][0][x] = 0;
        }
      }
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
  
  disp_screen = (disp_screen+1) & OFFCYCLES;
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
