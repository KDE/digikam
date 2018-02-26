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

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "tparsergetters.h"


int panoScriptGetImagesCount(pt_script* script)
{
    assert(script != NULL);
    return script->iInputImagesCount;
}

int panoScriptGetImagePrevCommentsCount(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->iImage_prevCommentsCount[i];
}

char* panoScriptGetImageComment(pt_script* script, int i, int c)
{
    assert(c >= 0 && c < panoScriptGetImagePrevCommentsCount(script, i));
    return script->image_prevComments[i][c];
}

int panoScriptGetImageProjection(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].projection;
}

int panoScriptGetImageWidth(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].width;
}

int panoScriptGetImageHeight(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].height;
}

double panoScriptGetImageHFOV(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].fHorFOV;
}

int panoScriptGetImageHFOVRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].fHorFOVRef;
}

double panoScriptGetImageYaw(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].yaw;
}

double panoScriptGetImagePitch(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].pitch;
}

double panoScriptGetImageRoll(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].roll;
}

double panoScriptGetImageCoefA(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[0];
}

int panoScriptGetImageCoefARef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[0];
}

double panoScriptGetImageCoefB(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[1];
}

int panoScriptGetImageCoefBRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[1];
}

double panoScriptGetImageCoefC(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[2];
}

int panoScriptGetImageCoefCRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[2];
}

double panoScriptGetImageCoefD(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[3];
}

int panoScriptGetImageCoefDRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[3];
}

double panoScriptGetImageCoefE(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[4];
}

int panoScriptGetImageCoefERef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[4];
}

double panoScriptGetImageSheerX(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[5];
}

int panoScriptGetImageSheerXRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[5];
}

double panoScriptGetImageSheerY(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoef[6];
}

int panoScriptGetImageSheerYRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].geometryCoefRef[6];
}

double panoScriptGetImageExposure(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].imageEV;
}

int panoScriptGetImageExposureRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].imageEVRef;
}

double panoScriptGetImageWBRed(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].whiteBalanceFactorRed;
}

int panoScriptGetImageWBRedRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].whiteBalanceFactorRedRef;
}

double panoScriptGetImageWBBlue(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].whiteBalanceFactorBlue;
}

int panoScriptGetImageWBBlueRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].whiteBalanceFactorBlueRef;
}

double panoScriptGetImagePhotometricCoeffA(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoef[0];
}

int panoScriptGetImagePhotometricCoeffARef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoefRef[0];
}

double panoScriptGetImagePhotometricCoeffB(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoef[1];
}

int panoScriptGetImagePhotometricCoeffBRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoefRef[1];
}

double panoScriptGetImagePhotometricCoeffC(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoef[2];
}

int panoScriptGetImagePhotometricCoeffCRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoefRef[2];
}

double panoScriptGetImagePhotometricCoeffD(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoef[3];
}

int panoScriptGetImagePhotometricCoeffDRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoefRef[3];
}

double panoScriptGetImagePhotometricCoeffE(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoef[4];
}

int panoScriptGetImagePhotometricCoeffERef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].photometricCoefRef[4];
}

int panoScriptGetImageVignettingMode(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionMode;
}

int panoScriptGetImageVignettingModeRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionModeRef;
}

double panoScriptGetImageVignettingCoeffA(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[0];
}

int panoScriptGetImageVignettingCoeffARef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[0];
}

double panoScriptGetImageVignettingCoeffB(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[1];
}

int panoScriptGetImageVignettingCoeffBRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[1];
}

double panoScriptGetImageVignettingCoeffC(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[2];
}

int panoScriptGetImageVignettingCoeffCRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[2];
}

double panoScriptGetImageVignettingCoeffD(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[3];
}

int panoScriptGetImageVignettingCoeffDRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[3];
}

double panoScriptGetImageVignettingCoeffX(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[4];
}

int panoScriptGetImageVignettingCoeffXRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[4];
}

double panoScriptGetImageVignettingCoeffY(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoef[5];
}

int panoScriptGetImageVignettingCoeffYRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingCorrectionCoefRef[5];
}

char* panoScriptGetImageVignettingFlatField(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].vignettingFlatFieldFile;
}

double panoScriptGetImageCameraTranslationX(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cameraPosition[0];
}

double panoScriptGetImageCameraTranslationY(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cameraPosition[1];
}

double panoScriptGetImageCameraTranslationZ(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cameraPosition[2];
}

double panoScriptGetImageProjectionPlaneYaw(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].projectionPlaneRotation[0];
}

double panoScriptGetImageProjectionPlanePitch(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].projectionPlaneRotation[1];
}

char* panoScriptGetImageName(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].name;
}

int panoScriptGetImageCropLeft(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cropArea[0];
}

int panoScriptGetImageCropRight(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cropArea[1];
}

int panoScriptGetImageCropTop(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cropArea[2];
}

int panoScriptGetImageCropBottom(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].cropArea[3];
}

int panoScriptGetImageStack(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].stack;
}

int panoScriptGetImageStackRef(pt_script* script, int i)
{
    assert(script != NULL && i >= 0 && i < script->iInputImagesCount);
    return script->inputImageSpec[i].stackRef;
}






int panoScriptGetPanoPrevCommentsCount(pt_script* script)
{
    assert(script != NULL);
    return script->iPano_prevCommentsCount;
}

char* panoScriptGetPanoComment(pt_script* script, int c)
{
    assert(script != NULL && c >= 0 && c >= 0 && c < panoScriptGetPanoPrevCommentsCount(script));
    return script->pano_prevComments[c];
}

int panoScriptGetPanoWidth(pt_script* script)
{
    assert(script != NULL);
    return script->pano.width;
}

int panoScriptGetPanoHeight(pt_script* script)
{
    assert(script != NULL);
    return script->pano.height;
}

int panoScriptGetPanoCropLeft(pt_script* script)
{
    assert(script != NULL);
    return script->pano.cropArea[0];
}

int panoScriptGetPanoCropRight(pt_script* script)
{
    assert(script != NULL);
    return script->pano.cropArea[1];
}

int panoScriptGetPanoCropTop(pt_script* script)
{
    assert(script != NULL);
    return script->pano.cropArea[2];
}

int panoScriptGetPanoCropBottom(pt_script* script)
{
    assert(script != NULL);
    return script->pano.cropArea[3];
}

int panoScriptGetPanoProjection(pt_script* script)
{
    assert(script != NULL);
    return script->pano.projection;
}

double panoScriptGetPanoProjectionParmsCount(pt_script* script)
{
    assert(script != NULL);
    return script->pano.projectionParmsCount;
}

double panoScriptGetPanoProjectionParm(pt_script* script, int index)
{
    assert(script != NULL && index < script->pano.projectionParmsCount);
    return script->pano.projectionParms[index];
}

double panoScriptGetPanoHFOV(pt_script* script)
{
    assert(script != NULL);
    return script->pano.fHorFOV;
}

int panoScriptGetPanoOutputFormat(pt_script* script)
{
    char* str = NULL;

    assert(script != NULL);

    str = script->pano.outputFormat;
    if (str == 0) {
        return 4;
    }

    switch (str[0]) {
        case 'P':
            if (strncmp("NG", str + 1, 2) == 0)
                return 0;
            break;
        case 'T':
            if (strncmp("IFF", str + 1, 3) == 0) {
                if (strncmp("_m", str + 4, 2) == 0) {
                    if (strncmp("ultilayer", str + 6, 9) == 0)
                        return 3;
                    return 2;
                }
                return 1;
            }
            break;
        case 'J':
            if (strncmp("PEG", str + 1, 3) == 0)
                return 4;
            break;
        default:
            break;
    }
    return -1;
}

int panoScriptGetPanoOutputCompression(pt_script* script)
{
    char* str = NULL;

    assert(script != NULL && panoScriptGetPanoOutputFormat(script) > 0 && panoScriptGetPanoOutputFormat(script) < 4);

    str = script->pano.outputFormat;
    while (str != NULL) {
        str = strchr(str, ' ');
        if (str != NULL) {
            if (str[1] == 'c' && str[2] == ':') {
                str += 3;
                switch (str[0]) {
                    case 'N':
                        if (strncmp("ONE", str + 1, 3) == 0)
                            return 0;
                        break;
                    case 'L':
                        if (strncmp("ZW", str + 1, 2) == 0)
                            return 1;
                        break;
                    case 'D':
                        if (strncmp("EFLATE", str + 1, 6) == 0)
                            return 2;
                        break;
                    default:
                        break;
                }
                return -1;
            }
            str++;
        }
    }
    return -1;
}

int panoScriptGetPanoOutputSaveCoordinates(pt_script* script)
{
    char* str = NULL;

    assert(script != NULL && panoScriptGetPanoOutputFormat(script) > 1 && panoScriptGetPanoOutputFormat(script) < 4);

    str = script->pano.outputFormat;
    while (str != NULL) {
        str = strchr(str, ' ');
        if (str != NULL) {
            if (str[1] == 'p') {
                if (str[2] == '1')
                    return 1;
                return 0;
            }
            str++;
        }
    }
    return 0;
}

int panoScriptGetPanoOutputCropped(pt_script* script)
{
    char* str = NULL;

    assert(script != NULL && panoScriptGetPanoOutputFormat(script) > 1 && panoScriptGetPanoOutputFormat(script) < 4);

    str = script->pano.outputFormat;
    while (str != NULL) {
        str = strchr(str, ' ');
        if (str != NULL) {
            if (str[1] == 'r') {
                if (strncmp(":CROP", str + 2, 5) == 0)
                    return 1;
                return 0;
            }
            str++;
        }
    }
    return 0;
}

int panoScriptGetPanoOutputQuality(pt_script* script)
{
    char* str = NULL;

    assert(script != NULL && panoScriptGetPanoOutputFormat(script) == 4);

    str = script->pano.outputFormat;
    while (str != NULL) {
        str = strchr(str, ' ');
        if (str != NULL) {
            if (str[1] == 'q') {
                char* last;
                int q = strtol(str + 3, &last, 10);
                if (last != str + 3)
                    return q;
                else
                    return -1;
            }
            str++;
        }
    }
    return -1;
}

int panoScriptGetPanoIsHDR(pt_script* script)
{
    assert(script != NULL);
    return script->pano.dynamicRangeMode;
}

int panoScriptGetPanoBitDepth(pt_script* script)
{
    assert(script != NULL);
    return (int) script->pano.bitDepthOutput;
}

double panoScriptGetPanoExposure(pt_script* script)
{
    assert(script != NULL);
    return script->pano.exposureValue;
}

int panoScriptGetPanoImageReference(pt_script* script)
{
    assert(script != NULL);
    return script->pano.iImagePhotometricReference;
}









int panoScriptGetOptimizePrevCommentsCount(pt_script* script)
{
    assert(script != NULL);
    return script->iOptimize_prevCommentsCount;
}

char* panoScriptGetOptimizeComment(pt_script* script, int c)
{
    assert(script != NULL && c < panoScriptGetOptimizePrevCommentsCount(script));
    return script->optimize_prevComments[c];
}

double panoScriptGetOptimizeGamma(pt_script* script)
{
    assert(script != NULL);
    return script->optimize.fGamma;
}

int panoScriptGetOptimizeInterpolator(pt_script* script)
{
    assert(script != NULL);
    return script->optimize.interpolator;
}

int panoScriptGetOptimizeSpeedUp(pt_script* script)
{
    assert(script != NULL);
    return script->optimize.fastFT;
}

double panoScriptGetOptimizeHuberSigma(pt_script* script)
{
    assert(script != NULL);
    return script->optimize.huberEstimator;
}

double panoScriptGetOptimizePhotometricHuberSigma(pt_script* script)
{
    assert(script != NULL);
    return script->optimize.photometricHuberSigma;
}




int panoScriptGetVarsToOptimizeCount(pt_script* script)
{
    assert(script != NULL);
    return script->iVarsToOptimizeCount;
}

int panoScriptGetVarsToOptimizePrevCommentCount(pt_script* script, int v)
{
    assert(script != NULL);
    return script->iVarsToOptimize_prevCommentsCount[v];
}

char* panoScriptGetVarsToOptimizeComment(pt_script* script, int v, int c)
{
    assert(script != NULL);
    return script->varsToOptimize_prevComments[v][c];
}

int panoScriptGetVarsToOptimizeImageId(pt_script* script, int v)
{
    assert(script != NULL && v >= 0 && v < script->iVarsToOptimizeCount);
    return script->varsToOptimize[v].varIndex;
}

int panoScriptGetVarsToOptimizeName(pt_script* script, int v)
{
    char* var = NULL;

    assert(script != NULL && v >= 0 && v < script->iVarsToOptimizeCount);

    var = script->varsToOptimize[v].varName;
    switch (var[0]) {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
            return var[0] - 'a';
        case 'v':
            return 5;
        case 'y':
            return 6;
        case 'p':
            return 7;
        case 'r':
            return 8;
        case 'E':
        {
            switch (var[1]) {
                case 'e':
                    return 9;
                case 'r':
                    return 10;
                case 'b':
                    return 11;
                default:
                    return 23;
            }
        }
        case 'V':
        {
            switch(var[1]) {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                    return var[1] - 'a' + 12;
                case 'x':
                case 'y':
                    return var[1] - 'x' + 16;
                default:
                    return 23;
            }
        }
        case 'R':
        {
            switch (var[1]) {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                    return var[1] - 'a' + 18;
                default:
                    return 23;
            }
        }
        default:
            return 23;
    }
}







int panoScriptGetCtrlPointCount(pt_script* script)
{
    assert(script != NULL);
    return script->iCtrlPointsCount;
}

int panoScriptGetCtrlPointPrevCommentCount(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->iCtrlPoints_prevCommentsCount[cp];
}

char* panoScriptGetCtrlPointComment(pt_script* script, int cp, int c)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPoints_prevComments[cp][c];
}

int panoScriptGetCtrlPointImage1(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].iImage1;
}

int panoScriptGetCtrlPointImage2(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].iImage2;
}

double panoScriptGetCtrlPointX1(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].p1.x;
}

double panoScriptGetCtrlPointX2(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].p2.x;
}

double panoScriptGetCtrlPointY1(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].p1.y;
}

double panoScriptGetCtrlPointY2(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].p2.y;
}

int panoScriptGetCtrlPointType(pt_script* script, int cp)
{
    assert(script != NULL && cp >= 0 && cp < panoScriptGetCtrlPointCount(script));
    return script->ctrlPointsSpec[cp].type;
}







int panoScriptGetMaskCount(pt_script* script)
{
    assert(script != NULL);
    return script->iMasksCount;
}

int panoScriptGetMaskPrevCommentCount(pt_script* script, int m)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->iCtrlPoints_prevCommentsCount[m];
}

char* panoScriptGetMaskComment(pt_script* script, int m, int c)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks_prevComments[m][c];
}

int panoScriptGetMaskImage(pt_script* script, int m)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks[m]->iImage;
}

int panoScriptGetMaskType(pt_script* script, int m)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks[m]->type;
}

int panoScriptGetMaskPointCount(pt_script* script, int m)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks[m]->iPointsCount;
}

int panoScriptGetMaskPointX(pt_script* script, int m, int p)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks[m]->points[p].x;
}

int panoScriptGetMaskPointY(pt_script* script, int m, int p)
{
    assert(script != NULL && m >= 0 && m < script->iMasksCount);
    return script->masks[m]->points[p].y;
}










int panoScriptGetEndingCommentCount(pt_script* script)
{
    assert(script != NULL);
    return script->iEndingCommentsCount;
}

char* panoScriptGetEndingComment(pt_script* script, int c)
{
    assert(script != NULL && c >= 0 && c < script->iEndingCommentsCount);
    return script->endingComments[c];
}
