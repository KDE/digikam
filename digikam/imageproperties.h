/* ============================================================
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
class QRect;
class QCheckBox;
class QGroupBox;

class KSqueezedTextLabel;
class KPopupMenu;
class KSeparator;

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
    
    ExifThumbLabel(QWidget * parent);
    ~ExifThumbLabel();
    
    void setOrientationMenu(KExifData *currExifData, KURL currentUrl);

protected:    
    
    KPopupMenu*    m_popmenu;
    
    KURL           m_currentUrl;
    
    virtual void mousePressEvent ( QMouseEvent * e );
    
};

//////////////////////////////////////////////////////////////////////////

class ImageProperties : public KDialogBase
{
    Q_OBJECT

public:

    // Digikam embedded mode constructor.
    
    ImageProperties(AlbumIconView* view, AlbumIconItem* currItem, 
                    QWidget *parent=0L, 
                    QRect *selectionArea=0L);  // Used for histogram image selection computation.

    // Stand Alone mode constructor.
                        
    ImageProperties(KURL::List filesList, KURL currentFile, 
                    QWidget *parent=0L,
                    QRect *selectionArea=0L);
                    
    ~ImageProperties();

public slots:    
    
    // For histogram viever.
    
    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);
        
private:

    // For main dialog.    
    
    AlbumIconView                *m_view;
    
    AlbumIconItem                *m_currItem;
    
    KURL                          m_currfileURL;
    
    KURL::List                    m_urlList;        // Used only in Stand Alone mode.
    KURL::List::Iterator          m_urlListIt;
    
    void setupGui(void);
    
    // For General tab.
    
    QLabel                       *m_generalThumb;
    QLabel                       *m_albumLabel;
    QLabel                       *m_commentsLabel;
    QLabel                       *m_tagsLabel;
    QLabel                       *m_pathLabel;
    
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
    
    KSeparator                   *m_sep1;    
    KSeparator                   *m_sep2;
    
    QGuardedPtr<ThumbnailJob>     m_generalThumbJob;
        
    void setupGeneralTab(void);

    // For Exif viever.
                                 
    QLabel                       *m_mainExifThumb;
    
    QComboBox                    *m_levelExifCB;
    
    QCheckBox                    *m_embeddedThumbBox; 
    
    QGroupBox                    *m_embeddedThumbnail;
    
    ExifThumbLabel               *m_exifThumb;

    KExifData                    *m_ExifData;
    
    KExifListView                *m_listview;
    
    QString                       m_currentGeneralExifItemName;
    QString                       m_currentExtendedExifItemName;
    QString                       m_currentAllExifItemName;
    
    void setupExifViewer(void);
    void getCurrentExifItem(void);
    void setCurrentExifItem(void);
    
    // For histogram viever.
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    QComboBox                    *m_colorsCB;    
    QComboBox                    *m_renderingCB;    
        
    QSpinBox                     *m_minInterv;
    QSpinBox                     *m_maxInterv;
    
    QLabel                       *m_histogramThumb;
    QLabel                       *m_labelMeanValue;
    QLabel                       *m_labelPixelsValue;
    QLabel                       *m_labelStdDevValue;
    QLabel                       *m_labelCountValue;
    QLabel                       *m_labelMedianValue;
    QLabel                       *m_labelPercentileValue;
    QLabel                       *m_renderingLabel;
    
    QImage                        m_image;
    QImage                        m_imageSelection;
    
    KURL                          m_IEcurrentURL;
    
    QRect                        *m_selectionArea;
    
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
    
    void slotFailedGeneralThumbnail(const KURL&);

    // For Exif viever.
      
    void slotLevelExifChanged(int level);
    void slotToogleEmbeddedThumb(bool toogle);
    
    // For histogram viever.
    
    void slotRefreshOptions(void);
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotIntervChanged(int);
    void slotGotHistogramThumbnail(const KURL&, const QPixmap& pix,
                                   const KFileMetaInfo*);  
    void slotFailedHistogramThumbnail(const KURL&);
  
};

#endif  // IMAGEPROPERTIES_H
