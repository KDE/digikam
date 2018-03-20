/* This file was taken from PerfectRaw ver. 0.65
   on May 14, 2010, taking dcraw ver.8.88/rev.1.405
   as basis.
   http://dl.dropbox.com/u/602348/perfectRAW%200.65%20source%20code.zip

   As PerfectRaw source code was published, the release under
   GPL Version 2 or later option could be applied, so this file
   is taken under this premise.
*/

/*
Adaptive Filtered Demosaicking interpolation is adapted from
the work of Lian Naixiang, Chang Lanlan and Prof. Tan Yap Peng.
*/


#define FC_BK(row,col) \
	(filters_bk >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define PIX_SORT(a,b) { if ((a)>(b)) {temp=(a);(a)=(b);(b)=temp;} }
void CLASS afd_interpolate_pl(int afd_passes, int clip_on)
{
  int row, col, rr, cc, rr1, cc1, c, d, ii, jj, ba, lum_iter;
  int p12, p13, p14, p21, p22, p23, p24, p25;
  int p41, p42, p43, p44, p45, p52, p53, p54;
  float (*pix)[4], (*dimage)[4], (*w)[4], *glut;
  double v0, v1, v2, dG, dC0, dC1, dC2, dC3, dC4, temp;
  double ws, w0, w1, w2, w3, dt;
  clock_t t1, t2;
#ifdef DCRAW_VERBOSE
  if (verbose) {
    fprintf(stderr,_("AFD interpolation with pattern matching...\n"));
    fprintf(stderr,_("\tafd_passes, clip_on = %d, %d\n"),afd_passes,clip_on); }
#endif
  t1 = clock();

  // allocate work with boundary
  ba = 6;
  rr1 = height + 2*ba;
  cc1 = width + 2*ba;

  // Set up working arrays
  dimage = (float (*)[4])calloc(rr1*cc1,             sizeof *dimage);
  w      = (float (*)[4])calloc((int)((1+rr1*cc1)/2),sizeof *w);

  // copy CFA values and apply gamma correction
  glut = (float *)calloc(65536,sizeof *glut);
  for (ii=0; ii < 65536; ii++) {
    v0 = (float)ii / 65535.0;
    glut[ii] = (v0 <= 0.0031308 ? v0*12.92 : 1.055*pow(v0,1./2.4) - 0.055); }
  //
  for (rr=0; rr < rr1; rr++)
    for (cc=0, row=rr-ba; cc < cc1; cc++) {
      col = cc - ba;
      ii = rr*cc1 + cc;
      c = FC(rr,cc);
      if ((row >= 0) & (row < height) & (col >= 0) & (col < width))
	dimage[ii][c] = glut[image[row*width+col][c]];
      else
	dimage[ii][c] = 0; }
  //
  free(glut);

  // Set up indices
  p12 = -2*cc1-1; p13 = p12+1; p14 = p13+1;
  p21 =   -cc1-2; p22 = p21+1; p23 = p22+1; p24 = p23+1; p25 = p24+1;
  p41 =    cc1-2; p42 = p41+1; p43 = p42+1; p44 = p43+1; p45 = p44+1;
  p52 =  2*cc1-1; p53 = p52+1; p54 = p53+1;

  // Low pass filtering to extract luminance at GREEN pixels
  for (rr=2; rr < rr1-2; rr++)
    for (cc=2+(FC(rr,3)&1), c=FC(rr,cc+1); cc < cc1-2; cc+=2) {
      pix = dimage + rr*cc1+cc;
      d = 2 - c;
      v0  = (pix[p12][c] + pix[p14][c] + pix[p21][d] + pix[p25][d] +
	     pix[p41][d] + pix[p45][d] + pix[p52][c] + pix[p54][c]
	     -  2.0*(pix[p13][1] + pix[ -2][1] + pix[  2][1] + pix[p53][1])
	     -  4.0*(pix[p22][1] + pix[p24][1] + pix[p42][1] + pix[p44][1])
	     +  6.0*(pix[p23][d] + pix[ -1][c] + pix[  1][c] + pix[p43][d])
	     + 56.0*pix[0][1]) / 64.0;
      if (clip_on) v0 = LIM(v0,0.0,1.0);
      pix[0][3] = v0; }
  // Compute adapative weighting matrix
  for (ii=0, rr=2; rr < rr1-2; rr++)
    for (cc=2+(FC(rr,2)&1), c=FC(rr,cc); cc < cc1-2; cc+=2, ii++) {
      pix = dimage + rr*cc1+cc;
      v0 = pix[0][c];
      // horizontal
      dG = pix[1][1] - pix[-1][1];
      if (dG < 0) dG = -dG;
      v1 = v0 - pix[ -2][c];
      if (v1 < 0) v1 = -v1;
      v2 = v0 - pix[  2][c];
      if (v2 < 0) v2 = -v2;
      w0 = 1.0/(1.0 + v1 + dG);
      w2 = 1.0/(1.0 + v2 + dG);
      // vertical
      dG = pix[p43][1] - pix[p23][1];
      if (dG < 0) dG = -dG;
      v1 = v0 - pix[p13][c];
      if (v1 < 0) v1 = -v1;
      v2 = v0 - pix[p53][c];
      if (v2 < 0) v2 = -v2;
      w1 = 1.0/(1.0 + v1 + dG);
      w3 = 1.0/(1.0 + v2 + dG);
      //
      ws = w0 + w1 + w2 + w3;
      w[ii][0] = w0/ws;
      w[ii][1] = w1/ws;
      w[ii][2] = w2/ws;
      w[ii][3] = w3/ws; }
  // Calculate initial red/blue at GREEN pixels with bilinear
  for (rr=1; rr < rr1-1; rr++)
    for (cc=1+(FC(rr,2)&1), c=FC(rr,cc+1); cc < cc1-1; cc+=2) {
      pix = dimage + rr*cc1+cc;
      pix[0][c] = 0.5*(pix[ -1][c] + pix[  1][c]);
      c = 2 - c;
      pix[0][c] = 0.5*(pix[p23][c] + pix[p43][c]);
      c = 2 - c; }
  for (lum_iter=0; lum_iter < afd_passes; lum_iter++) {
    // Estimate luminance at RED/BLUE pixels
    for (ii=0, rr=2; rr < rr1-2; rr++)
      for (cc=2+(FC(rr,2)&1), c=FC(rr,cc); cc < cc1-2; cc+=2, ii++) {
	pix = dimage + rr*cc1+cc;
 	v0 = pix[0][c] +
	  w[ii][0]*(pix[ -1][3] - pix[ -1][c]) +
	  w[ii][2]*(pix[  1][3] - pix[  1][c]) +
	  w[ii][1]*(pix[p23][3] - pix[p23][c]) +
	  w[ii][3]*(pix[p43][3] - pix[p43][c]);
	if (clip_on) v0 = LIM(v0,0.0,1.0);
	pix[0][3] = v0; }
    // Interpolote red/blue pixels on BLUE/RED pixel locations
    // using pattern regcognition on (r-L or b-L)
    for (rr=1; rr < rr1-1; rr++)
      for (cc=1+(FC(rr,1)&1), c=2-FC(rr,cc); cc < cc1-1; cc+=2) {
	pix = dimage + rr*cc1+cc;
	dC1 = pix[p22][c] - pix[p22][3];
	dC2 = pix[p24][c] - pix[p24][3];
	dC3 = pix[p42][c] - pix[p42][3];
	dC4 = pix[p44][c] - pix[p44][3];
	dC0 = 0.25*(dC1 + dC2 + dC3 + dC4);
	jj = (dC1 > dC0) + (dC2 > dC0) + (dC3 > dC0) + (dC4 > dC0);
	if (jj == 3 || jj == 1) {
	  // edge-corner pattern: median of colorr differential values
	  PIX_SORT(dC1,dC2);
	  PIX_SORT(dC3,dC4);
	  PIX_SORT(dC1,dC3);
	  PIX_SORT(dC2,dC4);
	  dC0 = 0.5*(dC2 + dC3); }
	else {
	  // stripe pattern: average along diagonal
	  v1 = pix[p22][c] - pix[p44][c];
	  if (v1 < 0) v1 = -v1;
	  v2 = pix[p24][c] - pix[p42][c];
	  if (v2 < 0) v2 = -v2;
	  dC0 = (v1 < v2 ? 0.5*(dC1 + dC4) : 0.5*(dC2 + dC3)); }
	v0 = pix[0][3] + dC0;
	if (clip_on) v0 = LIM(v0,0.0,1.0);
	pix[0][c] = v0; }
    // Interpolote red/blue pixels on GREEN pixel locations
    // using pattern regcognition on (r-L or b-L)
    for (rr=1; rr < rr1-1; rr++)
      for (cc=1+(FC(rr,2)&1), c=FC(rr,cc+1); cc < cc1-1; cc+=2) {
	pix = dimage + rr*cc1+cc;
	for (ii=0; ii < 2; c=2-c, ii++) {
	  dC1 = pix[p23][c] - pix[p23][3];
	  dC2 = pix[ -1][c] - pix[ -1][3];
	  dC3 = pix[  1][c] - pix[  1][3];
	  dC4 = pix[p43][c] - pix[p43][3];
	  dC0 = 0.25*(dC1 + dC2 + dC3 + dC4);
	  jj = (dC1 > dC0) + (dC2 > dC0) + (dC3 > dC0) + (dC4 > dC0);
	  if (jj == 3 || jj == 1) {
	    // edge-corner pattern: median of color differential values
	    PIX_SORT(dC1,dC2);
	    PIX_SORT(dC3,dC4);
	    PIX_SORT(dC1,dC3);
	    PIX_SORT(dC2,dC4);
	    dC0 = 0.5*(dC2 + dC3); }
	  else {
	    // stripe pattern: average along diagonal
	    v1 = pix[p23][c] - pix[p43][c];
	    if (v1 < 0) v1 = -v1;
	    v2 = pix[ -1][c] - pix[  1][c];
	    if (v2 < 0) v2 = -v2;
	    dC0 = (v1 < v2 ? 0.5*(dC1 + dC4) : 0.5*(dC2 + dC3)); }
	  v0 = pix[0][3] + dC0;
	  if (clip_on) v0 = LIM(v0,0.0,1.0);
	  pix[0][c] = v0; }
      }
  }
  free(w);

  // Interpolate green pixels at RED/BLUE
  for (rr=2; rr < rr1-2; rr++)
    for (cc=2+(FC(rr,2)&1), c=2-FC(rr,cc); cc < cc1-2; cc+=2) {
      pix = dimage + rr*cc1+cc;
      // green
      pix[0][1] = pix[0][3] + 0.25*(pix[ -1][1] - pix[ -1][3] +
				    pix[  1][1] - pix[  1][3] +
				    pix[p23][1] - pix[p23][3] +
				    pix[p43][1] - pix[p43][3]); }

  // copy result back to image matrix
  for (row=0; row < height; row++)
    for (col=0, rr=row+ba; col < width; col++) {
      cc = col+ba;
      c = FC(row,col);
      pix = dimage + rr*cc1+cc;
      ii = row*width + col;
      for (jj=0; jj < 3; jj++)
	if (jj != c) {
	  v0 = pix[0][jj];
	  v1 = (v0 <= 0.04045 ? v0/12.92 : pow((v0 + 0.055)/1.055,2.4));
	  image[ii][jj] = CLIP((int)(65535.0*v1 + 0.5)); } }
  free(dimage);

  // Done
  t2 = clock();
  dt = ((double)(t2-t1)) / CLOCKS_PER_SEC;
#ifdef DCRAW_VERBOSE
  if (verbose) fprintf(stderr,_("\telapsed time = %5.3fs\n"),dt);
#endif
}

//void CLASS afd_noise_filter_pl()
//{
//  int i,c,row,col;
//  int shrinked;
//	float mean,std_dev;
//	float noise_attenuation;
//  ushort value;
//  int offset;
//  ushort (*img)[4], *lum;
//
//  if (half_size) return;
//	shrinked=0;
//	if (shrink){
//	  shrinked=shrink;
//	  unshrink_image();
//	}
//
//  img = (ushort (*)[4]) calloc (height*width, sizeof *img);
//  merror(img,"afd_noise_filter_pl");
//  memcpy(img,image,height*width*sizeof *img);
//
//  // preinterpolate
//  filters_bk=filters;
//  for (row = FC(1,0) >> 1; row < height; row+=2)
//	  for (col = FC(row,1) & 1; col < width; col+=2)
//      image[row*width+col][1] = image[row*width+col][3];
//      filters &= ~((filters & 0x55555555) << 1);
//  // interpolate
//  afd_interpolate_pl(2,1);
//
//  // Get luminance
//  lum = (ushort *)calloc(width*height,sizeof *lum);
//  for(i=0;i<width*height;i++) lum[i]=(ushort)CLIP(CIE_L(i));
//
//  for(row=1;row<height-1;row++)
//	  for(col=1;col<width-1;col++){
//		  // Calculate 3x3 mean
//      offset=row*width+col;
//      c=FC(row,col);
//		  value=image[offset][c];
//
//      mean=
//        (lum[offset-width-1]+
//         lum[offset-width]+
//         lum[offset-width+1]+
//         lum[offset-1]+
//         lum[offset]+
//         lum[offset+1]+
//         lum[offset+width-1]+
//         lum[offset+width]+
//         lum[offset+width+1])/9.0;
//
//      // Calculate 3x3 standard deviation
//      std_dev=sqrt
//        ((lum[offset-width-1]-mean)*(lum[offset-width-1]-mean)+
//         (lum[offset-width]-mean)*(lum[offset-width]-mean)+
//         (lum[offset-width+1]-mean)*(lum[offset-width+1]-mean)+
//         (lum[offset-1]-mean)*(lum[offset-1]-mean)+
//         (lum[offset]-mean)*(lum[offset]-mean)+
//         (lum[offset+1]-mean)*(lum[offset+1]-mean)+
//         (lum[offset+width-1]-mean)*(lum[offset+width-1]-mean)+
//         (lum[offset+width]-mean)*(lum[offset+width]-mean)+
//         (lum[offset+width+1]-mean)*(lum[offset+width+1]-mean))/9.0;
//
//			// Salt and pepper identification
//		  if(1.0*std_dev<abs((float)lum[offset]-mean)){
//        mean=
//        (image[offset-width-1][c]+
//         image[offset-width][c]+
//         image[offset-width+1][c]+
//         image[offset-1][c]+
//         image[offset+1][c]+
//         image[offset+width-1][c]+
//         image[offset+width][c]+
//         image[offset+width+1][c])/8.0;
//
//        noise_attenuation=((float)afd_noise_thres)/exp(10.0*mean/(float)maximum);
//        value=(ushort)CLIP((mean*((float)noise_attenuation)+(float)value*(100.0-(float)noise_attenuation))/100.0);
//        image[offset][c]=img[offset][FC_BK(row,col)]=value;
//        lum[offset]=(ushort)CLIP(CIE_L(offset));
//      }
//    }
//  // Restore uninterpolated image
//  memcpy(image,img,width*height*sizeof *img);
//  filters=filters_bk;
//
//  free(img);
//  free(lum);
//
//  if (shrinked) {
//    shrink=shrinked;
//	  shrink_image();
//	}
//}
#undef PIX_SORT
#undef FC_BK
