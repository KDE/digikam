/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   ImageConverter.cpp
 * @author Sebastien Fourey
 * @date   Jul 2010
 * @brief  Definition of the methods of the class ImageConverter
 * 
 * This file is part of the ZArt software's source code.
 * 
 * Copyright Sebastien Fourey / GREYC Ensicaen (2010-...) 
 * 
 *                    https://foureys.users.greyc.fr/
 * 
 * This software is a computer program whose purpose is to demonstrate
 * the possibilities of the GMIC image processing language by offering the
 * choice of several manipulations on a video stream aquired from a webcam. In
 * other words, ZArt is a GUI for G'MIC real-time manipulations on the output
 * of a webcam.
 * 
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". See also the directory "Licence" which comes
 * with this source code for the full text of the CeCILL licence. 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */
#include <iostream>
#include <QPainter>
#include <QImage>
#include <QMutex>
#include "ImageConverter.h"
#include <cassert>
#include "Common.h"

IplImage * ImageConverter::_image = 0;

void ImageConverter::convert( const IplImage * in, QImage * out )
{
   assert(in->depth== IPL_DEPTH_8U);
   assert(in->nChannels == 3);
   const unsigned int w3 = 3 * in->width;
   const unsigned int qiOffset = (w3%4)?(4-(w3%4)):0;
   const unsigned int iplOffset = in->widthStep - w3;
   unsigned char * src = reinterpret_cast<unsigned char *>( in->imageData );
   unsigned char * dst = reinterpret_cast<unsigned char *>( out->scanLine( 0 ) );
   unsigned char * endSrc;
   unsigned int line = in->height;
   while ( line-- ) {
      endSrc = src + w3;
      while ( src != endSrc ) {
         *dst++ = src[2];
         *dst++ = src[1];
         *dst++ = src[0];
         src += 3;
      }
      dst += qiOffset;
      src += iplOffset;
   }
   return;
}

void ImageConverter::convert( const cimg_library::CImg<float> & in, QImage * out )
{
   const int spectrum = in.spectrum();
   unsigned char * dst = out->scanLine(0);
   const float *srcR = in.data( 0,0,0,0 );
   const float * endSrcR = srcR;
   const float *srcG = in.data( 0,0,0,(spectrum>=2)?1:0 );
   const float *srcB = in.data( 0,0,0,(spectrum>=3)?2:0 );
   unsigned int height = out->height();
   const unsigned int width = out->width();
   const unsigned int offset = ((width*3)%4)?(4-((width*3)%4)):0;
   while ( height-- ) {
      endSrcR += width;
      while ( srcR != endSrcR ) {
         *dst++ = static_cast<unsigned char>( *srcR++ );
         *dst++ = static_cast<unsigned char>( *srcG++ );
         *dst++ = static_cast<unsigned char>( *srcB++ );
      }
      dst += offset;
   }
}

void ImageConverter::merge( IplImage * iplImage,
			    const cimg_library::CImg<float> & cimgImage,
			    QImage * out,
			    QMutex * imageMutex,
			    MergeDirection direction )
{
   IplImage * cameraImage = iplImage;
   if ( iplImage->width != cimgImage.width() || iplImage->height!= cimgImage.height() ) {
      if ( _image )
         cvReleaseImage(&_image);
      _image = cvCreateImage(cvSize(cimgImage.width(),cimgImage.height()),IPL_DEPTH_8U,3);
      cvResize(iplImage,_image,CV_INTER_LINEAR);
      cameraImage = _image;
      imageMutex->lock();
      QSize size( cimgImage.width(), cimgImage.height());
      if ( out->size() != size ) {
         *out = QImage( size, QImage::Format_RGB888 );
      }
      imageMutex->unlock();
   }

   switch (direction) {
   case MergeTop:
      mergeTop( cameraImage, cimgImage, out );
      break;
   case MergeLeft:
      mergeLeft( cameraImage, cimgImage, out );
      break;
   case MergeBottom:
      mergeBottom( cameraImage, cimgImage, out );
      break;
   case MergeRight:
      mergeRight( cameraImage, cimgImage, out );
      break;
   }
}

void ImageConverter::mergeTop( IplImage * iplImage,
                              const cimg_library::CImg<float> & cimgImage,
                              QImage * out )
{
   const int height = iplImage->height;
   const int width = iplImage->width;
   const int spectrum = cimgImage.spectrum();
   const float *srcR = cimgImage.data( 0,0,0,0 );
   const float *srcG = cimgImage.data( 0,0,0,(spectrum>=2)?1:0 );
   const float *srcB = cimgImage.data( 0,0,0,(spectrum>=3)?2:0 );
   unsigned char * dst = out->scanLine(0);
   unsigned char * endDst;
   const unsigned int qiOffset = ((width*3)%4)?(4-((width*3)%4)):0;
   const unsigned int iplOffset = iplImage->widthStep - 3 * width;

   // Copy from cimgImage
   unsigned int lines = height/2;
   while ( lines-- ) {
      endDst= dst + 3 * width;
      while ( dst != endDst ) {
         *dst++ = static_cast<unsigned char>( *srcR++ );
         *dst++ = static_cast<unsigned char>( *srcG++ );
         *dst++ = static_cast<unsigned char>( *srcB++ );
      }
      dst += qiOffset;
   }

   // Copy from iplImage
   unsigned char * srcIpl = reinterpret_cast<unsigned char *>( iplImage->imageData );
   srcIpl +=  (height/2) * iplImage->widthStep;
   lines = height - height/2;
   while ( lines-- ) {
      endDst= dst + 3 * width;
      while ( dst != endDst ) {
         *dst++ = srcIpl[2];
         *dst++ = srcIpl[1];
         *dst++ = srcIpl[0];
         srcIpl += 3;
      }
      dst += qiOffset;
      srcIpl += iplOffset;
   }
}

void ImageConverter::mergeLeft( IplImage * iplImage,
                              const cimg_library::CImg<float> & cimgImage,
                              QImage * out )
{
   const int width = iplImage->width;
   const int spectrum = cimgImage.spectrum();
   const float *srcR = cimgImage.data( 0,0,0,0 );
   const float *srcG = cimgImage.data( 0,0,0,(spectrum>=2)?1:0 );
   const float *srcB = cimgImage.data( 0,0,0,(spectrum>=3)?2:0 );
   unsigned char * srcIpl = reinterpret_cast<unsigned char *>( iplImage->imageData );
   unsigned char * dst = out->scanLine(0);
   unsigned char * endDst;
   const unsigned int qiOffset = ((width*3)%4)?(4-((width*3)%4)):0;
   const unsigned int iplOffset = iplImage->widthStep - 3 * width;

   const unsigned int firstHalf = width/2;
   const unsigned int secondHalf = width - width/2;
   int height = iplImage->height;
   while ( height-- ) {
      endDst= dst + 3 * firstHalf;
      while ( dst != endDst ) {
         *dst++ = static_cast<unsigned char>( *srcR++ );
         *dst++ = static_cast<unsigned char>( *srcG++ );
         *dst++ = static_cast<unsigned char>( *srcB++ );
      }
      srcR += secondHalf;
      srcG += secondHalf;
      srcB += secondHalf;

      srcIpl += 3 * firstHalf;
      endDst= dst + 3 * secondHalf;
      while ( dst != endDst ) {
         *dst++ = srcIpl[2];
         *dst++ = srcIpl[1];
         *dst++ = srcIpl[0];
         srcIpl += 3;
      }
      srcIpl += iplOffset;
      dst += qiOffset;
   }
}

void ImageConverter::mergeBottom( IplImage * iplImage,
                                 const cimg_library::CImg<float> & cimgImage,
                                 QImage * out )
{
   const int height = iplImage->height;
   const int width = iplImage->width;
   unsigned char * dst = out->scanLine(0);
   unsigned char * endDst;
   const unsigned int qiOffset = ((width*3)%4)?(4-((width*3)%4)):0;
   const unsigned int iplOffset = iplImage->widthStep - 3 * width;


   // Copy from iplImage
   unsigned char * srcIpl = reinterpret_cast<unsigned char *>( iplImage->imageData );
   unsigned int lines = height/2;
   while ( lines-- ) {
      endDst= dst + 3 * width;
      while ( dst != endDst ) {
         *dst++ = srcIpl[2];
         *dst++ = srcIpl[1];
         *dst++ = srcIpl[0];
         srcIpl += 3;
      }
      dst += qiOffset;
      srcIpl += iplOffset;
   }

   // Copy from cimgImage
   const unsigned int cimgOffset = width * (height/2);
   const int spectrum = cimgImage.spectrum();
   const float *srcR = cimgImage.data( 0,0,0,0 ) + cimgOffset;
   const float *srcG = cimgImage.data( 0,0,0,(spectrum>=2)?1:0 ) + cimgOffset;
   const float *srcB = cimgImage.data( 0,0,0,(spectrum>=3)?2:0 ) + cimgOffset;
   lines = height - height/2;
   while ( lines-- ) {
      endDst= dst + 3 * width;
      while ( dst != endDst ) {
         *dst++ = static_cast<unsigned char>( *srcR++ );
         *dst++ = static_cast<unsigned char>( *srcG++ );
         *dst++ = static_cast<unsigned char>( *srcB++ );
      }
      dst += qiOffset;
   }
}

void ImageConverter::mergeRight( IplImage * iplImage,
                                const cimg_library::CImg<float> & cimgImage,
                                QImage * out )
{
   const int width = iplImage->width;
   const unsigned int firstHalf = width/2;
   const unsigned int secondHalf = width - width/2;

   const int spectrum = cimgImage.spectrum();
   const float *srcR = cimgImage.data( 0,0,0,0 ) + firstHalf;
   const float *srcG = cimgImage.data( 0,0,0,(spectrum>=2)?1:0 ) + firstHalf;
   const float *srcB = cimgImage.data( 0,0,0,(spectrum>=3)?2:0 ) + firstHalf;

   unsigned char * srcIpl = reinterpret_cast<unsigned char *>( iplImage->imageData );
   const unsigned int iplOffset = iplImage->widthStep - 3 * width;
   const unsigned int iplShift = 3 * secondHalf + iplOffset;

   unsigned char * dst = out->scanLine(0);
   unsigned char * endDst;
   const unsigned int qiOffset = ((width*3)%4)?(4-((width*3)%4)):0;

   int height = iplImage->height;
   while ( height-- ) {
      // First half from iplImage
      endDst= dst + 3 * firstHalf;
      while ( dst != endDst ) {
         *dst++ = srcIpl[2];
         *dst++ = srcIpl[1];
         *dst++ = srcIpl[0];
         srcIpl += 3;
      }
      srcIpl += iplShift;
      // Second half from cimgImage
      endDst= dst + 3 * secondHalf;
      while ( dst != endDst ) {
         *dst++ = static_cast<unsigned char>( *srcR++ );
         *dst++ = static_cast<unsigned char>( *srcG++ );
         *dst++ = static_cast<unsigned char>( *srcB++ );
      }
      srcR += firstHalf;
      srcG += firstHalf;
      srcB += firstHalf;

      dst += qiOffset;
   }
}

void ImageConverter::convert( const IplImage * in, cimg_library::CImg<float> & out )
{
   assert(in->depth== IPL_DEPTH_8U);
   assert(in->nChannels == 3);
   const int spectrum = out.spectrum();
   float * dstR = out.data( 0,0,0,0 );
   float * dstG = out.data( 0,0,0,(spectrum>=2)?1:0 );
   float * dstB = out.data( 0,0,0,(spectrum>=3)?2:0 );
   const unsigned char * src = reinterpret_cast<unsigned char*>(in->imageData );
   const unsigned char * endSrc;
   const unsigned int w3 = in->width * 3;
   const unsigned int iplOffset = in->widthStep - w3;
   unsigned int height = in->height;
   while ( height-- ) {
      endSrc = src + w3;
      while ( src != endSrc ) {
         *dstB++ = static_cast<float>(*src++);
         *dstG++ = static_cast<float>(*src++);
         *dstR++ = static_cast<float>(*src++);
      }
      src += iplOffset;
   }
}
