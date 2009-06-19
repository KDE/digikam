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
#endif /* __LQR_DEBUG__ */

/**** SEAMS BUFFER FUNCTIONS ****/

/* LQR_PUBLIC */
LqrVMap *
lqr_vmap_new(gint *buffer, gint width, gint height, gint depth, gint orientation)
{
    LqrVMap *vmap;

    LQR_TRY_N_N(vmap = g_try_new(LqrVMap, 1));
    vmap->buffer = buffer;
    vmap->width = width;
    vmap->height = height;
    vmap->orientation = orientation;
    vmap->depth = depth;
    return vmap;
}

/* LQR_PUBLIC */
void
lqr_vmap_destroy(LqrVMap *vmap)
{
    g_free(vmap->buffer);
    g_free(vmap);
}

/* LQR_PUBLIC */
gint *
lqr_vmap_get_data(LqrVMap *vmap)
{
    return vmap->buffer;
}

/* LQR_PUBLIC */
gint
lqr_vmap_get_width(LqrVMap *vmap)
{
    return vmap->width;
}

/* LQR_PUBLIC */
gint
lqr_vmap_get_height(LqrVMap *vmap)
{
    return vmap->height;
}

/* LQR_PUBLIC */
gint
lqr_vmap_get_depth(LqrVMap *vmap)
{
    return vmap->depth;
}

/* LQR_PUBLIC */
gint
lqr_vmap_get_orientation(LqrVMap *vmap)
{
    return vmap->orientation;
}

/* dump the visibility level of the image */
/* LQR_PUBLIC */
LqrVMap *
lqr_vmap_dump(LqrCarver *r)
{
    LqrVMap *vmap;
    gint w, h, w1, x, y, z0, vs;
    gint *buffer;
    gint depth;

    /* save current size */
    w1 = r->w;

    /* temporarily set the size to the original */
    lqr_carver_set_width(r, r->w_start);

    w = lqr_carver_get_width(r);
    h = lqr_carver_get_height(r);
    depth = r->w0 - r->w_start;

    LQR_TRY_N_N(buffer = g_try_new(gint, w * h));

    lqr_cursor_reset(r->c);
    for (y = 0; y < r->h; y++) {
        for (x = 0; x < r->w; x++) {
            vs = r->vs[r->c->now];
            if (!r->transposed) {
                z0 = y * r->w + x;
            } else {
                z0 = x * r->h + y;
            }
            if (vs == 0) {
                buffer[z0] = 0;
            } else {
                buffer[z0] = vs - depth;
            }
            lqr_cursor_next(r->c);
        }
    }

    /* recover size */
    lqr_carver_set_width(r, w1);
    lqr_cursor_reset(r->c);

    LQR_TRY_N_N(vmap = lqr_vmap_new(buffer, w, h, depth, r->transposed));

    return vmap;
}

/* dump the visibility level of the image */
/* LQR_PUBLIC */
LqrRetVal
lqr_vmap_internal_dump(LqrCarver *r)
{
    LqrVMap *vmap;
    gint w, h, w1, x, y, z0, vs;
    gint *buffer;
    gint depth;

    LQR_CATCH_CANC(r);

    /* save current size */
    w1 = r->w;

    /* temporarily set the size to the original */
    lqr_carver_set_width(r, r->w_start);

    w = lqr_carver_get_width(r);
    h = lqr_carver_get_height(r);
    depth = r->w0 - r->w_start;

    LQR_CATCH_MEM(buffer = g_try_new(gint, w * h));

    lqr_cursor_reset(r->c);
    for (y = 0; y < r->h; y++) {
        for (x = 0; x < r->w; x++) {
            vs = r->vs[r->c->now];
            if (!r->transposed) {
                z0 = y * r->w + x;
            } else {
                z0 = x * r->h + y;
            }
            if (vs == 0) {
                buffer[z0] = 0;
            } else {
                buffer[z0] = vs - depth;
            }
            lqr_cursor_next(r->c);
        }
    }

    /* recover size */
    lqr_carver_set_width(r, w1);
    lqr_cursor_reset(r->c);

    LQR_CATCH_MEM(vmap = lqr_vmap_new(buffer, w, h, depth, r->transposed));

    LQR_CATCH_MEM(r->flushed_vs = lqr_vmap_list_append(r->flushed_vs, vmap));

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_vmap_load(LqrCarver *r, LqrVMap *vmap)
{
    gint w, h;
    gint x, y, z0, z1;

    w = vmap->width;
    h = vmap->height;

    LQR_CATCH_CANC(r);
    LQR_CATCH_F(!r->active);

    if (!r->transposed) {
        LQR_CATCH_F((r->w_start == w) && (r->h_start == h));
    } else {
        LQR_CATCH_F((r->w_start == h) && (r->h_start == w));
    }

    LQR_CATCH(lqr_carver_flatten(r));

    if (vmap->orientation != r->transposed) {
        LQR_CATCH(lqr_carver_transpose(r));
    }

    for (y = 0; y < r->h; y++) {
        for (x = 0; x < r->w; x++) {
            if (!r->transposed) {
                z0 = y * r->w + x;
            } else {
                z0 = x * r->h + y;
            }
            z1 = y * r->w + x;

            r->vs[z1] = vmap->buffer[z0];
        }
    }

    LQR_CATCH(lqr_carver_inflate(r, vmap->depth));

    lqr_cursor_reset(r->c);

    lqr_carver_set_enl_step(r, 2.0);

    return LQR_OK;
}
