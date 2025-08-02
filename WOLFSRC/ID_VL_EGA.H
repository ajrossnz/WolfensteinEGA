// ID_VL_EGA.H
//
// Prototype for the EGA video initialisation routine.  This header may be
// included from modules that need to call VL_SetEGAPlaneMode() when
// compiling for 16‑colour EGA.  See ID_VL_EGA.C for the implementation.

#ifndef ID_VL_EGA_H
#define ID_VL_EGA_H

// Switches the display into 320×200 16‑colour planar EGA mode (BIOS mode
// 0x0D) and configures the VGA/EGA controller for unchained planar access.
// See documentation in ID_VL_EGA.C for more details.
void VL_SetEGAPlaneMode(void);

#endif /* ID_VL_EGA_H */
