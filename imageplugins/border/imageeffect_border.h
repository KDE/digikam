/* ============================================================
 * File  : imageeffect_border.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-20
 * Description : a Digikam image plugin for add a border  
 *               to an image.
 * 
 * Copyright 2005 by Gilles Caulier
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

#include <qcolor.h>
#include <qimage.h>

// KDE includes.

#include <kdialogbase.h>

class QComboBox;
class QLabel;

class KIntNumInput;
class KColorButton;

namespace Digikam
{
class ImageWidget;
}

namespace DigikamBorderImagesPlugin
{

class ImageEffect_Border : public KDialogBase
{
    Q_OBJECT
    
public:

    ImageEffect_Border(QWidget *parent);
    ~ImageEffect_Border();

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    QLabel               *m_labelForeground;
    QLabel               *m_labelBackground;
    
    QWidget              *m_parent;
    
    QPushButton          *m_helpButton;

    QComboBox            *m_borderType;
    
    QColor                m_solidColor;
    QColor                m_niepceBorderColor;
    QColor                m_niepceLineColor;
    QColor                m_bevelUpperLeftColor; 
    QColor                m_bevelLowerRightColor;
    QColor                m_liquidBackgroundColor;
    QColor                m_liquidForegroundColor;
    QColor                m_roundCornerBackgroundColor;
    
    KIntNumInput         *m_borderWidth;
    
    KColorButton         *m_foregroundColorButton;
    KColorButton         *m_backgroundColorButton;
    
    Digikam::ImageWidget *m_previewWidget;

    void readSettings(void);
    void writeSettings(void);
    
    void solid(QImage &src, QImage &dest, const QColor &fg, int borderWidth);
    void niepce(QImage &src, QImage &dest, const QColor &fg, int borderWidth, const QColor &bg, int lineWidth);
    void liquid(QImage &src, QImage &dest, const QColor &fg, const QColor &bg, int borderWidth);
    void bevel(QImage &src, QImage &dest, const QColor &topColor, const QColor &btmColor, int borderWidth);
    void roundCorner(QImage &src, QImage &dest, const QColor &bg);
    
    void copyImageSecondaryAlpha(QImage &dest, int dx, int dy, int dw, int dh,
                                 QImage &src, int sx, int sy, int sw, int sh);
    void tileImage(QImage &dest, int dx, int dy, int dw, int dh, 
                   QImage &src, int sx, int sy, int sw, int sh);

private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotUser1();
    void slotBorderTypeChanged(int borderType);
    void slotColorForegroundChanged(const QColor &color);
    void slotColorBackgroundChanged(const QColor &color);
    
};

}  // NameSpace DigikamBorderImagesPlugin

#endif /* IMAGEEFFECT_BORDER_H */
