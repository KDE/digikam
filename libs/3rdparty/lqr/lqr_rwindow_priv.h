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

#ifndef __LQR_READER_WINDOW_PRIV_H__
#define __LQR_READER_WINDOW_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_rwindow_priv.h"
#endif /* __LQR_BASE_H__ */

struct _LqrReadingWindow {
    gdouble **buffer;
    gint radius;
    LqrEnergyReaderType read_t;
    gint channels;
    gboolean use_rcache;
    LqrCarver *carver;
    gint x;
    gint y;
};

typedef gdouble (*LqrReadFunc) (LqrCarver *, gint, gint);
typedef gdouble (*LqrReadFuncWithCh) (LqrCarver *, gint, gint, gint);
/* typedef glfoat (*LqrReadFuncAbs) (LqrCarver*, gint, gint, gint, gint); */

LqrRetVal lqr_rwindow_fill_std(LqrReadingWindow *rwindow, LqrCarver *r, gint x, gint y);
LqrRetVal lqr_rwindow_fill_rgba(LqrReadingWindow *rwindow, LqrCarver *r, gint x, gint y);
LqrRetVal lqr_rwindow_fill_custom(LqrReadingWindow *rwindow, LqrCarver *r, gint x, gint y);
LqrRetVal lqr_rwindow_fill(LqrReadingWindow *rwindow, LqrCarver *r, gint x, gint y);

gdouble lqr_rwindow_read_bright(LqrReadingWindow *rwindow, gint x, gint y);
gdouble lqr_rwindow_read_luma(LqrReadingWindow *rwindow, gint x, gint y);
gdouble lqr_rwindow_read_rgba(LqrReadingWindow *rwindow, gint x, gint y, gint channel);
gdouble lqr_rwindow_read_custom(LqrReadingWindow *rwindow, gint x, gint y, gint channel);

LqrReadingWindow *lqr_rwindow_new_std(gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
LqrReadingWindow *lqr_rwindow_new_rgba(gint radius, gboolean use_rcache);
LqrReadingWindow *lqr_rwindow_new_custom(gint radius, gboolean use_rcache, gint channels);
LqrReadingWindow *lqr_rwindow_new(gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache);
void lqr_rwindow_destroy(LqrReadingWindow *rwindow);

#endif /* __LQR_READER_WINDOW_PRIV_H__ */
