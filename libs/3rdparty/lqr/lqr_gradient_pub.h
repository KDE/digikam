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

#ifndef __LQR_GRADIENT_PUB_H__
#define __LQR_GRADIENT_PUB_H__

enum _LqrGradFuncType {
    LQR_GF_NORM,                        /* gradient norm : sqrt(x^2 + y^2)            */
    LQR_GF_NORM_BIAS,                   /* NOT IMPLEMENTED DO NOT USE                 */
    LQR_GF_SUMABS,                      /* sum of absulte values : |x| + |y|          */
    LQR_GF_XABS,                        /* x absolute value : |x|                     */
    LQR_GF_YABS,                        /* NOT IMPLEMENTED DO NOT USE                 */
    LQR_GF_NULL                         /* 0 */
};

typedef enum _LqrGradFuncType LqrGradFuncType;

#endif /* __LQR_GRADIENT_PUB_H__ */
