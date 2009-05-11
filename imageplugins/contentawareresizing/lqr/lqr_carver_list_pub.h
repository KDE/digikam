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

#ifndef __LQR_CARVER_LIST_PUB_H__
#define __LQR_CARVER_LIST_PUB_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver_list_pub.h"
#endif /* __LQR_BASE_H__ */

/**** LQR_DATA_TOK STRUCT DEFINITION ****/
union _LqrDataTok;
typedef union _LqrDataTok LqrDataTok;

union _LqrDataTok {
    LqrCarver *carver;
    gint integer;
    gpointer data;
};

typedef LqrRetVal (*LqrCarverFunc) (LqrCarver *carver, LqrDataTok data);

/**** LQR_CARVER_LIST CLASS DECLARATION ****/
struct _LqrCarverList;

typedef struct _LqrCarverList LqrCarverList;

/* LQR_CARVER_LIST PUBLIC FUNCTIONS */

LQR_PUBLIC LqrCarverList *lqr_carver_list_start(LqrCarver *r);
LQR_PUBLIC LqrCarver *lqr_carver_list_current(LqrCarverList *list);
LQR_PUBLIC LqrCarverList *lqr_carver_list_next(LqrCarverList *list);
LQR_PUBLIC LqrRetVal lqr_carver_list_foreach(LqrCarverList *list, LqrCarverFunc func, LqrDataTok data);
LQR_PUBLIC LqrRetVal lqr_carver_list_foreach_recursive(LqrCarverList *list, LqrCarverFunc func, LqrDataTok data);

#endif /* __LQR_CARVER_LIST_PUB_H__ */
