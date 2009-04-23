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


#include <math.h>
#include <lqr/lqr_gradient.h>

/**** GRADIENT FUNCTIONS ****/

double
lqr_grad_norm (double x, double y)
{
  return sqrt (x * x + y * y);
}

double
lqr_grad_norm_bias (double x, double y)
{
  return sqrt (x * x + 0.1 * y * y);
}

double
lqr_grad_sumabs (double x, double y)
{
  return (fabs (x) + fabs (y)) / 2;
}

double
lqr_grad_xabs (double x, double y)
{
  return fabs (x);
}

double
lqr_grad_yabs (double x, double y)
{
  return fabs (y);
}

double
lqr_grad_zero (double x, double y)
{
  return 0;
}


/**** END OF GRADIENT FUNCTIONS ****/
