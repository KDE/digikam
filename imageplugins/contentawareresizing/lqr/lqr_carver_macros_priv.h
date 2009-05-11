/* LiquidRescaling Library
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This file was automatically generated.
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

/* Macros for update_mmap speedup : without rigidity */

#define DATADOWN(y, x) (r->raw[(y) - 1][(x)])
#define MDOWN(y, x) (r->m[DATADOWN((y), (x))])

#define MMIN01G(y, x) (least = DATADOWN((y), (x)), MDOWN((y), (x)))
#define MMINTESTL(y, x1, x2) (MDOWN((y), (x1)) <= MDOWN((y), (x2)))
#define MMINTESTR(y, x1, x2) (MDOWN((y), (x1)) <  MDOWN((y), (x2)))

#define MMINL01G(y, x01)                     MMIN01G((y), (x01))
#define MMINL02G(y, x01, x02)                MMINTESTL((y), (x01), (x02)) ? MMINL01G((y), (x01)) : MMINL01G((y), (x02))
#define MMINL03G(y, x01, x02, x03)           MMINTESTL((y), (x01), (x02)) ? MMINL02G((y), (x01), (x03)) : MMINL02G((y), (x02), (x03))
#define MMINL04G(y, x01, x02, x03, x04)      MMINTESTL((y), (x01), (x02)) ? MMINL03G((y), (x01), (x03), (x04)) : MMINL03G((y), (x02), (x03), (x04))
#define MMINL05G(y, x01, x02, x03, x04, x05) MMINTESTL((y), (x01), (x02)) ? MMINL04G((y), (x01), (x03), (x04), (x05)) : MMINL04G((y), (x02), (x03), (x04), (x05))

#define MMINR01G(y, x01)                     MMIN01G((y), (x01))
#define MMINR02G(y, x01, x02)                MMINTESTR((y), (x01), (x02)) ? MMINR01G((y), (x01)) : MMINR01G((y), (x02))
#define MMINR03G(y, x01, x02, x03)           MMINTESTR((y), (x01), (x02)) ? MMINR02G((y), (x01), (x03)) : MMINR02G((y), (x02), (x03))
#define MMINR04G(y, x01, x02, x03, x04)      MMINTESTR((y), (x01), (x02)) ? MMINR03G((y), (x01), (x03), (x04)) : MMINR03G((y), (x02), (x03), (x04))
#define MMINR05G(y, x01, x02, x03, x04, x05) MMINTESTR((y), (x01), (x02)) ? MMINR04G((y), (x01), (x03), (x04), (x05)) : MMINR04G((y), (x02), (x03), (x04), (x05))

#define MMINL01(y, x) MMINL01G((y), (x))
#define MMINL02(y, x) MMINL02G((y), (x), (x) + 1)
#define MMINL03(y, x) MMINL03G((y), (x), (x) + 1, (x) + 2)
#define MMINL04(y, x) MMINL04G((y), (x), (x) + 1, (x) + 2, (x) + 3)
#define MMINL05(y, x) MMINL05G((y), (x), (x) + 1, (x) + 2, (x) + 3, (x) + 4)

#define MMINR01(y, x) MMINR01G((y), (x))
#define MMINR02(y, x) MMINR02G((y), (x), (x) + 1)
#define MMINR03(y, x) MMINR03G((y), (x), (x) + 1, (x) + 2)
#define MMINR04(y, x) MMINR04G((y), (x), (x) + 1, (x) + 2, (x) + 3)
#define MMINR05(y, x) MMINR05G((y), (x), (x) + 1, (x) + 2, (x) + 3, (x) + 4)

/* Macros for update_mmap speedup : with rigidity */

#define MRDOWN(y, x, dx) (r->m[DATADOWN((y), (x))] + r_fact * r->rigidity_map[(dx)])

#define MRSET01(y, x, dx) (mc[(dx)] = MRDOWN((y), (x), (dx)))
#define MRSET02(y, x, dx) (MRSET01((y), (x), (dx)), MRSET01((y), (x) + 1, (dx) + 1))
#define MRSET03(y, x, dx) (MRSET02((y), (x), (dx)), MRSET01((y), (x) + 2, (dx) + 2))
#define MRSET04(y, x, dx) (MRSET03((y), (x), (dx)), MRSET01((y), (x) + 3, (dx) + 3))
#define MRSET05(y, x, dx) (MRSET04((y), (x), (dx)), MRSET01((y), (x) + 4, (dx) + 4))

#define MRMIN01G(y, x, dx) (least = DATADOWN((y), (x)), mc[(dx)])
#define MRMINTESTL(dx1, dx2) (mc[(dx1)] <= mc[(dx2)])
#define MRMINTESTR(dx1, dx2) (mc[(dx1)] < mc[(dx2)])

#define MRMINL01G(y, x01, dx01)                                             MRMIN01G((y), (x01), (dx01))
#define MRMINL02G(y, x01, dx01, x02, dx02)                                  MRMINTESTL((dx01), (dx02)) ? MRMINL01G((y), (x01), (dx01)) : MRMINL01G((y), (x02), (dx02))
#define MRMINL03G(y, x01, dx01, x02, dx02, x03, dx03)                       MRMINTESTL((dx01), (dx02)) ? MRMINL02G((y), (x01), (dx01), (x03), (dx03)) : MRMINL02G((y), (x02), (dx02), (x03), (dx03))
#define MRMINL04G(y, x01, dx01, x02, dx02, x03, dx03, x04, dx04)            MRMINTESTL((dx01), (dx02)) ? MRMINL03G((y), (x01), (dx01), (x03), (dx03), (x04), (dx04)) : MRMINL03G((y), (x02), (dx02), (x03), (dx03), (x04), (dx04))
#define MRMINL05G(y, x01, dx01, x02, dx02, x03, dx03, x04, dx04, x05, dx05) MRMINTESTL((dx01), (dx02)) ? MRMINL04G((y), (x01), (dx01), (x03), (dx03), (x04), (dx04), (x05), (dx05)) : MRMINL04G((y), (x02), (dx02), (x03), (dx03), (x04), (dx04), (x05), (dx05))

#define MRMINR01G(y, x01, dx01)                                             MRMIN01G((y), (x01), (dx01))
#define MRMINR02G(y, x01, dx01, x02, dx02)                                  MRMINTESTR((dx01), (dx02)) ? MRMINR01G((y), (x01), (dx01)) : MRMINR01G((y), (x02), (dx02))
#define MRMINR03G(y, x01, dx01, x02, dx02, x03, dx03)                       MRMINTESTR((dx01), (dx02)) ? MRMINR02G((y), (x01), (dx01), (x03), (dx03)) : MRMINR02G((y), (x02), (dx02), (x03), (dx03))
#define MRMINR04G(y, x01, dx01, x02, dx02, x03, dx03, x04, dx04)            MRMINTESTR((dx01), (dx02)) ? MRMINR03G((y), (x01), (dx01), (x03), (dx03), (x04), (dx04)) : MRMINR03G((y), (x02), (dx02), (x03), (dx03), (x04), (dx04))
#define MRMINR05G(y, x01, dx01, x02, dx02, x03, dx03, x04, dx04, x05, dx05) MRMINTESTR((dx01), (dx02)) ? MRMINR04G((y), (x01), (dx01), (x03), (dx03), (x04), (dx04), (x05), (dx05)) : MRMINR04G((y), (x02), (dx02), (x03), (dx03), (x04), (dx04), (x05), (dx05))

#define MRMINL01(y, x, dx) MRMINL01G((y), (x), (dx))
#define MRMINL02(y, x, dx) MRMINL02G((y), (x), (dx), (x) + 1, (dx) + 1)
#define MRMINL03(y, x, dx) MRMINL03G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2)
#define MRMINL04(y, x, dx) MRMINL04G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2, (x) + 3, (dx) + 3)
#define MRMINL05(y, x, dx) MRMINL05G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2, (x) + 3, (dx) + 3, (x) + 4, (dx) + 4)

#define MRMINR01(y, x, dx) MRMINR01G((y), (x), (dx))
#define MRMINR02(y, x, dx) MRMINR02G((y), (x), (dx), (x) + 1, (dx) + 1)
#define MRMINR03(y, x, dx) MRMINR03G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2)
#define MRMINR04(y, x, dx) MRMINR04G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2, (x) + 3, (dx) + 3)
#define MRMINR05(y, x, dx) MRMINR05G((y), (x), (dx), (x) + 1, (dx) + 1, (x) + 2, (dx) + 2, (x) + 3, (dx) + 3, (x) + 4, (dx) + 4)

#define UPDATE_MMAP_OPTIMISED_CASES \
     case 1: \
       m = r->leftright ? MMINR01(y, x1_min) : MMINL01(y, x1_min); \
       break; \
     case 2: \
       m = r->leftright ? MMINR02(y, x1_min) : MMINL02(y, x1_min); \
       break; \
     case 3: \
       m = r->leftright ? MMINR03(y, x1_min) : MMINL03(y, x1_min); \
       break; \
     case 4: \
       m = r->leftright ? MMINR04(y, x1_min) : MMINL04(y, x1_min); \
       break; \
     case 5: \
       m = r->leftright ? MMINR05(y, x1_min) : MMINL05(y, x1_min); \
       break;

#define UPDATE_MMAP_OPTIMISED_CASES_RIG \
     case 1: \
       MRSET01(y, x1_min, dx); \
       m = r->leftright ? MRMINR01(y, x1_min, dx) : MRMINL01(y, x1_min, dx); \
       break; \
     case 2: \
       MRSET02(y, x1_min, dx); \
       m = r->leftright ? MRMINR02(y, x1_min, dx) : MRMINL02(y, x1_min, dx); \
       break; \
     case 3: \
       MRSET03(y, x1_min, dx); \
       m = r->leftright ? MRMINR03(y, x1_min, dx) : MRMINL03(y, x1_min, dx); \
       break; \
     case 4: \
       MRSET04(y, x1_min, dx); \
       m = r->leftright ? MRMINR04(y, x1_min, dx) : MRMINL04(y, x1_min, dx); \
       break; \
     case 5: \
       MRSET05(y, x1_min, dx); \
       m = r->leftright ? MRMINR05(y, x1_min, dx) : MRMINL05(y, x1_min, dx); \
       break;
