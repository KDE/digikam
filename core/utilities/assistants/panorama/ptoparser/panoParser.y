/*============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Description : Hugin pto file parser
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

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tparserdebug.h"
#include "tparserprivate.h"

int yylex (void);
void yyerror (char const *);
/*  Keeps track of the current type of input line in the file */
int currentLine = -1;

pt_script script;

/* defining it gives better error messages. It might be an overkill */
/* #define YYERROR_VERBOSE 1  */

static pt_script_image *image = NULL;
static pt_script_ctrl_point *ctrlPoint = NULL;
static pt_script_mask *mask = NULL;
static int nbProjParms = 0;
static double* projParms = NULL;
static int nbCommentLine = 0;
static char** commentLines = NULL;

/* copy a string while allocating and checking for memory */
static void ParserStringCopy(char **dest, const char *from)
{
    if (*dest)
        free(*dest);
    *dest = strdup(from);
    if (*dest == NULL)
        yyerror("Not enough memory");
}


%}

%defines

%union {
    int     iVal;
    double  fVal;
    char    strVal[PT_TOKEN_MAX_LEN + 1];
    char    cVal;
}

%type  <iVal>   varsparmsmask
%type  <iVal>   varparmmask
%type  <iVal>   int
%type  <fVal>   float
%type  <fVal>   intorfloat

%token <iVal>   PT_TOKEN_NUMBER_INT
%token <fVal>   PT_TOKEN_NUMBER_FLOAT
%token <strVal> PT_TOKEN_STRING
%token <strVal> PT_TOKEN_HUGIN_KEYWORD
%token <cVal>   PT_TOKEN_KEYWORD
%token <strVal> PT_TOKEN_KEYWORD_MULTICHAR
%token <cVal>   PT_TOKEN_KEYWORD_CROPPING
%token <strVal> PT_TOKEN_COMMENT

%token PT_TOKEN_EOL
%token PT_TOKEN_SEP
%token PT_TOKEN_INPUT_LINE
%token PT_TOKEN_PANO_LINE
%token PT_TOKEN_CONTROL_PT_LINE
%token PT_TOKEN_PANO_OPTIMIZE_OPT_LINE
%token PT_TOKEN_PANO_OPTIMIZE_VARS_LINE
%token PT_TOKEN_KEYWORD_MASK
%token PT_TOKEN_KEYWORD_PROJPARAMS
%token PT_TOKEN_COMMA
%token PT_TOKEN_REFERENCE
%token PT_TOKEN_MASK_PT_LINE
%token PT_TOKEN_ERROR
%token PT_TOKEN_EOF

%start input

%% /* Grammar rules and actions follow.  */

input: lines commentlines
    {
        script.iEndingCommentsCount = nbCommentLine;
        script.endingComments = commentLines;
        nbCommentLine = 0;
        commentLines = NULL;
    }

lines:  lines line
    | line

line: commentlines realline
    {
        nbCommentLine = 0;
        commentLines = NULL;
    }

commentlines: /* no comment line */
    | commentlines commentline

commentline: PT_TOKEN_COMMENT eoln
    {
        char** curComment = (char**) panoScriptReAlloc((void**) &commentLines,
                                                       sizeof(char*),
                                                       &nbCommentLine);

        if (curComment == NULL) {
            yyerror("Not enough memory");
            return -1;
        }

        *curComment = strdup($1);
    }

realline: inputline eoln
    {
        int* curImageCommentsCount = NULL;
        char*** curImageComments = NULL;
        int prevNbImages = script.iInputImagesCount - 1;

        curImageCommentsCount = (int*) panoScriptReAlloc((void**) &(script.iImage_prevCommentsCount),
                                                         sizeof(int),
                                                         &prevNbImages);
        if (curImageCommentsCount == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        *curImageCommentsCount = nbCommentLine;

        prevNbImages--;
        curImageComments = (char***) panoScriptReAlloc((void**) &(script.image_prevComments),
                                                       sizeof(char**),
                                                       &prevNbImages);
        if (curImageComments == NULL) {
            /* In that case, script.iImage_prevCommentsCount and script.image_prevComments are not coherent */
            /* Resizing to remove the element just introduced */
            script.iImage_prevCommentsCount = (int*) realloc((void*) script.iImage_prevCommentsCount, prevNbImages * sizeof(char**));
            yyerror("Not enough memory");
            return -1;
        }
        *curImageComments = commentLines;
    }
    | panoline eoln
    {
        script.iPano_prevCommentsCount = nbCommentLine;
        script.pano_prevComments = commentLines;
    }
    | optimizeOptsline eoln
    {
        script.iOptimize_prevCommentsCount = nbCommentLine;
        script.optimize_prevComments = commentLines;
    }
    | optimizeVarsline eoln
    {
        int* curVarCommentsCount = NULL;
        char*** curVarComments = NULL;
        int prevNbVars = script.iVarsToOptimizeCount - 1;

        curVarCommentsCount = (int*) panoScriptReAlloc((void**) &(script.iVarsToOptimize_prevCommentsCount),
                                                       sizeof(int),
                                                       &prevNbVars);
        if (curVarCommentsCount == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        *curVarCommentsCount = nbCommentLine;

        prevNbVars--;
        curVarComments = (char***) panoScriptReAlloc((void**) &(script.varsToOptimize_prevComments),
                                                     sizeof(char**),
                                                     &prevNbVars);
        if (curVarComments == NULL) {
            /* In that case, script.iVarsToOptimize_prevCommentsCount and script.varsToOptimize_prevComments are not coherent */
            /* Resizing to remove the element just introduced */
            script.iVarsToOptimize_prevCommentsCount = (int*) realloc((void*) script.iVarsToOptimize_prevCommentsCount, prevNbVars * sizeof(char**));
            yyerror("Not enough memory");
            return -1;
        }
        *curVarComments = commentLines;
    }
    | optimizeVarslineEmpty eoln /* Prev comments go to the next line entry */
    | ctrlPtsLine eoln
    {
        int* curCPCommentsCount = NULL;
        char*** curCPComments = NULL;
        int prevNbCP = script.iCtrlPointsCount - 1;

        curCPCommentsCount = (int*) panoScriptReAlloc((void**) &(script.iCtrlPoints_prevCommentsCount),
                                                      sizeof(int),
                                                      &prevNbCP);
        if (curCPCommentsCount == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        *curCPCommentsCount = nbCommentLine;

        prevNbCP--;
        curCPComments = (char***) panoScriptReAlloc((void**) &(script.ctrlPoints_prevComments),
                                                    sizeof(char**),
                                                    &prevNbCP);
        if (curCPComments == NULL) {
            /* In that case, script.iCtrlPoints_prevCommentsCount and script.ctrlPoints_prevComments are not coherent */
            /* Resizing to remove the element just introduced */
            script.iCtrlPoints_prevCommentsCount = (int*) realloc((void*) script.iCtrlPoints_prevCommentsCount, prevNbCP * sizeof(char**));
            yyerror("Not enough memory");
            return -1;
        }
        *curCPComments = commentLines;
    }
    | maskPtsLine eoln
    {
        int* curMaskCommentsCount = NULL;
        char*** curMaskComments = NULL;
        int prevNbMasks = script.iMasksCount - 1;

        curMaskCommentsCount = (int*) panoScriptReAlloc((void**) &(script.iMasks_prevCommentsCount),
                                                        sizeof(int),
                                                        &prevNbMasks);
        if (curMaskCommentsCount == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        *curMaskCommentsCount = nbCommentLine;

        prevNbMasks--;
        curMaskComments = (char***) panoScriptReAlloc((void**) &(script.masks_prevComments),
                                                      sizeof(char**),
                                                      &prevNbMasks);
        if (curMaskComments == NULL) {
            /* In that case, script.iMasks_prevCommentsCount and script.masks_prevComments are not coherent */
            /* Resizing to remove the element just introduced */
            script.iMasks_prevCommentsCount = (int*) realloc((void*) script.iMasks_prevCommentsCount, prevNbMasks * sizeof(char**));
            yyerror("Not enough memory");
            return -1;
        }
        *curMaskComments = commentLines;
    }


inputline: PT_TOKEN_INPUT_LINE PT_TOKEN_SEP
    {
        int i;
        currentLine = PT_TOKEN_INPUT_LINE;

        image = (pt_script_image*) panoScriptReAlloc((void**) &(script.inputImageSpec),
                                                     sizeof(pt_script_image),
                                                     &script.iInputImagesCount);

        if (image == NULL) {
            yyerror("Not enough memory");
            return -1;
        }

        image->fHorFOVRef = -1;
        image->yawRef = -1;
        image->pitchRef = -1;
        for (i = 0; i < PANO_PARSER_COEF_COUNT; i++) {
            image->geometryCoefRef[i] = -1;
        }
        image->imageEVRef = -1;
        image->whiteBalanceFactorRedRef = -1;
        image->whiteBalanceFactorBlueRef = -1;
        for (i = 0; i < PANO_PARSER_RESP_CURVE_COEF_COUNT; i++) {
            image->photometricCoefRef[i] = -1;
        }
        image->vignettingCorrectionModeRef = -1;
        for (i = 0; i < PANO_PARSER_VIGN_COEF_COUNT; i++) {
            image->vignettingCorrectionCoefRef[i] = -1;
        }
        image->stackRef = -1;
    }
    varsinput

panoline: PT_TOKEN_PANO_LINE PT_TOKEN_SEP
    {
        currentLine = PT_TOKEN_PANO_LINE;
    }
    vars

optimizeOptsline: PT_TOKEN_PANO_OPTIMIZE_OPT_LINE PT_TOKEN_SEP
    {
        currentLine = PT_TOKEN_PANO_OPTIMIZE_OPT_LINE;
    }
    vars


/* We get a new variable to optimize */
optimizeVarsline: PT_TOKEN_PANO_OPTIMIZE_VARS_LINE PT_TOKEN_SEP
    {
        currentLine = PT_TOKEN_PANO_OPTIMIZE_VARS_LINE;
    } varsOpt

optimizeVarslineEmpty: PT_TOKEN_PANO_OPTIMIZE_VARS_LINE

ctrlPtsLine: PT_TOKEN_CONTROL_PT_LINE  PT_TOKEN_SEP
    {
        currentLine = PT_TOKEN_CONTROL_PT_LINE;
        ctrlPoint = (pt_script_ctrl_point*) panoScriptReAlloc((void**) &script.ctrlPointsSpec,
                                                              sizeof(pt_script_ctrl_point),
                                                              &script.iCtrlPointsCount);
        if (ctrlPoint == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
    }
    varsparms

maskPtsLine: PT_TOKEN_MASK_PT_LINE  PT_TOKEN_SEP
    {
        currentLine = PT_TOKEN_MASK_PT_LINE;
        mask = (pt_script_mask*) malloc(sizeof(pt_script_mask));
        if (mask == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
    }
    varsparmsmask
    {
        pt_script_mask** maskPtr = NULL;

        if ($4 == -1) {
            yyerror("Mask line without an image reference");
            return -1;
        }
        if ($4 >= script.iInputImagesCount) {
            yyerror("Mask line referencing a missing input image");
            return -1;
        }
        maskPtr = (pt_script_mask**) panoScriptReAlloc((void**) &script.masks,
                                                       sizeof(pt_script_mask*),
                                                       &script.iMasksCount);

        if (maskPtr == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        *maskPtr = mask;
    }

eoln: PT_TOKEN_EOL
    {
        DEBUG_1("ENDOFLINE");
        currentLine = -1; /* This says we don't know the type of line being processed */
    }





/* Variable to be optimized */

varsOpt:  varOpt
        | varsOpt PT_TOKEN_SEP varOpt

varOpt: PT_TOKEN_KEYWORD_MULTICHAR int
    {
        pt_script_optimize_var* varToOptimize = (pt_script_optimize_var*) panoScriptReAlloc((void**) &script.varsToOptimize,
                                                                                            sizeof(pt_script_optimize_var),
                                                                                            &script.iVarsToOptimizeCount);
        if (varToOptimize == NULL) {
            yyerror("Not enough memory");
            return -1;
        }

        varToOptimize->varName = strdup($1);
        varToOptimize->varIndex = $2;
    }
    | PT_TOKEN_KEYWORD int
    {
        char keyword[2];

        pt_script_optimize_var* varToOptimize = (pt_script_optimize_var*) panoScriptReAlloc((void**) &script.varsToOptimize,
                                                                                            sizeof(pt_script_optimize_var),
                                                                                            &script.iVarsToOptimizeCount);
        if (varToOptimize == NULL) {
            yyerror("Not enough memory");
            return -1;
        }

        keyword[0] = $1;
        keyword[1] = 0;

        varToOptimize->varName = strdup(keyword);
        varToOptimize->varIndex = $2;
    }

varsinput: varinput
    | varsinput PT_TOKEN_SEP varinput

vars: var
    | vars PT_TOKEN_SEP var

/* a variable can be a cropping one (with 4 parms), a one-parm one, a reference to another variable,
 * or finally, a name only */
varinput: varcropping
    | varreference
    | varparameter

var:  varcropping
    | varparameter
    | varonly

varsparms: varparameter
    | varsparms PT_TOKEN_SEP varparameter

varsparmsmask: varparmmask
    {
        $$ = $1;
    }
    | varsparmsmask PT_TOKEN_SEP varparmmask
    {
        if ($1 != -1)
            $$ = $1;
        else
            $$ = $3;
    }

varparmmask: PT_TOKEN_KEYWORD int
    {
        $$ = -1;
        switch ($1) {
        case 'i':
            mask->iImage = $2;
            $$ = $2;
            break;
        case 't':
            mask->type = $2;
            break;
        default:
            panoScriptParserError("Invalid variable name [%c] in mask line.\n", $1);
            return -1;
        }
    }
    | PT_TOKEN_KEYWORD_MASK mask
    {
        $$ = -1;
        mask->points = NULL;
        mask->iPointsCount = 0;
    }

mask: maskpoint
    | mask PT_TOKEN_COMMA maskpoint

maskpoint: int PT_TOKEN_COMMA int
    {
        pt_point* maskPointPtr = (pt_point*) panoScriptReAlloc((void**) &mask->points,
                                                               sizeof(pt_point),
                                                               &mask->iPointsCount);

        if (maskPointPtr == NULL) {
            yyerror("Not enough memory");
            return -1;
        }
        maskPointPtr->x = $1;
        maskPointPtr->y = $3;
    }

/* Rule for [CS]<x>,<x>,<x>,<x> */
varcropping: PT_TOKEN_KEYWORD_CROPPING int PT_TOKEN_COMMA int PT_TOKEN_COMMA int PT_TOKEN_COMMA int
    {
        int* cropArea = NULL;
        printf("Cropping...\n");

        if (currentLine != PT_TOKEN_PANO_LINE && currentLine != PT_TOKEN_INPUT_LINE) {
            panoScriptParserError("Error: There shouldn't be any cropping parameter here!\n");
            return -1;
        }

        if (currentLine == PT_TOKEN_PANO_LINE) {
            cropArea = script.pano.cropArea;
        } else {
            cropArea = image->cropArea;
        }
        switch ($1) {
        case 'C':
        case 'S':
            cropArea[0] = $2;
            cropArea[1] = $4;
            cropArea[2] = $6;
            cropArea[3] = $8;
            break;
        default:
            panoScriptParserError("Invalid variable name- [%c] in image line\n", $1);
            return -1;
        }
    }

/* Rule for input image field references <var>=<index> */
varreference: PT_TOKEN_KEYWORD_MULTICHAR PT_TOKEN_REFERENCE int
    {
        int imageRef = $3;
        char *keyword = $1;

        if (currentLine != PT_TOKEN_INPUT_LINE) {
            panoScriptParserError("Error: References should only be present on i lines!\n");
            return -1;
        }

        switch (*keyword) {
        case 'R':
        {
            switch (*(keyword + 1)) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                if (*(keyword + 2) == '\0') {
                    image->photometricCoefRef[*(keyword + 1) - 'a'] = imageRef;
                    break;
                }
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        case 'V':
        {
            if (*(keyword + 2) != '\0') {
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            switch (*(keyword + 1)) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                image->vignettingCorrectionCoefRef[*(keyword + 1) - 'a'] = imageRef;
                break;
            case 'm':
                image->vignettingCorrectionModeRef = imageRef;
                break;
            case 'x':
            case 'y':
                image->vignettingCorrectionCoefRef[*(keyword + 1) - 'x' + 4] = imageRef;
                break;
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        case 'E':
        {
            if (strcmp(keyword, "Eev") == 0) {
                image->imageEVRef = imageRef;
                break;
            }


            if (*(keyword + 2) != '\0') {
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            switch (*(keyword + 1)) {
            case 'r':
                image->whiteBalanceFactorRedRef = imageRef;
                break;
            case 'b':
                image->whiteBalanceFactorBlueRef = imageRef;
                break;
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        default:
            panoScriptParserError("Invalid variable name [%s]\n", keyword);
            return -1;
        }
    }
   | PT_TOKEN_KEYWORD PT_TOKEN_REFERENCE int
    {
        int imageRef = $3;

        if (currentLine != PT_TOKEN_INPUT_LINE) {
            panoScriptParserError("Error: References should only be present on i lines!\n");
            return -1;
        }
        switch ($1) {
        case 'v':
            image->fHorFOVRef = imageRef;
            break;
        case 'y':
            image->yawRef = imageRef;
            break;
        case 'p':
            image->pitchRef = imageRef;
            break;
        case 'r':
            image->rollRef = imageRef;
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
            image->geometryCoefRef[$1 - 'a'] = imageRef;
            break;
        case 'g':
            image->geometryCoefRef[5] = imageRef;
            break;
        case 't':
            image->geometryCoefRef[6] = imageRef;
            break;
        case 'j':
            image->stackRef = imageRef;
            break;
        default:
            panoScriptParserError("Invalid variable name [%c=] in input line.\n", $1);
            return -1;
        }
    }


/* Rules for <variable><parameter> */
varparameter: PT_TOKEN_KEYWORD PT_TOKEN_STRING
    {
        /* For the case where the keyword is one char, followed by a string */

        DEBUG_2("Token %s", $2);

        /* Processing of string variables */
        switch (currentLine) {
        case PT_TOKEN_PANO_LINE:
            switch ($1) {
            case 'n':
                ParserStringCopy(&script.pano.outputFormat, $2);
                break;
            case 'T':
                if (strncmp($2, "UINT", 4) == 0) {
                    if (strcmp($2 + 4, "8") == 0) {
                        script.pano.bitDepthOutput = BD_UINT8;
                    } else if (strcmp($2 + 4, "16") == 0) {
                        script.pano.bitDepthOutput = BD_UINT16;
                    } else {
                        panoScriptParserError("Invalid bitdepth [%s] in pano line\n", $2);
                        return -1;
                    }
                } else if (strcmp($2, "FLOAT") == 0) {
                        script.pano.bitDepthOutput = BD_FLOAT;
                } else {
                    panoScriptParserError("Invalid bitdepth [%s] in pano line\n", $2);
                    return -1;
                }
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in pano line\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_INPUT_LINE:
            if ($1 != 'n') {
                panoScriptParserError("Invalid variable name [%c] in image line...\n", $1);
                return -1;
            }
            ParserStringCopy(&image->name, $2);
            break;
        default:
            panoScriptParserError("Error Not handled case [%c]\n", $1);
            return -1;
        }
    }
    | PT_TOKEN_KEYWORD_PROJPARAMS projparams
    {
        if (currentLine != PT_TOKEN_PANO_LINE) {
            panoScriptParserError("Unexpected 'P' parameter!!\n");
        }
        nbProjParms = 0;
        projParms = NULL;
    }
    | PT_TOKEN_KEYWORD int
    {
        /* Processing of int variables with keyword of one character only */
        switch (currentLine) {
        case PT_TOKEN_CONTROL_PT_LINE:
            switch ($1) {
            case 'n':
                ctrlPoint->iImage1 = $2;
                break;
            case 'N':
                ctrlPoint->iImage2 = $2;
                break;
            case 'x':
                ctrlPoint->p1.x = $2;
                break;
            case 'y':
                ctrlPoint->p1.y = $2;
                break;
            case 'X':
                ctrlPoint->p2.x = $2;
                break;
            case 'Y':
                ctrlPoint->p2.y = $2;
                break;
            case 't':
                ctrlPoint->type = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in control point line.\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_PANO_LINE:
            switch ($1) {
            case 'w':
                script.pano.width = $2;
                break;
            case 'h':
                script.pano.height = $2;
                break;
            case 'f':
                script.pano.projection = $2;
                break;
            case 'v':
                script.pano.fHorFOV = $2;
                break;
            case 'k':
                script.pano.iImagePhotometricReference = $2;
                break;
            case 'E':
                script.pano.exposureValue = $2;
                break;
            case 'R':
                script.pano.dynamicRangeMode = $2;
                break;
            default:
                panoScriptParserError("Error Invalid variable name [%c] in pano line\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_INPUT_LINE:
            switch ($1) {
            case 'w':
                image->width = $2;
                break;
            case 'h':
                image->height = $2;
                break;
            case 'f':
                image->projection = $2;
                break;
            case 'v':
                image->fHorFOV = $2;
                break;
            case 'y':
                image->yaw = $2;
                break;
            case 'p':
                image->pitch = $2;
                break;
            case 'r':
                image->roll = $2;
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                image->geometryCoef[$1 - 'a'] = $2;
                break;
            case 'g':
                image->geometryCoef[5] = $2;
                break;
            case 't':
                image->geometryCoef[6] = $2;
                break;
            case 'j':
                image->stack = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in image line...\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_PANO_OPTIMIZE_OPT_LINE:
            switch ($1) {
            case 'g':
                script.optimize.fGamma = $2;
                if (script.optimize.fGamma <= 0.0) {
                    panoScriptParserError("Invalid value for gamma %f. Must be bigger than zero\n", script.optimize.fGamma);
                }
            break;
            case 'i':
                script.optimize.interpolator = $2;
                break;
            case 'f':
                script.optimize.fastFT = 2 - $2;
                break;
            case 'm':
                script.optimize.huberEstimator = $2;
                break;
            case 'p':
                script.optimize.photometricHuberSigma = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in optimize line\n", $1);
                return -1;
            }
            break;
        default:
            panoScriptParserError("Error. Not handled (token int [%c])\n", $1);
            return -1;
        }
    }
    | PT_TOKEN_KEYWORD float
    {
        /* Processing of int variables with keyword of one character only */
        switch (currentLine) {
        case PT_TOKEN_CONTROL_PT_LINE:
            switch ($1) {
            case 'x':
                ctrlPoint->p1.x = $2;
                break;
            case 'y':
                ctrlPoint->p1.y = $2;
                break;
            case 'X':
                ctrlPoint->p2.x = $2;
                break;
            case 'Y':
                ctrlPoint->p2.y = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in control point line.\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_PANO_LINE:
            switch ($1) {
            case 'v':
                script.pano.fHorFOV = $2;
                break;
            case 'E':
                script.pano.exposureValue = $2;
                break;
            default:
                panoScriptParserError("Error Invalid variable name [%c] in pano line\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_INPUT_LINE:
            switch ($1) {
            case 'v':
                image->fHorFOV = $2;
                break;
            case 'y':
                image->yaw = $2;
                break;
            case 'p':
                image->pitch = $2;
                break;
            case 'r':
                image->roll = $2;
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                image->geometryCoef[$1 - 'a'] = $2;
                break;
            case 'g':
                image->geometryCoef[5] = $2;
                break;
            case 't':
                image->geometryCoef[6] = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in image line...\n", $1);
                return -1;
            }
            break;
        case PT_TOKEN_PANO_OPTIMIZE_OPT_LINE:
            switch ($1) {
            case 'g':
                script.optimize.fGamma = $2;
                if (script.optimize.fGamma <= 0.0) {
                    panoScriptParserError("Invalid value for gamma %f. Must be bigger than zero\n", script.optimize.fGamma);
                }
                break;
            case 'm':
                script.optimize.huberEstimator = $2;
                break;
            case 'p':
                script.optimize.photometricHuberSigma = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%c] in optimize line\n", $1);
                return -1;
            }
            break;
        default:
            panoScriptParserError("Error. Not handled (token int [%c])\n", $1);
            return -1;
        }
    }
    | PT_TOKEN_KEYWORD_MULTICHAR PT_TOKEN_STRING
    {
        if (currentLine != PT_TOKEN_INPUT_LINE) {
            panoScriptParserError("Invalid variable name [%s]\n", $1);
            return -1;
        }
        if (strcmp($1, "Vf") != 0) {
            panoScriptParserError("Invalid variable name [%s] in image line...\n", $1);
            return -1;
        }
        ParserStringCopy(&image->vignettingFlatFieldFile, $2);
        break;
    }
    | PT_TOKEN_KEYWORD_MULTICHAR intorfloat
    {
        char *keyword = $1;

        switch (*keyword) {
        case 'R':
        {
            switch (*(keyword + 1)) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                if (*(keyword + 2) == '\0') {
                    image->photometricCoef[*(keyword + 1) - 'a'] = $2;
                    break;
                }
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        case 'V':
        {
            if (*(keyword + 2) != '\0') {
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            switch (*(keyword + 1)) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
                image->vignettingCorrectionCoef[*(keyword + 1) - 'a'] = $2;
                break;
            case 'm':
                image->vignettingCorrectionMode = lround($2);
                break;
            case 'x':
            case 'y':
                image->vignettingCorrectionCoef[*(keyword + 1) - 'x' + 4] = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        case 'E':
        {
            if (strcmp(keyword + 1, "ev") == 0) {
                image->imageEV = $2;
                break;
            }


            if (*(keyword + 2) != '\0') {
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            switch (*(keyword + 1)) {
            case 'r':
                image->whiteBalanceFactorRed = $2;
                break;
            case 'b':
                image->whiteBalanceFactorBlue = $2;
                break;
            default:
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        case 'T':
        {
            if (*(keyword + 1) == 'r' && *(keyword + 3) == '\0') {
                switch (*(keyword + 2)) {
                case 'X':
                case 'Y':
                case 'Z':
                    image->cameraPosition[*(keyword + 2) - 'X'] = $2;
                    break;
                default:
                    panoScriptParserError("Invalid variable name [%s]\n", keyword);
                    return -1;
                }
            } else if (*(keyword + 1) == 'p' && *(keyword + 3) == '\0') {
                switch (*(keyword + 2)) {
                case 'y':
                    image->projectionPlaneRotation[0] = $2;
                    break;
                case 'p':
                    image->projectionPlaneRotation[1] = $2;
                    break;
                default:
                    panoScriptParserError("Invalid variable name [%s]\n", keyword);
                    return -1;
                }
            } else {
                panoScriptParserError("Invalid variable name [%s]\n", keyword);
                return -1;
            }
            break;
        }
        default:
            panoScriptParserError("Invalid variable name [%s]\n", keyword);
            return -1;
        }
    }



varonly: PT_TOKEN_KEYWORD
    {
        switch (currentLine) {
        case PT_TOKEN_PANO_LINE:
            if ($1 != 'T') {
                panoScriptParserError("Invalid variable name [%c] in pano line\n", $1);
                return -1;
            }
            script.pano.bitDepthOutput = BD_UINT8;
            break;
        case PT_TOKEN_INPUT_LINE:
            panoScriptParserError("Invalid variable name [%c] in image line....\n", $1);
            return -1;
        default:
            panoScriptParserError("Error Not handled 3\n");
            return -1;
        }
    }

projparams: intorfloat
    {
        double* param = (double*) panoScriptReAlloc((void**) &projParms,
                                                    sizeof(double),
                                                    &nbProjParms);
        *param = $1;
    }
    | projparams PT_TOKEN_COMMA intorfloat
    {
        double* param = (double*) panoScriptReAlloc((void**) &projParms,
                                                    sizeof(double),
                                                    &nbProjParms);
        *param = $3;
    }

float: PT_TOKEN_NUMBER_FLOAT {$$ = $1;}

int: PT_TOKEN_NUMBER_INT {$$ = $1;}

intorfloat : PT_TOKEN_NUMBER_FLOAT {$$ = $1;}
    | PT_TOKEN_NUMBER_INT {$$ = $1;}


%%

