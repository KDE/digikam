/*
  #
  #  File        : greycstoration.h
  #                ( C++ header file - CImg plug-in )
  #
  #  Description : GREYCstoration plug-in allowing easy integration in
  #                third parties softwares.
  #                ( http://www.greyc.ensicaen.fr/~dtschump/greycstoration/ )
  #                This file is a part of the CImg Library project.
  #                ( http://cimg.sourceforge.net )
  #
  #  THIS PLUG-IN IS INTENDED FOR DEVELOPERS ONLY. IT EASES THE INTEGRATION ALGORITHM IN
  #  THIRD PARTIES SOFTWARES. IF YOU ARE A USER OF GREYCSTORATION, PLEASE LOOK
  #  AT THE FILE 'greycstoration.cpp' WHICH IS THE SOURCE OF THE COMPLETE
  #  COMMAND LINE GREYCSTORATION TOOL.
  #
  #  Copyright   : David Tschumperle
  #                ( http://www.greyc.ensicaen.fr/~dtschump/ )
  #
  #  License     : CeCILL v2.0
  #                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
  #
  #  This software is governed by the CeCILL  license under French law and
  #  abiding by the rules of distribution of free software.  You can  use,
  #  modify and/ or redistribute the software under the terms of the CeCILL
  #  license as circulated by CEA, CNRS and INRIA at the following URL
  #  "http://www.cecill.info".
  #
  #  As a counterpart to the access to the source code and  rights to copy,
  #  modify and redistribute granted by the license, users are provided only
  #  with a limited warranty  and the software's author,  the holder of the
  #  economic rights,  and the successive licensors  have only  limited
  #  liability.
  #
  #  In this respect, the user's attention is drawn to the risks associated
  #  with loading,  using,  modifying and/or developing or reproducing the
  #  software by the user in light of its specific status of free software,
  #  that may mean  that it is complicated to manipulate,  and  that  also
  #  therefore means  that it is reserved for developers  and  experienced
  #  professionals having in-depth computer knowledge. Users are therefore
  #  encouraged to load and test the software's suitability as regards their
  #  requirements in conditions enabling the security of their systems and/or
  #  data to be ensured and,  more generally, to use and operate it in the
  #  same conditions as regards security.
  #
  #  The fact that you are presently reading this means that you have had
  #  knowledge of the CeCILL license and that you accept its terms.
  #
*/

#ifndef cimg_plugin_greycstoration
#define cimg_plugin_greycstoration

//------------------------------------------------------------------------------
// GREYCstoration parameter structure, storing important informations about
// algorithm parameters and computing threads.
// ** This structure has not to be manipulated by the API user, so please just
// ignore it if you want to **
//-------------------------------------------------------------------------------
struct _greycstoration_params {

  // Tell if the patch-based algorithm is selected
  bool patch_based;

  // Parameters specific to the non-patch regularization algorithm
  float amplitude;
  float sharpness;
  float anisotropy;
  float alpha;
  float sigma;
  float gfact;
  float dl;
  float da;
  float gauss_prec;
  unsigned int interpolation;

  // Parameters specific to the patch-based regularization algorithm
  unsigned int patch_size;
  float sigma_s;
  float sigma_p;
  unsigned int lookup_size;

  // Non-specific parameters of the algorithms.
  CImg<T> *source;
  const CImg<unsigned char> *mask;
  CImg<T> *temporary;
  unsigned long *counter;
  unsigned int tile;
  unsigned int tile_border;
  unsigned int thread;
  unsigned int nb_threads;
  bool fast_approx;
  bool is_running;
  bool *stop_request;
#if cimg_OS==1 && defined(_PTHREAD_H)
  pthread_mutex_t
  *mutex;
#elif cimg_OS==2
  HANDLE mutex;
#else
  void *mutex;
#endif

  // Default constructor
  _greycstoration_params():patch_based(false),amplitude(0),sharpness(0),anisotropy(0),alpha(0),sigma(0),gfact(1),
       dl(0),da(0),gauss_prec(0),interpolation(0),patch_size(0),
       sigma_s(0),sigma_p(0),lookup_size(0),source(0),mask(0),temporary(0),counter(0),tile(0),
       tile_border(0),thread(0),nb_threads(0),fast_approx(false),is_running(false), stop_request(0), mutex(0) {}
};

_greycstoration_params greycstoration_params[16];

//----------------------------------------------------------
// Public functions of the GREYCstoration API.
// Use the functions below for integrating GREYCstoration
// in your own C++ code.
//----------------------------------------------------------

//! Test if GREYCstoration threads are still running.
bool greycstoration_is_running() const {
  return greycstoration_params->is_running;
}

//! Force the GREYCstoration threads to stop.
CImg& greycstoration_stop() {
  if (greycstoration_is_running()) {
    *(greycstoration_params->stop_request) = true;
    while (greycstoration_params->is_running) cimg::wait(50);
  }
  return *this;
}

//! Return the GREYCstoration progress bar indice (between 0 and 100).
float greycstoration_progress() const {
  if (!greycstoration_is_running()) return 0.0f;
  const unsigned long counter = greycstoration_params->counter?*(greycstoration_params->counter):0;
  const float
    da = greycstoration_params->da,
    factor = greycstoration_params->patch_based?1:(1+360/da);
  float maxcounter = 0;
  if (greycstoration_params->tile==0) maxcounter = width*height*depth*factor;
  else {
    const unsigned int
      t = greycstoration_params->tile,
      b = greycstoration_params->tile_border,
      n = (1+(width-1)/t)*(1+(height-1)/t)*(1+(depth-1)/t);
    maxcounter = (width*height*depth + n*4*b*(b + t))*factor;
  }
  return cimg::min(counter*99.9f/maxcounter,99.9f);
}

//! Run the non-patch version of the GREYCstoration algorithm on the instance image, using a mask.
CImg& greycstoration_run(const CImg<unsigned char>& mask,
                         const float amplitude=60, const float sharpness=0.7f, const float anisotropy=0.3f,
                         const float alpha=0.6f, const float sigma=1.1f, const float gfact=1.0f,
                         const float dl=0.8f, const float da=30.0f,
                         const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                         const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int nb_threads=1) {

  if (greycstoration_is_running())
    throw CImgInstanceException("CImg<T>::greycstoration_run() : A GREYCstoration thread is already running on"
                                " the instance image (%u,%u,%u,%u,%p).",width,height,depth,dim,data);

  else {
    if (!mask.is_empty() && !mask.is_sameXY(*this))
      throw CImgArgumentException("CImg<%s>::greycstoration_run() : Given mask (%u,%u,%u,%u,%p) and instance image "
                                  "(%u,%u,%u,%u,%p) have different dimensions.",
                                  pixel_type(),mask.width,mask.height,mask.depth,mask.dim,mask.data,width,height,depth,dim,data);
    if (nb_threads>16) cimg::warn("CImg<%s>::greycstoration_run() : Multi-threading mode limited to 16 threads max.");
    const unsigned int
      ntile = (tile && (tile<width || tile<height || (depth>1 && tile<depth)))?tile:0,
#if cimg_OS==1 && !defined(_PTHREAD_H)
      nthreads = 0;
#else
    nthreads = ntile?cimg::min(nb_threads,16U):cimg::min(nb_threads,1U);
#endif

    CImg<T> *const temporary = ntile?new CImg<T>(*this):0;
    unsigned long *const counter = new unsigned long;
    *counter = 0;
    bool *const stop_request = new bool;
    *stop_request = false;

    for (unsigned int k=0; k<(nthreads?nthreads:1); k++) {
      greycstoration_params[k].patch_based = false;
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
      greycstoration_params[k].mask = &mask;
      greycstoration_params[k].temporary = temporary;
      greycstoration_params[k].counter = counter;
      greycstoration_params[k].tile = ntile;
      greycstoration_params[k].tile_border = tile_border;
      greycstoration_params[k].thread = k;
      greycstoration_params[k].nb_threads = nthreads;
      greycstoration_params[k].is_running = true;
      greycstoration_params[k].stop_request = stop_request;
      if (k) greycstoration_params[k].mutex = greycstoration_params[0].mutex;
      else greycstoration_mutex_create(greycstoration_params[0]);
    }
    if (nthreads) {  // Threaded version
#if cimg_OS==1
#ifdef _PTHREAD_H
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      for (unsigned int k=0; k<greycstoration_params->nb_threads; k++) {
        pthread_t thread;
        const int err = pthread_create(&thread, &attr, greycstoration_thread, (void*)(greycstoration_params+k));
        if (err) throw CImgException("CImg<%s>::greycstoration_run() : pthread_create returned error %d",
                                     pixel_type(), err);
      }
#endif
#elif cimg_OS==2
      for (unsigned int k=0; k<greycstoration_params->nb_threads; k++) {
        unsigned long ThreadID = 0;
        CreateThread(0,0,greycstoration_thread,(void*)(greycstoration_params+k),0,&ThreadID);
      }
#else
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Threads are not supported, please define cimg_OS first.");
#endif
    } else greycstoration_thread((void*)greycstoration_params); // Non-threaded version
  }
  return *this;
}

//! Run the non-patch version of the GREYCstoration algorithm on the instance image.
CImg& greycstoration_run(const float amplitude=50, const float sharpness=0.7f, const float anisotropy=0.3f,
                         const float alpha=0.6f, const float sigma=1.1f, const float gfact=1.0f,
                         const float dl=0.8f, const float da=30.0f,
                         const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
                         const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int nb_threads=1) {
  static const CImg<unsigned char> empty_mask;
  return greycstoration_run(empty_mask,amplitude,sharpness,anisotropy,alpha,sigma,gfact,dl,da,gauss_prec,
                            interpolation,fast_approx,tile,tile_border,nb_threads);
}

//! Run the patch-based version of the GREYCstoration algorithm on the instance image.
CImg& greycstoration_patch_run(const unsigned int patch_size=5, const float sigma_p=10, const float sigma_s=100,
                               const unsigned int lookup_size=20, const bool fast_approx=true,
                               const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int nb_threads=1) {

  static const CImg<unsigned char> empty_mask;
  if (greycstoration_is_running())
    throw CImgInstanceException("CImg<T>::greycstoration_run() : A GREYCstoration thread is already running on"
                                " the instance image (%u,%u,%u,%u,%p).",width,height,depth,dim,data);

  else {
    if (nb_threads>16) cimg::warn("CImg<%s>::greycstoration_run() : Multi-threading mode limited to 16 threads max.");
    const unsigned int
      ntile = (tile && (tile<width || tile<height || (depth>1 && tile<depth)))?tile:0,
#if cimg_OS==1 && !defined(_PTHREAD_H)
      nthreads = 0;
#else
    nthreads = ntile?cimg::min(nb_threads,16U):cimg::min(nb_threads,1U);
#endif

    CImg<T> *const temporary = ntile?new CImg<T>(*this):0;
    unsigned long *const counter = new unsigned long;
    *counter = 0;
    bool *const stop_request = new bool;
    *stop_request = false;

    for (unsigned int k=0; k<(nthreads?nthreads:1); k++) {
      greycstoration_params[k].patch_based = true;
      greycstoration_params[k].patch_size = patch_size;
      greycstoration_params[k].sigma_s = sigma_s;
      greycstoration_params[k].sigma_p = sigma_p;
      greycstoration_params[k].lookup_size = lookup_size;
      greycstoration_params[k].source = this;
      greycstoration_params[k].mask = &empty_mask;
      greycstoration_params[k].temporary = temporary;
      greycstoration_params[k].counter = counter;
      greycstoration_params[k].tile = ntile;
      greycstoration_params[k].tile_border = tile_border;
      greycstoration_params[k].thread = k;
      greycstoration_params[k].nb_threads = nthreads;
      greycstoration_params[k].fast_approx = fast_approx;
      greycstoration_params[k].is_running = true;
      greycstoration_params[k].stop_request = stop_request;
      if (k) greycstoration_params[k].mutex = greycstoration_params[0].mutex;
      else greycstoration_mutex_create(greycstoration_params[0]);
    }
    if (nthreads) {  // Threaded version
#if cimg_OS==1
#ifdef _PTHREAD_H
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      for (unsigned int k=0; k<greycstoration_params->nb_threads; k++) {
        pthread_t thread;
        const int err = pthread_create(&thread, &attr, greycstoration_thread, (void*)(greycstoration_params+k));
        if (err) throw CImgException("CImg<%s>::greycstoration_run() : pthread_create returned error %d",
                                     pixel_type(), err);
      }
#endif
#elif cimg_OS==2
      for (unsigned int k=0; k<greycstoration_params->nb_threads; k++) {
        unsigned long ThreadID = 0;
        CreateThread(0,0,greycstoration_thread,(void*)(greycstoration_params+k),0,&ThreadID);
      }
#else
      throw CImgInstanceException("CImg<T>::greycstoration_run() : Threads support have not been enabled in this version of GREYCstoration.");
#endif
    } else greycstoration_thread((void*)greycstoration_params); // Non-threaded version
  }
  return *this;
}

//------------------------------------------------------------------------------
// GREYCstoration private functions.
// Should not be used directly by the API user.
//-------------------------------------------------------------------------------

static void greycstoration_mutex_create(_greycstoration_params &p) {
  if (p.nb_threads>1) {
#if cimg_OS==1 && defined(_PTHREAD_H)
    p.mutex = new pthread_mutex_t;
    pthread_mutex_init(p.mutex,0);
#elif cimg_OS==2
    p.mutex = CreateMutex(0,FALSE,0);
#endif
  }
}

static void greycstoration_mutex_lock(_greycstoration_params &p) {
  if (p.nb_threads>1) {
#if cimg_OS==1 && defined(_PTHREAD_H)
    if (p.mutex) pthread_mutex_lock(p.mutex);
#elif cimg_OS==2
    WaitForSingleObject(p.mutex,INFINITE);
#endif
  }
}

static void greycstoration_mutex_unlock(_greycstoration_params &p) {
  if (p.nb_threads>1) {
#if cimg_OS==1 && defined(_PTHREAD_H)
    if (p.mutex) pthread_mutex_unlock(p.mutex);
#elif cimg_OS==2
    ReleaseMutex(p.mutex);
#endif
  }
}

static void greycstoration_mutex_destroy(_greycstoration_params &p) {
  if (p.nb_threads>1) {
#if cimg_OS==1 && defined(_PTHREAD_H)
    if (p.mutex) pthread_mutex_destroy(p.mutex);
#elif cimg_OS==2
    CloseHandle(p.mutex);
#endif
    p.mutex = 0;
  }
}

#if cimg_OS==1
static void* greycstoration_thread(void *arg) {
#elif cimg_OS==2
  static DWORD WINAPI greycstoration_thread(void *arg) {
#endif
    _greycstoration_params &p = *(_greycstoration_params*)arg;
    greycstoration_mutex_lock(p);
    const CImg<unsigned char> &mask = *(p.mask);
    CImg<T> &source = *(p.source);

    if (!p.tile) {

      // Non-tiled version
      //------------------
      if (p.patch_based) source.blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
      else source.blur_anisotropic(mask,p.amplitude,p.sharpness,p.anisotropy,p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,
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
              if (!p.nb_threads || ((ctile++)%p.nb_threads)==p.thread) {
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
                greycstoration_mutex_unlock(p);
                if (p.patch_based) img.blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
                else img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                          p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
                greycstoration_mutex_lock(p);
                temporary.draw_image(x,y,z,img.crop(b,b,b,img.width-b,img.height-b,img.depth-b));
              }
      } else {
        for (unsigned int y=0; y<source.height && !*(p.stop_request); y+=p.tile)
          for (unsigned int x=0; x<source.width && !*(p.stop_request); x+=p.tile)
            if (!p.nb_threads || ((ctile++)%p.nb_threads)==p.thread) {
              const unsigned int
                x1 = x+p.tile-1,
                y1 = y+p.tile-1,
                xe = x1<source.width?x1:source.width-1,
                ye = y1<source.height?y1:source.height-1;
              CImg<T> img = source.get_crop(x-b,y-b,xe+b,ye+b,true);
              CImg<unsigned char> mask_tile = mask.is_empty()?mask:mask.get_crop(x-b,y-b,xe+b,ye+b,true);
              img.greycstoration_params[0] = p;
              greycstoration_mutex_unlock(p);
              if (p.patch_based) img.blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
              else img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                        p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
              temporary.draw_image(x,y,img.crop(b,b,img.width-b,img.height-b));
              greycstoration_mutex_lock(p);
            }
      }
    }
    greycstoration_mutex_unlock(p);

    if (!p.thread) {
      if (p.nb_threads>1) {
        bool stopflag = true;
        do {
          stopflag = true;
          for (unsigned int k=1; k<p.nb_threads; k++) if (source.greycstoration_params[k].is_running) stopflag = false;
          if (!stopflag) cimg::wait(50);
        } while (!stopflag);
      }
      if (p.counter) delete p.counter;
      if (p.temporary) { source = *(p.temporary); delete p.temporary; }
      if (p.stop_request) delete p.stop_request;
      p.mask = 0;
      p.amplitude = p.sharpness = p.anisotropy = p.alpha = p.sigma = p.gfact = p.dl = p.da = p.gauss_prec = p.sigma_s = p.sigma_p = 0;
      p.patch_size = p.interpolation = p.lookup_size = 0;
      p.fast_approx = false;
      p.source = 0;
      p.temporary = 0;
      p.counter = 0;
      p.tile = p.tile_border = p.thread = p.nb_threads = 0;
      (*p.stop_request) = false;
      greycstoration_mutex_destroy(p);
    }
    p.is_running = false;

    if (p.nb_threads) {
#if cimg_OS==1 && defined(_PTHREAD_H)
      pthread_exit(arg);
      return arg;
#elif cimg_OS==2
      ExitThread(0);
#endif
    }
    return 0;
  }


#define cimg_plugin_greycstoration_count \
  if (!*(greycstoration_params->stop_request)) ++(*greycstoration_params->counter); else return *this;
#define cimg_plugin_greycstoration_lock \
  greycstoration_mutex_lock(greycstoration_params[0]);
#define cimg_plugin_greycstoration_unlock \
  greycstoration_mutex_unlock(greycstoration_params[0]);

#endif
