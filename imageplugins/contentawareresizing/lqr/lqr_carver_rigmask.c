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

/**** LQR_CARVER_RIGMASK STRUCT FUNTIONS ****/

/* LQR_PUBLIC */
void
lqr_carver_rigmask_clear(LqrCarver *r)
{
    g_free(r->rigidity_mask);
    r->rigidity_mask = NULL;
}

LqrRetVal
lqr_carver_rigmask_init(LqrCarver *r)
{
    /* gint y, x; */

    LQR_CATCH_CANC(r);

    LQR_CATCH_F(r->active);

    LQR_CATCH_MEM(r->rigidity_mask = g_try_new0(gfloat, r->w0 * r->h0));

#if 0
    for (y = 0; y < r->h0; y++) {
        for (x = 0; x < r->w0; x++) {
            r->rigidity_mask[y * r->w0 + x] = 1;
        }
    }
#endif

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_rigmask_add_xy(LqrCarver *r, gdouble rigidity, gint x, gint y)
{
    gint xt, yt;

    LQR_CATCH_CANC(r);

    LQR_CATCH_F(r->active);

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }

    if (r->rigidity_mask == NULL) {
        LQR_CATCH(lqr_carver_rigmask_init(r));
    }
#if 0
    if (r->rigidity == 0) {
        return LQR_OK;
    }
#endif

    xt = r->transposed ? y : x;
    yt = r->transposed ? x : y;

    r->rigidity_mask[yt * r->w0 + xt] += (gfloat) rigidity;

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_rigmask_add_area(LqrCarver *r, gdouble *buffer, gint width, gint height, gint x_off, gint y_off)
{
    gint x, y;
    gint xt, yt;
    gint wt, ht;
    gint x0, y0, x1, y1, x2, y2;

    LQR_CATCH_CANC(r);

    LQR_CATCH_F(r->active);

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }
#if 0
    if (r->rigidity == 0) {
        return LQR_OK;
    }
#endif

    if (r->rigidity_mask == NULL) {
        LQR_CATCH(lqr_carver_rigmask_init(r));
    }

    wt = r->transposed ? r->h : r->w;
    ht = r->transposed ? r->w : r->h;

    x0 = MIN(0, x_off);
    y0 = MIN(0, y_off);
    x1 = MAX(0, x_off);
    y1 = MAX(0, y_off);
    x2 = MIN(wt, width + x_off);
    y2 = MIN(ht, height + y_off);

    for (y = 0; y < y2 - y1; y++) {
        for (x = 0; x < x2 - x1; x++) {
            xt = r->transposed ? y : x;
            yt = r->transposed ? x : y;

            r->rigidity_mask[(yt + y1) * r->w0 + (xt + x1)] = (gfloat) buffer[(y - y0) * width + (x - x0)];
        }

    }

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_rigmask_add(LqrCarver *r, gdouble *buffer)
{
    return lqr_carver_rigmask_add_area(r, buffer, r->w0, r->h0, 0, 0);
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_rigmask_add_rgb_area(LqrCarver *r, guchar *rgb, gint channels, gint width, gint height, gint x_off,
                                gint y_off)
{
    gint x, y, k, c_channels;
    gboolean has_alpha;
    gint xt, yt;
    gint wt, ht;
    gint x0, y0, x1, y1, x2, y2;
    gint sum;
    gdouble rigmask;

    LQR_CATCH_CANC(r);

    LQR_CATCH_F(r->active);

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }
#if 0
    if (r->rigidity == 0) {
        return LQR_OK;
    }
#endif

    if (r->rigidity_mask == NULL) {
        LQR_CATCH(lqr_carver_rigmask_init(r));
    }

    has_alpha = (channels == 2 || channels >= 4);
    c_channels = channels - (has_alpha ? 1 : 0);

    wt = r->transposed ? r->h : r->w;
    ht = r->transposed ? r->w : r->h;

    x0 = MIN(0, x_off);
    y0 = MIN(0, y_off);
    x1 = MAX(0, x_off);
    y1 = MAX(0, y_off);
    x2 = MIN(wt, width + x_off);
    y2 = MIN(ht, height + y_off);

    for (y = 0; y < y2 - y1; y++) {
        for (x = 0; x < x2 - x1; x++) {
            sum = 0;
            for (k = 0; k < c_channels; k++) {
                sum += rgb[((y - y0) * width + (x - x0)) * channels + k];
            }

            rigmask = (gdouble) sum / (255 * c_channels);
            if (has_alpha) {
                rigmask *= (gdouble) rgb[((y - y0) * width + (x - x0) + 1) * channels - 1] / 255;
            }

            xt = r->transposed ? y : x;
            yt = r->transposed ? x : y;

            r->rigidity_mask[(yt + y1) * r->w0 + (xt + x1)] = (gfloat) rigmask;

        }

    }

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_rigmask_add_rgb(LqrCarver *r, guchar *rgb, gint channels)
{
    return lqr_carver_rigmask_add_rgb_area(r, rgb, channels, r->w0, r->h0, 0, 0);
}

/**** END OF LQR_CARVER_RIGMASK CLASS FUNCTIONS ****/
