/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-10
 * Description : a widget to display CIE tongue from
 *               an ICC profile.
 *
 * Copyright 2006 by Gilles Caulier
 *
 * Any source code are inspired from lprof project and
 * Copyright (C) 1998-2001 Marti Maria
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

#include <cmath>

// Qt includes.

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qtimer.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "lcmsprf.h"
#include "cietonguewidget.h"

namespace Digikam
{

    // The  following  table  gives  the  CIE  colour  matching  functions
    // \bar{x}(\lambda),  \bar{y}(\lambda),  and   \bar{z}(\lambda),   for
    // wavelengths  \lambda  at 5 nanometre increments from 380 nm through
    // 780 nm.  This table is used in conjunction with  Planck's  law  for
    // the  energy spectrum of a black body at a given temperature to plot
    // the black body curve on the CIE chart. 

    // The following table gives the  spectral  chromaticity  co-ordinates
    // x(\lambda) and y(\lambda) for wavelengths in 5 nanometre increments
    // from 380 nm through  780  nm.   These  co-ordinates  represent  the
    // position in the CIE x-y space of pure spectral colours of the given
    // wavelength, and  thus  define  the  outline  of  the  CIE  "tongue"
    // diagram.

    static const double spectral_chromaticity[81][3] =
    {
        { 0.1741, 0.0050 },               // 380 nm 
        { 0.1740, 0.0050 },
        { 0.1738, 0.0049 },
        { 0.1736, 0.0049 },
        { 0.1733, 0.0048 },
        { 0.1730, 0.0048 },
        { 0.1726, 0.0048 },
        { 0.1721, 0.0048 },
        { 0.1714, 0.0051 },
        { 0.1703, 0.0058 },
        { 0.1689, 0.0069 },
        { 0.1669, 0.0086 },
        { 0.1644, 0.0109 },
        { 0.1611, 0.0138 },
        { 0.1566, 0.0177 },
        { 0.1510, 0.0227 },
        { 0.1440, 0.0297 },
        { 0.1355, 0.0399 },
        { 0.1241, 0.0578 },
        { 0.1096, 0.0868 },
        { 0.0913, 0.1327 },
        { 0.0687, 0.2007 },
        { 0.0454, 0.2950 },
        { 0.0235, 0.4127 },
        { 0.0082, 0.5384 },
        { 0.0039, 0.6548 },
        { 0.0139, 0.7502 },
        { 0.0389, 0.8120 },
        { 0.0743, 0.8338 },
        { 0.1142, 0.8262 },
        { 0.1547, 0.8059 },
        { 0.1929, 0.7816 },
        { 0.2296, 0.7543 },
        { 0.2658, 0.7243 },
        { 0.3016, 0.6923 },
        { 0.3373, 0.6589 },
        { 0.3731, 0.6245 },
        { 0.4087, 0.5896 },
        { 0.4441, 0.5547 },
        { 0.4788, 0.5202 },
        { 0.5125, 0.4866 },
        { 0.5448, 0.4544 },
        { 0.5752, 0.4242 },
        { 0.6029, 0.3965 },
        { 0.6270, 0.3725 },
        { 0.6482, 0.3514 },
        { 0.6658, 0.3340 },
        { 0.6801, 0.3197 },
        { 0.6915, 0.3083 },
        { 0.7006, 0.2993 },
        { 0.7079, 0.2920 },
        { 0.7140, 0.2859 },
        { 0.7190, 0.2809 },
        { 0.7230, 0.2770 },
        { 0.7260, 0.2740 },
        { 0.7283, 0.2717 },
        { 0.7300, 0.2700 },
        { 0.7311, 0.2689 },
        { 0.7320, 0.2680 },
        { 0.7327, 0.2673 },
        { 0.7334, 0.2666 },
        { 0.7340, 0.2660 },
        { 0.7344, 0.2656 },
        { 0.7346, 0.2654 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 },
        { 0.7347, 0.2653 }  // 780 nm 
    };

class CIETongueWidgetPriv
{
public:

    CIETongueWidgetPriv()
    {
        hMonitorProfile      = 0;
        hXYZProfile          = 0;
        hXFORM               = 0;

        Measurement.Patches  = 0;
        Measurement.Allowed  = 0;
        blinkTimer           = 0;

        profileDataAvailable = true;
        loadingImageMode     = false;
        loadingImageSucess   = false;
        blinkFlag            = false;
    }

    bool             profileDataAvailable;
    bool             loadingImageMode;
    bool             loadingImageSucess;
    bool             blinkFlag;

    double           gridside;

    int              xBias;
    int              yBias;
    int              pxcols;
    int              pxrows;

    QPainter         painter;
    QPixmap          pixmap;
    QTimer          *blinkTimer;

    cmsHPROFILE      hMonitorProfile;
    cmsHPROFILE      hXYZProfile;
    cmsHTRANSFORM    hXFORM;
    cmsCIExyYTRIPLE  Primaries;
    cmsCIEXYZ        MediaWhite;

    MEASUREMENT      Measurement;
};

CIETongueWidget::CIETongueWidget(int w, int h, QWidget *parent, cmsHPROFILE hMonitor)
               : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CIETongueWidgetPriv;
    d->blinkTimer = new QTimer( this );
    setMinimumSize(w, h);
    cmsErrorAction(LCMS_ERROR_SHOW);    

    if (hMonitor)
        d->hMonitorProfile = hMonitor;
    else
        d->hMonitorProfile = cmsCreate_sRGBProfile();

    d->hXYZProfile = cmsCreateXYZProfile();
    d->hXFORM      = cmsCreateTransform(d->hXYZProfile, TYPE_XYZ_16,
                                        d->hMonitorProfile, TYPE_RGB_8,
                                        INTENT_PERCEPTUAL, 0);

    connect(d->blinkTimer, SIGNAL(timeout()),
            this, SLOT(slotBlinkTimerDone()));
}

CIETongueWidget::~CIETongueWidget()
{
    if (d->Measurement.Patches)
        free(d->Measurement.Patches);

    if (d->Measurement.Allowed)
        free(d->Measurement.Allowed);

    cmsDeleteTransform(d->hXFORM);
    cmsCloseProfile(d->hXYZProfile);
    cmsCloseProfile(d->hMonitorProfile);
    delete d;
}

int CIETongueWidget::grids(double val) const
{
    return (int) floor(val * d->gridside + 0.5);
}

bool CIETongueWidget::setProfileData(const QByteArray &profileData)
{    
    if (!profileData.isEmpty())
    {
        cmsHPROFILE hProfile = cmsOpenProfileFromMem(profileData.data(),
                                                    (DWORD)profileData.size());

        if (!hProfile)
        {
            d->profileDataAvailable = false;
            d->loadingImageSucess   = false;
        }
        else
        {
            setProfile(hProfile);
            cmsCloseProfile(hProfile);
            d->profileDataAvailable = true;
            d->loadingImageSucess   = true;
        }
    }
    else
    {
        d->profileDataAvailable = false;
        d->loadingImageSucess   = false;
    }

    d->loadingImageMode = false;
    
    d->blinkTimer->stop();
    repaint(false);
    return (d->profileDataAvailable);
}

bool CIETongueWidget::setProfileFromFile(const KURL& file)
{
    if (!file.isEmpty() && file.isValid())
    {
        cmsHPROFILE hProfile = cmsOpenProfileFromFile(QFile::encodeName(file.path()), "r");

        if (!hProfile)
        {
            d->profileDataAvailable = false;
            d->loadingImageSucess   = false;
        }
        else
        {
            setProfile(hProfile);
            cmsCloseProfile(hProfile);
            d->profileDataAvailable = true;
            d->loadingImageSucess   = true;
        }
    }
    else
    {
        d->profileDataAvailable = false;
        d->loadingImageSucess   = false;
    }

    d->blinkTimer->stop();
    repaint(false);
    return (d->profileDataAvailable);
}

void CIETongueWidget::setProfile(cmsHPROFILE hProfile)
{
    // Get the white point.

    ZeroMemory(&(d->MediaWhite), sizeof(cmsCIEXYZ));
    cmsTakeMediaWhitePoint(&(d->MediaWhite), hProfile);
    cmsCIExyY White;
    cmsXYZ2xyY(&White, &(d->MediaWhite));

    // Get the colorant matrix.

    ZeroMemory(&(d->Primaries), sizeof(cmsCIExyYTRIPLE));

    if (cmsIsTag(hProfile, icSigRedColorantTag) &&
        cmsIsTag(hProfile, icSigGreenColorantTag) &&
        cmsIsTag(hProfile, icSigBlueColorantTag))
    {
        MAT3 Mat;

        if (cmsReadICCMatrixRGB2XYZ(&Mat, hProfile))
        {
            // Undo chromatic adaptation
            if (cmsAdaptMatrixFromD50(&Mat, &White))
            {
                cmsCIEXYZ tmp;

                tmp.X = Mat.v[0].n[0];
                tmp.Y = Mat.v[1].n[0];
                tmp.Z = Mat.v[2].n[0];

                // ScaleToWhite(&MediaWhite, &tmp);
                cmsXYZ2xyY(&(d->Primaries.Red), &tmp);

                tmp.X = Mat.v[0].n[1];
                tmp.Y = Mat.v[1].n[1];
                tmp.Z = Mat.v[2].n[1];
                // ScaleToWhite(&MediaWhite, &tmp);
                cmsXYZ2xyY(&(d->Primaries.Green), &tmp);

                tmp.X = Mat.v[0].n[2];
                tmp.Y = Mat.v[1].n[2];
                tmp.Z = Mat.v[2].n[2];
                // ScaleToWhite(&MediaWhite, &tmp);
                cmsXYZ2xyY(&(d->Primaries.Blue), &tmp);
            }
        }
    }

    // Get target data stored in profile

    ZeroMemory(&(d->Measurement), sizeof(MEASUREMENT));
    char*  CharTarget;
    size_t CharTargetSize;

    if (cmsTakeCharTargetData(hProfile, &CharTarget, &CharTargetSize))
    {
        LCMSHANDLE hSheet = cmsxIT8LoadFromMem(CharTarget, CharTargetSize);
        if (hSheet != NULL)
        {
            cmsxPCollLoadFromSheet(&(d->Measurement),  hSheet);
            cmsxIT8Free(hSheet);
            cmsxPCollValidatePatches(&(d->Measurement), PATCH_HAS_XYZ|PATCH_HAS_RGB);
        }
    }
}

void CIETongueWidget::mapPoint(int& icx, int& icy, LPcmsCIExyY xyY)
{
    icx = (int) floor((xyY->x * (d->pxcols - 1)) + .5);
    icy = (int) floor(((d->pxrows - 1) - xyY->y * (d->pxrows - 1)) + .5);
}

void CIETongueWidget::biasedLine(int x1, int y1, int x2, int y2)
{
    d->painter.drawLine(x1 + d->xBias, y1, x2 + d->xBias, y2);
}

void CIETongueWidget::biasedText(int x, int y, QString Txt)
{
    d->painter.drawText(QPoint(d->xBias + x, y), Txt);
}

QRgb CIETongueWidget::colorByCoord(double x, double y)
{
    // Get xyz components scaled from coordinates

    double cx =       ((double) x) / (d->pxcols - 1);
    double cy = 1.0 - ((double) y) / (d->pxrows - 1);
    double cz = 1.0 - cx - cy;

    // Project xyz to XYZ space. Note that in this
    // particular case we are substituting XYZ with xyz

    cmsCIEXYZ XYZ = { cx , cy , cz };

    WORD XYZW[3];
    BYTE RGB[3];

    cmsFloat2XYZEncoded(XYZW, &XYZ);
    cmsDoTransform(d->hXFORM, XYZW, RGB, 1);

    return qRgb(RGB[0], RGB[1], RGB[2]);
}

void CIETongueWidget::outlineTongue()
{
    int lx = 0, ly = 0;
    int fx=0, fy=0;

    for (int x = 380; x <= 700; x += 5)
    {
        int ix = (x - 380) / 5;

        cmsCIExyY p = {spectral_chromaticity[ix][0], 
                       spectral_chromaticity[ix][1], 1};

        int icx, icy;
        mapPoint(icx, icy, &p);

        if (x > 380)
        {
            biasedLine(lx, ly, icx, icy);
        }
        else
        {
            fx = icx;
            fy = icy;
        }

        lx = icx;
        ly = icy;
     }

    biasedLine(lx, ly, fx, fy);
}

void CIETongueWidget::fillTongue()
{

    QImage Img = d->pixmap.convertToImage();

    int x;

    for (int y = 0; y < d->pxrows; y++)
    {
        int xe = 0;

        // Find horizontal extents of tongue on this line. 

        for (x = 0; x < d->pxcols; x++)
        {
            if ((QColor) Img.pixel(x + d->xBias, y) != Qt::black)
            {
                for (xe = d->pxcols - 1; xe >= x; xe--)
                {
                    if ((QColor) Img.pixel(xe + d->xBias, y) != Qt::black)
                    {
                        break;
                    }
                }

                break;
            }
        }

        if (x < d->pxcols)
        {
            for ( ; x <= xe; x++)
            {
                QRgb Color = colorByCoord(x, y);
                Img.setPixel(x + d->xBias, y, Color);
            }
        }
    }

    d->pixmap.convertFromImage(Img, QPixmap::AvoidDither );
}

void CIETongueWidget::drawTongueAxis()
{
    QFont font;
    font.setPointSize(6);
    d->painter.setFont(font);

    d->painter.setPen(qRgb(255, 255, 255));

    biasedLine(0, 0,          0,          d->pxrows - 1);
    biasedLine(0, d->pxrows-1, d->pxcols-1, d->pxrows - 1);

    for (int y = 1; y <= 9; y += 1)
    {
        QString s;
        int xstart = (y * (d->pxcols - 1)) / 10;
        int ystart = (y * (d->pxrows - 1)) / 10;

        s.sprintf("0.%d", y);
        biasedLine(xstart, d->pxrows - grids(1), xstart,   d->pxrows - grids(4));
        biasedText(xstart - grids(11), d->pxrows + grids(15), s);

        s.sprintf("0.%d", 10 - y);
        biasedLine(0, ystart, grids(3), ystart);    
        biasedText(grids(-25), ystart + grids(5), s);
    }
}

void CIETongueWidget::drawTongueGrid()
{
    d->painter.setPen(qRgb(80, 80, 80));

    for (int y = 1; y <= 9; y += 1)
    {
        int xstart =  (y * (d->pxcols - 1)) / 10;
        int ystart =  (y * (d->pxrows - 1)) / 10;

        biasedLine(xstart, grids(4), xstart,   d->pxrows - grids(4) - 1);
        biasedLine(grids(7), ystart, d->pxcols-1-grids(7), ystart);
    }
}

void CIETongueWidget::drawLabels()
{
    QFont font;
    font.setPointSize(5);
    d->painter.setFont(font);

    for (int x = 450; x <= 650; x += (x > 470 && x < 600) ? 5 : 10)
    {
        QString wl;
        int bx = 0, by = 0, tx, ty;

        if (x < 520)
        {
            bx = grids(-22);
            by = grids(2);
        }
        else if (x < 535)
        {
            bx = grids(-8);
            by = grids(-6);
        }
        else
        {
            bx = grids(4);
        }

        int ix = (x - 380) / 5;

        cmsCIExyY p = {spectral_chromaticity[ix][0],
                       spectral_chromaticity[ix][1], 1};

        int icx, icy;
        mapPoint(icx, icy, &p);

        tx = icx + ((x < 520) ? grids(-2) : ((x >= 535) ? grids(2) : 0));
        ty = icy + ((x < 520) ? 0 : ((x >= 535) ? grids(-1) : grids(-2))); 

        d->painter.setPen(qRgb(255, 255, 255));
        biasedLine(icx, icy, tx, ty);

        QRgb Color = colorByCoord(icx, icy);
        d->painter.setPen(Color);

        wl.sprintf("%d", x);
        biasedText(icx+bx, icy+by, wl);
    }
}

void CIETongueWidget::drawSmallElipse(LPcmsCIExyY xyY, BYTE r, BYTE g, BYTE b, int sz)
{
    int icx, icy;

    mapPoint(icx, icy, xyY);
    d->painter.setPen(qRgb(r, g, b));
    d->painter.drawEllipse(icx + d->xBias- sz/2, icy-sz/2, sz, sz);
}

void CIETongueWidget::drawPatches(void)
{
    for (int i=0; i < d->Measurement.nPatches; i++)
    {
        LPPATCH p = d->Measurement.Patches + i;

        if (d->Measurement.Allowed[i])
        {
            LPcmsCIEXYZ XYZ = &p ->XYZ;
            cmsCIExyY xyY;              
            cmsXYZ2xyY(&xyY, XYZ);

            drawSmallElipse(&xyY,  0, 0, 0, 4);

            if (p->dwFlags & PATCH_HAS_XYZ_PROOF)
            {
                if (p->XYZ.Y < 0.03)
                   continue;

                if (p->XYZProof.Y < 0.03)
                   continue;

                cmsCIExyY Pt;
                cmsXYZ2xyY(&Pt, &p->XYZProof);
                int icx1, icx2, icy1, icy2;

                mapPoint(icx1, icy1, &xyY);
                mapPoint(icx2, icy2, &Pt);

                if (icx2 < 5 || icy2 < 5 || icx1 < 5 || icy1 < 5)
                    continue;

                d->painter.setPen(qRgb(255, 255, 0));
                biasedLine(icx1, icy1, icx2, icy2);
            }
        }
    }
}

void CIETongueWidget::drawColorantTriangle(void)
{
    drawSmallElipse(&(d->Primaries.Red),   255, 128, 128, 6);
    drawSmallElipse(&(d->Primaries.Green), 128, 255, 128, 6);
    drawSmallElipse(&(d->Primaries.Blue),  128, 128, 255, 6);

    int x1, y1, x2, y2, x3, y3;

    mapPoint(x1, y1, &(d->Primaries.Red));
    mapPoint(x2, y2, &(d->Primaries.Green));
    mapPoint(x3, y3, &(d->Primaries.Blue));

    d->painter.setPen(qRgb(255, 255, 255));

    biasedLine(x1, y1, x2, y2);
    biasedLine(x2, y2, x3, y3);
    biasedLine(x3, y3, x1, y1);
}

void CIETongueWidget::sweep_sRGB(void)
{
    int r, g, b;
    cmsHPROFILE hXYZ, hsRGB;

    hXYZ = cmsCreateXYZProfile();
    hsRGB = cmsCreate_sRGBProfile();

    cmsHTRANSFORM xform = cmsCreateTransform(hsRGB, TYPE_RGB_16, hXYZ, TYPE_XYZ_16,
                                             INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
    WORD RGB[3], XYZ[3];
    cmsCIEXYZ xyz, MediaWhite;
    cmsCIExyY xyY, WhitePt;
    int x1, y1;

    cmsTakeMediaWhitePoint(&MediaWhite, hsRGB);
    cmsXYZ2xyY(&WhitePt, &MediaWhite);

    for (r=0; r < 65536; r += 1024)
    {
        for (g=0; g < 65536; g += 1024)
        {
            for (b=0; b < 65536; b += 1024)
            {
                RGB[0] = r; RGB[1] = g; RGB[2] = b;
                cmsDoTransform(xform, RGB, XYZ, 1);
                cmsXYZEncoded2Float(&xyz, XYZ);
                cmsXYZ2xyY(&xyY, &xyz);
                mapPoint(x1, y1, &xyY);
                d->painter.drawPoint(x1 + d->xBias, y1);
            }
        }
    }

    cmsDeleteTransform(xform);
    cmsCloseProfile(hXYZ);
    cmsCloseProfile(hsRGB);
}

void CIETongueWidget::drawWhitePoint(void)
{
    cmsCIExyY Whitem_pntxyY;
    cmsXYZ2xyY(&Whitem_pntxyY, &(d->MediaWhite));
    drawSmallElipse(&Whitem_pntxyY,  255, 255, 255, 8);
}

void CIETongueWidget::loadingStarted()
{
    d->loadingImageMode   = true;
    d->loadingImageSucess = false;
    repaint(false);
    d->blinkTimer->start(200);
}

void CIETongueWidget::loadingFailed()
{
    d->blinkTimer->stop();
    d->loadingImageMode   = false;
    d->loadingImageSucess = false;
    repaint(false);
}

void CIETongueWidget::paintEvent( QPaintEvent * )
{
    d->pixmap = QPixmap(size());
    d->pixmap.setOptimization(QPixmap::BestOptim);

    // Widget is disable : drawing grayed frame.

    if ( !isEnabled() )
    {
        d->painter.begin(&d->pixmap);
        d->painter.fillRect(0, 0, size().width(), size().height(), palette().disabled().background());
        d->painter.setPen(QPen(palette().disabled().foreground(), 1, Qt::SolidLine));
        d->painter.drawRect(0, 0, width(), height());
        d->painter.end();
        bitBlt(this, 0, 0, &d->pixmap);
        return;
    }

    // Loading image mode.

    if (d->loadingImageMode && !d->loadingImageSucess)
    {
        d->painter.begin(&d->pixmap);
        d->painter.fillRect(0, 0, size().width(), size().height(), palette().disabled().background());
        d->painter.setPen(QPen(palette().disabled().foreground(), 1, Qt::SolidLine));
        d->painter.drawRect(0, 0, width(), height());

        if (d->blinkFlag)
            d->painter.setPen(Qt::green);
        else 
            d->painter.setPen(Qt::darkGreen);

        d->painter.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                            i18n("Loading image..."));

        d->painter.end();
        bitBlt(this, 0, 0, &d->pixmap);
        return;
    }

    // No profile data to show.

    if (!d->profileDataAvailable || (!d->loadingImageMode && !d->loadingImageSucess))
    {
        d->painter.begin(&d->pixmap);
        d->painter.fillRect(0, 0, size().width(), size().height(), palette().disabled().background());
        d->painter.setPen(QPen(palette().disabled().foreground(), 1, Qt::SolidLine));
        d->painter.drawRect(0, 0, width(), height());
        d->painter.setPen(Qt::red);
        d->painter.drawText(0, 0, size().width(), size().height(), Qt::AlignCenter,
                            i18n("No profile available..."));

        d->painter.end();
        bitBlt(this, 0, 0, &d->pixmap);
        return;
    }

    // Draw the CIE tongue curve.

    d->pixmap.fill(Qt::black);
    d->painter.begin(&d->pixmap);

    int pixcols = d->pixmap.width();
    int pixrows = d->pixmap.height();

    d->gridside = (QMIN(pixcols, pixrows)) / 512.0;
    d->xBias = grids(32);
    d->yBias = grids(20);
    d->pxcols = pixcols - d->xBias;
    d->pxrows = pixrows - d->yBias;

    d->painter.setBackgroundColor(qRgb(0, 0, 0));
    d->painter.setPen(qRgb(255, 255, 255));

    outlineTongue();
    fillTongue();

    drawTongueAxis();
    drawLabels();
    drawTongueGrid();

    if (d->MediaWhite.Y > 0.0)
        drawWhitePoint();

    if (d->Primaries.Red.Y != 0.0)
        drawColorantTriangle();

    if (d->Measurement.Patches && d->Measurement.Allowed)
        drawPatches();

    d->painter.end();

    bitBlt(this, 0, 0, &d->pixmap);
}

void CIETongueWidget::slotBlinkTimerDone(void)
{
    repaint(false);     
    d->blinkFlag = !d->blinkFlag;
    d->blinkTimer->start( 200 );
}

}  // namespace Digikam

#include "cietonguewidget.moc"

