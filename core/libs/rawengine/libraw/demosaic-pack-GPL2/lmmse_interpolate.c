/* This file was taken from PerfectRaw ver. 0.65 published
   in 2010, taking dcraw ver.8.88/rev.1.405
   as basis.
   http://dl.dropbox.com/u/602348/perfectRAW%200.65%20source%20code.zip

   As PerfectRaw source code was published, the release under
   GPL Version 2 or later option could be applied, so this file
   is taken under this premise.
*/

// LSMME demosaicing algorithm
// L. Zhang and X. Wu,
// Color demosaicking via directional linear minimum mean square-error
// estimation, IEEE Trans. on Image Processing, vol. 14, pp. 2167-2178,
// Dec. 2005.
#define PIX_SORT(a,b) { if ((a)>(b)) {temp=(a);(a)=(b);(b)=temp;} }
void CLASS lmmse_interpolate(int gamma_apply)
{
  ushort (*pix)[4];
  int row, col, c, d, w1, w2, w3, w4, ii, ba, rr1, cc1, rr, cc, pass;
  float h0, h1, h2, h3, h4, hs;
  float p1, p2, p3, p4, p5, p6, p7, p8, p9, temp;
  float Y, v0, mu, vx, vn, xh, vh, xv, vv;
  float (*rix)[6], (*qix)[6];
  float (*glut);
  char  *buffer;
  clock_t t1, t2;
  double dt;
#ifdef DCRAW_VERBOSE
  if (verbose) fprintf(stderr,_("LMMSE interpolation...\n"));
#endif
  t1 = clock();
  // allocate work with boundary
  ba = 10;
  rr1 = height + 2*ba;
  cc1 = width + 2*ba;
  if (gamma_apply)
    buffer = (char *)calloc(rr1*cc1*6*sizeof(float)+65536*sizeof(float),1);
  else
    buffer = (char *)calloc(rr1*cc1*6*sizeof(float),1);
  merror(buffer,"lmmse_interpolate()");
  qix = (float (*)[6])buffer;
  if (gamma_apply) {
    glut = (float *)(buffer + rr1*cc1*24);
    for (ii=0; ii < 65536; ii++) {
      v0 = (float)ii / 65535.0;
      if (v0 <= 0.0031308)
	glut[ii] = v0*12.92;
      else
	glut[ii] = 1.055*pow((double)v0,1./2.4) - 0.055; } }
  // indices
  w1 = cc1;
  w2 = 2*w1;
  w3 = 3*w1;
  w4 = 4*w1;
  // define low pass filter (sigma=2, L=4)
  h0 = 1.0;
  h1 = exp( -1.0/8.0);
  h2 = exp( -4.0/8.0);
  h3 = exp( -9.0/8.0);
  h4 = exp(-16.0/8.0);
  hs = h0 + 2.0*(h1 + h2 + h3 + h4);
  h0 /= hs;
  h1 /= hs;
  h2 /= hs;
  h3 /= hs;
  h4 /= hs;
  // copy CFA values
  for (rr=0; rr < rr1; rr++)
    for (cc=0, row=rr-ba; cc < cc1; cc++) {
      col = cc - ba;
      rix = qix + rr*cc1 + cc;
      if ((row >= 0) & (row < height) & (col >= 0) & (col < width))
	if (gamma_apply)
	  rix[0][4] = glut[image[row*width+col][FC(row,col)]];
	else
	  rix[0][4] = (double)image[row*width+col][FC(row,col)]/65535.0;
      else
	rix[0][4] = 0; }
  // G-R(B)
  for (rr=2; rr < rr1-2; rr++) {
    // G-R(B) at R(B) location
    for (cc=2+(FC(rr,2)&1); cc < cc1-2; cc+=2) {
      rix = qix + rr*cc1 + cc;
      // v0 = 0.25R + 0.25B, Y = 0.25R + 0.5B + 0.25B
      v0 = 0.0625*(rix[-w1-1][4]+rix[-w1+1][4]+rix[w1-1][4]+rix[w1+1][4]) +
	0.25*rix[0][4];
      // horizontal
      rix[0][0] = -0.25*(rix[ -2][4] + rix[ 2][4])
	+ 0.5*(rix[ -1][4] + rix[0][4] + rix[ 1][4]);
      Y = v0 + 0.5*rix[0][0];
      if (rix[0][4] > 1.75*Y)
	rix[0][0] = ULIM(rix[0][0],rix[ -1][4],rix[ 1][4]);
      else
	rix[0][0] = LIM(rix[0][0],0.0,1.0);
      rix[0][0] -= rix[0][4];
      // vertical
      rix[0][1] = -0.25*(rix[-w2][4] + rix[w2][4])
	+ 0.5*(rix[-w1][4] + rix[0][4] + rix[w1][4]);
      Y = v0 + 0.5*rix[0][1];
      if (rix[0][4] > 1.75*Y)
	rix[0][1] = ULIM(rix[0][1],rix[-w1][4],rix[w1][4]);
      else
	rix[0][1] = LIM(rix[0][1],0.0,1.0);
      rix[0][1] -= rix[0][4]; }
    // G-R(B) at G location
    for (cc=2+(FC(rr,3)&1); cc < cc1-2; cc+=2) {
      rix = qix + rr*cc1 + cc;
      rix[0][0] = 0.25*(rix[ -2][4] + rix[ 2][4])
	- 0.5*(rix[ -1][4] + rix[0][4] + rix[ 1][4]);
      rix[0][1] = 0.25*(rix[-w2][4] + rix[w2][4])
	- 0.5*(rix[-w1][4] + rix[0][4] + rix[w1][4]);
      rix[0][0] = LIM(rix[0][0],-1.0,0.0) + rix[0][4];
      rix[0][1] = LIM(rix[0][1],-1.0,0.0) + rix[0][4];
    } }
  // apply low pass filter on differential colors
  for (rr=4; rr < rr1-4; rr++)
    for (cc=4; cc < cc1-4; cc++) {
      rix = qix + rr*cc1 + cc;
      rix[0][2] = h0*rix[0][0] +
	h1*(rix[ -1][0] + rix[ 1][0]) + h2*(rix[ -2][0] + rix[ 2][0]) +
	h3*(rix[ -3][0] + rix[ 3][0]) + h4*(rix[ -4][0] + rix[ 4][0]);
      rix[0][3] = h0*rix[0][1] +
	h1*(rix[-w1][1] + rix[w1][1]) + h2*(rix[-w2][1] + rix[w2][1]) +
	h3*(rix[-w3][1] + rix[w3][1]) + h4*(rix[-w4][1] + rix[w4][1]); }
  // interpolate G-R(B) at R(B)
  for (rr=4; rr < rr1-4; rr++)
    for (cc=4+(FC(rr,4)&1); cc < cc1-4; cc+=2) {
      rix = qix + rr*cc1 + cc;
      // horizontal
      mu = (rix[-4][2] + rix[-3][2] + rix[-2][2] + rix[-1][2] + rix[0][2]+
	    rix[ 1][2] + rix[ 2][2] + rix[ 3][2] + rix[ 4][2]) / 9.0;
      p1 = rix[-4][2] - mu;
      p2 = rix[-3][2] - mu;
      p3 = rix[-2][2] - mu;
      p4 = rix[-1][2] - mu;
      p5 = rix[ 0][2] - mu;
      p6 = rix[ 1][2] - mu;
      p7 = rix[ 2][2] - mu;
      p8 = rix[ 3][2] - mu;
      p9 = rix[ 4][2] - mu;
      vx = 1e-7+p1*p1+p2*p2+p3*p3+p4*p4+p5*p5+p6*p6+p7*p7+p8*p8+p9*p9;
      p1 = rix[-4][0] - rix[-4][2];
      p2 = rix[-3][0] - rix[-3][2];
      p3 = rix[-2][0] - rix[-2][2];
      p4 = rix[-1][0] - rix[-1][2];
      p5 = rix[ 0][0] - rix[ 0][2];
      p6 = rix[ 1][0] - rix[ 1][2];
      p7 = rix[ 2][0] - rix[ 2][2];
      p8 = rix[ 3][0] - rix[ 3][2];
      p9 = rix[ 4][0] - rix[ 4][2];
      vn = 1e-7+p1*p1+p2*p2+p3*p3+p4*p4+p5*p5+p6*p6+p7*p7+p8*p8+p9*p9;
      xh = (rix[0][0]*vx + rix[0][2]*vn)/(vx + vn);
      vh = vx*vn/(vx + vn);
      // vertical
      mu = (rix[-w4][3] + rix[-w3][3] + rix[-w2][3] + rix[-w1][3] + rix[0][3]+
	    rix[ w1][3] + rix[ w2][3] + rix[ w3][3] + rix[ w4][3]) / 9.0;
      p1 = rix[-w4][3] - mu;
      p2 = rix[-w3][3] - mu;
      p3 = rix[-w2][3] - mu;
      p4 = rix[-w1][3] - mu;
      p5 = rix[  0][3] - mu;
      p6 = rix[ w1][3] - mu;
      p7 = rix[ w2][3] - mu;
      p8 = rix[ w3][3] - mu;
      p9 = rix[ w4][3] - mu;
      vx = 1e-7+p1*p1+p2*p2+p3*p3+p4*p4+p5*p5+p6*p6+p7*p7+p8*p8+p9*p9;
      p1 = rix[-w4][1] - rix[-w4][3];
      p2 = rix[-w3][1] - rix[-w3][3];
      p3 = rix[-w2][1] - rix[-w2][3];
      p4 = rix[-w1][1] - rix[-w1][3];
      p5 = rix[  0][1] - rix[  0][3];
      p6 = rix[ w1][1] - rix[ w1][3];
      p7 = rix[ w2][1] - rix[ w2][3];
      p8 = rix[ w3][1] - rix[ w3][3];
      p9 = rix[ w4][1] - rix[ w4][3];
      vn = 1e-7+p1*p1+p2*p2+p3*p3+p4*p4+p5*p5+p6*p6+p7*p7+p8*p8+p9*p9;
      xv = (rix[0][1]*vx + rix[0][3]*vn)/(vx + vn);
      vv = vx*vn/(vx + vn);
      // interpolated G-R(B)
      rix[0][4] = (xh*vv + xv*vh)/(vh + vv); }
  // copy CFA values
  for (rr=0; rr < rr1; rr++)
    for (cc=0, row=rr-ba; cc < cc1; cc++) {
      col=cc-ba;
      rix = qix + rr*cc1 + cc;
      c = FC(rr,cc);
      if ((row >= 0) & (row < height) & (col >= 0) & (col < width))
	if (gamma_apply)
	  rix[0][c] = glut[image[row*width+col][c]];
	else
	  rix[0][c] = (double)image[row*width+col][c]/65535.0;
      else
	rix[0][c] = 0;
      if (c != 1) rix[0][1] = rix[0][c] + rix[0][4]; }
  // bilinear interpolation for R/B
  // interpolate R/B at G location
  for (rr=1; rr < rr1-1; rr++)
    for (cc=1+(FC(rr,2)&1), c=FC(rr,cc+1); cc < cc1-1; cc+=2) {
      rix = qix + rr*cc1 + cc;
      rix[0][c] = rix[0][1]
	+ 0.5*(rix[ -1][c] - rix[ -1][1] + rix[ 1][c] - rix[ 1][1]);
      c = 2 - c;
      rix[0][c] = rix[0][1]
	+ 0.5*(rix[-w1][c] - rix[-w1][1] + rix[w1][c] - rix[w1][1]);
      c = 2 - c; }
  // interpolate R/B at B/R location
  for (rr=1; rr < rr1-1; rr++)
    for (cc=1+(FC(rr,1)&1), c=2-FC(rr,cc); cc < cc1-1; cc+=2) {
      rix = qix + rr*cc1 + cc;
      rix[0][c] = rix[0][1]
	+ 0.25*(rix[-w1][c] - rix[-w1][1] + rix[ -1][c] - rix[ -1][1]+
		rix[  1][c] - rix[  1][1] + rix[ w1][c] - rix[ w1][1]); }
  // median filter
  for (pass=1; pass <= 3; pass++) {
    for (c=0; c < 3; c+=2) {
      // Compute median(R-G) and median(B-G)
      d = c + 3;
      for (ii=0; ii < rr1*cc1; ii++) qix[ii][d] = qix[ii][c] - qix[ii][1];
      // Apply 3x3 median fileter
      for (rr=1; rr < rr1-1; rr++)
	for (cc=1; cc < cc1-1; cc++) {
	  rix = qix + rr*cc1 + cc;
	  // Assign 3x3 differential color values
	  p1 = rix[-w1-1][d]; p2 = rix[-w1][d]; p3 = rix[-w1+1][d];
	  p4 = rix[   -1][d]; p5 = rix[  0][d]; p6 = rix[    1][d];
	  p7 = rix[ w1-1][d]; p8 = rix[ w1][d]; p9 = rix[ w1+1][d];
	  // Sort for median of 9 values
	  PIX_SORT(p2,p3); PIX_SORT(p5,p6); PIX_SORT(p8,p9);
	  PIX_SORT(p1,p2); PIX_SORT(p4,p5); PIX_SORT(p7,p8);
	  PIX_SORT(p2,p3); PIX_SORT(p5,p6); PIX_SORT(p8,p9);
	  PIX_SORT(p1,p4); PIX_SORT(p6,p9); PIX_SORT(p5,p8);
	  PIX_SORT(p4,p7); PIX_SORT(p2,p5); PIX_SORT(p3,p6);
	  PIX_SORT(p5,p8); PIX_SORT(p5,p3); PIX_SORT(p7,p5);
	  PIX_SORT(p5,p3);
	  rix[0][4] = p5; }
      for (ii=0; ii < rr1*cc1; ii++) qix[ii][d] = qix[ii][4]; }
    // red/blue at GREEN pixel locations
    for (rr=0; rr < rr1; rr++)
      for (cc=(FC(rr,1)&1), c=FC(rr,cc+1); cc < cc1; cc+=2) {
	rix = qix + rr*cc1 + cc;
	rix[0][0] = rix[0][1] + rix[0][3];
	rix[0][2] = rix[0][1] + rix[0][5]; }
    // red/blue and green at BLUE/RED pixel locations
    for (rr=0; rr < rr1; rr++)
      for (cc=(FC(rr,0)&1), c=2-FC(rr,cc), d=c+3; cc < cc1; cc+=2) {
	rix = qix + rr*cc1 + cc;
	rix[0][c] = rix[0][1] + rix[0][d];
	rix[0][1] = 0.5*(rix[0][0] - rix[0][3] + rix[0][2] - rix[0][5]); } }
  // copy result back to image matrix
  for (row=0; row < height; row++)
    for (col=0, rr=row+ba; col < width; col++) {
      cc = col+ba;
      pix = image + row*width + col;
      rix = qix + rr*cc1 + cc;
      c = FC(row,col);
      if (gamma_apply) {
	for (ii=0; ii < 3; ii++)
	  if (ii != c) {
	    v0 = rix[0][ii];
	    if (v0 <= 0.04045)
	      v0 /= 12.92;
	    else
	      v0 = pow((v0 + 0.055)/1.055,2.4);
	    pix[0][ii] = CLIP((int)(65535.0*v0 + 0.5)); } }
      else
	for (ii=0; ii < 3; ii++)
	  if (ii != c)
	    pix[0][ii] = CLIP((int)(65535.0*rix[0][ii] + 0.5));
    }
  // Done
  free(buffer);
  t2 = clock();
  dt = ((double)(t2-t1)) / CLOCKS_PER_SEC;
#ifdef DCRAW_VERBOSE
  if (verbose) fprintf(stderr,_("\telapsed time     = %5.3fs\n"),dt);
#endif
}
