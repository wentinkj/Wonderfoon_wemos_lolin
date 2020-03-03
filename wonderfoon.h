#ifndef WONDERFOON_H
#define WONDERFOON_H

/* Settings: defaults used upon initialisation or reset
 * 
 */
#define VOLUMEDEFAULT 15
#define FOLDERDEFAULT 1
#define RANDOMDEFAULT false

/* phone type configuration, TDK8591, TDK8574 or ROTARY
 *  uncomment one
 */
//#define TDK8591
#define TDK8574
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

#ifdef TDK8574
#define TDK
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
#define V3PORT    5
#define A1PORT    4
#define A2PORT    3
#define A3PORT    2
#define A4PORT    1
#define EASTERPORT 0
#endif

#ifdef TDK8591
#define TDK
/* i2c address of the expander
 * port numbers below are those of the expander
 * A1 to 4 are the analog input ports for the rows
 * V1ANALOG to V3ANALOG are the ADC values to be read for the different columns
 * this solution requires more components than strictly needed, however with a single ADC port 
 * it was more difficult to get a good stable reading to identify the buttons
 * 
 * 3v3 --o V1 --| 6k8ohm |--o V3 --| 6k8ohm |---o V2
 * 
 * {keypad}---o A1 --| 10kOhm |-- GND
 *         ---o A2 --| 10kOhm |-- GND
 *         ---o A3 --| 10kOhm |-- GND
 *         ---o A4 --| 10kOhm |-- GND
 *  --o A1 ---o pcf8591/port 1
 *  --o A2 ---o pcf8591/port 2
 *  --o A3 ---o pcf8591/port 3
 *  --o A4 ---o pcf8591/port 4
 */
#define EXPANDER  0x48
#define A1PORT    4
#define A2PORT    3
#define A3PORT    2
#define A4PORT    1

// analog values read are V1{179} V2{76} and V3{107}
// using the first 6 bits as identification to allow for some tolerance
#define V1ANALOG  176
#define V2ANALOG  76
#define V3ANALOG  104

// the A1PORT up until the V3PORT are used to set the bits in a byte so that the further handling
// is the same as that of the PCF8574, each row & column is represented by a single bit
#define V1PORT    7
#define V2PORT    6
#define V3PORT    5

#endif

#ifdef TDK
#ifdef ROTARY
#error "Uncomment only ROTARY or TDK to define the type of phone"
#endif
#endif

#endif
