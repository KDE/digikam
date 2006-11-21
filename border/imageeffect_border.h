/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#ifndef IMAGEEFFECT_BORDER_H
#define IMAGEEFFECT_BORDER_H

// Qt includes.

#include <qstring.h>

// Digikam includes.

#include <digikamheaders.h>

class QComboBox;
class QLabel;

class KIntNumInput;
class KColorButton;

namespace DigikamBorderImagesPlugin
{

class ImageEffect_Border : public Digikam::ImageGuideDlg
{
    Q_OBJECT
    
public:

    ImageEffect_Border(QWidget *parent, QString title, QFrame* banner);
    ~ImageEffect_Border();

private:

    QLabel               *m_labelForeground;
    QLabel               *m_labelBackground;

    QComboBox            *m_borderType;
    
    QColor                m_solidColor;
    QColor                m_niepceBorderColor;
    QColor                m_niepceLineColor;
    QColor                m_bevelUpperLeftColor; 
    QColor                m_bevelLowerRightColor;
    QColor                m_decorativeFirstColor; 
    QColor                m_decorativeSecondColor;
    
    KIntNumInput         *m_borderRatio;
    
    KColorButton         *m_firstColorButton;
    KColorButton         *m_secondColorButton;
    
private:

    QString getBorderPath(int border);
    
private slots:

    void slotBorderTypeChanged(int borderType);
    void slotColorForegroundChanged(const QColor &color);
    void slotColorBackgroundChanged(const QColor &color);
    void readUserSettings(void);
    
protected:
    
    void writeUserSettings(void);
    void prepareEffect(void);
    void prepareFinal(void);
    void putPreviewData(void);
    void putFinalData(void);
    void resetValues(void);   
    void renderingFinished(void);    
};

}  // NameSpace DigikamBorderImagesPlugin

#endif /* IMAGEEFFECT_BORDER_H */
