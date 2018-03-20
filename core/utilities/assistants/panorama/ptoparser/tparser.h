/*============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Description : Hugin parser API
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
//  * ============================================================ */

#ifndef T_PARSER_H
#define T_PARSER_H

/* Maximum size for an input token */
#define PARSER_MAX_LINE 1000
#define PT_TOKEN_MAX_LEN PARSER_MAX_LINE

#define PANO_PARSER_MAX_PROJECTION_PARMS 10
#define PANO_PARSER_MAX_MASK_POINTS 20
/* Data structure where the entire input file will be read */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#define PANO_PARSER_COEF_COUNT 7
#define PANO_PARSER_RESP_CURVE_COEF_COUNT 5
#define PANO_PARSER_VIGN_COEF_COUNT 6
#define PANO_TRANSLATION_COEF_COUNT 3
#define PANO_PROJECTION_COEF_COUNT 2

typedef struct
{
    int x;
    int y;
} pt_point;

typedef enum
{
    NEGATIVE = 0,
    POSITIVE = 1,
    NEGATIVESTACKAWARE = 2,
    POSITVESTACKAWARE = 3,
    NEGATIVELENS = 4
} pt_mask_type;

typedef struct
{
    int iImage;
    pt_mask_type type;

    int iPointsCount;
    pt_point* points;
} pt_script_mask;

typedef struct
{
    char* varName;
    int  varIndex;
} pt_script_optimize_var;

typedef struct
{
    double x;
    double y;
} pt_point_double;

typedef struct
{
    int iImage1;
    int iImage2;
    pt_point_double p1;
    pt_point_double p2;
    int type;
} pt_script_ctrl_point;

typedef enum
{
    BD_UINT8 = 0,
    BD_UINT16 = 1,
    BD_FLOAT = 2
} pt_bitdepthoutput;

typedef struct
{
    int width;
    int height;
    int cropArea[PANO_PARSER_COEF_COUNT]; /* the rectangle to crop to */

    int projection;
    int projectionParmsCount;
    double projectionParms[PANO_PARSER_MAX_PROJECTION_PARMS];

    double fHorFOV;
    char* outputFormat;  /* n : file format of output */

    /* Hugin parameters */
    int dynamicRangeMode; /* R[01] 0 -> LDR; 1 -> HDR */
    pt_bitdepthoutput bitDepthOutput;
    double exposureValue;  /* E exposure value of final panorama */
    int iImagePhotometricReference;
}  pt_script_pano;

typedef struct
{
    int projection;
    int width;
    int height;

    double fHorFOV;
    double yaw;
    double pitch;
    double roll;

    double geometryCoef[PANO_PARSER_COEF_COUNT]; /* a, b, c, d, e, g, t */

    /* Exposure related */
    double imageEV;  /* Exposure value of image Eev */
    double whiteBalanceFactorRed;  /* Er */
    double whiteBalanceFactorBlue; /* Eb */

    double photometricCoef[PANO_PARSER_RESP_CURVE_COEF_COUNT]; /* R[abcde] */

    int vignettingCorrectionMode; /* Vm */
    double vignettingCorrectionCoef[PANO_PARSER_VIGN_COEF_COUNT]; /* V[abcdxy] */
    char* vignettingFlatFieldFile;
    double cameraPosition[PANO_TRANSLATION_COEF_COUNT]; /* TrX and TpX params */
    double projectionPlaneRotation[PANO_PROJECTION_COEF_COUNT]; /* TpX params */


    char* name;
    int cropArea[PANO_PARSER_COEF_COUNT]; /* the rectangle to crop to */

    int stack;

    /* these variables hold pointers to equivalent variables in other images
     *  they are equivalent to the format <var>=<index> where
     * <var> is variable name, and index is a base-zero pointer to another image
     * If they are -1 they are unused */
    int fHorFOVRef;
    int yawRef;
    int pitchRef;
    int rollRef;

    int geometryCoefRef[PANO_PARSER_COEF_COUNT]; /* a, b, c, d, e, g, t */

    /* image references for de-referencing (var=index) */

    int imageEVRef; /*Exposure value of image */
    int whiteBalanceFactorRedRef;  /* Er */
    int whiteBalanceFactorBlueRef; /* Eb */

    int photometricCoefRef[PANO_PARSER_RESP_CURVE_COEF_COUNT]; /* R[abcde] */

    int vignettingCorrectionModeRef; /* Vm */
    int vignettingCorrectionCoefRef[PANO_PARSER_VIGN_COEF_COUNT]; /* V[abcdxy] */

    int stackRef;
}  pt_script_image;


typedef struct
{
    double  fGamma;
    int     interpolator;
    int     fastFT;
    double  huberEstimator;
    double  photometricHuberSigma;
} pt_script_optimize;

typedef struct
{
    int iPano_prevCommentsCount;
    char** pano_prevComments;
    pt_script_pano pano;

    int iInputImagesCount;
    int* iImage_prevCommentsCount;
    char*** image_prevComments;
    pt_script_image* inputImageSpec;

    int iOptimize_prevCommentsCount;
    char** optimize_prevComments;
    pt_script_optimize optimize;

    int iVarsToOptimizeCount;
    int* iVarsToOptimize_prevCommentsCount;
    char*** varsToOptimize_prevComments;
    pt_script_optimize_var* varsToOptimize;

    int iCtrlPointsCount;
    int* iCtrlPoints_prevCommentsCount;
    char*** ctrlPoints_prevComments;
    pt_script_ctrl_point* ctrlPointsSpec;

    int iMasksCount;
    int* iMasks_prevCommentsCount;
    char*** masks_prevComments;
    pt_script_mask** masks;

    int iEndingCommentsCount;
    char** endingComments;
}  pt_script;

void    panoScriptParserSetDefaults(pt_script* ptr);
int     panoScriptParse(const char* const filename, pt_script* scriptOut);
void    panoScriptFree(pt_script* ptr);

#endif // T_PARSER_H
