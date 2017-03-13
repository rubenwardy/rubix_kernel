#ifndef __SP804_H
#define __SP804_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RSVD(x,y,z) uint8_t reserved##x[ z - y + 1 ];

/* The ARM Dual-Timer Module (SP804) is documented at
 * 
 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0271d/index.html
 * 
 * In particular, Section 3 explains the programmer's model, i.e., how to 
 * interact with it: this includes 
 * 
 * - Section 3.2, which summarises the device register layout in Table 3.1
 *   (including an offset from the device base address, in the memory map,
 *   for each register), and
 * - Section 3.3, which summarises the internal structure of each device
 *   register.
 * 
 * Note that the field identifiers used here follow the documentation in a
 * general sense, but with a some minor alterations to improve clarity and
 * consistency.
 */

typedef volatile struct {
  uint32_t Timer1Load;       // 0x0000          :            load
  uint32_t Timer1Value;      // 0x0004          : current value
  uint32_t Timer1Ctrl;       // 0x0008          : control
  uint32_t Timer1IntClr;     // 0x000C          :        interrupt clear
  uint32_t Timer1RIS;        // 0x0010          : raw    interrupt status
  uint32_t Timer1MIS;        // 0x0014          : masked interrupt status
  uint32_t Timer1BGLoad;     // 0x0018          : background load
  RSVD( 0, 0x001C, 0x001F ); // 0x001C...0x001F : reserved
  uint32_t Timer2Load;       // 0x0020          :            load
  uint32_t Timer2Value;      // 0x0024          : current value
  uint32_t Timer2Ctrl;       // 0x0028          : control
  uint32_t Timer2IntClr;     // 0x002C          :        interrupt clear
  uint32_t Timer2RIS;        // 0x0030          : raw    interrupt status
  uint32_t Timer2MIS;        // 0x0034          : masked interrupt status
  uint32_t Timer2BGLoad;     // 0x0038          : background load  
  RSVD( 1, 0x003C, 0x0EFF ); // 0x003C...0x0EFF : reserved
  uint32_t TimerITCR;        // 0x0F00          : integration test
  uint32_t TimerITOP;        // 0x0F04          : integration test
  RSVD( 2, 0x0F08, 0x0FDF ); // 0x0F08...0x0FDF : reserved
  uint32_t PeriphID0;        // 0x0FE0          : peripheral ID
  uint32_t PeriphID1;        // 0x0FE4          : peripheral ID
  uint32_t PeriphID2;        // 0x0FE8          : peripheral ID
  uint32_t PeriphID3;        // 0x0FEC          : peripheral ID
  uint32_t  PCellID0;        // 0x0FF0          : PrimeCell  ID
  uint32_t  PCellID1;        // 0x0FF4          : PrimeCell  ID
  uint32_t  PCellID2;        // 0x0FF8          : PrimeCell  ID
  uint32_t  PCellID3;        // 0x0FFC          : PrimeCell  ID
} SP804_t;

/* Per Table 4.2 (for example: the information is in several places) of
 * 
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0417d/index.html
 * 
 * we know the registers are mapped to fixed addresses in memory, so we
 * can just define a (structured) pointer to each one to support access.
 */

extern volatile SP804_t* TIMER0; // timer module 0 -> timers #0 and #1
extern volatile SP804_t* TIMER1; // timer module 1 -> timers #2 and #3
extern volatile SP804_t* TIMER2; // timer module 2 -> timers #4 and #5
extern volatile SP804_t* TIMER3; // timer module 3 -> timers #6 and #7

#endif
