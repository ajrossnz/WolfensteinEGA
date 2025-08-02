// ID_HEAD.H (modified for WolfensteinEGA)
//
// This header overrides the original ID_HEAD.H so that GRMODE is set to
// EGAGR, instructing the Wolfenstein build system to compile against the
// EGA code path.  The remainder of the file is identical to the original.

#ifndef ID_HEAD_H_MODIFIED
#define ID_HEAD_H_MODIFIED

// The base extension of game data files.  Unchanged from original.
#define     EXTENSION       "WLF"
#define     WOLF

// Graphic mode identifiers.  These values are referenced by conditional
// compilation in the original sources.  EGAGR corresponds to 16‑colour
// EGA graphics.
#define     TEXTGR  0
#define     CGAGR   1
#define     EGAGR   2
#define     VGAGR   3

// Set the graphic mode to EGAGR instead of the default VGAGR.  This
// instructs the engine to use EGA 16‑colour routines instead of VGA 256‑colour
// or CGA 4‑colour ones.
#define     GRMODE      EGAGR

#include "VERSION.H"

typedef     enum    {false,true}    boolean;
typedef     unsigned        char            byte;
typedef     unsigned        int                     word;
typedef     unsigned        long            longword;
typedef     byte *                                  Ptr;

typedef     struct
            {
                    int     x,y;
            } Point;

typedef     struct
            {
                    Point   ul,lr;
            } Rect;

void        Quit (char *error);             // defined in user program

#endif /* ID_HEAD_H_MODIFIED */
