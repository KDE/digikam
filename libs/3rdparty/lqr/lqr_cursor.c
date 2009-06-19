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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <lqr/lqr_all.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif

/**** LQR_CURSOR STRUCT FUNTIONS ****/

/*** constructor and destructor ***/

LqrCursor *
lqr_cursor_create(LqrCarver *owner)
{
    LqrCursor *c;

    LQR_TRY_N_N(c = g_try_new(LqrCursor, 1));
    c->o = owner;
    c->eoc = 0;
#ifdef __LQR_DEBUG__
    c->initialized = 1;
#endif
    lqr_cursor_reset(c);

    return c;
}

void
lqr_cursor_destroy(LqrCursor *c)
{
    g_free(c);
}

/*** functions for moving around ***/

/* resets to starting point */
void
lqr_cursor_reset(LqrCursor *c)
{
    /* make sure the pointers are initialized */
#ifdef __LQR_DEBUG__
    assert(c->initialized);
#endif /* __LQR_DEBUG__ */

    /* reset end of carver flag */
    c->eoc = 0;

    /* reset coordinates */
    c->x = 0;
    c->y = 0;

    /* set the current point to the beginning of the map */
    c->now = 0;

    /* skip invisible points */
    while ((c->o->vs[c->now] != 0) && (c->o->vs[c->now] < c->o->level)) {
        c->now++;
#ifdef __LQR_DEBUG__
        assert(c->now < c->o->w0);
#endif /* __LQR_DEBUG__ */
    }
}

/* go to next data (first rows, then columns;
 * does nothing if we are already at the top-right corner) */
void
lqr_cursor_next(LqrCursor *c)
{
#ifdef __LQR_DEBUG__
    assert(c->initialized);
#endif /* __LQR_DEBUG__ */

    /* are we at the end? */
    if (c->eoc) {
        return;
    }

    /* update coordinates */
    if (c->x == c->o->w - 1) {
        if (c->y == c->o->h - 1) {
            /* top-right corner, set eoc flag */
            c->eoc = 1;
            return;
        }
        /* end-of-line, carriage return */
        c->x = 0;
        c->y++;
    } else {
        /* simple right move */
        c->x++;
    }

    /* first move */
    c->now++;
#ifdef __LQR_DEBUG__
    assert(c->now < (c->o->w0 * c->o->h0));
#endif /* __LQR_DEBUG__ */

    /* skip invisible points */
    while ((c->o->vs[c->now] != 0) && (c->o->vs[c->now] < c->o->level)) {
        c->now++;
#ifdef __LQR_DEBUG__
        assert(c->now < (c->o->w0 * c->o->h0));
#endif /* __LQR_DEBUG__ */
    }
}

/* go to previous data (behaves opposite to next) */
void
lqr_cursor_prev(LqrCursor *c)
{

    /* are we at the end of carver ? */
    if (c->eoc) {
        return;
    }

    /* update coordinates */
    if (c->x == 0) {
        if (c->y == 0) {
            /* bottom-right corner, do nothing */
            return;
        }
        /* carriage return */
        c->x = c->o->w - 1;
        c->y--;
    } else {
        /* simple left move */
        c->x--;
    }

    /* first move */
    c->now--;
#ifdef __LQR_DEBUG__
    assert(c->now >= 0);
#endif /* __LQR_DEBUG__ */

    /* skip invisible points */
    while ((c->o->vs[c->now] != 0) && (c->o->vs[c->now] < c->o->level)) {
        c->now--;
#ifdef __LQR_DEBUG__
        assert(c->now >= 0);
#endif /* __LQR_DEBUG__ */
    }
}

/*** methods for exploring neighborhoods ***/

/* these return pointers to neighboring data
 * it is an error to ask for out-of-bounds data */

gint
lqr_cursor_left(LqrCursor *c)
{
    /* create an auxiliary pointer */
    gint ret = c->now;

#ifdef __LQR_DEBUG__
    assert(c->initialized);
    assert(c->x > 0);
    assert(c->eoc == 0);
#endif /* __LQR_DEBUG__ */

    /* first move */
    ret--;
#ifdef __LQR_DEBUG__
    assert(ret >= 0);
#endif /* __LQR_DEBUG__ */

    /* skip invisible points */
    while ((c->o->vs[ret] != 0) && c->o->vs[ret] < c->o->level) {
        ret--;
#ifdef __LQR_DEBUG__
        assert(ret >= 0);
#endif /* __LQR_DEBUG__ */
    }
    return ret;
}

/**** END OF LQR_CURSOR_CURSOR CLASS FUNCTIONS ****/
