/* ============================================================
 * File  : imageproperties.h
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2003 by Gilles Caulier
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
 
#ifndef IMAGEPROPERTIES_H
#define IMAGEPROPERTIES_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>
#include <qguardedptr.h>
#include <qlabel.h>

// KDE includes.

#include <kdialogbase.h>
#include <kurl.h>

// Local includes.

#include "thumbnailjob.h"

class QComboBox;
class QSpinBox;
class QTextEdit;
class QMouseEvent;
class QPopupMenu;

class KSqueezedTextLabel;

class AlbumIconView;
class AlbumIconItem;
class AlbumLister;

class KExifListView;
class KExifData;

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
}

class ExifThumbLabel : public QLabel
{
    Q_OBJECT

public:
    
    ExifThumbLabel(QWidget * parent, AlbumIconView* currItemView);
    ~ExifThumbLabel();
    
    void setOrientationMenu(KExifData *currExifData);

protected:    
    
    QPopupMenu*    m_popmenu;
    
    AlbumIconView* m_IconView;
    
    virtual void mousePressEvent ( QMouseEvent * e );
    
};

class ImageProperties : public KDialogBase
{
    Q_OBJECT

public:

    ImageProperties(AlbumIconView* view, AlbumIconItem* currItem);
    ~ImageProperties();

public slots:    
    
    // For histogram viever.
    
    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);
        
private:

    // For main dialog.    
    
    AlbumIconView                *m_view;
    
    AlbumIconItem                *m_currItem;
    
    AlbumLister                  *m_lister;
    
    // For General tab.
    
    QLabel                       *m_generalThumb;
    
    KSqueezedTextLabel           *m_filename;
    KSqueezedTextLabel           *m_filetype;
    KSqueezedTextLabel           *m_filedim;
    KSqueezedTextLabel           *m_filepath;
    KSqueezedTextLabel           *m_filedate;
    KSqueezedTextLabel           *m_filesize;
    KSqueezedTextLabel           *m_fileowner;
    KSqueezedTextLabel           *m_filepermissions;
    KSqueezedTextLabel           *m_filealbum;
    KSqueezedTextLabel           *m_filecomments;
    KSqueezedTextLabel           *m_filetags;
    
    QGuardedPtr<ThumbnailJob>     m_generalThumbJob;
        
    void setupGeneralTab(void);

    // For Exif viever.
    
    ExifThumbLabel               *m_exifThumb;

    KExifData                    *mExifData;
    
    KExifListView                *m_listview;
    
    void setupExifViewer(void);
    
    // For histogram viever.
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    QComboBox                    *m_colorsCB;    
        
    QSpinBox                     *m_minInterv;
    QSpinBox                     *m_maxInterv;
    
    QLabel                       *m_histogramThumb;
    QLabel                       *m_labelMeanValue;
    QLabel                       *m_labelPixelsValue;
    QLabel                       *m_labelStdDevValue;
    QLabel                       *m_labelCountValue;
    QLabel                       *m_labelMedianValue;
    QLabel                       *m_labelPercentileValue;
    
    QImage                        m_image;
    
    QGuardedPtr<ThumbnailJob>     m_HistogramThumbJob;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::HistogramWidget     *m_histogramWidget;
    
    void setupHistogramViewer(void);

    void updateInformations();

        
private slots:

    // For main dialog.    

    void slotItemChanged();
    void slotUser1();
    void slotUser2();
    
    // For General tab.

    void slotGotGeneralThumbnail(const KURL&, const QPixmap& pix,
                                 const KFileMetaInfo*);  

    // For histogram viever.
    
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotIntervChanged(int);
    void slotGotHistogramThumbnail(const KURL&, const QPixmap& pix,
                                   const KFileMetaInfo*);  
  
};

#endif  // IMAGEPROPERTIES_H
