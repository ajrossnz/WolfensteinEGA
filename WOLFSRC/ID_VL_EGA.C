// ID_VL_EGA.C
//
// This file adds a simple 16-colour EGA video mode to the Wolfenstein 3D CGA
// renderer.  The code is modelled after the existing VGA 4-plane mode
// initialisation in ID_VL.C but calls BIOS mode 0x0D instead of 0x13.  Mode
// 0x0D is defined in the IBM EGA BIOS as a 320×200 graphics mode with
// 16 colours【836434466270854†L0-L4】.  After switching modes the code unchains the
// video memory, enables writes to all four planes and sets the line width to
// 40 bytes (320/8).

#include "ID_HEAD.H"
#include "ID_VL.H"

#ifdef WITH_VGA

/*
=======================
= VL_SetEGAPlaneMode
=
= Switches to EGA mode 0x0D (320×200, 16 colours), unchains the planar
= framebuffer and prepares the line width.  This routine mirrors
= VL_SetVGAPlaneMode() but uses the correct BIOS mode for EGA.  See
*EGA_benchmark* documentation for proof that 0x0D is the 16-colour EGA mode
【836434466270854†L0-L4】.  After calling this function the caller should set the
= palette using VL_SetPalette().
=======================
*/
void VL_SetEGAPlaneMode(void)
{
    // Enter EGA 320×200×16 mode via BIOS interrupt 10h.
    asm mov     ax,0x000d
    asm int     0x10

    // Unchain planar memory; reuse VGA routine which disables mode X latching
    // and sets the GC registers appropriately.
    VL_DePlaneVGA();

    // Enable writing to all four planes (bits 0-3) using the sequencer map
    // mask.  Each bit corresponds to a plane.
    VGAMAPMASK(15);

    // The logical width of a 320-pixel line is 40 bytes in planar modes.
    VL_SetLineWidth(40);
}

#endif /* WITH_VGA */
