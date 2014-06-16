/** -*- mode: c++ ; c-basic-offset: 3 -*-
 * @file   WebcamGrabber.cpp
 * @author Sebastien Fourey
 * @date   July 2010
 * @brief Definition of methods of the class WebcamGrabber
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
#include "Common.h"
#include "WebcamGrabber.h"
#include <QCoreApplication>
#include <QDir>
#include <QStringList>
using namespace std;

WebcamGrabber::WebcamGrabber()
{   
   _capture = 0;
   _image = 0;
   _width = 0;
   _height = 0;
}

WebcamGrabber::~WebcamGrabber()
{
   cvReleaseCapture( &_capture );
}

void
WebcamGrabber::capture()
{
   _image = cvQueryFrame( _capture );
}

QList<int>
WebcamGrabber::getWebcamList()
{
   QList<int> camList;
#if defined(_IS_UNIX_)
   int i = 0;
   for ( i = 0; i < 6 ; ++i ) {
      QFile file(QString("/dev/video%1").arg(i));
      if ( file.open(QFile::ReadOnly) ) {
         file.close();
         camList.push_back(i);
      }
   }
#else
   CvCapture *capture = 0;
   int i = 0;
   for ( i = 0; i < 6 ; ++i ) {
      capture = cvCaptureFromCAM( i );
      if ( capture ) {
	 cvReleaseCapture( &capture );
	 camList.push_back(i);
      }
   }
#endif
   return camList;
}

void
WebcamGrabber::setCameraIndex( int i )
{
  if ( _capture ) {
    cvReleaseCapture( &_capture );
  }
  _capture = cvCaptureFromCAM( i );
  _image = cvQueryFrame( _capture );
  if ( _image->width != 640 || _image->height != 480 ) {
     cvSetCaptureProperty(_capture,CV_CAP_PROP_FRAME_WIDTH,640);
     cvSetCaptureProperty(_capture,CV_CAP_PROP_FRAME_HEIGHT,480);
     _image = cvQueryFrame( _capture );
     _image = cvQueryFrame( _capture );
  }
  _width = _image->width;
  _height = _image->height;
  _cameraIndex = i;
}
