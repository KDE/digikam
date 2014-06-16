/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   FilterThread.cpp
 * @author Sebastien Fourey
 * @date   July 2010
 * @brief Definition of the methods of the class FilterThread.
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
#include "FilterThread.h"
#include "ImageConverter.h"
#include "WebcamGrabber.h"
#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QMutex>
#include <QPainter>
#include <QImage>
#include <iostream>
using namespace cimg_library;

const unsigned int FilterThread::_gmic_variables_sizes[27] = { 0 };

FilterThread::FilterThread( WebcamGrabber & webcam,
                           const QString & command,
                           QImage * outputImage,
                           QMutex * imageMutex,
                           PreviewMode previewMode,
                           int frameSkip )
   : _webcam( webcam ),
     _outputImage( outputImage ),
     _imageMutex( imageMutex ),
     _previewMode( previewMode ),
     _frameSkip( frameSkip ),
     _continue( true ),
     _xMouse( -1 ),
     _yMouse( -1 ),
     _buttonsMouse( 0 ),
     _gmic_images(),
     _gmic("-v - ")
{
   setCommand(command);
}

FilterThread::~FilterThread()
{
}

void
FilterThread::setMousePosition( int x, int y, int buttons )
{
  _xMouse = x;
  _yMouse = y;
  _buttonsMouse = buttons;
}

void
FilterThread::setPreviewMode( PreviewMode pm )
{
   _previewMode = pm;
}

void
FilterThread::setFrameSkip( int n )
{
   _frameSkip = n;
}

void
FilterThread::stop()
{
   _continue = false;
}

void
FilterThread::run()
{
   int n;
   while ( _continue ) {
      // Skip some frames and grab an image from the webcam
      n = _frameSkip + 1;
      while ( n-- ) {
         _webcam.capture();
      }
      if ( !_gmic_images )
	 _gmic_images.assign(1);
      if ( !_gmic_images[0].is_sameXYZC(_webcam.width(),_webcam.height(),1,3) )
	 _gmic_images[0].assign(_webcam.width(),_webcam.height(),1,3);

      ImageConverter::convert( _webcam.image(), _gmic_images[0] );
      
      // Call the G'MIC interpreter.
      try {
	 QString c( _command );
	 QString strX = QString("%1").arg( _xMouse );
	 QString strY = QString("%1").arg( _yMouse );
	 QString strB = QString("%1").arg( _buttonsMouse );
	 c.replace( "@x", strX )
	    .replace( "@y", strY )
	    .replace( "@b", strB );
	 c.replace( "@{!,x}", strX )
	    .replace( "@{!,y}", strY )
	    .replace( "@{!,b}", strB );
	 _gmic.parse( c.toAscii().constData(),
		      _gmic_images,
		      _gmic_images_names);

	 switch ( _previewMode ) {
	 case Full:
	    if (_gmic_images && _gmic_images[0]) {
	       _imageMutex->lock();
	       QSize size( _gmic_images[0].width(), _gmic_images[0].height() );
	       if ( _outputImage->size() != size ) {
		  *_outputImage = QImage( size, QImage::Format_RGB888 );
	       }
	       _imageMutex->unlock();
	       ImageConverter::convert( _gmic_images[0], _outputImage );
	    }
	    break;
	 case Camera: {
	    _imageMutex->lock();
	    QSize size( _webcam.width(), _webcam.height() );
	    if ( _outputImage->size() != size ) {
	       *_outputImage = QImage( size, QImage::Format_RGB888 );
	    }
	    _imageMutex->unlock();
	    ImageConverter::convert( _webcam.image(), _outputImage );
	 }
	    break;
	 case LeftHalf:
	    ImageConverter::merge( _webcam.image(), _gmic_images[0], _outputImage,
				   _imageMutex, ImageConverter::MergeLeft );
	    break;
	 case TopHalf:
	    ImageConverter::merge( _webcam.image(), _gmic_images[0], _outputImage,
				   _imageMutex, ImageConverter::MergeTop );
	    break;
	 case BottomHalf:
	    ImageConverter::merge( _webcam.image(), _gmic_images[0], _outputImage,
				   _imageMutex, ImageConverter::MergeBottom );
	    break;
	 case RightHalf:
	    ImageConverter::merge( _webcam.image(), _gmic_images[0], _outputImage,
				   _imageMutex, ImageConverter::MergeRight );
	    break;
	 default:
	    _outputImage->fill( QColor(255,255,255).rgb() );
	    break;
	 }

      } catch (gmic_exception &e) {
	 const unsigned char col1[] = { 0,255,0 }, col2[] = { 0,0,0 };
	 CImg<unsigned char> src( reinterpret_cast<unsigned char*>(_webcam.image()->imageData),
				  3, _webcam.width(), _webcam.height(), 1, true );
	 _gmic_images = src.get_permute_axes("yzcx")
	    .channel(0).resize(-100,-100,1,3).draw_text(10,10,"Syntax Error",col1,col2,0.5,57);
	 std::cerr << e.what() << std::endl;
	 QSize size( _webcam.image()->width, _webcam.image()->height );
	 if ( _outputImage->size() != size ) {
	    _imageMutex->lock();
	    *_outputImage = QImage( size, QImage::Format_RGB888 );
	    _imageMutex->unlock();
	 }
	 ImageConverter::convert( _gmic_images[0], _outputImage );
      }
      emit imageAvailable();
   }
}

/*
 * Private methods
 */

void
FilterThread::setCommand( const QString & command )
{
   QByteArray str = command.toAscii();
   str.replace( 10, ' ' );
   str.replace( 13, ' ' );
   _command = str.constData();
}
