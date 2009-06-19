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

/**** LQR_CARVER_BIAS STRUCT FUNTIONS ****/

/* LQR_PUBLIC */
void
lqr_carver_bias_clear(LqrCarver *r)
{
    g_free(r->bias);
    r->bias = NULL;
    r->nrg_uptodate = FALSE;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_bias_add_xy(LqrCarver *r, gdouble bias, gint x, gint y)
{
    gint xt, yt;

    if (bias == 0) {
        return LQR_OK;
    }

    LQR_CATCH_CANC(r);
    if (r->nrg_active == FALSE) {
        LQR_CATCH(lqr_carver_init_energy_related(r));
    }

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }
    if (r->bias == NULL) {
        LQR_CATCH_MEM(r->bias = g_try_new0(gfloat, r->w0 * r->h0));
    }

    xt = r->transposed ? y : x;
    yt = r->transposed ? x : y;

    r->bias[yt * r->w0 + xt] += (gfloat) bias / 2;

    r->nrg_uptodate = FALSE;

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_bias_add_area(LqrCarver *r, gdouble *buffer, gint bias_factor, gint width, gint height, gint x_off,
                         gint y_off)
{
    gint x, y;
    gint xt, yt;
    gint wt, ht;
    gint x0, y0, x1, y1, x2, y2;
    gfloat bias;

    LQR_CATCH_CANC(r);

    if (bias_factor == 0) {
        return LQR_OK;
    }

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }

    if (r->nrg_active == FALSE) {
        LQR_CATCH(lqr_carver_init_energy_related(r));
    }

    if (r->bias == NULL) {
        LQR_CATCH_MEM(r->bias = g_try_new0(gfloat, r->w * r->h));
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
            bias = (gfloat) ((gdouble) bias_factor * buffer[(y - y0) * width + (x - x0)] / 2);

            xt = r->transposed ? y : x;
            yt = r->transposed ? x : y;

            r->bias[(yt + y1) * r->w0 + (xt + x1)] += bias;
        }
    }

    r->nrg_uptodate = FALSE;

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_bias_add(LqrCarver *r, gdouble *buffer, gint bias_factor)
{
    return lqr_carver_bias_add_area(r, buffer, bias_factor, lqr_carver_get_width(r), lqr_carver_get_height(r), 0, 0);
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_bias_add_rgb_area(LqrCarver *r, guchar *rgb, gint bias_factor, gint channels, gint width, gint height,
                             gint x_off, gint y_off)
{
    gint x, y, k, c_channels;
    gboolean has_alpha;
    gint xt, yt;
    gint wt, ht;
    gint x0, y0, x1, y1, x2, y2;
    gint sum;
    gdouble bias;

    LQR_CATCH_CANC(r);

    if ((r->w != r->w0) || (r->w_start != r->w0) || (r->h != r->h0) || (r->h_start != r->h0)) {
        LQR_CATCH(lqr_carver_flatten(r));
    }

    if (r->nrg_active == FALSE) {
        LQR_CATCH(lqr_carver_init_energy_related(r));
    }

    if (bias_factor == 0) {
        return LQR_OK;
    }

    if (r->bias == NULL) {
        LQR_CATCH_MEM(r->bias = g_try_new0(gfloat, r->w * r->h));
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

            bias = ((gdouble) bias_factor * sum / (2 * 255 * c_channels));
            if (has_alpha) {
                bias *= (gdouble) rgb[((y - y0) * width + (x - x0) + 1) * channels - 1] / 255;
            }

            xt = r->transposed ? y : x;
            yt = r->transposed ? x : y;

            r->bias[(yt + y1) * r->w0 + (xt + x1)] += (gfloat) bias;
        }
    }

    r->nrg_uptodate = FALSE;

    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_carver_bias_add_rgb(LqrCarver *r, guchar *rgb, gint bias_factor, gint channels)
{
    return lqr_carver_bias_add_rgb_area(r, rgb, bias_factor, channels, lqr_carver_get_width(r),
                                        lqr_carver_get_height(r), 0, 0);
}

/**** END OF LQR_CARVER_BIAS CLASS FUNCTIONS ****/
