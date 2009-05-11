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

#ifndef __LQR_ENERGY_PUB_H__
#define __LQR_ENERGY_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_energy_pub.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_READER_WINDOW_PUB_H__
#error "lqr_rwindow_pub.h must be included prior to lqr_energy_pub.h"
#endif /* __LQR_READER_WINDOW_PUB_H__ */

enum _LqrEnergyFuncBuiltinType {
    LQR_EF_GRAD_NORM,                   /* gradient norm : sqrt(x^2 + y^2)            */
    LQR_EF_GRAD_SUMABS,                 /* sum of absulte values : |x| + |y|          */
    LQR_EF_GRAD_XABS,                   /* x absolute value : |x|                     */
    LQR_EF_LUMA_GRAD_NORM,              /* gradient norm : sqrt(x^2 + y^2)            */
    LQR_EF_LUMA_GRAD_SUMABS,            /* sum of absulte values : |x| + |y|          */
    LQR_EF_LUMA_GRAD_XABS,              /* x absolute value : |x|                     */
    LQR_EF_NULL                         /* 0 */
};

typedef enum _LqrEnergyFuncBuiltinType LqrEnergyFuncBuiltinType;

typedef gfloat (*LqrEnergyFunc) (gint x, gint y, gint img_width, gint img_height, LqrReadingWindow *rwindow,
                                 gpointer extra_data);

LQR_PUBLIC LqrRetVal lqr_carver_set_energy_function_builtin(LqrCarver *r, LqrEnergyFuncBuiltinType ef_ind);
LQR_PUBLIC LqrRetVal lqr_carver_set_energy_function(LqrCarver *r, LqrEnergyFunc en_func, gint radius,
                                                    LqrEnergyReaderType reader_type, gpointer extra_data);

LQR_PUBLIC LqrRetVal lqr_carver_get_energy(LqrCarver *r, gfloat *buffer, gint orientation);
LQR_PUBLIC LqrRetVal lqr_carver_get_true_energy(LqrCarver *r, gfloat *buffer, gint orientation);
LQR_PUBLIC LqrRetVal lqr_carver_get_energy_image(LqrCarver *r, void *buffer, gint orientation, LqrColDepth col_depth,
                                                 LqrImageType image_type);

#endif /* __LQR_ENERGY_PUB_H__ */
