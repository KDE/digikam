/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   ImageView.cpp
 * @author Sebastien Fourey
 * @date   July 2010
 * @brief Definition of the methods of the class ImageView.
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
 * 
 */
#include "ImageView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QThread>
#include <QFrame>
#include <QLayout>
#include <QRect>
#include <iostream>
#include "Common.h"

ImageView::ImageView( QWidget * parent )
  : QWidget( parent )
{
   setAutoFillBackground( false );
   _image = QImage( 640, 480, QImage::Format_RGB888 );
   _image.fill( 0 );
   setMinimumSize(320,200);
   _imagePosition = geometry();
   _scaleFactor = 1.0;
   _zoomOriginal= false;
}

void
ImageView::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  _imageMutex.lock();
  if ( _image.size() == size() ) {
     painter.drawImage( 0, 0, _image );
     _imagePosition = rect();
     _scaleFactor = 1.0;
     _imageMutex.unlock();
     return;
  }
  QImage scaled;
  const double imageRatio = _image.width() / static_cast<double>(_image.height() );
  const double widgetRatio = width() / static_cast<double>( height() );
  if ( imageRatio > widgetRatio ) {
      scaled = _image.scaledToWidth(width());
      _imagePosition = QRect(0,(height()-scaled.height())/2,scaled.width(),scaled.height());
      _scaleFactor = scaled.width()/static_cast<double>(_image.width());
      painter.drawImage( _imagePosition.topLeft(), scaled );
  } else {
      scaled = _image.scaledToHeight(height());
      _imagePosition = QRect((width()-scaled.width())/2,0,scaled.width(),scaled.height());
      _scaleFactor = scaled.height()/static_cast<double>(_image.height());
      painter.drawImage( _imagePosition.topLeft(), scaled );
  }
  _imageMutex.unlock();
}

void
ImageView::mousePressEvent( QMouseEvent * e )
{
   if ( !_imagePosition.contains(e->pos()) ) return;
   *e = mapMousePositionToImage( e );
   emit mousePress( e );
}

void
ImageView::mouseReleaseEvent( QMouseEvent * )
{
}

void
ImageView::mouseMoveEvent( QMouseEvent * e )
{
   if ( !_imagePosition.contains(e->pos()) ) return;
   *e = mapMousePositionToImage( e );
   emit mouseMove( e );
}

void
ImageView::resizeEvent( QResizeEvent * )
{
}

void ImageView::zoomOriginal()
{
   _zoomOriginal = true;
   setMinimumSize(_image.size());
   resize(_image.size());
   setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

void ImageView::zoomFitBest()
{
   _zoomOriginal = false;
   QFrame * frame = dynamic_cast<QFrame*>(parent());
   if ( frame ) {
      QRect rect = frame->layout()->contentsRect();
      setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
      resize(rect.width(),rect.height());
   }
}

void ImageView::checkSize()
{
   if ( !_zoomOriginal ) return;
   if ( size() != _image.size() ) {
      setMinimumSize(_image.size());
      resize(_image.size());
      setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
   }
}

QMouseEvent
ImageView::mapMousePositionToImage( QMouseEvent * e )
{
   int x = (e->pos().x()-_imagePosition.left()) / _scaleFactor;
   int y = (e->pos().y()-_imagePosition.top()) / _scaleFactor;
   return QMouseEvent(e->type(),QPoint(x,y),e->button(),e->buttons(),e->modifiers());
}
