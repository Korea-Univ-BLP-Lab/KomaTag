#ifndef ENV_H
#define ENV_H

#include "postype.h"

/* environments for NE */
extern char FSTPath[80];
extern char FSTinfoPath[80];

extern char PosName[NUM_POS][40];

extern void KOMA_ReadPosName (void);
extern void KOMA_ViewPosName (void);
extern void KOMA_ViewEnv (void);
extern short KOMA_CheckEnv (void);

#endif
