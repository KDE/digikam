/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006 Gilles Caulier
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

// C++ includes.

#include <cstdlib>
#include <cstdio>

// Qt includes.

#include <qpainter.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE include.

#include <kcursor.h>
#include <kprocess.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kfilemetainfo.h>

// Local includes.

#include "rawfiles.h"
#include "themeengine.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "imagepreviewwidget.h"

namespace Digikam
{

class ImagePreviewWidgetPriv
{
public:

    ImagePreviewWidgetPriv(){}

    QString  path;

    QPixmap  pixmap;

    QImage   preview;
};

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
                  : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePreviewWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(QWidget::StrongFocus);
    setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    setMargin(0);
    setLineWidth(1);

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
              this, SLOT(slotThemeChanged()));
}

ImagePreviewWidget::~ImagePreviewWidget()
{
    delete d;
}

void ImagePreviewWidget::setImagePath( const QString& path )
{
    if (path == d->path) return;

    kapp->setOverrideCursor( KCursor::waitCursor() );

    // Try to load preview using dcraw extraction
    if (!loadDCRawPreview(d->preview, path))
    {
        // Try to load the whole image using QImage.
        d->preview.load(path);
    }

    if (AlbumSettings::instance()->getExifRotate() && !d->preview.isNull())
        exifRotate(path, d->preview);

    d->path   = path;
    d->pixmap = QPixmap(contentsRect().size());
    updatePixmap();
    repaint(false);
    kapp->restoreOverrideCursor();
}

void ImagePreviewWidget::updatePixmap( void )
{
    QPainter p(&(d->pixmap));
    d->pixmap.fill(ThemeEngine::instance()->baseColor());

    if (!d->preview.isNull())
    {
        QPixmap pix(d->preview.scale(contentsRect().size(), QImage::ScaleMin));
        p.drawPixmap((contentsRect().width()-pix.width())/2,
                     (contentsRect().height()-pix.height())/2, pix,
                     0, 0, pix.width(), pix.height());
    }
    else
    {
        p.setPen(QPen(Qt::red));
        p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                   Qt::AlignCenter|Qt::WordBreak, 
                   i18n("Cannot display image preview!"));
    }

    p.end();
}

void ImagePreviewWidget::drawContents(QPainter *)
{
    bitBlt(this, contentsRect().topLeft(), &(d->pixmap), contentsRect(), Qt::CopyROP);
}

void ImagePreviewWidget::slotThemeChanged()
{
    updatePixmap();
    repaint(false);
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *)
{
    blockSignals(true);
    d->pixmap = QPixmap(contentsRect().size());
    updatePixmap();
    repaint(false);
    blockSignals(false);
}

bool ImagePreviewWidget::loadDCRawPreview(QImage& image, const QString& path)
{
    QFileInfo fileInfo(path);
    QString rawFilesExt(raw_file_extentions);

    if (!fileInfo.exists() || !rawFilesExt.upper().contains( fileInfo.extension().upper() ))
        return false;

    // Try to extract embedded thumbnail using dcraw with options:
    // -c : write to stdout
    // -e : Extract the camera-generated thumbnail, not the raw image (JPEG or a PPM file).
    // Note : this code require at least dcraw version 8.21

    QCString command  = "dcraw -c -e ";
    command += QFile::encodeName( KProcess::quote( path ) );
    kdDebug() << "Running dcraw command " << command << endl;

    FILE* f = popen( command.data(), "r" );

    if ( !f )
        return false;

    QByteArray imgData;
    const int  MAX_IPC_SIZE = (1024*32);
    char       buffer[MAX_IPC_SIZE];
    QFile      file;
    Q_LONG     len;

    file.open( IO_ReadOnly,  f );

    while ((len = file.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            file.close();
            return false;
        }
        else
        {
            int oldSize = imgData.size();
            imgData.resize( imgData.size() + len );
            memcpy(imgData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
            return true;
    }

    return false;
}

void ImagePreviewWidget::exifRotate(const QString& filePath, QImage& thumb)
{
    // Check if the file is an JPEG image
    KFileMetaInfo metaInfo(filePath, "image/jpeg", KFileMetaInfo::Fastest);

    if (metaInfo.isValid())
    {
        if (metaInfo.mimeType() == "image/jpeg" &&
            metaInfo.containsGroup("Jpeg EXIF Data"))
        {
            // Rotate thumbnail from JPEG files based on EXIF rotate tag

            QWMatrix matrix;
            DMetadata metadata(filePath);
            DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

            bool doXform = (orientation != DMetadata::ORIENTATION_NORMAL &&
                            orientation != DMetadata::ORIENTATION_UNSPECIFIED);

            switch (orientation) 
            {
                case DMetadata::ORIENTATION_NORMAL:
                case DMetadata::ORIENTATION_UNSPECIFIED:
                    break;

                case DMetadata::ORIENTATION_HFLIP:
                    matrix.scale(-1, 1);
                    break;

                case DMetadata::ORIENTATION_ROT_180:
                    matrix.rotate(180);
                    break;

                case DMetadata::ORIENTATION_VFLIP:
                    matrix.scale(1, -1);
                    break;

                case DMetadata::ORIENTATION_ROT_90_HFLIP:
                    matrix.scale(-1, 1);
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_90:
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_90_VFLIP:
                    matrix.scale(1, -1);
                    matrix.rotate(90);
                    break;

                case DMetadata::ORIENTATION_ROT_270:
                    matrix.rotate(270);
                    break;
            }

            //transform accordingly
            if ( doXform )
                thumb = thumb.xForm( matrix );
        }
    }
}

void ImagePreviewWidget::wheelEvent( QWheelEvent * e )
{
    if (e->delta() > 0)
        emit prevItem();

    if (e->delta() < 0)
        emit nextItem();
}

void ImagePreviewWidget::keyPressEvent( QKeyEvent * e )
{
    if ( e->state() == 0 )
    {
        switch ( e->key() )
        {
            case Key_PageUp:
                emit prevItem();
            break;

            case Key_PageDown:
                emit nextItem();
            break;

            case Key_Home:
                emit firstItem();
            break;

            case Key_End:
                emit lastItem();
            break;

            case Key_Escape:
                emit escapeSignal();
            break;

            default:
                return;
        }
    }
}

}  // NameSpace Digikam

#include "imagepreviewwidget.moc"
