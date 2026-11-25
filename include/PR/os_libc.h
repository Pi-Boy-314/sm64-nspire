#ifndef _OS_LIBC_H_
#define _OS_LIBC_H_

#include "ultratypes.h"

// Old deprecated functions from strings.h, replaced by memcpy/memset.
#ifdef HOST_BUILD
#include <strings.h> // macOS declares them (as macros) here
#else
extern void bcopy(const void *, void *, size_t);
extern void bzero(void *, size_t);
#endif

#endif /* !_OS_LIBC_H_ */
