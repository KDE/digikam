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

#ifndef __LQR_H__
#define __LQR_H__

#include <glib.h>

G_BEGIN_DECLS

#include <lqr/lqr_base.h>
#include <lqr/lqr_gradient_pub.h>
#include <lqr/lqr_rwindow_pub.h>
#include <lqr/lqr_energy_pub.h>
#include <lqr/lqr_cursor_pub.h>
#include <lqr/lqr_progress_pub.h>
#include <lqr/lqr_vmap_pub.h>
#include <lqr/lqr_vmap_list_pub.h>
#include <lqr/lqr_carver_list_pub.h>
#include <lqr/lqr_carver_bias_pub.h>
#include <lqr/lqr_carver_rigmask_pub.h>
#include <lqr/lqr_carver_pub.h>

G_END_DECLS

#endif /* __LQR_H__ */
