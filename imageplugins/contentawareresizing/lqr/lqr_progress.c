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

#include <glib.h>

#include <lqr/lqr_base.h>
#include <lqr/lqr_progress.h>

/* LQR_PUBLIC */
LqrProgress *
lqr_progress_new(void)
{
    LqrProgress *progress;
    LQR_TRY_N_N(progress = g_try_new0(LqrProgress, 1));

    lqr_progress_set_init_width_message(progress, "Resizing width...");
    lqr_progress_set_init_height_message(progress, "Resizing height...");
    lqr_progress_set_end_width_message(progress, "done");
    lqr_progress_set_end_height_message(progress, "done");
    lqr_progress_set_update_step(progress, (float) 0.02);

    return progress;
}

LqrRetVal
lqr_progress_init(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p != NULL);
    if (p->init) {
        return p->init(message);
    } else {
        return LQR_OK;
    }
}

LqrRetVal
lqr_progress_update(LqrProgress * p, gdouble percentage)
{
    LQR_CATCH_F(p != NULL);
    if (p->update) {
        return p->update(percentage);
    } else {
        return LQR_OK;
    }
}

LqrRetVal
lqr_progress_end(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p != NULL);
    if (p->end) {
        return p->end(message);
    } else {
        return LQR_OK;
    }
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_init(LqrProgress * p, LqrProgressFuncInit init_func)
{
    p->init = init_func;
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_update(LqrProgress * p, LqrProgressFuncUpdate update_func)
{
    p->update = update_func;
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_end(LqrProgress * p, LqrProgressFuncEnd end_func)
{
    p->end = end_func;
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_update_step(LqrProgress * p, gfloat update_step)
{
    p->update_step = update_step;
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_init_width_message(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p);
    g_strlcpy(p->init_width_message, message, LQR_PROGRESS_MAX_MESSAGE_LENGTH);
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_init_height_message(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p != NULL);
    g_strlcpy(p->init_height_message, message, LQR_PROGRESS_MAX_MESSAGE_LENGTH);
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_end_width_message(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p != NULL);
    g_strlcpy(p->end_width_message, message, LQR_PROGRESS_MAX_MESSAGE_LENGTH);
    return LQR_OK;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_progress_set_end_height_message(LqrProgress * p, const gchar *message)
{
    LQR_CATCH_F(p != NULL);
    g_strlcpy(p->end_height_message, message, LQR_PROGRESS_MAX_MESSAGE_LENGTH);
    return LQR_OK;
}
