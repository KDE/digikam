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

// NOTE: You need to include
// #include <QMutex>
// #include <QMutexLocker>
//
// #include "dynamicthread.h"

class GreycstorationParameters {
public:

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
  unsigned int tile;
  unsigned int tile_border;
  bool fast_approx;

  // Default constructor
  GreycstorationParameters()
    : patch_based(false),
      amplitude(0),
      sharpness(0),
      anisotropy(0),
      alpha(0),
      sigma(0),
      gfact(1),
      dl(0),
      da(0),
      gauss_prec(0),
      interpolation(0),
      patch_size(0),
      sigma_s(0),
      sigma_p(0),
      lookup_size(0),
      tile(0),
      tile_border(0),
      fast_approx(false) {}
};

class GreycstorationThreadManager;
// class just to have a constructor to initialize the variable
class GreycstorationThreadManagerContainer
{
public:
    GreycstorationThreadManagerContainer() : threadManager(0) {}
    GreycstorationThreadManager* threadManager;
};
GreycstorationThreadManagerContainer threadManagerContainer;

void setThreadManager(GreycstorationThreadManager* threadManager)
{
    threadManagerContainer.threadManager = threadManager;
}

void resetThreadManager()
{
    setThreadManager(0);
}

class GreycstorationWorkingThread : public Digikam::DynamicThread
{
public:
    GreycstorationWorkingThread(GreycstorationThreadManager* manager, int threadNumber)
        : manager(manager), threadNumber(threadNumber)
    {
    }

    virtual void run()
    {
        manager->workerMethod(threadNumber);
    }

    GreycstorationThreadManager* const manager;
    const int threadNumber;
};

//template <typename T>
class GreycstorationThreadManager
{
public:
    GreycstorationThreadManager()
        : counter(0), stopRequest(false), activeThreads(0),
          source(0), temporary(0), empty_mask(new CImg<unsigned char>())
    {
    }

    ~GreycstorationThreadManager()
    {
        stop();
        wait();
        finish();
        qDeleteAll(threads);
        delete temporary;
        delete empty_mask;
    }


    //! Run the non-patch version of the GREYCstoration algorithm on the instance image, using a mask.
    void start(CImg& s, const CImg<unsigned char>* m,
               const float amplitude=60, const float sharpness=0.7f,
               const float anisotropy=0.3f, const float alpha=0.6f,
               const float sigma=1.1f, const float gfact=1.0f,
               const float dl=0.8f, const float da=30.0f,
               const float gauss_prec=2.0f, const unsigned int interpolation=0,
               const bool fast_approx=true, const unsigned int tile=0,
               const unsigned int tile_border=0,
               const unsigned int nb_threads=1)
    {

        GreycstorationParameters params;
        params.patch_based = false;

        params.amplitude = amplitude;
        params.sharpness = sharpness;
        params.anisotropy = anisotropy;
        params.alpha = alpha;
        params.sigma = sigma;
        params.gfact = gfact;
        params.dl = dl;
        params.da = da;
        params.gauss_prec = gauss_prec;
        params.interpolation = interpolation;
        params.fast_approx = fast_approx;
        params.tile_border = tile_border;

        mask = m;
        start(&s, params, tile, nb_threads);
    }

    //! Run the non-patch version of the GREYCstoration algorithm on the instance image.
    void start(CImg& s, const float amplitude=50, const float sharpness=0.7f, const float anisotropy=0.3f,
               const float alpha=0.6f, const float sigma=1.1f, const float gfact=1.0f,
               const float dl=0.8f, const float da=30.0f,
               const float gauss_prec=2.0f, const unsigned int interpolation=0, const bool fast_approx=true,
               const unsigned int tile=0, const unsigned int tile_border=0, const unsigned int nb_threads=1)
    {
        return start(s, empty_mask, amplitude,sharpness,anisotropy,alpha,sigma,gfact,dl,da,gauss_prec,
                    interpolation,fast_approx,tile,tile_border,nb_threads);
    }

    //! Run the patch-based version of the GREYCstoration algorithm on the instance image.
    void start(CImg& s, const unsigned int patch_size=5,
               const float sigma_p=10,
               const float sigma_s=100,
               const unsigned int lookup_size=20,
               const bool fast_approx=true,
               const unsigned int tile=0,
               const unsigned int tile_border=0,
               const unsigned int nb_threads=1)
    {
        GreycstorationParameters params;

        params.patch_based = true;
        params.patch_size = patch_size;
        params.sigma_s = sigma_s;
        params.sigma_p = sigma_p;
        params.lookup_size = lookup_size;
        params.mask = empty_mask;
        params.tile_border = tile_border;
        params.fast_approx = fast_approx;

        start(&s, params, tile, nb_threads);
    }

    bool isRunning() const
    {
        foreach(GreycstorationWorkingThread* thread, threads)
        {
            if (thread->isRunning())
            {
                return true;
            }
        }
        return false;
    }

    void wait()
    {
        foreach(GreycstorationWorkingThread* thread, threads)
        {
            thread->wait();
        }
    }

    void finish()
    {
        if (temporary)
        {
            (*source) = (*temporary);
        }
        if (source)
        {
            source->resetThreadManager();
        }
    }

    void stop()
    {
        foreach(GreycstorationWorkingThread* thread, threads)
        {
            thread->stop();
        }
        stopRequest = true;
    }

    float progress() const
    {
        const float
        da = p.da,
        factor = p.patch_based ? 1 : (1+360/da);
        float maxcounter = 0;
        if (p.tile==0)
        {
            maxcounter = source->width*source->height*source->depth*factor;
        }
        else
        {
            const unsigned int
            t = p.tile,
            b = p.tile_border,
            n = (1+(source->width-1)/t)*(1+(source->height-1)/t)*(1+(source->depth-1)/t);
            maxcounter = (source->width*source->height*source->depth + n*4*b*(b + t))*factor;
        }
        return cimg::min(counter*99.9f/maxcounter,99.9f);
    }

    // Waits at most the specified number of milliseconds, then returns current progress
    float waitABit(unsigned int msecs)
    {
        QMutexLocker lock(&mutex);
        if (activeThreads)
        {
            condVar.wait(&mutex, msecs);
        }
        return progress();
    }

protected:

    friend struct CImg<T>;

    QMutex         mutex;
    QWaitCondition condVar;
    volatile int   counter;
    volatile bool  stopRequest;
    volatile int   activeThreads;

    GreycstorationParameters p;

    CImg<T>*                   source;
    const CImg<unsigned char> *mask;
    CImg<T>                   *temporary;

    CImg<unsigned char> *empty_mask;

    QList<GreycstorationWorkingThread*> threads;

    void start(CImg* s, const GreycstorationParameters& params, unsigned int tile, unsigned int numberOfThreads)
    {
        /*if (!mask.is_empty() && !mask.is_sameXY(*this))
        {
            throw CImgArgumentException("CImg<%s>::greycstoration_run() : Given mask (%u,%u,%u,%u,%p) and instance image "
                                        "(%u,%u,%u,%u,%p) have different dimensions.",
                                        pixel_type(),mask.width,mask.height,mask.depth,mask.dim,mask.data,width,height,depth,dim,data);
        }*/

        if (numberOfThreads>16)
        {
            cimg::warn("CImg<%s>::greycstoration_run() : Multi-threading mode limited to 16 threads max.");
        }

        source = s;
        s->setThreadManager(this);
        p = params;

        p.tile = (tile && (tile<source->width || tile<source->height ||
                        (source->depth>1 && tile<source->depth)))?tile:0,
        numberOfThreads = p.tile ? cimg::min(numberOfThreads,16U) : cimg::min(numberOfThreads,1U);

        counter = 0;
        stopRequest = false;
        activeThreads = 0;
        delete temporary;
        temporary = p.tile ? new CImg<T>(*source) : 0;

        if (numberOfThreads)
        {
            for (int k=0; k<(int)numberOfThreads; k++)
            {
                if (threads.size() == k) {
                    GreycstorationWorkingThread* thread = new GreycstorationWorkingThread(this, k);
                    ++activeThreads;
                    thread->start();
                    threads << thread;
                } else {
                    threads[k]->start();
                }
            }
        }
        else
        {
            // direct execution
            ++activeThreads;
            workerMethod(0);
        }
    }

    friend class GreycstorationWorkingThread;
    void workerMethod(unsigned int threadIndex)
    {
        QMutexLocker lock(&mutex);

        if (!p.tile)
        {
            // Non-tiled version
            //------------------
            if (p.patch_based)
            {
                source->blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
            }
            else
            {
                source->blur_anisotropic(*mask,p.amplitude,p.sharpness,p.anisotropy,p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,
                                        p.interpolation,p.fast_approx,p.gfact);
            }
        }
        else
        {
            // Tiled version
            //---------------
            const bool threed = (source->depth>1);
            const unsigned int b = p.tile_border;
            unsigned int ctile = 0;

            if (threed)
            {
                for (unsigned int z=0; z<source->depth && !stopRequest; z+=p.tile)
                {
                    for (unsigned int y=0; y<source->height && !stopRequest; y+=p.tile)
                    {
                        for (unsigned int x=0; x<source->width && !stopRequest; x+=p.tile)
                        {
                            if (threads.isEmpty() || ((ctile++)%threads.size())==(unsigned int)threadIndex)
                            {
                                const unsigned int
                                x1 = x+p.tile-1,
                                y1 = y+p.tile-1,
                                z1 = z+p.tile-1,
                                xe = x1<source->width?x1:source->width-1,
                                ye = y1<source->height?y1:source->height-1,
                                ze = z1<source->depth?z1:source->depth-1;

                                CImg<T> img = source->get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
                                CImg<unsigned char> mask_tile = mask->is_empty() ? *mask :
                                                                mask->get_crop(x-b,y-b,z-b,xe+b,ye+b,ze+b,true);
                                //??img.greycstoration_params[0] = p;

                                lock.unlock();
                                if (p.patch_based)
                                {
                                    img.blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
                                }
                                else
                                {
                                    img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                                         p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
                                }
                                lock.relock();

                                temporary->draw_image(x,y,z,img.crop(b,b,b,img.width-b,img.height-b,img.depth-b));
                            }
                        }
                    }
                }
            }
            else
            {
                for (unsigned int y=0; y<source->height && !stopRequest; y+=p.tile)
                {
                    for (unsigned int x=0; x<source->width && !stopRequest; x+=p.tile)
                    {
                        if (threads.isEmpty() || ((ctile++)%threads.size())==(unsigned int)threadIndex)
                        {
                            const unsigned int
                            x1 = x+p.tile-1,
                            y1 = y+p.tile-1,
                            xe = x1<source->width?x1:source->width-1,
                            ye = y1<source->height?y1:source->height-1;
                            CImg<T> img = source->get_crop(x-b,y-b,xe+b,ye+b,true);
                            CImg<unsigned char> mask_tile = mask->is_empty() ? *mask : mask->get_crop(x-b,y-b,xe+b,ye+b,true);
                            //img.greycstoration_params[0] = p;

                            lock.unlock();
                            if (p.patch_based)
                            {
                                img.blur_patch(p.patch_size,p.sigma_p,p.sigma_s,p.lookup_size,p.fast_approx);
                            }
                            else
                            {
                                img.blur_anisotropic(mask_tile,p.amplitude,p.sharpness,p.anisotropy,
                                                     p.alpha,p.sigma,p.dl,p.da,p.gauss_prec,p.interpolation,p.fast_approx,p.gfact);
                            }
                            lock.relock();

                            temporary->draw_image(x,y,img.crop(b,b,img.width-b,img.height-b));
                        }
                    }
                }
            }
        }
        --activeThreads;
        condVar.wakeAll();
    }

};

#define cimg_plugin_greycstoration_count \
  if (threadManagerContainer.threadManager)\
  { if (threadManagerContainer.threadManager->stopRequest) \
        return *this; \
    ++threadManagerContainer.threadManager->counter; }

#define cimg_plugin_greycstoration_lock \
  if (threadManagerContainer.threadManager) threadManagerContainer.threadManager->mutex.lock();
#define cimg_plugin_greycstoration_unlock \
  if (threadManagerContainer.threadManager) threadManagerContainer.threadManager->mutex.unlock();

#endif
