/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Jaromir Malenko <malenko at email.cz>
 * Date   : 2004-12-09
 * Description : image selection widget used by ratio crop tool.
 * 
 * Copyright 2004-2007 by Gilles Caulier
 * Copyright 2007 by Jaromir Malenko
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

#ifndef IMAGESELECTIONWIDGET_H
#define IMAGESELECTIONWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qrect.h>
#include <qcolor.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ImageSelectionWidgetPriv;

class DIGIKAM_EXPORT ImageSelectionWidget : public QWidget
{
Q_OBJECT

public:

    enum RatioAspect           // Contrained Aspect Ratio list.
    {
        RATIOCUSTOM=0,             // Custom aspect ratio.
        RATIO01X01,                // 1:1
        RATIO02x03,                // 2:3
        RATIO03X04,                // 3:4
        RATIO04X05,                // 4:5
        RATIO05x07,                // 5:7
        RATIO07x10,                // 7:10
        RATIOGOLDEN,               // Golden ratio : 1:1.618
        RATIONONE                  // No aspect ratio.
    };

    enum Orient
    {
        Landscape = 0,
        Portrait
    };

    enum CenterType
    {
        CenterWidth = 0,           // Center selection to the center of image width.
        CenterHeight,              // Center selection to the center of image height.
        CenterImage                // Center selection to the center of image.
    };

    // Proportion : Golden Ratio and Rule of Thirds. More information at this url: 
    // http://photoinf.com/General/Robert_Berdan/Composition_and_the_Elements_of_Visual_Design.htm

    enum GuideLineType
    {
        RulesOfThirds = 0,         // Line guides position to 1/3 width and height.
        HarmoniousTriangles,       // Harmonious Triangle to improve composition.
        GoldenMean,                // Guides tools using Phi ratio (1.618).
        GuideNone                  // No guide line.
    };

public:

    ImageSelectionWidget(int width, int height, QWidget *parent=0, 
                         float aspectRatioValue=1.0, int aspectRatio=RATIO01X01, 
                         int orient=Landscape, int guideLinesType=GuideNone);
    ~ImageSelectionWidget();

    void  setCenterSelection(int centerType=CenterImage);
    void  setSelectionX(int x);
    void  setSelectionY(int y);
    void  setSelectionWidth(int w);
    void  setSelectionHeight(int h);
    void  setSelectionOrientation(int orient);
    void  setAutoOrientation(bool orientation);
    void  setSelectionAspectRatioType(int aspectRatioType);
    void  setSelectionAspectRatioValue(float aspectRatioValue);
    void  setGoldenGuideTypes(bool drawGoldenSection,  bool drawGoldenSpiralSection,
                              bool drawGoldenSpiral,   bool drawGoldenTriangle,
                              bool flipHorGoldenGuide, bool flipVerGoldenGuide);

    int   getOriginalImageWidth(void);
    int   getOriginalImageHeight(void);
    QRect getRegionSelection(void);

    int   getMinWidthRange(void);
    int   getMinHeightRange(void);

    void  resetSelection(void);
    void  maxAspectSelection(void);

    ImageIface* imageIface();

public slots:

    void slotGuideLines(int guideLinesType);
    void slotChangeGuideColor(const QColor &color);
    void slotChangeGuideSize(int size);

signals:

    void signalSelectionMoved( QRect rect );
    void signalSelectionChanged( QRect rect );
    void signalSelectionWidthChanged( int newWidth );
    void signalSelectionHeightChanged( int newHeight );
    void signalSelectionOrientationChanged( int newOrientation );

protected:

    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    void resizeEvent(QResizeEvent * e);

protected slots:

    void slotTimerDone(void);

private:

    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    void regionSelectionMoved( bool targetDone );

    void regionSelectionChanged(bool targetDone);
    void realToLocalRegion(bool updateSizeOnly=false);
    void localToRealRegion(void);
    void normalizeRegion(void);
    void applyAspectRatio(bool WOrH, bool repaintWidget=true, bool updateChange=true);
    void updatePixmap(void);
    QPoint computeAspectRatio(QPoint pm, int coef=1);
    QPoint opposite(void);
    float distance(QPoint a, QPoint b);
    void placeSelection(QPoint pm, bool symetric, QPoint center);
    void setCursorResizing(void);

private:

    ImageSelectionWidgetPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGESELECTIONWIDGET_H */
