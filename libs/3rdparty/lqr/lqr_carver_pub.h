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

#ifndef __LQR_CARVER_PUB_H__
#define __LQR_CARVER_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_CURSOR_PUB_H__
#error "lqr_cursor_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_CURSOR_PUB_H__ */

#ifndef __LQR_GRADIENT_PUB_H__
#error "lqr_gradient_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_GRADIENT_PUB_H__ */

#ifndef __LQR_ENERGY_PUB_H__
#error "lqr_energy_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_ENERGY_PUB_H__ */

#ifndef __LQR_CURSOR_PUB_H__
#error "lqr_cursor_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_CURSOR_H__ */

#ifndef __LQR_PROGRESS_PUB_H__
#error "lqr_progress_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_PROGRESS_H__ */

#ifndef __LQR_CARVER_LIST_PUB_H__
#error "lqr_carver_list_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_CARVER_LIST_PUB_H__ */

#ifndef __LQR_VMAP_LIST_PUB_H__
#error "lqr_vmap_list_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_VMAP_LIST_PUB_H__ */

#ifndef __LQR_PROGRESS_PUB_H__
#error "lqr_progress_pub.h must be included prior to lqr_carver_pub.h"
#endif /* __LQR_PROGRESS_PUB_H__ */

/* LQR_CARVER CLASS PUBLIC FUNCTIONS */

/* constructor & destructor */
LQR_PUBLIC LqrCarver *lqr_carver_new(guchar *buffer, gint width, gint height, gint channels);
LQR_PUBLIC LqrCarver *lqr_carver_new_ext(void *buffer, gint width, gint height, gint channels,
                                         LqrColDepth colour_depth);
LQR_PUBLIC void lqr_carver_destroy(LqrCarver *r);

/* initialize */
LQR_PUBLIC LqrRetVal lqr_carver_init(LqrCarver *r, gint delta_x, gfloat rigidity);

/* set attributes */
LQR_PUBLIC LqrRetVal lqr_carver_set_image_type(LqrCarver *r, LqrImageType image_type);
LQR_PUBLIC LqrRetVal lqr_carver_set_alpha_channel(LqrCarver *r, gint channel_index);
LQR_PUBLIC LqrRetVal lqr_carver_set_black_channel(LqrCarver *r, gint channel_index);
LQR_PUBLIC void lqr_carver_set_dump_vmaps(LqrCarver *r);
LQR_PUBLIC void lqr_carver_set_no_dump_vmaps(LqrCarver *r);
LQR_PUBLIC void lqr_carver_set_resize_order(LqrCarver *r, LqrResizeOrder resize_order);
LQR_PUBLIC void lqr_carver_set_side_switch_frequency(LqrCarver *r, guint switch_frequency);
LQR_PUBLIC LqrRetVal lqr_carver_set_enl_step(LqrCarver *r, gfloat enl_step);
LQR_PUBLIC void lqr_carver_set_use_cache(LqrCarver *r, gboolean use_cache);
LQR_PUBLIC LqrRetVal lqr_carver_attach(LqrCarver *r, LqrCarver *aux);
LQR_PUBLIC void lqr_carver_set_progress(LqrCarver *r, LqrProgress * p);
LQR_PUBLIC void lqr_carver_set_preserve_input_image(LqrCarver *r);
/* THIS FUNCTION IS ONLY MAINTAINED FOR BACK-COMPATIBILITY PURPOSES */
/* lqr_carver_set_energy_function_builtin() should be used instead */
G_GNUC_DEPRECATED
LQR_PUBLIC void lqr_carver_set_gradient_function(LqrCarver *r, LqrGradFuncType gf_ind);

/* image manipulations */
LQR_PUBLIC LqrRetVal lqr_carver_resize(LqrCarver *r, gint w1, gint h1); /* liquid resize */
LQR_PUBLIC LqrRetVal lqr_carver_flatten(LqrCarver *r);  /* flatten the multisize image */
LQR_PUBLIC LqrRetVal lqr_carver_cancel(LqrCarver *r);   /* cancel the current action from a different thread */

/* readout */
LQR_PUBLIC void lqr_carver_scan_reset(LqrCarver *r);
LQR_PUBLIC gboolean lqr_carver_scan(LqrCarver *r, gint *x, gint *y, guchar **rgb);
LQR_PUBLIC gboolean lqr_carver_scan_ext(LqrCarver *r, gint *x, gint *y, void **rgb);
LQR_PUBLIC gboolean lqr_carver_scan_line(LqrCarver *r, gint *n, guchar **rgb);
LQR_PUBLIC gboolean lqr_carver_scan_line_ext(LqrCarver *r, gint *n, void **rgb);
LQR_PUBLIC gboolean lqr_carver_scan_by_row(LqrCarver *r);
G_GNUC_DEPRECATED
LQR_PUBLIC gint lqr_carver_get_bpp(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_channels(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_width(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_height(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_ref_width(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_ref_height(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_orientation(LqrCarver *r);
LQR_PUBLIC LqrColDepth lqr_carver_get_col_depth(LqrCarver *r);
LQR_PUBLIC LqrImageType lqr_carver_get_image_type(LqrCarver *r);
LQR_PUBLIC gfloat lqr_carver_get_enl_step(LqrCarver *r);
LQR_PUBLIC gint lqr_carver_get_depth(LqrCarver *r);

#endif /* __LQR_CARVER_PUB_H__ */
