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
 * ============================================================ */

#ifndef T_PARSER_GETTERS_H
#define T_PARSER_GETTERS_H

#include "tparser.h"

/* NOTE: any function name ending with "Ref" returns -1 if there are no such reference */

int     panoScriptGetImagesCount(pt_script* script);
int     panoScriptGetImagePrevCommentsCount(pt_script* script, int i);
char*   panoScriptGetImageComment(pt_script* script, int i, int c);
/* 0: Rectilinear, 1: Panoramic, 2: Circular fisheye, 3: FF fisheye, 4: equirectangular */
int     panoScriptGetImageProjection(pt_script* script, int i);
int     panoScriptGetImageWidth(pt_script* script, int i);
int     panoScriptGetImageHeight(pt_script* script, int i);
double  panoScriptGetImageHFOV(pt_script* script, int i);
int     panoScriptGetImageHFOVRef(pt_script* script, int i);
double  panoScriptGetImageYaw(pt_script* script, int i);
double  panoScriptGetImagePitch(pt_script* script, int i);
double  panoScriptGetImageRoll(pt_script* script, int i);
double  panoScriptGetImageCoefA(pt_script* script, int i);
int     panoScriptGetImageCoefARef(pt_script* script, int i);
double  panoScriptGetImageCoefB(pt_script* script, int i);
int     panoScriptGetImageCoefBRef(pt_script* script, int i);
double  panoScriptGetImageCoefC(pt_script* script, int i);
int     panoScriptGetImageCoefCRef(pt_script* script, int i);
double  panoScriptGetImageCoefD(pt_script* script, int i);
int     panoScriptGetImageCoefDRef(pt_script* script, int i);
double  panoScriptGetImageCoefE(pt_script* script, int i);
int     panoScriptGetImageCoefERef(pt_script* script, int i);
double  panoScriptGetImageSheerX(pt_script* script, int i);
int     panoScriptGetImageSheerXRef(pt_script* script, int i);
double  panoScriptGetImageSheerY(pt_script* script, int i);
int     panoScriptGetImageSheerYRef(pt_script* script, int i);
double  panoScriptGetImageExposure(pt_script* script, int i);
int     panoScriptGetImageExposureRef(pt_script* script, int i);
double  panoScriptGetImageWBRed(pt_script* script, int i);
int     panoScriptGetImageWBRedRef(pt_script* script, int i);
double  panoScriptGetImageWBBlue(pt_script* script, int i);
int     panoScriptGetImageWBBlueRef(pt_script* script, int i);
double  panoScriptGetImagePhotometricCoeffA(pt_script* script, int i);
int     panoScriptGetImagePhotometricCoeffARef(pt_script* script, int i);
double  panoScriptGetImagePhotometricCoeffB(pt_script* script, int i);
int     panoScriptGetImagePhotometricCoeffBRef(pt_script* script, int i);
double  panoScriptGetImagePhotometricCoeffC(pt_script* script, int i);
int     panoScriptGetImagePhotometricCoeffCRef(pt_script* script, int i);
double  panoScriptGetImagePhotometricCoeffD(pt_script* script, int i);
int     panoScriptGetImagePhotometricCoeffDRef(pt_script* script, int i);
double  panoScriptGetImagePhotometricCoeffE(pt_script* script, int i);
int     panoScriptGetImagePhotometricCoeffERef(pt_script* script, int i);
/* Bit0: radial, Bit1: flatfield, Bit2: proportional */
int     panoScriptGetImageVignettingMode(pt_script* script, int i);
int     panoScriptGetImageVignettingModeRef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffA(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffARef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffB(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffBRef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffC(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffCRef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffD(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffDRef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffX(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffXRef(pt_script* script, int i);
double  panoScriptGetImageVignettingCoeffY(pt_script* script, int i);
int     panoScriptGetImageVignettingCoeffYRef(pt_script* script, int i);
char*   panoScriptGetImageVignettingFlatField(pt_script* script, int i);
double  panoScriptGetImageCameraTranslationX(pt_script* script, int i);
double  panoScriptGetImageCameraTranslationY(pt_script* script, int i);
double  panoScriptGetImageCameraTranslationZ(pt_script* script, int i);
double  panoScriptGetImageProjectionPlaneYaw(pt_script* script, int i);
double  panoScriptGetImageProjectionPlanePitch(pt_script* script, int i);
char*   panoScriptGetImageName(pt_script* script, int i);
int     panoScriptGetImageCropLeft(pt_script* script, int i);
int     panoScriptGetImageCropRight(pt_script* script, int i);
int     panoScriptGetImageCropTop(pt_script* script, int i);
int     panoScriptGetImageCropBottom(pt_script* script, int i);
int     panoScriptGetImageStack(pt_script* script, int i);
int     panoScriptGetImageStackRef(pt_script* script, int i);

int     panoScriptGetPanoPrevCommentsCount(pt_script* script);
char*   panoScriptGetPanoComment(pt_script* script, int c);
int     panoScriptGetPanoWidth(pt_script* script);
int     panoScriptGetPanoHeight(pt_script* script);
int     panoScriptGetPanoCropLeft(pt_script* script);
int     panoScriptGetPanoCropRight(pt_script* script);
int     panoScriptGetPanoCropTop(pt_script* script);
int     panoScriptGetPanoCropBottom(pt_script* script);
int     panoScriptGetPanoProjection(pt_script* script);
double  panoScriptGetPanoProjectionParmsCount(pt_script* script);
double  panoScriptGetPanoProjectionParm(pt_script* script, int index);
double  panoScriptGetPanoHFOV(pt_script* script);
int     panoScriptGetPanoOutputFormat(pt_script* script);   /* 0: PNG, 1: TIFF, 2: TIFF_m, 3: TIFF_multilayer, 4: JPEG */
int     panoScriptGetPanoOutputCompression(pt_script* script); /* 0: PANO_NONE, 1: LZW, 2: DEFLATE */
int     panoScriptGetPanoOutputCropped(pt_script* script);
int     panoScriptGetPanoOutputSaveCoordinates(pt_script* script);
int     panoScriptGetPanoOutputQuality(pt_script* script);
int     panoScriptGetPanoIsHDR(pt_script* script);
int     panoScriptGetPanoBitDepth(pt_script* script);   /* 0: 8bit, 1: 16bits,2: float */
double  panoScriptGetPanoExposure(pt_script* script);
int     panoScriptGetPanoImageReference(pt_script* script);

int     panoScriptGetOptimizePrevCommentsCount(pt_script* script);
char*   panoScriptGetOptimizeComment(pt_script* script, int c);
double  panoScriptGetOptimizeGamma(pt_script* script);
/* 0: poly3, 1: spline16, 2: spline36, 3: sinc256, 4: spline64, 5: bilinear, 6: nearest neighbor, 7: sinc1024 */
int     panoScriptGetOptimizeInterpolator(pt_script* script);
int     panoScriptGetOptimizeSpeedUp(pt_script* script); /* 0: no speedup, 1: medium speedup, 2: maximum speedup */
double  panoScriptGetOptimizeHuberSigma(pt_script* script);
double  panoScriptGetOptimizePhotometricHuberSigma(pt_script* script);

int     panoScriptGetVarsToOptimizeCount(pt_script* script);
int     panoScriptGetVarsToOptimizePrevCommentCount(pt_script* script, int v);
char*   panoScriptGetVarsToOptimizeComment(pt_script* script, int v, int c);
int     panoScriptGetVarsToOptimizeImageId(pt_script* script, int v);
/* 0-4: Lens A-E, 5: hfov, 6-8: yaw / pitch / roll, 9: exposure, 10-11: WB (red / blue)
 * 12-15: Vignetting A-D, 16-17: Vignetting X-Y, 18-22: photometric A-E, 23: unknown */
int     panoScriptGetVarsToOptimizeName(pt_script* script, int v);

int     panoScriptGetCtrlPointCount(pt_script* script);
int     panoScriptGetCtrlPointPrevCommentCount(pt_script* script, int cp);
char*   panoScriptGetCtrlPointComment(pt_script* script, int cp, int c);
int     panoScriptGetCtrlPointImage1(pt_script* script, int cp);
int     panoScriptGetCtrlPointImage2(pt_script* script, int cp);
double  panoScriptGetCtrlPointX1(pt_script* script, int cp);
double  panoScriptGetCtrlPointX2(pt_script* script, int cp);
double  panoScriptGetCtrlPointY1(pt_script* script, int cp);
double  panoScriptGetCtrlPointY2(pt_script* script, int cp);
int     panoScriptGetCtrlPointType(pt_script* script, int cp);

int     panoScriptGetMaskCount(pt_script* script);
int     panoScriptGetMaskPrevCommentCount(pt_script* script, int m);
char*   panoScriptGetMaskComment(pt_script* script, int m, int c);
int     panoScriptGetMaskImage(pt_script* script, int m);
int     panoScriptGetMaskType(pt_script* script, int m); /* bit0: positive, bit1: stackaware, bit2(only): negativelens */
int     panoScriptGetMaskPointCount(pt_script* script, int m);
int     panoScriptGetMaskPointX(pt_script* script, int m, int p);
int     panoScriptGetMaskPointY(pt_script* script, int m, int p);

int     panoScriptGetEndingCommentCount(pt_script* script);
char*   panoScriptGetEndingComment(pt_script* script, int c);

#endif // T_PARSER_GETTERS_H
