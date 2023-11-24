#ifndef _MW_SERIALREAD_H_
#define _MW_SERIALREAD_H_
#include <inttypes.h>
#include "rtwtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void tftSetup(void);
void setTextSize(int size);
void displayHeartRate(int txtcolor, int heart_rate, uint_T x, uint_T y);
unsigned int getTftWidth(void);
unsigned int getTftHeight(void);
void setTftFillscreen(void);
void DrawTftLine(uint_T x1, uint_T y1, uint_T x2, uint_T y2, int linecolor);

#ifdef __cplusplus
}
#endif
#endif //_MW_SERIALREAD_H_
