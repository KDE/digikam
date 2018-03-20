/*
   This file is part of the KDE project
   Copyright (C) 2008 Christian Ehrlicher <ch.ehrlicher@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   png to ico format converter
*/

#include "qicohandler.h"
#include "qcurhandler.h"
#include "qanihandler.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QTextStream>
#include <QtGui/QImage>
#include <QtGui/QPainter>

static void usage ( const QString &errMsg )
{
  QString appname = QCoreApplication::instance()->arguments() [0];
  fprintf ( stderr, "%s\n", qPrintable ( errMsg ) );
  fprintf ( stdout, "%s version 0.1\n", qPrintable ( appname ) );
  fprintf ( stdout, "USAGE: %s file.ico [--rcfile rcfile] pngfile1 [pngfile2 ...]\n", qPrintable ( appname ) );
  fprintf ( stdout, "USAGE: %s file.cur [--hotspotx hotspotx] [--hotspoty hotspoty] pngfile1\n", qPrintable ( appname ) );
  fprintf ( stdout, "USAGE: %s file.ani [--hotspotx hotspotx] [--hotspoty hotspoty] [--framerate framerate] pngfile1 [pngfile2 ...]\n", qPrintable ( appname ) );
  exit ( 1 );
}

static void warning ( const QString &wrnMsg )
{
  fprintf ( stdout, "%s\n", qPrintable ( wrnMsg ) );
}

static void fatal ( const QString &errMsg )
{
  fprintf ( stderr, "%s\n", qPrintable ( errMsg ) );
  exit ( 2 );
}

QImage scaleImage(const QImage &source)
{
    //make an empty image and fill with transparent color
    QImage result(32, 32, QImage::Format_ARGB32);
    result.fill(0);

    QPainter paint(&result);
    paint.translate(0, 0);
    if( source.width() > 32 || source.height() > 32 )
    {
        QImage scaled = source.scaled( 32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        paint.drawImage(QPoint(0, 0), scaled);
    } else {
        paint.drawImage(QPoint(0, 0), source);
    }

    return result;
}

static bool sortQImageForSize ( const QImage &i1, const QImage &i2 )
{
  int s1 = i1.size().width() * i1.size().height();
  int s2 = i2.size().width() * i2.size().height();
  return s1 < s2;
}

int main ( int argc, char **argv )
{
  QCoreApplication app ( argc, argv );

  QHash<int, QImage> imagesToUse;
  QList<QImage> images;
  const QSize size16 ( 16,16 );
  const QSize size32 ( 32,32 );
  const QSize size48 ( 48,48 );
  if ( argc < 3 )
    usage ( "To few arguments" );

  QString rcFileName;
  QString icoFileName;
  int hotspotx = 0;
  int hotspoty = 0;
  int framerate = 3;
  for ( int i = 1; i < argc; i++ ) {
    const QString arg = app.arguments() [i];
    if ( arg == QLatin1String ( "--rcfile" ) ) {
      if ( i + 1 < argc ) {
        rcFileName = app.arguments() [i+1];
        i++;
        continue;
      } else {
        usage ( "option '--rcfile' without filename" );
      }
    } else if (arg == "--hotspotx") {
       if ( i + 1 < argc ) {
        hotspotx = app.arguments()[i+1].toInt();
        i++;
        continue;
       } else {
        usage ( "option '--hotspotx' without value" );
       }
    } else if (arg == "--hotspoty") {
       if ( i + 1 < argc ) {
        hotspoty = app.arguments()[i+1].toInt();
        i++;
        continue;
    } else {
        usage ( "option '--hotspoty' without value" );
       }
    } else if (arg == "--framerate") {
       if ( i + 1 < argc ) {
        framerate = app.arguments()[i+1].toInt();
        i++;
        continue;
    } else {
        usage ( "option '--framerate' without value" );
       }
    }

    if ( icoFileName.isEmpty() ) {
      icoFileName = arg;
      continue;
    }
    QImage img;
    img.load ( arg );
    if ( img.isNull() ) {
      warning ( QString ( "Can not load image %1" ).arg ( arg ) );
      continue;
    }

    if (icoFileName.endsWith(".ico", Qt::CaseInsensitive)) {
        images += img;
        if ( img.size() == size16 ) {
        if ( imagesToUse.contains ( 16 ) ) {
            warning ( QString ( "Already have an Image with 16x16 - overwriting with %1" ).arg ( arg ) );
        }
        imagesToUse.insert ( 16, img );
        continue;
        }
        if ( img.size() == size32 ) {
        if ( imagesToUse.contains ( 32 ) ) {
            warning ( QString ( "Already have an Image with 32x32 - overwriting with %1" ).arg ( arg ) );
        }
        imagesToUse.insert ( 32, img );
        continue;
        }
        if ( img.size() == size48 ) {
        if ( imagesToUse.contains ( 48 ) ) {
            warning ( QString ( "Already have an Image with 48x48- overwriting with %1" ).arg ( arg ) );
        }
        imagesToUse.insert ( 48, img );
        continue;
        }
    } else {
        if (img.size() != QSize(32, 32)) {
            img = scaleImage(img);
        }

        images += img;
    }
  }
  if ( images.count() == 0 )
    usage ( "No valid images found!" );
  if (icoFileName.endsWith(".ico", Qt::CaseInsensitive)) {
    qSort ( images.begin(), images.end(), sortQImageForSize );
    // 48x48 available -> if not create one
    if ( !imagesToUse.contains ( 48 ) ) {
        QImage img;
        Q_FOREACH ( const QImage &i, images ) {
        if ( img.width() >= 32 && img.height() >= 32 ) {
            img = i;
        }
        }
        if ( img.isNull() ) {
        // none found -> use the last (==biggest) available
        img = images.last();
        }
        imagesToUse.insert ( 48, img.scaled ( 48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
    }
    // 32x32 available -> if not create one
    if ( !imagesToUse.contains ( 32 ) ) {
        QImage img;
        Q_FOREACH ( const QImage &i, images ) {
        if ( img.width() >= 24 && img.height() >= 24 ) {
            img = i;
            // no need to scale from an higher size when we've 48x48
            if ( img.width() >= 48 && img.height() >= 48 )
            break;
        }
        }
        if ( img.isNull() ) {
        // none found -> use the last (==biggest) available
        img = images.last();
        }
        imagesToUse.insert ( 32, img.scaled ( 32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
    }
    // 16x16 available -> if not create one
    if ( !imagesToUse.contains ( 16 ) ) {
        QImage img;
        Q_FOREACH ( const QImage &i, images ) {
        if ( img.width() >= 10 && img.height() >= 10 ) {
            img = i;
            // no need to scale from an higher size when we've 32x32
            if ( img.width() >= 32 && img.height() >= 32 )
            break;
        }
        }
        if ( img.isNull() ) {
        // none found -> use the last (==biggest) available
        img = images.last();
        }
        imagesToUse.insert ( 16, img.scaled ( 16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );
    }
    images.clear();
    images += imagesToUse[16].convertToFormat ( QImage::Format_ARGB32,   Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
    images += imagesToUse[16].convertToFormat ( QImage::Format_Indexed8, Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
    images += imagesToUse[32].convertToFormat ( QImage::Format_ARGB32,   Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
    images += imagesToUse[32].convertToFormat ( QImage::Format_Indexed8, Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
    images += imagesToUse[48].convertToFormat ( QImage::Format_ARGB32,   Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
    images += imagesToUse[48].convertToFormat ( QImage::Format_Indexed8, Qt::ColorOnly|Qt::DiffuseAlphaDither|Qt::AvoidDither );
  }

  QFile f ( icoFileName );
  if ( !f.open ( QIODevice::WriteOnly ) ) {
    fatal ( QString ( "Can not open %1 for writing" ).arg ( icoFileName ) );
    return 2;
  }

  if (icoFileName.endsWith(".cur", Qt::CaseInsensitive)) {
    QtCurHandler ico ( &f );
    if ( !ico.write ( images, hotspotx, hotspoty ) ) {
        fatal ( "Can not create cur data" );
        return 2;
    }
    f.close();

  } else if (icoFileName.endsWith(".ani", Qt::CaseInsensitive)) {
    QtAniHandler ico ( &f );
    if ( !ico.write ( images, hotspotx, hotspoty, framerate ) ) {
        fatal ( "Can not create ani data" );
        return 2;
    }
    f.close();
  } else if(icoFileName.endsWith(".ico", Qt::CaseInsensitive)) {
    QtIcoHandler ico ( &f );
    if ( !ico.write ( images ) ) {
        fatal ( "Can not create ico data" );
        return 2;
    }
    f.close();
  } else {
    f.close();
  }

  if ( !rcFileName.isEmpty() ) {
    QFile rcFile ( rcFileName );
    if ( !rcFile.open ( QIODevice::WriteOnly ) ) {
      fatal ( QString ( "Can not open %1 for writing" ).arg ( rcFileName ) );
      return 2;
    }
    QTextStream ts(&rcFile);
    ts << QString( "IDI_ICON1        ICON        DISCARDABLE    \"%1\"\n" ).arg ( icoFileName );
    rcFile.close();
  }
  return 0;
}
