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

#include <lqr/lqr_all.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif /* __LQR_DEBUG__ */

/**** VMAP LIST FUNCTIONS ****/

LqrVMapList *
lqr_vmap_list_append(LqrVMapList *list, LqrVMap *buffer)
{
    LqrVMapList *prev = NULL;
    LqrVMapList *now = list;
    while (now != NULL) {
        prev = now;
        now = now->next;
    }
    LQR_TRY_N_N(now = g_try_new(LqrVMapList, 1));
    now->next = NULL;
    now->current = buffer;
    if (prev) {
        prev->next = now;
    }
    if (list == NULL) {
        return now;
    } else {
        return list;
    }
}

void
lqr_vmap_list_destroy(LqrVMapList *list)
{
    LqrVMapList *now = list;
    if (now != NULL) {
        lqr_vmap_list_destroy(now->next);
        lqr_vmap_destroy(now->current);
    }
}

/* LQR_PUBLIC */
LqrVMapList *
lqr_vmap_list_start(LqrCarver *r)
{
    return r->flushed_vs;
}

/* LQR_PUBLIC */
LqrVMapList *
lqr_vmap_list_next(LqrVMapList *list)
{
    LQR_TRY_N_N(list);
    return list->next;
}

/* LQR_PUBLIC */
LqrVMap *
lqr_vmap_list_current(LqrVMapList *list)
{
    LQR_TRY_N_N(list);
    return list->current;
}

/* LQR_PUBLIC */
LqrRetVal
lqr_vmap_list_foreach(LqrVMapList *list, LqrVMapFunc func, gpointer data)
{
    LqrVMapList *now = list;
    if (now != NULL) {
        LQR_CATCH(func(now->current, data));
        return lqr_vmap_list_foreach(now->next, func, data);
    }
    return LQR_OK;
}
