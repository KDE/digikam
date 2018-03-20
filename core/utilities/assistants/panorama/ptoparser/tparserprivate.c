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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "tparserprivate.h"
#include "tparserdebug.h"

int g_debug = 0;

static FILE* g_file = NULL;

static int g_eof = 0;
static int g_nRow = 0;
static int g_nBuffer = 0;
static int g_lBuffer = 0;
static int g_nTokenStart = 0;
static int g_nTokenLength = 0;
static int g_nTokenNextStart = 0;
static char g_buffer[PARSER_MAX_LINE + 1];
static int g_lMaxBuffer = PARSER_MAX_LINE;

extern char* yytext;

int panoScriptScannerGetNextLine(void)
{
    char* p;

    /* Reset line counters */
    g_nBuffer = 0;
    g_nTokenStart = -1;
    g_nTokenNextStart = 1;
    /* Reset marker for end of file */

    p = fgets(g_buffer, g_lMaxBuffer, g_file);

    if (p == NULL)
    {
        if (ferror(g_file))
            return -1;

        g_eof = TRUE;
        return 1;
    }

    g_nRow += 1;
    g_lBuffer = strlen(g_buffer);
    return 0;
}



int panoScriptDataReset(void)
{
    if (g_file != NULL)
    {
        return FALSE;
    }

    g_eof = FALSE;

    return TRUE;
}

int panoScriptParserInit(const char* const filename)
{
    if (g_file != NULL)
    {
        return FALSE;
    }

    g_file = fopen(filename, "r");

    if (g_file == NULL)
    {
        fprintf(stderr, "Unable to open input file");
        return FALSE;
    }

    if (panoScriptScannerGetNextLine())
    {
        panoScriptParserError("Input file is empty");
        panoScriptParserClose();
        return FALSE;
    }

    return TRUE;
}

void panoScriptParserClose(void)
{
    if (g_file != NULL)
    {
        fclose(g_file);
        g_file = NULL;
    }
}

/* This is the function that lex will use to read the next character */
int panoScriptScannerGetNextChar(char* b, int maxBuffer)
{
    int frc;

    (void) maxBuffer; /* Avoid a warning about unused parameter */

    if (g_eof)
        return 0;

    /* read next line if at the end of the current */
    while (g_nBuffer >= g_lBuffer)
    {
        frc = panoScriptScannerGetNextLine();

        if (frc != 0)
            return 0;
    }

    /* ok, return character */
    b[0] = g_buffer[g_nBuffer];
    g_nBuffer += 1;

    if (g_debug)
    {
        printf("GetNextChar() => '%c'0x%02x at %d\n", isprint(b[0]) ? b[0] : '@', b[0], g_nBuffer);
    }

    /* if string is empty, return 0 otherwise 1 */
    return b[0] == 0 ? 0 : 1;
}

void panoScriptScannerTokenBegin(char* t)
{
    /* Record where a token begins */
    g_nTokenStart = g_nTokenNextStart;
    g_nTokenLength = strlen(t);
    g_nTokenNextStart = g_nTokenStart + g_nTokenLength;
    DEBUG_4("Scanner token begin start[%d]len[%d]nextstart[%d]", g_nTokenStart, g_nTokenLength, g_nTokenNextStart);
}

/* Display parsing error, including the current line and a pointer to the error token */
void panoScriptParserError(char const* errorstring, ...)
{
    va_list args;

    int start = g_nTokenNextStart;
    int end = start + g_nTokenLength - 1;
    int i;

    DEBUG_1("Entering panoscriptparserror\n");

    fprintf(stdout, "Parsing error. Unexpected [%s]\n", yytext);

    fprintf(stdout, "\n%6d |%.*s", g_nRow, g_lBuffer, g_buffer);

    if (g_eof)
    {
        printf("       !");

        for (i = 0; i < g_lBuffer; i++)
            printf(".");

        printf("^-EOF\n");
    }
    else
    {
        printf("       !");

        for (i = 1; i < start; i++)
            printf(".");

        for (i = start; i <= end; i++)
            printf("^");

        printf("   at line %d column %d\n", g_nRow, start);
    }

    /* print it using variable arguments -----------------------------*/
    va_start(args, errorstring);
    vfprintf(stdout, errorstring, args);
    va_end(args);

    printf("\n");
}

void yyerror(char const* st)
{
    panoScriptParserError("%s\n", st);
}

/* Reallocs ptr by size, count is the variable with the current number of records allocated
 *  actual data is in 1000
 *  array located at  20 contains 1000
 *  ptr has value 20
 *  *ptr is 1000
 */
void* panoScriptReAlloc(void** ptr, size_t size, int* count)
{
    char* temp;

    void* new_ptr = realloc(*ptr, ((*count) + 1) * size);

    if (new_ptr == NULL)
    {
        /* In that case, *ptr must be freed... */
        yyerror("Not enough memory");
        return NULL;
    }

    (*count)++;
    *ptr = new_ptr;

    /* point to the newly allocated record */
    temp = (char*) *ptr;
    temp += size * ((*count) - 1);
    /* clear the area */
    memset(temp, 0, size);
    return (void*) temp;
}
