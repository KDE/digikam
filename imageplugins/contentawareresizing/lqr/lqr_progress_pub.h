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

#ifndef __LQR_PROGRESS_PUB_H__
#define __LQR_PROGRESS_PUB_H__

#define LQR_PROGRESS_MAX_MESSAGE_LENGTH (1024)

/* LQR_PROGRESS CLASS DECLARATION */
struct _LqrProgress;

typedef struct _LqrProgress LqrProgress;

/* LQR_PROGRESS HOOKS DECLARATIONS */
typedef LqrRetVal (*LqrProgressFuncInit) (const gchar *);
typedef LqrRetVal (*LqrProgressFuncUpdate) (gdouble);
typedef LqrRetVal (*LqrProgressFuncEnd) (const gchar *);

/* LQR_PROGRESS CLASS PUBLIC FUNCTIONS */

LQR_PUBLIC LqrProgress *lqr_progress_new(void);

LQR_PUBLIC LqrRetVal lqr_progress_set_update_step(LqrProgress * p, gfloat update_step);

LQR_PUBLIC LqrRetVal lqr_progress_set_init(LqrProgress * p, LqrProgressFuncInit init_func);
LQR_PUBLIC LqrRetVal lqr_progress_set_update(LqrProgress * p, LqrProgressFuncUpdate update_func);
LQR_PUBLIC LqrRetVal lqr_progress_set_end(LqrProgress * p, LqrProgressFuncEnd end_func);

LQR_PUBLIC LqrRetVal lqr_progress_set_init_width_message(LqrProgress * p, const gchar *message);
LQR_PUBLIC LqrRetVal lqr_progress_set_init_height_message(LqrProgress * p, const gchar *message);
LQR_PUBLIC LqrRetVal lqr_progress_set_end_width_message(LqrProgress * p, const gchar *message);
LQR_PUBLIC LqrRetVal lqr_progress_set_end_height_message(LqrProgress * p, const gchar *message);

#endif /* __LQR_PROGRESS_PUB_H__ */
