/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef GREYCSTORATIONFILTER_H
#define GREYCSTORATIONFILTER_H

// Qt includes

#include <QImage>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class DIGIKAM_EXPORT GreycstorationContainer
{

public:

    enum INTERPOLATION
    {
        NearestNeighbor = 0,
        Linear,
        RungeKutta
    };

public:

    GreycstorationContainer()
    {
        setRestorationDefaultSettings();
    };

    ~GreycstorationContainer()
    {
    };

    void setRestorationDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 1;
        interp     = NearestNeighbor;

        amplitude  = 60.0;
        sharpness  = 0.7F;
        anisotropy = 0.3F;
        alpha      = 0.6F;
        sigma      = 1.1F;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

    void setInpaintingDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 30;
        interp     = NearestNeighbor;

        amplitude  = 20.0;
        sharpness  = 0.3F;
        anisotropy = 1.0;
        alpha      = 0.8F;
        sigma      = 2.0;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

    void setResizeDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 3;
        interp     = NearestNeighbor;

        amplitude  = 20.0;
        sharpness  = 0.2F;
        anisotropy = 0.9F;
        alpha      = 0.1F;
        sigma      = 1.5;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

public:

    bool  fastApprox;

    int   tile;
    int   btile;

    uint  nbIter;
    uint  interp;

    float amplitude;
    float sharpness;
    float anisotropy;
    float alpha;
    float sigma;
    float gaussPrec;
    float dl;
    float da;
};

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT GreycstorationFilter : public DImgThreadedFilter
{

public:

    enum MODE
    {
        Restore = 0,
        InPainting,
        Resize,
        SimpleResize    // Mode to resize image without to use Greycstoration algorithm.
    };

public:

    /** Contructor without argument. Before to use it,
        you need to call in order: setSettings(), setMode(), optionally setInPaintingMask(),
        setOriginalImage(), and necessary setup() at end.
     */
    explicit GreycstorationFilter(QObject* const parent=0);

    /** Contructor with all arguments. Ready to use.
     */
    GreycstorationFilter(DImg* const orgImage,
                         const GreycstorationContainer& settings,
                         int mode=Restore,
                         int newWidth=0, int newHeight=0,
                         const QImage& inPaintingMask=QImage(),
                         QObject* const parent=0);

    ~GreycstorationFilter();

    void setMode(int mode, int newWidth=0, int newHeight=0);
    void setSettings(const GreycstorationContainer& settings);
    void setInPaintingMask(const QImage& inPaintingMask);

    void setup();

    virtual void cancelFilter();

    static QString cimgVersionString();

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:GreycstorationFilter");
    }

    static QString          DisplayableName()
    {
        return QString::fromUtf8(I18N_NOOP("Greycstoration Filter"));
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }

    virtual FilterAction    filterAction();
    void                    readParameters(const FilterAction& action);

private:

    void computeChildrenThreads();
    void restoration();
    void inpainting();
    void resize();
    void simpleResize();
    void iterationLoop(uint iter);

    virtual void initFilter();
    virtual void filterImage();

private:


    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* GREYCSTORATIONFILTER_H */
