/*============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Description : Hugin parser debug header
 *
 * Copyright (C) 2007 by Daniel M German <dmgerman at uvic doooot ca>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

#ifndef T_PARSER_DEBUG_H
#define T_PARSER_DEBUG_H

/* #define YYDEBUG 1 */

#ifdef YYDEBUG
#   define DEBUG_1(a) fprintf(stderr, #a "\n");
#   define DEBUG_2(a,b) fprintf(stderr, #a "\n", b);
#   define DEBUG_3(a,b,c) fprintf(stderr, #a "\n", b, c);
#   define DEBUG_4(a,b,c,d) fprintf(stderr, #a "\n", b, c, d);
#else
#   define DEBUG_1(a)
#   define DEBUG_2(a,b)
#   define DEBUG_3(a,b,c)
#   define DEBUG_4(a,b,c,d)
#endif

#endif // T_PARSER_DEBUG_H
