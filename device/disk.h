#ifndef __DISK_H
#define __DISK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "PL011.h"

/* Each of the following functions adopts the same approach to
 * reporting success vs. failure, as indicated by the response 
 * produced by the disk: they return an r st.
 * 
 * r <  0 means failure
 * r >= 0 means success
 *
 * Rather than give up immediately if a given request fails, it
 * will (automatically) retry for some fixed number of times.
 */

#define DISK_RETRY   (  3 )

#define DISK_SUCCESS (  0 )
#define DISK_FAILURE ( -1 )

// query the disk block count
extern int disk_get_block_num();
// query the disk block length
extern int disk_get_block_len();

// write an n-byte block of data x to   the disk at block address a
extern int disk_wr( uint32_t a, const uint8_t* x, int n );
// read  an n-byte block of data x from the disk at block address a
extern int disk_rd( uint32_t a,       uint8_t* x, int n );

#endif
