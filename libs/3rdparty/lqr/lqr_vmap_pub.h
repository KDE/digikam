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

#ifndef __LQR_VMAP_PUB_H__
#define __LQR_VMAP_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_vmap_pub.h"
#endif /* __LQR_BASE_H__ */

/*** LQR_VMAP CLASS DECLARATION ***/

typedef struct _LqrVMap LqrVMap;

/* LQR_VMAP FUNCTION DECLARATION */

typedef LqrRetVal (*LqrVMapFunc) (LqrVMap *vmap, gpointer data);

/* LQR_VMAP PUBLIC FUNCTIONS */

LQR_PUBLIC LqrVMap *lqr_vmap_new(gint *buffer, gint width, gint heigth, gint depth, gint orientation);
LQR_PUBLIC void lqr_vmap_destroy(LqrVMap *vmap);

LQR_PUBLIC gint *lqr_vmap_get_data(LqrVMap *vmap);
LQR_PUBLIC gint lqr_vmap_get_width(LqrVMap *vmap);
LQR_PUBLIC gint lqr_vmap_get_height(LqrVMap *vmap);
LQR_PUBLIC gint lqr_vmap_get_depth(LqrVMap *vmap);
LQR_PUBLIC gint lqr_vmap_get_orientation(LqrVMap *vmap);

LQR_PUBLIC LqrRetVal lqr_vmap_internal_dump(LqrCarver *r);
LQR_PUBLIC LqrVMap *lqr_vmap_dump(LqrCarver *r);
LQR_PUBLIC LqrRetVal lqr_vmap_load(LqrCarver *r, LqrVMap *vmap);

#endif /* __LQR_VMAP_PUB_H__ */
