/* ============================================================
 * File  : histogramproposplugin.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-07-28
 * Description : an image histogram viewer dialog.
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef HISTOGRAMPROPSPLUGIN_H
#define HISTOGRAMPROPSPLUGIN_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

// KDE includes.

#include <kpropertiesdialog.h>

class QComboBox;
class QSpinBox;
class QLabel;

namespace Digikam
{
class HistogramWidget;
}

class GradientWidget;

class HistogramPropsPlugin : public KPropsDlgPlugin
{
   Q_OBJECT

public:

   HistogramPropsPlugin( KPropertiesDialog *propsDlg, QString imageFile);
   ~HistogramPropsPlugin();
   
   virtual void applyChanges();

public slots:    
    
    void slotUpdateMinInterv(int min);
    void slotUpdateMaxInterv(int max);
        
protected:

    //void closeEvent(QCloseEvent *e);
    
private:
    
    QComboBox                *m_channelCB;    
    QComboBox                *m_scaleCB;    
    
    QSpinBox                 *m_minInterv;
    QSpinBox                 *m_maxInterv;
    
    QLabel                   *m_labelMeanValue;
    QLabel                   *m_labelPixelsValue;
    QLabel                   *m_labelStdDevValue;
    QLabel                   *m_labelCountValue;
    QLabel                   *m_labelMedianValue;
    QLabel                   *m_labelPercentileValue;
                
    GradientWidget           *m_hGradient;
    
    QImage                    m_image;
    
    Digikam::HistogramWidget *m_histogramWidget;
    
    void setupGui(KPropertiesDialog *dialog, uint *imageData, uint width, uint height);
    void updateInformations();

private slots:

    void slotChannelChanged(int channel);
    void slotScaleChanged(int scale);
    void slotIntervChanged(int);

};


#endif /* HISTOGRAMPROPSPLUGIN_H */
