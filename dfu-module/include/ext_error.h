#ifndef _INC_EXT_ERROR_H
#define _INC_EXT_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


void set_ext_error_code(int code);

int get_ext_error_code();

#ifdef __cplusplus
}   /* ... extern "C" */
#endif  /* __cplusplus */


#endif // _INC_EXT_ERROR_H
