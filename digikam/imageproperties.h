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

#include <qguardedptr.h>
#include <qpixmap.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "thumbnailjob.h"

class QLabel;
class QListView;
class QPixmap;
class QCheckListItem;
class QCheckBox;

class KFileMetaInfo;
class KTextEdit;

class AlbumIconView;
class AlbumIconItem;
class AlbumLister;
class TAlbum;

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

    AlbumIconView *m_view;
    AlbumIconItem *m_currItem;
    AlbumLister   *m_lister;
    QLabel        *m_thumbLabel;
    QLabel        *m_nameLabel;
    KTextEdit     *m_commentsEdit;
    QListView     *m_tagsView;
    QCheckBox     *m_autoSaveBox;
    bool           m_modified;

    QGuardedPtr<ThumbnailJob> m_thumbJob;

    void populateTags();
    void populateTags(QCheckListItem* parItem, TAlbum* parAlbum);

    
    // For histogram viever.
    
    QComboBox                    *m_channelCB;    
    QComboBox                    *m_scaleCB;    
    QComboBox                    *m_colorsCB;    
        
    QSpinBox                     *m_minInterv;
    QSpinBox                     *m_maxInterv;
    
    QLabel                       *m_labelMeanValue;
    QLabel                       *m_labelPixelsValue;
    QLabel                       *m_labelStdDevValue;
    QLabel                       *m_labelCountValue;
    QLabel                       *m_labelMedianValue;
    QLabel                       *m_labelPercentileValue;
    
    QImage                        m_image;
    
    Digikam::ColorGradientWidget *m_hGradient;
    Digikam::HistogramWidget     *m_histogramWidget;
    
    void setupHistogramViewer(uint *imageData, uint width, uint height);

    void updateInformations();

        
private slots:

    // For histogram viever.
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotIntervChanged(int);
    
    
    void slotItemChanged();
    void slotUser1();
    void slotUser2();
  
};

#endif  // IMAGEPROPERTIES_H
