/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-28
 * Description : CImg threaded interface.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Some code come from the Fast Anisotropic Smoothing of 
 * Multi-valued Images, using Curvature-Preserving PDE's
 * by David Tschumperlé. 
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

 // C Ansi includes

extern "C"
{
#include <unistd.h>
}
  
// C++ includes. 
 
#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes.

#include <qobject.h>
#include <qevent.h>
#include <qfile.h>
#include <qdatetime.h> 

// KDE includes.

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes.
 
#include "cimgiface.h"

using namespace cimg_library;

namespace DigikamImagePlugins
{

CimgIface::CimgIface(uint *data, uint width, uint height, 
                     uint blurIt, double timeStep, double integralStep,
                     double angularStep, double blur, double detail,
                     double gradient, double gaussian, 
                     bool normalize, bool linearInterpolation, 
                     bool restoreMode, bool inpaintMode, bool resizeMode, 
                     char* visuflowMode, uint *newData, int newWidth, int newHeight,
                     QImage *inPaintingMask, QObject *parent)
         : QThread()
{ 
    m_imageData   = data;
    m_imageWidth  = width;
    m_imageHeight = height;    
    m_parent      = parent;
    m_cancel      = false;
        
    m_restore     = restoreMode;
    m_inpaint     = inpaintMode;
    m_resize      = resizeMode;
    m_visuflow    = visuflowMode;
    
    // Get the config data

    m_nb_iter     = blurIt;
    m_dt          = timeStep;
    m_dlength     = integralStep;
    m_dtheta      = angularStep;
    m_sigma       = blur;
    m_power1      = detail;
    m_power2      = gradient;
    m_gauss_prec  = gaussian;
    m_onormalize  = normalize;
    m_linear      = linearInterpolation;

    m_newData     = newData;
    m_newWidth    = newWidth;
    m_newHeight   = newHeight;
    
    m_tmpMaskFile = QString::null;
    
    if (m_inpaint && inPaintingMask)
       {
       KStandardDirs dir;
       m_tmpMaskFile = dir.saveLocation("tmp");
       m_tmpMaskFile.append(QString::number(getpid()));
       m_tmpMaskFile.append(".png");
       m_inPaintingMask = inPaintingMask->copy();
       m_inPaintingMask.save(m_tmpMaskFile, "PNG");
       kdDebug() << "CimgIface::InPainting Mask : " << m_tmpMaskFile << endl;
       }
        
    if (m_imageData && m_imageWidth && m_imageHeight)
       {
       if (m_parent)
          start();             // m_parent is valide, start thread ==> run()
       else
          startComputation();  // no parent : no using thread.
       }
    else  // No image data 
       {
       if (m_parent)           // If parent then send event about a problem.
          {
          CimgIface::EventData *d = new CimgIface::EventData;
          d->starting = false;
          d->success = false;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }
       }
}

CimgIface::~CimgIface()
{ 
    stopComputation();
    
    if (m_tmpMaskFile != QString::null)
       {
       // Remove temporary inpainting mask.
       QFile maskFile(m_tmpMaskFile);
       maskFile.remove();
       }
}

void CimgIface::stopComputation(void)
{
    m_cancel = true;
    wait();
    cleanup();
    img   = CImg<>();    
    eigen = CImgl<>(CImg<>(), CImg<>());
}

// List of threaded operations.

void CimgIface::run()
{
    startComputation();
}

void CimgIface::startComputation()
{
    CimgIface::EventData *d;
    
    QDateTime startDate = QDateTime::currentDateTime();
    kdDebug() << "CimgIface::Initialization..." << endl;
    
    if (m_parent)
       {
       d = new CimgIface::EventData;
       d->starting = true;
       d->success  = false;
       d->progress = 0;
       QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
       }
        
    // Copy the src data into a CImg type image with three channels and no alpha. This means that a CImg is always rgba.

    img = CImg<>(m_imageWidth, m_imageHeight, 1, 3);
    eigen = CImgl<>(CImg<>(2,1), CImg<>(2,2));    
    register int x, y, i=0;
    uchar* data = (uchar *)m_imageData;

    for (y = 0; y < m_imageHeight; y++) 
       {
       for (x = 0; x < m_imageWidth; x++, i+=4) 
          {
          img(x, y, 0) = data[ i ];
          img(x, y, 1) = data[i+1];
          img(x, y, 2) = data[i+2];
          }
       }

    kdDebug() << "CimgIface::Process Computation..." << endl;
              
    if (!process()) 
       {
       kdDebug() << "CimgIface::Error during CImg filter computation!" << endl;
       // Everything went wrong.
       
       if (m_parent)
          {
          CimgIface::EventData *d = new CimgIface::EventData;
          d->starting = false;
          d->success  = false;
          d->progress = 0;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }    
          
       return;
       }
    
    // Copy CImg onto destination.
    
    kdDebug() << "CimgIface::Finalization..." << endl;

    i = 0;
    int width, height;
    uchar* newData;
    
    if (m_resize)
       {
       width   = m_newWidth;
       height  = m_newHeight;
       newData = (uchar*)m_newData;
       }
    else
       {
       width   = m_imageWidth;
       height  = m_imageHeight;
       newData = (uchar*)m_imageData;
       }   
       
    for (y = 0; y < height; y++) 
       {
       for (x = 0; x < width; x++, i+=4) 
          {
          newData[ i ] = (uchar)img(x, y, 0);
          newData[i+1] = (uchar)img(x, y, 1);
          newData[i+2] = (uchar)img(x, y, 2);
          }
       }
    
    QDateTime endDate = QDateTime::currentDateTime();
    
    if (!m_cancel)
       {
       if (m_parent)
          {
          d = new CimgIface::EventData;
          d->starting = false;
          d->success  = true;
          d->progress = 0;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }
          
       kdDebug() << "CimgIface::End of computation !!! ... ( " << startDate.secsTo(endDate) << " s )" << endl;
       }
    else
       {
       if (m_parent)
          {
          d = new CimgIface::EventData;
          d->starting = false;
          d->success  = false;
          d->progress = 0;
          QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
          }
          
       kdDebug() << "CimgIface::Computation aborted... ( " << startDate.secsTo(endDate) << " s )" << endl;
       }
}

bool CimgIface::process()
{
    if (!prepare()) return false;

    // Begin regularization PDE iterations
        
    int counter = 0;
     
    for (unsigned int iter = 0; !m_cancel && (iter < m_nb_iter); iter++)
        {
        // Compute smoothed structure tensor field G
        compute_smoothed_tensor();

        // Compute normalized tensor field sqrt(T) in G
        compute_normalized_tensor();

        // Compute LIC's along different angle projections a_\alpha
        compute_LIC(counter);

        // Average all the LIC's
        compute_average_LIC();

        // Next step
        img = dest;
        }
    
    // Save result and end program
        
    if (!m_cancel && m_visuflow) dest.mul(flow.get_norm_pointwise()).normalize(0, 255);
    if (!m_cancel && m_onormalize) dest.normalize(0, 255);
        
    cleanup();
    
    if (m_cancel) 
      {
      kdDebug() << "CImg filter arborted!" << endl;
      return false;    
      }

    return true;
}

void CimgIface::cleanup()
{
    img0 = flow = G = dest = sum = W = CImg<>();    
    mask = CImg<uchar> ();
}

bool CimgIface::prepare()
{
    if (!m_restore && !m_inpaint && !m_resize && !m_visuflow) 
       {
       kdDebug() << "Unspecified CImg filter computation Mode!" << endl;
       return false;
       }

    // Init algorithm parameters
    
    if (m_restore)  if (!prepare_restore())  return false;
    if (m_inpaint)  if (!prepare_inpaint())  return false;
    if (m_resize)   if (!prepare_resize())   return false;
    if (m_visuflow) if (!prepare_visuflow()) return false;

    if (!check_args()) return false;

    // Init images
    
    dest = CImg<>(img.width,img.height,1,img.dim);
    sum  = CImg<>(img.width,img.height,1);
    W    = CImg<>(img.width,img.height,1,2);
    
    return true;
}

bool CimgIface::check_args()
{
    if (m_power2 < m_power1)
       {
       kdDebug() << "Error : p2<p1 !" << endl;
       return false;
       }
       
    return true;
}

bool CimgIface::prepare_restore()
{
    CImgStats stats(img, false);
    img.normalize((float)stats.min, (float)stats.max);
    img0 = img;
    G = CImg<>(img.width, img.height, 1, 3);
    return true;
}

bool CimgIface::prepare_resize()
{
    if (!m_newWidth && !m_newHeight) 
       {
       kdDebug() << "Unspecified output geometry !" << endl;
       return false;
       }

    mask = CImg<uchar>(img.dimx(), img.dimy(), 1, 1, 255);
    mask.resize(m_newWidth, m_newHeight, 1, 1, 1);
    img0 = img.get_resize(m_newWidth, m_newHeight, 1, -100, 1);
    img.resize(m_newWidth, m_newHeight, 1, -100, 3);
    G = CImg<>(img.width, img.height, 1, 3);
    
    return true;
}

bool CimgIface::prepare_inpaint()
{
    const char *file_m = m_tmpMaskFile.latin1();  // Input inpainting mask.
    
    if (!file_m) 
       {
       kdDebug() << "Unspecified CImg inpainting mask !" << endl;
       return false;
       }

    const unsigned int dilate  = 0; // Inpainting mask dilatation.
    const unsigned int ip_init = 3; // Inpainting init (0=black, 1=white, 2=noise, 3=unchanged, 4=interpol).
    
    if (cimg::strncasecmp("block",file_m,5)) 
        mask = CImg<uchar>(file_m);
    else 
       {
       int l = 16;
       std::sscanf(file_m,"block%d",&l);
       mask = CImg<uchar>(img.width/l,img.height/l);
       cimg_mapXY(mask,x,y) mask(x,y)=(x+y)%2;
       }
       
    mask.resize(img.width,img.height,1,1);
    
    if (dilate) mask.dilate(dilate);
    
    switch (ip_init) 
    {
    case 0 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 0; } break;
    case 1 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = 255; } break;
    case 2 : { cimg_mapXYV(img,x,y,k) if (mask(x,y)) img(x,y,k) = (float)(255*cimg::rand()); } break;
    case 3 : break;
    case 4 : 
        {
        CImg<uchar> tmask(mask),ntmask(tmask);
        CImg_3x3(M,uchar);
        CImg_3x3(I,float);
        while (!m_cancel && CImgStats(ntmask,false).max>0) 
        {
            cimg_map3x3(tmask,x,y,0,0,M) if (Mcc && (!Mpc || !Mnc || !Mcp || !Mcn)) 
            {
                const float ccp = Mcp?0.0f:1.0f, cpc = Mpc?0.0f:1.0f,
                    cnc = Mnc?0.0f:1.0f, ccn = Mcn?0.0f:1.0f, csum = ccp + cpc + cnc + ccn;
                cimg_mapV(img,k) 
                {
                    cimg_get3x3(img,x,y,0,k,I);
                    img(x,y,k) = (ccp*Icp + cpc*Ipc + cnc*Inc + ccn*Icn)/csum;
                }
                ntmask(x,y) = 0;
            }
            tmask = ntmask;
        }
        } break;    
    
    default: break;
    }
    
    img0=img;
    G = CImg<>(img.width,img.height,1,3,0);
    CImg_3x3(g,uchar);
    CImg_3x3(I,float);
    cimg_map3x3(mask,x,y,0,0,g) if (!gcc && !(gnc-gcc) && !(gcc-gpc) && !(gcn-gcc) && !(gcc-gcp)) cimg_mapV(img,k) 
        {
        cimg_get3x3(img,x,y,0,k,I);
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;    
        }
    G.blur(m_sigma);
    { cimg_mapXY(G,x,y) 
        {
            G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
            const float
                l1 = eigen(0)[0],
                l2 = eigen(0)[1],
                u  = eigen(1)[0],
                v  = eigen(1)[1],      
                ng = (float)std::sqrt(l1+l2),
                n1 = (float)(1.0/std::pow(1+ng,m_power1)),
                n2 = (float)(1.0/std::pow(1+ng,m_power2)),
               sr1 = (float)std::sqrt(n1),
               sr2 = (float)std::sqrt(n2);
          G(x,y,0) = sr1*u*u + sr2*v*v;
          G(x,y,1) = u*v*(sr1-sr2);
          G(x,y,2) = sr1*v*v + sr2*u*u;
        }    
    }
    return true;
}

/*
void CimgIface::get_geom(const char *geom, int &geom_w, int &geom_h) 
{
    char tmp[16];
    std::sscanf(geom,"%d%7[^0-9]%d%7[^0-9]",&geom_w,tmp,&geom_h,tmp+1);
    if (tmp[0]=='%') geom_w=-geom_w;
    if (tmp[1]=='%') geom_h=-geom_h;
}
*/

bool CimgIface::prepare_visuflow()
{
    //const char *geom     = "100%x100%"; //cimg_option("-g","100%x100%","Output geometry");
    //const char *file_i   = (const char *)NULL; //cimg_option("-i",(const char*)NULL,"Input init image");
    const bool normalize = false; //cimg_option("-norm",false,"Normalize input flow");

    int w=0, h=0; /*get_geom(geom,w,h);*/
    
    if (!cimg::strcasecmp(m_visuflow,"circle")) { // Create a circular vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = -(float)std::sin(ang);
            flow(x,y,1) = (float)std::cos(ang);
        }
    }
    
    if (!cimg::strcasecmp(m_visuflow,"radial")) { // Create a radial vector flow
        flow = CImg<>(400,400,1,2);
        cimg_mapXY(flow,x,y) {
            const float ang = (float)(std::atan2(y-0.5*flow.dimy(),x-0.5*flow.dimx()));
            flow(x,y,0) = (float)std::cos(ang);
            flow(x,y,1) = (float)std::sin(ang);
        }
    }
    
    if (!flow.data) flow = CImg<>(m_visuflow);
    
    flow.resize(w,h,1,2,3);
    
    if (normalize) flow.orientation_pointwise();
    /*    if (file_i) img = CImg<>(file_i);
          else img = CImg<>(flow.width,flow.height,1,1,0).noise(100,2); */
    img0=img;
    img0.fill(0);
    float color[3]={255,255,255};
    img0.draw_quiver(flow,color,15,-10);
    G = CImg<>(img.width,img.height,1,3);
    
    return true;
}

inline void CimgIface::compute_smoothed_tensor()
{
    if (m_visuflow || m_inpaint) return;
    
    CImg_3x3(I,float);
    G.fill(0);
    cimg_mapV(img,k) cimg_map3x3(img,x,y,0,k,I) 
        {
        const float ix = 0.5f*(Inc-Ipc), iy = 0.5f*(Icn-Icp);
        G(x,y,0)+= ix*ix; G(x,y,1)+= ix*iy; G(x,y,2)+= iy*iy;    
        }
    G.blur(m_sigma);
}

inline void CimgIface::compute_normalized_tensor()
{
    if (m_restore || m_resize) cimg_mapXY(G,x,y) 
        {
        G.get_tensor(x,y).symeigen(eigen(0),eigen(1));
        const float
            l1 = eigen(0)[0],
            l2 = eigen(0)[1],
            u = eigen(1)[0],
            v = eigen(1)[1],      
            n1 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*m_power1)),
            n2 = (float)(1.0/std::pow(1.0f+l1+l2,0.5f*m_power2));
        G(x,y,0) = n1*u*u + n2*v*v;
        G(x,y,1) = u*v*(n1-n2);
        G(x,y,2) = n1*v*v + n2*u*u;
        }    
    
    if (m_visuflow) cimg_mapXY(G,x,y) 
        {
        const float 
            u = flow(x,y,0),
            v = flow(x,y,1),
            n = (float)std::pow(u*u+v*v,0.25f),
            nn = n<1e-5?1:nn;
        G(x,y,0) = u*u/nn;
        G(x,y,1) = u*v/nn;
        G(x,y,2) = v*v/nn;
        }

    const CImgStats stats(G,false);
    G /= cimg::max(std::fabs(stats.max), std::fabs(stats.min));
}

inline void CimgIface::compute_W(float cost, float sint)
{
    cimg_mapXY(W,x,y) 
        {
        const float 
            a = G(x,y,0),
            b = G(x,y,1),
            c = G(x,y,2),
            u = a*cost + b*sint,
            v = b*cost + c*sint;
        W(x,y,0) = u;
        W(x,y,1) = v;
        }
}

inline void CimgIface::compute_LIC_back_forward(int x, int y)
{
    float l, X,Y, cu, cv, lsum=0;
    const float fsigma2 = 2*m_dt*(W(x,y,0)*W(x,y,0) + W(x,y,1)*W(x,y,1));
    const float length = m_gauss_prec*(float)std::sqrt(fsigma2);

    if (m_linear) 
        {
        // Integrate with linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y;
        
        for (l=0; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=m_dlength) 
            {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X+=m_dlength*u; Y+=m_dlength*v; cu=u; cv=v; lsum+=coef;
            }
            
        cu = W(x,y,0); cv = W(x,y,1); X=x-m_dlength*cu; Y=y-m_dlength*cv;
        
        for (l=m_dlength; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=m_dlength) 
            {
            float u = (float)(W.linear_pix2d(X,Y,0)), v = (float)(W.linear_pix2d(X,Y,1));
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=(float)(coef*img.linear_pix2d(X,Y,k));
            X-=m_dlength*u; Y-=m_dlength*v; cu=u; cv=v; lsum+=coef;
            }
        } 
    else 
        {
        // Integrate with non linear interpolation
        cu = W(x,y,0); cv = W(x,y,1); X=(float)x; Y=(float)y; 
        
        for (l=0; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=m_dlength) 
            {
            const int xi = (int)(X+0.5f), yi = (int)(Y+0.5f);
            float u = W(xi,yi,0), v = W(xi,yi,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=coef*img(xi,yi,k);
            X+=m_dlength*u; Y+=m_dlength*v; cu=u; cv=v; lsum+=coef;
            }
                
        cu = W(x,y,0); cv = W(x,y,1); X=x-m_dlength*cu; Y=y-m_dlength*cv;
        
        for (l = m_dlength; !m_cancel && l<length && X>=0 && Y>=0 && X<=W.dimx()-1 && Y<=W.dimy()-1; l+=m_dlength) 
            {
            const int xi = (int)(X+0.5f), yi = (int)(Y+0.5f);
            float u = W(xi,yi,0), v = W(xi,yi,1);
            const float coef = (float)std::exp(-l*l/fsigma2);
            if ((cu*u+cv*v)<0) { u=-u; v=-v; }
            cimg_mapV(dest,k) dest(x,y,k)+=coef*img(xi,yi,k);
            X-=m_dlength*u; Y-=m_dlength*v; cu=u; cv=v; lsum+=coef;
            }
    }
    sum(x,y)+=lsum;
}

inline void CimgIface::compute_LIC(int &counter)
{
    dest.fill(0);
    sum.fill(0);
    
    for (float theta = (180%(int)m_dtheta)/2.0f; !m_cancel && (theta < 180); theta += m_dtheta) 
        {
        const float rad = (float)(theta*cimg::PI/180.0);
        const float cost = (float)std::cos(rad);
        const float sint = (float)std::sin(rad);

        // Compute vector field w = sqrt(T)*a_alpha
        compute_W(cost, sint);

        // Compute the LIC along w in backward and forward directions
        cimg_mapXY(dest,x,y) 
           {
           counter++;

           if (m_parent && !m_cancel)
              {
              // Update the progress bar in dialog.
              
              double progress = counter;
              progress /= (double)dest.width * dest.height * m_nb_iter * (180 / m_dtheta);
              CimgIface::EventData *d = new CimgIface::EventData;
              d->starting = true;
              d->success = false;
              d->progress = (int)(100*progress);
              QApplication::postEvent(m_parent, new QCustomEvent(QEvent::User, d));
              }
        
           if (!mask.data || mask(x,y)) compute_LIC_back_forward(x,y);
           }
        }
}

inline void CimgIface::compute_average_LIC()
{
    cimg_mapXY(dest,x,y) 
    {
        if (sum(x,y)>0) 
            cimg_mapV(dest,k) dest(x,y,k) /= sum(x,y); 
        else 
            cimg_mapV(dest,k) dest(x,y,k) = img(x,y,k);
    }
}

}  // NameSpace DigikamImagePlugins

