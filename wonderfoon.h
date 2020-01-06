#ifndef WONDERFOON_H
#define WONDERFOON_H

#define VERSION "0.0.1"

#define DEBUGON

#ifdef DEBUGON
#define debug(text) Serial.println(text)
#define debug1(text) Serial.print(text)
#else
#define debug(text)
#define debug1(text)
#endif

/* Settings: defaults used upon initialisation or reset
 * 
 */
#define VOLUMEDEFAULT 15
#define FOLDERDEFAULT 1
#define RANDOMDEFAULT false

/* phone type configuration, TDK or ROTARY
 *  uncomment one
 */
#define TDK
//#define ROTARY

/* hookpin and busypin are used in TDK & ROTARY
 * connected to phone hook
 * connected to MP3 module busy pin to check if still playing
 */
#define HOOKPIN   14
#define BUSYPIN   16

/* ROTARY 
 * connected to rotary disc to detect the dial start
 * connected to rotary disc to count pulses (dialed digit)
 */
#ifdef ROTARY
#define DIALPIN   13
#define PULSEPIN  12
#endif

#ifdef TDK
/* i2c address of the expander
 * port numbers below are those of the expander
 * V1 to 3 are for the vertical columns
 * A1 to 4 are for the rows
 * optionally connect the earthpin to a port on the expander, then uncomment the define
 *    for easter egg (play a special feedback upon pressing the aardtoets https://nl.wikipedia.org/wiki/Aardtoets)
 */
#define EXPANDER  0x20
#define V1PORT    7
#define V2PORT    6
#define V3PORT    3
#define A1PORT    5
#define A2PORT    4
#define A3PORT    2
#define A4PORT    1
#define EASTERPORT 0
#endif

#ifdef TDK
#ifdef ROTARY
#error "Uncomment only ROTARY or TDK to define the type of phone"
#endif
#endif

#endif
