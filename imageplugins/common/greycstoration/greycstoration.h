/*-----------------------------------------------------------------------------

  File        : greycstoration.h

  Description : GREYCstoration PLUG-IN allowing easy integration in
  third parties softwares.

  (see http://www.greyc.ensicaen.fr/~dtschump/greycstoration/)

  THIS VERSION IS FOR DEVELOPERS ONLY. IT EASES THE INTEGRATION ALGORITHM IN
  THIRD PARTIES SOFTWARES. IF YOU ARE A USER OF GREYCSTORATION, PLEASE LOOK
  AT THE FILE 'greycstoration.cpp' WHICH IS THE SOURCE OF THE COMPLETE
  COMMAND LINE GREYCSTORATION TOOL.

  Copyright  : David Tschumperle - http://www.greyc.ensicaen.fr/~dtschump/

  This software is governed by the CeCILL  license under French law and
  abiding by the rules of distribution of free software.  You can  use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".

  As a counterpart to the access to the source code and  rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty  and the software's author,  the holder of the
  economic rights,  and the successive licensors  have only  limited
  liability.

  In this respect, the user's attention is drawn to the risks associated
  with loading,  using,  modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean  that it is complicated to manipulate,  and  that  also
  therefore means  that it is reserved for developers  and  experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and,  more generally, to use and operate it in the
  same conditions as regards security.

  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.

  ------------------------------------------------------------------------------*/

#ifndef cimg_plugin_greycstoration
// This tells you about the version of the GREYCstoration algorithm implemented
#define cimg_plugin_greycstoration 2.5

//------------------------------------------------------------------------------
// This is internal structures and functions, not to be used by the plugin user
// See functions below instead.
//-------------------------------------------------------------------------------
struct _greycstoration_params {
  const CImg<unsigned char> *mask;
  float amplitude,
    sharpness,
    anisotropy,
    alpha,
    sigma,
    dl,
    da,
    gauss_prec;
  unsigned int interpolation,
    tile,
    tile_border;
  bool fast_approx;
  unsigned long *counter;

  _greycstoration_params() {
    counter = new unsigned long;
    *counter = ~0UL;
  }

  _greycstoration_params& assign(const CImg<unsigned char> *const pmask=0,
                                 const float pamplitude=60, const float psharpness=0.7f, const float panisotropy=0.3f,
                                 const float palpha=0.6f,const float psigma=1.1f, const float pdl=0.8f, const float pda=30.0f,
                                 const float pgauss_prec=2.0f, const unsigned int pinterpolation=0, const bool pfast_approx=true,
                                 const unsigned int ptile=0, const unsigned int ptile_border=0) {
    mask = pmask;
    amplitude = pamplitude;
    sharpness = psharpness;
    anisotropy = panisotropy;
    alpha = palpha;
    sigma = psigma;
    dl = pdl;
    da = pda;
    gauss_prec = pgauss_prec;
    interpolation = pinterpolation;
    tile = ptile;
    tile_border = ptile_border;
    fast_approx = pfast_approx;
    *counter = 0UL;
    return *this;
  }

  ~_greycstoration_params() {
    if (counter) delete counter;
  }
};

_greycstoration_params greycstoration_params;

#if cimg_OS==1
static void* greycstoration_thread(void *arg) {
#elif cimg_OS==2
  static DWORD WINAPI greycstoration_thread(void *arg) {
#endif
    CImg<T>& img = *(CImg<T>*)arg;
    _greycstoration_params &p = img.greycstoration_params;
    const CImg<unsigned char>& mask = *(p.mask);

    if (!p.tile) // Non-tiled version
      img.blur_anisotropic(mask,p.amplitude,p.sharpness,p.anisotropy,p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx);
    else { // Tiled version (much slower, but require less memory, may avoid memory swap and finally save time !)
      const bool threed = (img.depth>1);
      CImg<T> res(img.width,img.height,img.depth,img.dim,0);
      const unsigned int b = p.tile_border;
      if (threed)
        for (unsigned int z=0; z<img.depth; z+=p.tile)
          for (unsigned int y=0; y<img.height; y+=p.tile)
            for (unsigned int x=0; x<img.width; x+=p.tile) {
              const unsigned int
                x1 = x+p.tile-1,
                y1 = y+p.tile-1,
                z1 = z+p.tile-1,
                xe = x1<img.width?x1:img.width-1,
                ye = y1<img.height?y1:img.height-1,
                ze = z1<img.depth?z1:img.depth-1;
              CImg<T> img_tile = img.get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
              CImg<unsigned char> mask_tile = mask.is_empty()?mask:mask.get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
              img_tile.greycstoration_params.assign(&mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                                    p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx);
              delete img_tile.greycstoration_params.counter;
              img_tile.greycstoration_params.counter = img.greycstoration_params.counter;
	      img_tile.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                        p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx);
              img_tile.crop(b,b,b,img_tile.width-b,img_tile.height-b,img_tile.depth-b);
              img_tile.greycstoration_params.counter = 0;
              res.draw_image(img_tile,x,y,z);
            }
      else
        for (unsigned int y=0; y<img.height; y+=p.tile)
          for (unsigned int x=0; x<img.width; x+=p.tile) {
            const unsigned int
              x1 = x+p.tile-1,
              y1 = y+p.tile-1,
              xe = x1<img.width?x1:img.width-1,
              ye = y1<img.height?y1:img.height-1;
            CImg<T> img_tile = img.get_crop(x-b,y-b,xe+b,ye+b,true);
            CImg<unsigned char> mask_tile = mask.is_empty()?mask:mask.get_crop(x-b,y-b,xe+b,ye+b,true);
            img_tile.greycstoration_params.assign(&mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                                  p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx);
            delete img_tile.greycstoration_params.counter;
            img_tile.greycstoration_params.counter = img.greycstoration_params.counter;
            img_tile.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                      p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx);
	    img_tile.crop(b,b,img_tile.width-b,img_tile.height-b);
            img_tile.greycstoration_params.counter = 0;
            res.draw_image(img_tile,x,y);
          }
      img = res;
    }
    img.greycstoration_stop();
#if cimg_OS==1
    pthread_exit(arg);
    return arg;
#elif cimg_OS==2
    ExitThread(0);
    return 0;
#endif
  }

  //----------------------------------------------------------
  // Public GREYCstoration plugin API
  // Use the functions below for integrating GREYCstoration
  // in your own code.
  //----------------------------------------------------------

  //! Stop the GREYCstoration thread
  CImg& greycstoration_stop() {
    *(greycstoration_params.counter) = ~0UL;
    return *this;
  }

  //! Run the threaded GREYCstoration algorithm on the instance image, using a mask.
  /**
     This function creates a new thread and returns immediately.
     One iteration of inpainting is done when the greycstoration_progress() functions returns 100.
  **/
  CImg& greycstoration_mask_run(const CImg<unsigned char>& mask,
                                const float amplitude=60, const float sharpness=0.7f, const float anisotropy=0.3f,
                                const float alpha=0.6f,const float sigma=1.1f, const float dl=0.8f,const float da=30.0f,
                                const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                                const unsigned int tile=0, const unsigned int tile_border=0) {
    if (greycstoration_is_running())
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Another GREYCstoration thread is already running on"
                                  " the instance image (%u,%u,%u,%u,%p).",width,height,depth,dim,data);

    else {
      if (!mask.is_empty() && !mask.is_sameXY(*this))
        throw CImgArgumentException("CImg<%s>::greycstoration_run() : Given mask (%u,%u,%u,%u,%p) and instance image "
                                    "(%u,%u,%u,%u,%p) have different dimensions.",
                                    mask.width,mask.height,mask.depth,mask.dim,mask.data,width,height,depth,dim,data);
      greycstoration_params.assign(&mask,amplitude,sharpness,anisotropy,alpha,sigma,dl,da,gauss_prec,interpolation,fast_approx,tile,tile_border);
#if cimg_OS==1
      pthread_t thread;
      pthread_create(&thread, 0, greycstoration_thread, (void*)this);
#elif cimg_OS==2
      unsigned long ThreadID = 0;
      CreateThread(0,0,greycstoration_thread,(void*)this,0,&ThreadID);
#else
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Threads are not supported, please define cimg_OS first.");
#endif
    }
    return *this;
  }

  //! Run the GREYCstoration algorithm on the instance image.
  /**
     This function creates a new thread and returns immediately.
     The denoising is done when the greycstoration_progress() functions returns 100.
  **/
  CImg& greycstoration_run(const float amplitude=50, const float sharpness=0.7f, const float anisotropy=0.3f,
                           const float alpha=0.6f,const float sigma=1.1f, const float dl=0.8f,const float da=30.0f,
                           const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                           const unsigned int tile=0, const unsigned int tile_border=0) {
    static const CImg<unsigned char> empty_mask;
    return greycstoration_mask_run(empty_mask,amplitude,sharpness,anisotropy,alpha,sigma,dl,da,gauss_prec,
                                   interpolation,fast_approx,tile,tile_border);
  }

  //! Return the progression indice of the GREYCstoration algorithm which is running.
  /**
     If it returns 100, then the algorithm has been completed successfully.
  **/
  float greycstoration_progress() const {
    if (!greycstoration_is_running()) return 0;
    const float
      counter = (float)*(greycstoration_params.counter),
      da = greycstoration_params.da;
    float maxcounter = 0;
    if (greycstoration_params.tile==0)
      maxcounter = width*height*(1 + 360/da);
    else {
      const unsigned int
        t = greycstoration_params.tile,
        b = greycstoration_params.tile_border,
        n = (1+(width-1)/t)*(1+(height-1)/t)*(1+(depth-1)/t);
      maxcounter = (width*height + n*4*b*(b + t))*(1 + 360/da);
    }
    return cimg::min(counter*99.9f/maxcounter,99.9f);
  }

  //! Return the state of the GREYCstoration algorithm (running or not).
  bool greycstoration_is_running() const {
    return (*(greycstoration_params.counter)!=~0UL);
  }

#endif
