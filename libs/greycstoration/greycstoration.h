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
#define cimg_plugin_greycstoration 2.5.1

//------------------------------------------------------------------------------
// GREYCstoration structure, storing important informations about algorithm
// parameters and computing threads
//-------------------------------------------------------------------------------
struct _greycstoration_params {

  // Parameters needed for the GREYCstoration regularization algorithm.
  const CImg<unsigned char>
    *mask;
  float
    amplitude,
    sharpness,
    anisotropy,
    alpha,
    sigma,
    gfact,
    dl,
    da,
    gauss_prec;
  unsigned int
    interpolation;
  bool
    fast_approx;

  // Parameters for the different threads.
  CImg<T>
    *source,
    *temporary;
  unsigned long
    *counter;
  unsigned int
    tile,
    tile_border,
    thread,
    threads;
  bool
    is_running,
    *stop_request;

  // Default constructor
  _greycstoration_params():mask(0),amplitude(0),sharpness(0),anisotropy(0),alpha(0),sigma(0),gfact(1),
                           dl(0),da(0),gauss_prec(0),interpolation(0),fast_approx(false),
                           source(0),temporary(0),counter(0),tile(0),tile_border(0),thread(0),threads(0),
                           is_running(false), stop_request(0) {}
};

_greycstoration_params greycstoration_params[16];

//------------------------------------------------------------------------------
// GREYCstoration threaded function
//-------------------------------------------------------------------------------
#if cimg_OS==1
static void* greycstoration_thread(void *arg) {
#elif cimg_OS==2
  static DWORD WINAPI greycstoration_thread(void *arg) {
#endif
    _greycstoration_params &p = *(_greycstoration_params*)arg;
    const CImg<unsigned char> &mask = *(p.mask);
    CImg<T> &source = *(p.source);

    if (!p.tile) {

      // Non-tiled version
      //------------------
      source.blur_anisotropic(mask,p.amplitude,p.sharpness,p.anisotropy,p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,
                              p.interpolation,p.fast_approx,p.gfact);

    } else {

      // Tiled version
      //---------------
      CImg<T> &temporary = *(p.temporary);
      const bool threed = (source.depth>1);
      const unsigned int b = p.tile_border;
      unsigned int ctile = 0;
      if (threed) {
        for (unsigned int z=0; z<source.depth && !*(p.stop_request); z+=p.tile)
          for (unsigned int y=0; y<source.height && !*(p.stop_request); y+=p.tile)
            for (unsigned int x=0; x<source.width && !*(p.stop_request); x+=p.tile)
              if (((ctile++)%p.threads)==p.thread) {
                const unsigned int
                  x1 = x+p.tile-1,
                  y1 = y+p.tile-1,
                  z1 = z+p.tile-1,
                  xe = x1<source.width?x1:source.width-1,
                  ye = y1<source.height?y1:source.height-1,
                  ze = z1<source.depth?z1:source.depth-1;
                CImg<T> img = source.get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
                CImg<unsigned char> mask_tile = mask.is_empty()?mask:mask.get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
                img.greycstoration_params[0] = p;
                img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                     p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
                temporary.draw_image(img.crop(b,b,b,img.width-b,img.height-b,img.depth-b),x,y,z);
              }
      } else {
        for (unsigned int y=0; y<source.height && !*(p.stop_request); y+=p.tile)
          for (unsigned int x=0; x<source.width && !*(p.stop_request); x+=p.tile)
            if (((ctile++)%p.threads)==p.thread) {
              const unsigned int
                x1 = x+p.tile-1,
                y1 = y+p.tile-1,
                xe = x1<source.width?x1:source.width-1,
                ye = y1<source.height?y1:source.height-1;
              CImg<T> img = source.get_crop(x-b,y-b,xe+b,ye+b,true);
              CImg<unsigned char> mask_tile = mask.is_empty()?mask:mask.get_crop(x-b,y-b,xe+b,ye+b,true);
              img.greycstoration_params[0]=p;
              img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                   p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
              temporary.draw_image(img.crop(b,b,img.width-b,img.height-b),x,y);
            }
      }
    }

    if (!p.thread) {
      if (p.threads>1) {
        bool stopflag = true;
        do {
          stopflag = true;
          for (unsigned int k=1; k<p.threads; k++) if (source.greycstoration_params[k].is_running) stopflag = false;
          if (!stopflag) cimg::wait(50);
        } while (!stopflag);
      }
      if (p.counter) delete p.counter;
      if (p.temporary) { source = *(p.temporary); delete p.temporary; }
      if (p.stop_request) delete p.stop_request;
      p.mask = 0;
      p.amplitude = p.sharpness = p.anisotropy = p.alpha = p.sigma = p.gfact = p.dl = p.da = p.gauss_prec = 0;
      p.interpolation = 0;
      p.fast_approx = false;
      p.source = 0;
      p.temporary = 0;
      p.counter = 0;
      p.tile = p.tile_border = p.thread = p.threads = 0;
      p.stop_request = false;
    }
    p.is_running = false;

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

  //! Test if GREYCstoration threads are currently running.
  bool greycstoration_is_running() const {
    return greycstoration_params->is_running;
  }

  //! Force GREYCstoration threads to stop.
  CImg& greycstoration_stop() {
    if (greycstoration_is_running()) {
      *(greycstoration_params->stop_request) = true;
      while (greycstoration_params->is_running) cimg::wait(50);
    }
    return *this;
  }

  //! Return the GREYCstoration progression indice.
  float greycstoration_progress() const {
    if (!greycstoration_is_running()) return 0.0f;
    const unsigned long counter = greycstoration_params->counter?*(greycstoration_params->counter):0;
    const float da = greycstoration_params->da;
    float maxcounter = 0;
    if (greycstoration_params->tile==0) maxcounter = width*height*(1 + 360/da);
    else {
      const unsigned int
        t = greycstoration_params->tile,
        b = greycstoration_params->tile_border,
        n = (1+(width-1)/t)*(1+(height-1)/t)*(1+(depth-1)/t);
      maxcounter = (width*height + n*4*b*(b + t))*(1 + 360/da);
    }
    return cimg::min(counter*99.9f/maxcounter,99.9f);
  }

  //! Run the threaded GREYCstoration algorithm on the instance image, using a mask.
  CImg& greycstoration_run(const CImg<unsigned char>& mask,
                           const float amplitude=60, const float sharpness=0.7f, const float anisotropy=0.3f,
                           const float alpha=0.6f,const float sigma=1.1f, const float gfact=1.0f,
                           const float dl=0.8f,const float da=30.0f,
                           const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                           const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int threads=1) {
    if (greycstoration_is_running())
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Another GREYCstoration thread is already running on"
                                  " instance image (%u,%u,%u,%u,%p).",width,height,depth,dim,data);

    else {
      if (!mask.is_empty() && !mask.is_sameXY(*this))
        throw CImgArgumentException("CImg<%s>::greycstoration_run() : Given mask (%u,%u,%u,%u,%p) and instance image "
                                    "(%u,%u,%u,%u,%p) have different dimensions.",
                                    pixel_type(),mask.width,mask.height,mask.depth,mask.dim,mask.data,width,height,depth,dim,data);
      cimg::warn(threads>16,"CImg<%s>::greycstoration_run() : Multi-threading mode limited to 16 threads max.");
      const unsigned int
        ntile = (tile && (tile<width || tile<height || (depth>1 && tile<depth)))?tile:0,
        nthreads = ntile?cimg::max(cimg::min(threads,16U),1U):1;

      CImg<T> *const temporary = ntile?new CImg<T>(*this):0;
      unsigned long *const counter = new unsigned long;
      *counter = 0;
      bool *const stop_request = new bool;
      *stop_request = false;

      for (unsigned int k=0; k<nthreads; k++) {
        greycstoration_params[k].mask = &mask;
        greycstoration_params[k].amplitude = amplitude;
        greycstoration_params[k].sharpness = sharpness;
        greycstoration_params[k].anisotropy = anisotropy;
        greycstoration_params[k].alpha = alpha;
        greycstoration_params[k].sigma = sigma;
        greycstoration_params[k].gfact = gfact;
        greycstoration_params[k].dl = dl;
        greycstoration_params[k].da = da;
        greycstoration_params[k].gauss_prec = gauss_prec;
        greycstoration_params[k].interpolation = interpolation;
        greycstoration_params[k].fast_approx = fast_approx;
        greycstoration_params[k].source = this;
        greycstoration_params[k].temporary = temporary;
        greycstoration_params[k].counter = counter;
        greycstoration_params[k].tile = ntile;
        greycstoration_params[k].tile_border = tile_border;
        greycstoration_params[k].thread = k;
        greycstoration_params[k].threads = nthreads;
        greycstoration_params[k].is_running = true;
        greycstoration_params[k].stop_request = stop_request;
      }
#if cimg_OS==1
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      for (unsigned int k=0; k<greycstoration_params->threads; k++) {
        pthread_t thread;
        const int err = pthread_create(&thread, &attr, greycstoration_thread, (void*)(greycstoration_params+k));
        if (err) throw CImgException("CImg<%s>::greycstoration_run() : pthread_create returned error %d",
                                     pixel_type(), err);
      }
#elif cimg_OS==2
      for (unsigned int k=0; k<greycstoration_params->threads; k++) {
        unsigned long ThreadID = 0;
        CreateThread(0,0,greycstoration_thread,(void*)(greycstoration_params+k),0,&ThreadID);
      }
#else
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Threads are not supported, please define cimg_OS first.");
#endif
    }
    return *this;
  }

  //! Run the GREYCstoration algorithm on the instance image.
  CImg& greycstoration_run(const float amplitude=50, const float sharpness=0.7f, const float anisotropy=0.3f,
                           const float alpha=0.6f,const float sigma=1.1f, const float gfact=1.0f,
                           const float dl=0.8f,const float da=30.0f,
                           const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                           const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int threads=1) {
    static const CImg<unsigned char> empty_mask;
    return greycstoration_run(empty_mask,amplitude,sharpness,anisotropy,alpha,sigma,gfact,dl,da,gauss_prec,
                              interpolation,fast_approx,tile,tile_border,threads);
  }

#endif
