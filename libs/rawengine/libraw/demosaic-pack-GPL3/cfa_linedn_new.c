// CFA line denoise by DCT filtering
// copyright Emil Martinec
// all rights reserved
// 3/12/2010 and november 2010
#define TS 256		// Tile size
//modified by Jacques Desmis - december 2010
//#define CLASS


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

#include "shrtdct_float.c"


#define SQRF(x) (((float)x)*((float)x))
#define ABS(x) (((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define ULIM(x,y,z) ((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
#define CLIP(x) LIM(x,0,65535)

void CLASS cfa_linedn(float linenoise){  
	double dt;
	clock_t t1, t2;

	float eps=1e-5;			//tolerance to avoid dividing by zero
	const float clip_pt = (1.0 / MIN(MIN(pre_mul[0],pre_mul[1]),pre_mul[2]));  // for highlights
	const float gauss[5] = {0.20416368871516755, 0.18017382291138087, 0.1238315368057753, 0.0662822452863612, 0.02763055063889883};
	const float rolloff[8] = {0, 0.135335, 0.249352, 0.411112, 0.606531, 0.800737, 0.945959, 1}; //gaussian with sigma=3
	const float window[8] = {0, .25, .75, 1, 1, .75, .25, 0}; //sine squared
	float aarr[4][8][8], *bbrr[4][8], **dctblock[4];
	for (int j=0; j<4; j++) {
		for (int i = 0; i < 8; i++) 
			bbrr[j][i] = aarr[j][i];
		dctblock[j] = bbrr[j];
	}
#ifdef DCRAW_VERBOSE
	if (verbose) fprintf (stderr,_("CFA line denoise v2b OMP [E.Martinec] %1.4f \n"), linenoise);
#endif
	t1 = clock();
	
	float noisevar=SQRF(3*linenoise); // _noise_ (as a fraction of saturation) is input to the algorithm
#pragma omp parallel
{	
	char			*buffer;			// TS*TS*16
	float        	(*cfain);			// TS*TS*4
	float         	(*cfablur);		// TS*TS*4
	float         	(*cfadiff);		// TS*TS*4
	float         	(*cfadn);			// TS*TS*4
	
	

	
	// assign working space
	buffer = (char *) calloc((3*sizeof(float)+sizeof(int))*TS*TS,1);// 16
	cfain			= (float (*))		buffer; //pointers to rows of array
 	cfablur			= (float (*))			(buffer +  sizeof(float)*TS*TS); //4
	cfadiff			= (float (*))			(buffer +  2*sizeof(float)*TS*TS);//8
	cfadn			= (float (*))			(buffer +  3*sizeof(float)*TS*TS);//12
	
	
	// Main algorithm: Tile loop
#if defined (LIBRAW_USE_OPENMP)
#pragma omp for schedule(dynamic) nowait 
#endif	
	for (int top=0; top < height-16; top += TS-32)
		for (int left=0; left < width-16; left += TS-32) {
			int bottom = MIN( top+TS,height);
			int right  = MIN(left+TS, width);
			int rr1 = bottom - top;
			int cc1 = right - left;
 			int numrows = bottom - top;
 			int numcols = right - left;
			// load CFA data
			/* rgb values should be floating point number between 0 and 1 
			 after white balance multipliers are applied;
			 data should be in linear gamma space */
			for (int rr=0; rr < rr1; rr++)
				for (int cc=0, row=rr+top; cc < cc1; cc++) {
					int col = cc+left;
					int c = FC(rr,cc);
					cfain[(rr)*TS+cc] = image[row*width+col][c]/65535.0; 
				}
			//pad the block to a multiple of 16 on both sides
			if (cc1 < TS) {
				int indx=cc1 % 16;
				for (int i=0; i<(16-indx); i++) 
					for (int rr=0; rr<rr1; rr++) 
						cfain[(rr)*TS+cc1+i+1]=cfain[(rr)*TS+cc1-i];
				cc1 += 16-indx;
			}
			if (rr1 < TS) {
				int indx=rr1 % 16;
				for (int i=0; i<(16-indx); i++)
					for (int cc=0; cc<cc1; cc++)
						cfain[(rr1+i+1)*TS+cc]=cfain[(rr1-i)*TS+cc];
				rr1 += 16-indx;
			}
			//The cleaning algorithm starts here
			//gaussian blur of CFA data
			for (int rr=8; rr < rr1-8; rr++)
				for (int cc=0; cc < cc1; cc++) {
					cfablur[(rr)*TS+cc]=gauss[0]*cfain[(rr)*TS+cc];
					for (int i=1; i<5; i++) {
						cfablur[(rr)*TS+cc] += gauss[i]*(cfain[(rr-2*i)*TS+cc]+cfain[(rr+2*i)*TS+cc]);
					}
				}
			for (int rr=8; rr < rr1-8; rr++)
				for (int cc=8; cc < cc1-8; cc++) {
					cfadn[(rr)*TS+cc] = gauss[0]*cfablur[(rr)*TS+cc];
					for (int i=1; i<5; i++) {
						cfadn[(rr)*TS+cc] += gauss[i]*(cfablur[(rr)*TS+cc-2*i]+cfablur[(rr)*TS+cc+2*i]);
					}
					cfadiff[(rr)*TS+cc]=cfain[(rr)*TS+cc]-cfadn[(rr)*TS+cc]; // hipass cfa data
				}
			//begin block DCT
			float linehvar[4], linevvar[4], noisefactor[4][8][2], coeffsq;
			#pragma omp critical  // exclude this part of code from OMP
	{
			for (int rr=8; rr < numrows-22; rr+=8) // (rr,cc) shift by 8 to overlap blocks
				for (int cc=8; cc < numcols-22; cc+=8) {
					for (int ey=0; ey<2; ey++) // (ex,ey) specify RGGB subarray
						for (int ex=0; ex<2; ex++) {
							//grab an 8x8 block of a given RGGB channel
							for (int i=0; i<8; i++) 
								for (int j=0; j<8; j++) {
									dctblock[2*ey+ex][i][j]=cfadiff[(rr+2*i+ey)*TS+cc+2*j+ex];
								}
							ddct8x8s(-1, dctblock[2*ey+ex]); //forward DCT
						}
					for (int ey=0; ey<2; ey++) // (ex,ey) specify RGGB subarray
						for (int ex=0; ex<2; ex++) {
							linehvar[2*ey+ex]=linevvar[2*ey+ex]=0;
							for (int i=4; i<8; i++) {
								linehvar[2*ey+ex] += SQRF(dctblock[2*ey+ex][0][i]);
							linevvar[2*ey+ex] += SQRF(dctblock[2*ey+ex][i][0]);		
							}
							//Wiener filter for line denoising; roll off low frequencies
							for (int i=1; i<8; i++) {
								coeffsq = SQRF(dctblock[2*ey+ex][i][0]);//vertical
								noisefactor[2*ey+ex][i][0] = coeffsq/(coeffsq+rolloff[i]*noisevar+eps);
								coeffsq = SQRF(dctblock[2*ey+ex][0][i]);//horizontal
								noisefactor[2*ey+ex][i][1] = coeffsq/(coeffsq+rolloff[i]*noisevar+eps);
								// noisefactor labels are [RGGB subarray][row/col position][0=vert,1=hor]
							}
							}
					//horizontal lines
					if (4*noisevar>(linehvar[0]+linehvar[1])) {//horizontal lines
						for (int i=1; i<8; i++) {
							dctblock[0][0][i] *= 0.5*(noisefactor[0][i][1]+noisefactor[1][i][1]);//or should we use MIN???
							dctblock[1][0][i] *= 0.5*(noisefactor[0][i][1]+noisefactor[1][i][1]);//or should we use MIN???
						}
					}
					if (4*noisevar>(linehvar[2]+linehvar[3])) {//horizontal lines
						for (int i=1; i<8; i++) {
							dctblock[2][0][i] *= 0.5*(noisefactor[2][i][1]+noisefactor[3][i][1]);//or should we use MIN???
							dctblock[3][0][i] *= 0.5*(noisefactor[2][i][1]+noisefactor[3][i][1]);//or should we use MIN???
						}
					}
					//vertical lines
					if (4*noisevar>(linevvar[0]+linevvar[2])) {//vertical lines
						for (int i=1; i<8; i++) {
							dctblock[0][i][0] *= 0.5*(noisefactor[0][i][0]+noisefactor[2][i][0]);//or should we use MIN???
							dctblock[2][i][0] *= 0.5*(noisefactor[0][i][0]+noisefactor[2][i][0]);//or should we use MIN???
						}
					}
					if (4*noisevar>(linevvar[1]+linevvar[3])) {//vertical lines
						for (int i=1; i<8; i++) {
							dctblock[1][i][0] *= 0.5*(noisefactor[1][i][0]+noisefactor[3][i][0]);//or should we use MIN???
							dctblock[3][i][0] *= 0.5*(noisefactor[1][i][0]+noisefactor[3][i][0]);//or should we use MIN???
						}
					}
					for (int ey=0; ey<2; ey++) // (ex,ey) specify RGGB subarray
						for (int ex=0; ex<2; ex++) {
							ddct8x8s(1, dctblock[2*ey+ex]); //inverse DCT
							
							for (int i=0; i<8; i++) 
								for (int j=0; j<8; j++) {
									cfadn[(rr+2*i+ey)*TS+cc+2*j+ex] += window[i]*window[j]*dctblock[2*ey+ex][i][j];
								}
						}	
				}
	}
			// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			
			// copy smoothed results back to image matrix
			for (int rr=16; rr < rr1-16; rr++)
				for (int cc=16,row=rr+top; cc < cc1-16; cc++) {
					int col = cc + left;
					int indx = row*width + col;
					int c = FC(row,col);
					if (image[indx][c] <65535*0.8*clip_pt && cfadn[(rr)*TS+cc]<0.8*clip_pt)
					image[indx][c] = CLIP((int)(65535*cfadn[(rr)*TS+cc]+ 0.5)); } 
	}
	
	// clean up
	free(buffer);
	}
	// done
	t2 = clock();
	dt = ((double)(t2-t1)) / CLOCKS_PER_SEC;
#ifdef DCRAW_VERBOSE
	if (verbose) {
		fprintf(stderr,_("elapsed time = %5.3fs\n"),dt);
	}
#endif	
	
}
#undef TS
//#undef TA
