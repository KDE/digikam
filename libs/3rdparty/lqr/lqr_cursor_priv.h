/* LiquidRescaling Library
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>
 */

#ifndef __LQR_CURSOR_PRIV_H__
#define __LQR_CURSOR_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_cursor_priv.h"
#endif /* __LQR_BASE_H__ */

/**** LQR_CURSOR CLASS DEFINITION ****/
/* The lqr_cursors can scan a multisize image according to its
 * current visibility level, skipping invisible points */
struct _LqrCursor {
#ifdef __LQR_DEBUG__
    gint initialized;                   /* initialization flag */
#endif
    gint x;                             /* x coordinate of current data */
    gint y;                             /* y coordinate of current data */
    gint now;                           /* current array position */
    LqrCarver *o;                       /* pointer to owner carver */
    gchar eoc;                          /* end of carver flag */
};

/* LQR_CURSOR CLASS PRIVATE FUNCTIONS */

/* constructor */
LqrCursor *lqr_cursor_create(LqrCarver *owner);

/* destructor */
void lqr_cursor_destroy(LqrCursor *c);

/* functions for moving around */
void lqr_cursor_reset(LqrCursor *c);
void lqr_cursor_next(LqrCursor *c);
void lqr_cursor_prev(LqrCursor *c);

/* methods for exploring neighborhoods */
gint lqr_cursor_left(LqrCursor *c);

#endif /* __LQR_CURSOR_PRIV_H__ */
