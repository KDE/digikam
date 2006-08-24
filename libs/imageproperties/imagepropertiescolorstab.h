/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-17
 * Description : A tab to display Colors image informations
 *
 * Copyright 2004-2006 by Gilles Caulier
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
 * ============================================================ */

#ifndef IMAGEPROPERTIESCOLORSTAB_H
#define IMAGEPROPERTIESCOLORSTAB_H

// Qt includes.

#include <qwidget.h>
#include <qcstring.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

class QRect;

namespace Digikam
{

class DImg;
class LoadingDescription;
class ImagePropertiesColorsTabPriv;

class DIGIKAM_EXPORT ImagePropertiesColorsTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesColorsTab(QWidget* parent, QRect* selectionArea, bool navBar=true);
    ~ImagePropertiesColorsTab();

    void setData(const KURL& url=KURL(), QRect *selectionArea=0, 
                 DImg *img=0, int itemType=0);

    void setSelection(QRect *selectionArea);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 

private:

    void loadImageFromUrl(const KURL& url);
    void updateInformations();
    void updateStatistiques();
    void getICCData();
    
private slots:

    void slotRefreshOptions(bool sixteenBit);
    void slotHistogramComputationFailed(void);
    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotColorsChanged(int color);
    void slotRenderingChanged(int rendering);
    void slotMinValueChanged(int);
    void slotMaxValueChanged(int);

    void slotUpdateInterval(int min, int max);
    void slotUpdateIntervRange(int range);

    void slotLoadImageFromUrlComplete(const LoadingDescription &loadingDescription, const DImg& img);
    void slotMoreCompleteLoadingAvailable(const LoadingDescription &oldLoadingDescription,
                                          const LoadingDescription &newLoadingDescription);

private:

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel,
        AlphaChannel,
        ColorChannels
    };
    
    enum AllColorsColorType
    {
        AllColorsRed=0,
        AllColorsGreen,
        AllColorsBlue
    };

    ImagePropertiesColorsTabPriv* d;
    
};

}  // NameSpace Digikam

#endif /* IMAGEPROPERTIESCOLORSTAB_H */
