/* */
/*  Little cms - profiler construction set */
/*  Copyright (C) 1998-2001 Marti Maria <marti@littlecms.com> */
/* */
/* THIS SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY */
/* WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. */
/* */
/* IN NO EVENT SHALL MARTI MARIA BE LIABLE FOR ANY SPECIAL, INCIDENTAL, */
/* INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, */
/* OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, */
/* WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF */
/* LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE */
/* OF THIS SOFTWARE. */
/* */
/* */
/* This library is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU Lesser General Public */
/* License as published by the Free Software Foundation; either */
/* version 2 of the License, or (at your option) any later version. */
/* */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU */
/* Lesser General Public License for more details. */
/* */
/* You should have received a copy of the GNU Lesser General Public */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "lcmsprf.h"


double cdecl _cmsxSaturate65535To255(double d);
double cdecl _cmsxSaturate255To65535(double d);


void   cdecl _cmsxClampXYZ100(LPcmsCIEXYZ xyz);


BOOL cdecl cmsxEmbedCharTarget(LPPROFILERCOMMONDATA hdr);
BOOL cdecl cmsxEmbedMatrixShaper(LPPROFILERCOMMONDATA hdr);
BOOL cdecl cmsxEmbedTextualInfo(LPPROFILERCOMMONDATA hdr);

/* ----------------------------------------------------------------- Implementation */


/* Convert from 0.0..65535.0 to 0.0..255.0 */

double _cmsxSaturate65535To255(double d)
{   
    double v;

    v = d / 257.0;

	if (v < 0)     return 0;
    if (v > 255.0) return 255.0;

    return v;
}


double _cmsxSaturate255To65535(double d)
{   
    double v;

    v = d * 257.0;

	if (v < 0)     return 0;
    if (v > 65535.0) return 65535.0;

    return v;
}



/* Cut off absurd values */

void _cmsxClampXYZ100(LPcmsCIEXYZ xyz)
{

        if (xyz->X > 199.996)           
                xyz->X = 199.996;
                
        if (xyz->Y > 199.996)           
                xyz->Y = 199.996;
        
        if (xyz->Z > 199.996)           
                xyz->Z = 199.996;
        
		 if (xyz->Y < 0)
                xyz->Y = 0;

        if (xyz->X < 0)
                xyz->X = 0;

        if (xyz->Z < 0)
                xyz->Z = 0;

}

static
int xfilelength(int fd)
{
#ifdef _MSC_VER
		return _filelength(fd);
#else
        struct stat sb;
        if (fstat(fd, &sb) < 0)
                return(-1);
        return(sb.st_size);
#endif


}


BOOL cmsxEmbedCharTarget(LPPROFILERCOMMONDATA hdr)
{
		LCMSHANDLE it8 = cmsxIT8Alloc();
		LPBYTE mem;
		size_t size, readed;
		FILE* f;
		BOOL lFreeOnExit = false;


		if (!hdr->m.Patches) {

			if (!hdr ->ReferenceSheet[0] && !hdr->MeasurementSheet[0]) return false;

			if (cmsxPCollBuildMeasurement(&hdr ->m, 
                                    hdr->ReferenceSheet, 
                                    hdr->MeasurementSheet,
                                    PATCH_HAS_RGB|PATCH_HAS_XYZ) == false) return false;
			lFreeOnExit = true;
			
		}

		cmsxIT8SetSheetType(it8,"LCMSEMBED");
		cmsxIT8SetProperty(it8, "ORIGINATOR",   (const char *) "Little cms");
		cmsxIT8SetProperty(it8, "DESCRIPTOR",   (const char *) hdr -> Description);
		cmsxIT8SetProperty(it8, "MANUFACTURER", (const char *) hdr ->Manufacturer);
					
		cmsxPCollSaveToSheet(&hdr->m, it8);
		cmsxIT8SaveToFile(it8, "TMP00.IT8");
		cmsxIT8Free(it8);

		f = fopen("TMP00.IT8", "rb");
		size = xfilelength(fileno(f));
		mem = (unsigned char*) malloc(size + 1);          // C->C++ : fixed cast
		readed = fread(mem, 1, size, f);
		fclose(f);

		mem[readed] = 0;
		unlink("TMP00.IT8");

		cmsAddTag(hdr->hProfile, icSigCharTargetTag, mem);
		free(mem);

		if (lFreeOnExit) {
			
			cmsxPCollFreeMeasurements(&hdr->m);
		}

		return true;
}


static
BOOL ComputeColorantMatrix(LPcmsCIEXYZTRIPLE Colorants, 
						   LPcmsCIExyY WhitePoint, 
						   LPcmsCIExyYTRIPLE Primaries)
{
	   MAT3 MColorants;
       
	   if (!cmsBuildRGB2XYZtransferMatrix(&MColorants, WhitePoint, Primaries))
       {       
              return false;
       }  

	  
	   cmsAdaptMatrixToD50(&MColorants, WhitePoint);

       Colorants->Red.X = MColorants.v[0].n[0];
       Colorants->Red.Y = MColorants.v[1].n[0];
       Colorants->Red.Z = MColorants.v[2].n[0];

       Colorants->Green.X = MColorants.v[0].n[1];
       Colorants->Green.Y = MColorants.v[1].n[1];
       Colorants->Green.Z = MColorants.v[2].n[1];

       Colorants->Blue.X = MColorants.v[0].n[2];
       Colorants->Blue.Y = MColorants.v[1].n[2];
       Colorants->Blue.Z = MColorants.v[2].n[2];

	   return true;

}


BOOL cmsxEmbedMatrixShaper(LPPROFILERCOMMONDATA hdr)
{
	cmsCIEXYZTRIPLE Colorant;
	cmsCIExyY MediaWhite;

		cmsXYZ2xyY(&MediaWhite, &hdr ->WhitePoint);

		if (ComputeColorantMatrix(&Colorant, &MediaWhite, &hdr ->Primaries)) {

		cmsAddTag(hdr ->hProfile, icSigRedColorantTag, &Colorant.Red);
		cmsAddTag(hdr ->hProfile, icSigGreenColorantTag, &Colorant.Green);
		cmsAddTag(hdr ->hProfile, icSigBlueColorantTag, &Colorant.Blue);
		}

		cmsAddTag(hdr ->hProfile, icSigRedTRCTag, hdr ->Gamma[0]);
		cmsAddTag(hdr ->hProfile, icSigGreenTRCTag, hdr ->Gamma[1]);
		cmsAddTag(hdr ->hProfile, icSigBlueTRCTag, hdr ->Gamma[2]);

		return true;
}


BOOL cmsxEmbedTextualInfo(LPPROFILERCOMMONDATA hdr)
{
	    if (*hdr ->Description)
           cmsAddTag(hdr ->hProfile, icSigProfileDescriptionTag, hdr ->Description);
     
        if (*hdr ->Copyright)
           cmsAddTag(hdr ->hProfile, icSigCopyrightTag,          hdr ->Copyright);
     
        if (*hdr ->Manufacturer)
           cmsAddTag(hdr ->hProfile, icSigDeviceMfgDescTag,      hdr ->Manufacturer);
     
        if (*hdr ->Model)
           cmsAddTag(hdr ->hProfile, icSigDeviceModelDescTag,    hdr ->Model);

		return true;
}



void cmsxChromaticAdaptationAndNormalization(LPPROFILERCOMMONDATA hdr, LPcmsCIEXYZ xyz, BOOL lReverse)
{
    
        if (hdr->lUseCIECAM97s) {

                cmsJCh JCh;

                /* Let's CIECAM97s to do the adaptation to D50 */
			   
               xyz->X *= 100.;
               xyz->Y *= 100.;
               xyz->Z *= 100.;

               _cmsxClampXYZ100(xyz);

			   if (lReverse) {
				   	cmsCIECAM97sForward(hdr->hPCS, xyz, &JCh);
					cmsCIECAM97sReverse(hdr->hDevice, &JCh, xyz);
			   }
			   else {

					cmsCIECAM97sForward(hdr->hDevice, xyz, &JCh);
					cmsCIECAM97sReverse(hdr->hPCS, &JCh, xyz);
			   }

               _cmsxClampXYZ100(xyz);

               xyz -> X /= 100.;
               xyz -> Y /= 100.;
               xyz -> Z /= 100.;
							   
        }
        else {
                    
              /* Else, use Bradford */

			if (lReverse) {
				 cmsAdaptToIlluminant(xyz, cmsD50_XYZ(), &hdr->WhitePoint,  xyz);            
			}
			else {
			  cmsAdaptToIlluminant(xyz, &hdr->WhitePoint, cmsD50_XYZ(), xyz);            
			}
               
       }
        
}


void cmsxInitPCSViewingConditions(LPPROFILERCOMMONDATA hdr)
{
	  
	    hdr->PCS.whitePoint.X = cmsD50_XYZ()->X * 100.;
		hdr->PCS.whitePoint.Y = cmsD50_XYZ()->Y * 100.;
		hdr->PCS.whitePoint.Z = cmsD50_XYZ()->Z * 100.;


        hdr->PCS.Yb = 20;                     /* 20% of surround */
        hdr->PCS.La = 20;                     /* Adapting field luminance */
        hdr->PCS.surround = AVG_SURROUND;
        hdr->PCS.D_value  = 1.0;			 /* Complete adaptation */

}


/* Build gamut hull by geometric means */
void cmsxComputeGamutHull(LPPROFILERCOMMONDATA hdr)
{	
	int i;
	int x0, y0, z0;
	int Inside, Outside, Boundaries;
	char code;


	hdr -> hRGBHull = cmsxHullInit();

	/* For all valid patches, mark RGB knots as 0 */
	for (i=0; i < hdr ->m.nPatches; i++) {

        if (hdr ->m.Allowed[i]) {

            LPPATCH p = hdr ->m.Patches + i;


			x0 = (int) floor(p->Colorant.RGB[0]  + .5);
			y0 = (int) floor(p->Colorant.RGB[1]  + .5);
			z0 = (int) floor(p->Colorant.RGB[2]  + .5);

			cmsxHullAddPoint(hdr->hRGBHull, x0, y0, z0);			
		}
	}
	
	cmsxHullComputeHull(hdr ->hRGBHull);

/* #ifdef DEBUG */
	cmsxHullDumpVRML(hdr -> hRGBHull, "rgbhull.wrl");
/* #endif */



	/* A check */

	Inside = Outside = Boundaries = 0;
	/* For all valid patches, mark RGB knots as 0 */
	for (i=0; i < hdr ->m.nPatches; i++) {

        if (hdr ->m.Allowed[i]) {

            LPPATCH p = hdr ->m.Patches + i;

			x0 = (int) floor(p->Colorant.RGB[0]  + .5);
			y0 = (int) floor(p->Colorant.RGB[1]  + .5);
			z0 = (int) floor(p->Colorant.RGB[2]  + .5);

			code = cmsxHullCheckpoint(hdr -> hRGBHull, x0, y0, z0);

			switch (code) {
			
			case 'i': Inside++; break;
			case 'o': Outside++; break;
			default:  Boundaries++; 				
			}

		}
	}

	if (hdr ->printf)
		hdr ->printf("Gamut hull: %d inside, %d outside, %d on boundaries", Inside, Outside, Boundaries);
	
}

BOOL cmsxChoosePCS(LPPROFILERCOMMONDATA hdr)
{

        double gamma_r, gamma_g, gamma_b;        
        cmsCIExyY SourceWhite;

        /* At first, compute aproximation on matrix-shaper */
        if (!cmsxComputeMatrixShaper(hdr ->ReferenceSheet,                                 
                                     hdr ->MeasurementSheet,
									 hdr -> Medium,
                                     hdr ->Gamma,
                                     &hdr ->WhitePoint,
                                     &hdr ->BlackPoint,
                                     &hdr ->Primaries)) return false;

        		

		cmsXYZ2xyY(&SourceWhite,   &hdr ->WhitePoint);	
		    
        gamma_r = cmsEstimateGamma(hdr ->Gamma[0]);
        gamma_g = cmsEstimateGamma(hdr ->Gamma[1]);
        gamma_b = cmsEstimateGamma(hdr ->Gamma[2]);

		

        if (gamma_r > 1.8 || gamma_g > 1.8 || gamma_b > 1.8 ||
             gamma_r == -1 || gamma_g == -1 || gamma_b == -1) {

                hdr ->PCSType = PT_Lab;

                if (hdr ->printf)
                       hdr ->printf("I have chosen Lab as PCS");                        

        }
        else {
  
                hdr ->PCSType = PT_XYZ;

                if (hdr ->printf)
                       hdr ->printf("I have chosen XYZ as PCS");                        
     }

                

     if (hdr ->printf) {

                   char Buffer[256] = "Infered ";
                        
                   _cmsIdentifyWhitePoint(Buffer, &hdr ->WhitePoint);
                   hdr ->printf("%s", Buffer);                        
                   hdr ->printf("Primaries (x-y): [Red: %2.2f, %2.2f] [Green: %2.2f, %2.2f] [Blue: %2.2f, %2.2f]", 
                                      hdr ->Primaries.Red.x, hdr ->Primaries.Red.y, 
                                      hdr ->Primaries.Green.x, hdr ->Primaries.Green.y,
                                      hdr ->Primaries.Blue.x, hdr ->Primaries.Blue.y);

                   if ((gamma_r != -1) && (gamma_g != -1) && (gamma_b != -1)) {
                        
                   hdr ->printf("Estimated gamma: [Red: %2.2f] [Green: %2.2f] [Blue: %2.2f]", 
                                                            gamma_r, gamma_g, gamma_b);
                   }
                                                

         }


		
        return true;
}
