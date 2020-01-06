#ifndef WONDERFOON_H
#define WONDERFOON_H

#ifdef DEBUGON
#define debug(text) Serial.println(text)
#define debug1(text) Serial.print(text)
#else
#define debug(text)
#define debug1(text)
#endif

// Settings: defaults
#define VOLUMEDEFAULT 15
#define FOLDERDEFAULT 1
#define RANDOMDEFAULT false

/* Dial: IO configuration
 * connected to phone hook
 * connected to rotary disc to detect the dial start
 * connected to rotary disc to count pulses (dialed digit)
 * connected to MP3 module busy pin to check if still playing
 */
#define HOOKPIN   14
#define DIALPIN   13
#define PULSEPIN  12
#define BUSYPIN   16

// i2c address of the expander
// port numbers below are those of the expander
// V1 to 3 are for the vertical columns
// A1 to 4 are for the rows
#define EXPANDER  0x20
#define V1PORT    7
#define V2PORT    6
#define V3PORT    5
#define A1PORT    4
#define A2PORT    3
#define A3PORT    2
#define A4PORT    1

#endif
