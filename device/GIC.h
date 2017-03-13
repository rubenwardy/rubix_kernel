#ifndef __GIC_H
#define __GIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RSVD(x,y,z) uint8_t reserved##x[ z - y + 1 ];

/* Although the GIC architecture is documented at
 *
 * http://infocenter.arm.com/help/topic/com.arm.doc.ihi0048b/index.html 
 *
 * the platform includes a bespoke implementation based on the combination
 * of 4 GIC components in total.
 * 
 * - Section 3.14 gives a high-level overview of the interrupt mechanism,
 *   noting in particular that GIC0 and GIC1 are those associated with the
 *   ARM core (managing IRQ- and FIQ-based interrupts respectively),
 * - Section 4.11 describes the GIC implementation, which sub-divides each
 *   GIC into interface and distributor components: it includes 
 * 
 *   - Table 4.44,           i.e., the mapping of interrupt signals from 
 *     other devices to interrupt IDs wrt. each GIC,
 *   - Tables 4.46 and 4.55, i.e., the device register layout (including 
 *     an offset from the device base address, in the memory map, for each 
 *     register), plus
 *   - a summary of the internal structure of each device register.
 * 
 * Note that the field identifiers used here follow the documentation in a
 * general sense, but with a some minor alterations to improve clarity and
 * consistency.
 */

typedef volatile struct {
  uint32_t       CTLR;       // 0x0000          : control
  uint32_t        PMR;       // 0x0004          : priority mask
  uint32_t        BPR;       // 0x0008          : binary point
  uint32_t        IAR;       // 0x000C          : interrupt acknowledge
  uint32_t       EOIR;       // 0x0010          : end of interrupt
  uint32_t        RPR;       // 0x0014          : running interrupt
  uint32_t      HPPIR;       // 0x0018          : highest pending interrupt
} GICC_t;

typedef volatile struct {
  uint32_t       CTLR;       // 0x0000          : control
  uint32_t      TYPER;       // 0x0004          : controller type
  RSVD( 0, 0x0008, 0x00FC ); // 0x0008...0x00FC : reserved
  uint32_t  ISENABLER0;      // 0x0100          :   set-enable
  uint32_t  ISENABLER1;      // 0x0104          :   set-enable
  uint32_t  ISENABLER2;      // 0x0108          :   set-enable
  RSVD( 1, 0x010C, 0x017C ); // 0x010C...0x017C : reserved
  uint32_t  ICENABLER0;      // 0x0180          : clear-enable
  uint32_t  ICENABLER1;      // 0x0184          : clear-enable
  uint32_t  ICENABLER2;      // 0x0188          : clear-enable
  RSVD( 2, 0x018C, 0x01FC ); // 0x018C...0x01FC : reserved
  uint32_t    ISPENDR0;      // 0x0200          :   set-pending
  uint32_t    ISPENDR1;      // 0x0204          :   set-pending
  uint32_t    ISPENDR2;      // 0x0208          :   set-pending
  RSVD( 3, 0x020C, 0x027C ); // 0x020C...0x027C : reserved
  uint32_t    ICPENDR0;      // 0x0280          : clear-pending
  uint32_t    ICPENDR1;      // 0x0284          : clear-pending
  uint32_t    ICPENDR2;      // 0x0288          : clear-pending
  RSVD( 4, 0x028C, 0x02FC ); // 0x028C...0x02FC : reserved
  uint32_t  ISACTIVER0;      // 0x0300          :   set-active
  uint32_t  ISACTIVER1;      // 0x0304          :   set-active
  uint32_t  ISACTIVER2;      // 0x0308          :   set-active
  RSVD( 5, 0x030C, 0x03FC ); // 0x030C...0x03FC : reserved
  uint32_t IPRIORITYR[ 24 ]; // 0x0400...0x045C : priority
  RSVD( 6, 0x0460, 0x07FC ); // 0x0460...0x07FC : reserved
  uint32_t  ITARGETSR[ 24 ]; // 0x0800...0x085C : processor target
  RSVD( 7, 0x0860, 0x0BFC ); // 0x0760...0x0BFC : reserved
  uint32_t      ICFGR0;      // 0x0C00          : configuration
  uint32_t      ICFGR1;      // 0x0C04          : configuration
  uint32_t      ICFGR2;      // 0x0C08          : configuration
  uint32_t      ICFGR3;      // 0x0C0C          : configuration
  uint32_t      ICFGR4;      // 0x0C10          : configuration
  uint32_t      ICFGR5;      // 0x0C14          : configuration
  RSVD( 8, 0x0C18, 0x0EFC ); // 0x0C18...0x0EFC : reserved
  uint32_t       SGIR;       // 0x0F00          : software interrupt
  RSVD( 9, 0x0F04, 0x0FFC ); // 0x0F04...0x0FFC : reserved
} GICD_t;

#define GIC_SOURCE_TIMER0 ( 36 )
#define GIC_SOURCE_TIMER1 ( 37 )
#define GIC_SOURCE_TIMER2 ( 73 )
#define GIC_SOURCE_TIMER3 ( 74 )

#define GIC_SOURCE_UART0  ( 44 )
#define GIC_SOURCE_UART1  ( 45 )
#define GIC_SOURCE_UART2  ( 46 )
#define GIC_SOURCE_UART3  ( 47 )

#define GIC_SOURCE_PS20   ( 52 )
#define GIC_SOURCE_PS21   ( 53 )

/* Per Table 4.2 (for example: the information is in several places) of
 * 
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0417d/index.html
 * 
 * we know the registers are mapped to fixed addresses in memory, so we
 * can just define a (structured) pointer to each one to support access.
 */

extern volatile GICC_t* GICC0;
extern volatile GICD_t* GICD0;
extern volatile GICC_t* GICC1;
extern volatile GICD_t* GICD1;
extern volatile GICC_t* GICC2;
extern volatile GICD_t* GICD2;
extern volatile GICC_t* GICC3;
extern volatile GICD_t* GICD3;

#endif
