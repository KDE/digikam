/*============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Description : Helper functions for the Hugin API
 *
 * Copyright (C) 2007 by Daniel M German <dmgerman at uvic doooot ca>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef T_PARSER_PRIVATE_H
#define T_PARSER_PRIVATE_H

#include "tparser.h"

/* void TokenBegin(char *t); */

int panoScriptDataReset(void);
int panoScriptParserInit(const char* const filename);
void panoScriptParserClose(void);

int  panoScriptScannerGetNextChar(char* b, int maxBuffer);
void panoScriptScannerTokenBegin(char* t);
#ifndef _MSC_VER
void panoScriptParserError(char const* errorstring, ...) __attribute__ ((format (printf, 1, 2)));
#else
void panoScriptParserError(char const* errorstring, ...);
#endif
void yyerror(char const* st);
void* panoScriptReAlloc(void** ptr, size_t size, int* count);

#endif // T_PARSER_PRIVATE_H
