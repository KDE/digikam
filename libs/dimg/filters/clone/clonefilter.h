/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-01
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-01 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef CLONEFILTER_H
#define CLONEFILTER_H

// Qt includes

#include <QSize>
#include <QColor>
#include <QPoint>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT CloneFilter : public DImgThreadedFilter
{

public:

        explicit CloneFilter(QObject* parent = 0);
        explicit CloneFilter(const DImg* orgImage, constDImg *Mask, const QPoint dis, QObject *parent = 0 ) ;
        ~CloneFilter();

        static QString          FilterIdentifier()
        {
            return "digikam:CloneFilter";
        }

        static QString          DisplayableName()
        {
            return I18N_NOOP("Clone Tool");
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
        void    readParameters(const FilterAction& action);

private:

        void filterImage();
        void divergents(float* I, float* O);
        bool inimage(DImg* *img, int x, int y );

public:

        DImg* getResultImg;

private:

        QColor MASK_BG;
        QPoint dis;
        DImg*  originalImage; // original image
        DImg*  maskImage;     // mask image
        DImg*  resultImage;
   };

} // namespace Digikam

#endif // CLONEFILTER_H

