// CFA pixel cleaning via directional average
// copyright Emil Martinec
// all rights reserved
// 2/18/2010
// modifié par J.Desmis - octobre 2010
#define TS 256		// Tile size

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define SQRF(x) (((float)x)*((float)x))
#define ABS(x) (((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define ULIM(x,y,z) ((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
#define CLIP(x) LIM(x,0,65535)

void CLASS cfa_impulse_gauss(float lclean, float cclean)
{  
	double dt;
	clock_t t1, t2;
	int winx=0, winy=0,winw=width, winh=height;

	int compt=0, compt10=0,compt50=0,compt250=0,compt500=0,comptmax=0;
	static const int border=8;
	int rayo=1,i,d;
	int compte1=0;
	static const float eps1=1e-10;
	
#ifdef DCRAW_VERBOSE
	if (verbose) fprintf (stderr,_("CFA impulse-hot and gaussian denoise [E.Martinec +JD] g:%1.4f  br:%1.4f\n"), lclean,cclean);
#endif
	t1 = clock();
	border_interpolate(16);
	float noisevar=SQRF(lclean); 
	float noisevarbr=SQRF(cclean);

#pragma omp parallel
{	
	int top,left;
	char		*buffer;				// TS*TS*20
	float         (*rgb)[3];		// TS*TS*12
	float         (*lpf);			// TS*TS*4
	float         (*hpf);			// TS*TS*4

	// assign working space
	buffer = (char *) calloc((4*sizeof(float)+sizeof(int))*TS*TS,1);
	
	// rgb array
	rgb         = (float (*)[3])		buffer; //pointers to rows of array
 	lpf			= (float (*))			(buffer +  3*sizeof(float)*TS*TS);
	hpf			= (float (*))			(buffer +  4*sizeof(float)*TS*TS);
	
	//This is stuff from AMaZE, hope it makes sense here
	
	// Fill border pixels
//	border_interpolate(16);
	// Fill G interpolated values with border interpolation and input values
	// Main algorithm: Tile loop

#if defined (LIBRAW_USE_OPENMP)
#pragma omp for schedule(dynamic) nowait 
#endif	
	for (top=0; top < winh-16; top += TS-32)
		for (left=0; left < winw-16; left += TS-32) {

			int bottom = MIN( top+TS,winh);
			int right  = MIN(left+TS, winw);
			int rr1 = bottom - top;
			int cc1 = right - left;
			
			int rr,cc;
			int c;
			int row, col;
			int indx;
			float  hfvar[3];
			float gin, g[8];
			float norm;
			float wtdsum;
			int dir,dirwt;
			float rbin,rb[8],hfnbrave;
			float v1;
			/*	t1_main = clock();  */
			// rgb from input CFA data
			/* rgb values should be floating point number between 0 and 1 
			 after white balance multipliers are applied */
			for (rr=0; rr < rr1; rr++)
				for (row=rr+top, cc=0; cc < cc1; cc++) {
					col = cc+left;
					c = FC(rr,cc);
					//if(info!=0) img[row*width+col][c]=image[row*width+col][c];
					rgb[rr*TS+cc][c] = image[row*width+col][c]/65535.0f; 
				}
			for (c=0; c<3; c++){hfvar[c]=0.0f;}
			
			//The cleaning algorithm starts here
			
			for (rr=2; rr < rr1-1; rr++)
				for (cc=2; cc < cc1-1; cc++) {
					c = FC(rr,cc);
					if (c==1) { 
						//G pixel neighbor average (nearest neighbors on quincunx grid)
						gin=rgb[(rr)*TS+cc][c];
						g[0]=rgb[(rr-2)*TS+cc][c];
						g[1]=rgb[(rr-1)*TS+cc-1][c];
						g[2]=rgb[(rr-1)*TS+cc+1][c];
						g[3]=rgb[(rr)*TS+cc-2][c];
						g[4]=rgb[(rr)*TS+cc+2][c];
						g[5]=rgb[(rr+1)*TS+cc-1][c];
						g[6]=rgb[(rr+1)*TS+cc+1][c];
						g[7]=rgb[(rr+2)*TS+cc][c];
						//evaluate directional weighted sum of neighbors
						norm=0.0f;
						wtdsum=0.0f;
						for (dir=0; dir<8; dir++){
							dirwt=1/(SQRF(gin-g[dir])+noisevar+eps1);//JD
							norm+=dirwt;
							wtdsum+=g[dir]*dirwt;
						}
						wtdsum=wtdsum/norm;
						//low pass and high pass filters from sum and difference
						lpf[(rr)*TS+cc]=(gin+wtdsum)/2;
						hpf[(rr)*TS+cc]=(gin-wtdsum)/2;
						hfvar[c]=hfvar[c]+SQRF(hpf[(rr)*TS+cc]);//running total for variance
					}
					else {
				
						//R or B pixel neighbor average
						rbin=rgb[(rr)*TS+cc][c];
						rb[0]=rgb[(rr-2)*TS+cc-2][c];
						rb[1]=rgb[(rr-2)*TS+cc][c];
						rb[2]=rgb[(rr-2)*TS+cc+2][c];
						rb[3]=rgb[(rr)*TS+cc-2][c];
						rb[4]=rgb[(rr)*TS+cc+2][c];
						rb[5]=rgb[(rr+2)*TS+cc-2][c];
						rb[6]=rgb[(rr+2)*TS+cc][c];
						rb[7]=rgb[(rr+2)*TS+cc+2][c];
						//evaluate directional weights
						norm=0.0f;
						wtdsum=0.0f;
						for (dir=0; dir<8; dir++)
						{
							dirwt=1/(SQRF(rbin-rb[dir])+noisevarbr+eps1);//JD
							norm+=dirwt;
							wtdsum+=rb[dir]*dirwt;
						}
						wtdsum=wtdsum/norm;
						//low pass and high pass filters from sum and difference
						lpf[(rr)*TS+cc]=(rbin+wtdsum)/2;
						hpf[(rr)*TS+cc]=(rbin-wtdsum)/2;
						hfvar[c]=hfvar[c]+SQRF(hpf[(rr)*TS+cc]);//running total for variance
					}
					//end initialization	
				}
			//normalize high frequency variances of color channels
			hfvar[1]=hfvar[1]/(0.5f*(rr1-4)*(cc1-4));
			hfvar[0]=hfvar[0]/(0.25f*(rr1-4)*(cc1-4));
			hfvar[2]=hfvar[2]/(0.25f*(rr1-4)*(cc1-4));
			//now smooth the cfa data
			for (rr=4; rr < rr1-3; rr++)
				for (cc=4; cc < cc1-3; cc++) {
					c = FC(rr,cc);
			
					if (c==1) { 
						//G pixel high freq neighbor average from closest eight neighbors
						hfnbrave=0.125f*(hpf[(rr-2)*TS+cc]+hpf[(rr-1)*TS+cc-1]+hpf[(rr-1)*TS+cc+1]+hpf[(rr)*TS+cc-2]+ \
										hpf[(rr)*TS+cc+2]+hpf[(rr+1)*TS+cc-1]+hpf[(rr+1)*TS+cc+1]+hpf[(rr+2)*TS+cc]);
					}
					else {
						//R or B pixel high freq neighbor average from closest eight neighbors
						
						hfnbrave=0.125f*(hpf[(rr-2)*TS+cc-2]+hpf[(rr-2)*TS+cc]+hpf[(rr-2)*TS+cc+2]+hpf[(rr)*TS+cc-2]+ \
										hpf[(rr)*TS+cc+2]+hpf[(rr+2)*TS+cc-2]+hpf[(rr+2)*TS+cc]+hpf[(rr+2)*TS+cc+2]);
					}
					/* smooth high frequencies if they are much more than tile variance, using Wiener filter */
					hpf[(rr)*TS+cc]=hpf[(rr)*TS+cc]*hfvar[c]/(SQRF(hpf[(rr)*TS+cc]-hfnbrave)+hfvar[c]);
					/* reconstruct cfa data */
					rgb[(rr)*TS+cc][c]=lpf[(rr)*TS+cc]+hpf[(rr)*TS+cc];
				}
			
			// copy smoothed results back to image matrix;
			for (rr=border; rr < rr1-border; rr++)
				for (row=rr+top, cc=border; cc < cc1-border; cc++) {
					col = cc + left;
					indx = row*width + col;
					c = FC(row,col);
					v1 = rgb[(rr)*TS+cc][c];
					// modif JD
				/*	if(c==1 && noisevar==0) image[indx][c]=img[indx][c];
					else if (c==0 && noisevarbr==0) image[indx][c]=img[indx][c];
					else if (c==2 && noisevarbr==0) image[indx][c]=img[indx][c];
					else */
					image[indx][c] = CLIP((int)(65535.0f*v1 + 0.5f)); 
				/*	if(info!=0) {
					if(abs(image[indx][c]-img[indx][c]) < 5) compt++;
					else if(abs(image[indx][c]-img[indx][c]) < 50) compt10++;
					else if(abs(image[indx][c]-img[indx][c]) < 500) compt50++;
					else if(abs(image[indx][c]-img[indx][c]) < 2500) compt250++;
					else if(abs(image[indx][c]-img[indx][c]) < 5000) compt500++;
					else if(abs(image[indx][c]-img[indx][c]) >= 5000) comptmax++;}
				*/
				} 
				//free(buffer);
		}
	// clean up
	free(buffer);
	}
	//JD
	//free(img);
//	if (info!=0) { printf("DeltaRGB < 5=%i 50=%i 500=%i 2500=%i 5000=%i >5000=%i\n",compt,compt10,compt50,compt250,compt500,comptmax);}
	// done 
	t2 = clock();
	dt = ((double)(t2-t1)) / CLOCKS_PER_SEC;
#ifdef DCRAW_VERBOSE
	if (verbose) {
		fprintf(stderr,_("done in = %5.3fs\n"),dt);  
		/*		fprintf(stderr,_("   MAIN       = %5.3fs\n"),
		 (double)t2_main/CLOCKS_PER_SEC);   */
	}
#endif	
	
}
//}
#undef TS
#undef TA
