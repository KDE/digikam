/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-10
 * Description : basic filter management for DImg builtin methods
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "dimgbuiltinfilter.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "filteraction.h"

namespace Digikam
{

DImgBuiltinFilter::DImgBuiltinFilter()
    : m_type(NoOperation)
{
}

DImgBuiltinFilter::DImgBuiltinFilter(const FilterAction& action)
{
    setAction(action);
}

DImgBuiltinFilter::DImgBuiltinFilter(Type type, const QVariant& arg)
{
    setAction(type, arg);
}

void DImgBuiltinFilter::setAction(const FilterAction& action)
{
    m_type = NoOperation;

    if (action.identifier() == QLatin1String("transform:rotate") && action.version() == 1)
    {
        int angle = action.parameter(QLatin1String("angle")).toInt();

        if (angle == 90)
        {
            m_type = Rotate90;
        }
        else if (angle == 180)
        {
            m_type = Rotate180;
        }
        else
        {
            m_type = Rotate270;
        }
    }
    else if (action.identifier() == QLatin1String("transform:flip") && action.version() == 1)
    {
        QString direction = action.parameter(QLatin1String("direction")).toString();

        if (direction == QLatin1String("vertical"))
        {
            m_type = FlipVertically;
        }
        else
        {
            m_type = FlipHorizontally;
        }
    }
    else if (action.identifier() == QLatin1String("transform:crop") && action.version() == 1)
    {
        m_type     = Crop;
        int x      = action.parameter(QLatin1String("x")).toInt();
        int y      = action.parameter(QLatin1String("y")).toInt();
        int width  = action.parameter(QLatin1String("width")).toInt();
        int height = action.parameter(QLatin1String("height")).toInt();
        m_arg      = QRect(x, y, width, height);
    }
    else if (action.identifier() == QLatin1String("transform:resize") && action.version() == 1)
    {
        m_type     = Resize;
        int width  = action.parameter(QLatin1String("width")).toInt();
        int height = action.parameter(QLatin1String("height")).toInt();
        m_arg      = QSize(width, height);
    }
    else if (action.identifier() == QLatin1String("transform:convertDepth") && action.version() == 1)
    {
        int depth = action.parameter(QLatin1String("depth")).toInt();

        if (depth == 16)
        {
            m_type = ConvertTo16Bit;
        }
        else
        {
            m_type = ConvertTo8Bit;
        }
    }
}

void DImgBuiltinFilter::setAction(Type type, const QVariant& arg)
{
    m_type = type;
    m_arg  = arg;
}

bool DImgBuiltinFilter::isValid() const
{
    switch (m_type)
    {
        case NoOperation:
            return false;

        case Crop:
            return m_arg.type() == QVariant::Rect;

        case Resize:
            return m_arg.type() == QVariant::Size;

        default:
            return true;
    }
}

void DImgBuiltinFilter::apply(DImg& image) const
{
    switch (m_type)
    {
        case NoOperation:
            break;

        case Rotate90:
            image.rotate(DImg::ROT90);
            break;

        case Rotate180:
            image.rotate(DImg::ROT180);
            break;

        case Rotate270:
            image.rotate(DImg::ROT270);
            break;

        case FlipHorizontally:
            image.flip(DImg::HORIZONTAL);
            break;

        case FlipVertically:
            image.flip(DImg::VERTICAL);
            break;

        case Crop:
            image.crop(m_arg.toRect());
            break;

        case Resize:
        {
            QSize s = m_arg.toSize();
            image.resize(s.width(), s.height());
            break;
        }

        case ConvertTo8Bit:
            image.convertToEightBit();
            break;

        case ConvertTo16Bit:
            image.convertToSixteenBit();
            break;
    }
}

FilterAction DImgBuiltinFilter::filterAction() const
{
    FilterAction action;

    switch (m_type)
    {
        case NoOperation:
        default:
            return action;

        case Rotate90:
        case Rotate180:
        case Rotate270:
        {
            action = FilterAction(QLatin1String("transform:rotate"), 1);
            int angle;

            if (m_type == Rotate90)
            {
                angle = 90;
            }
            else if (m_type == Rotate180)
            {
                angle = 180;
            }
            else
            {
                angle = 270;
            }

            action.addParameter(QLatin1String("angle"), angle);
            break;
        }

        case FlipHorizontally:
        case FlipVertically:
        {
            action = FilterAction(QLatin1String("transform:flip"), 1);
            action.addParameter(QLatin1String("direction"), m_type == FlipHorizontally ? QLatin1String("horizontal") : QLatin1String("vertical"));
            break;
        }

        case Crop:
        {
            action  = FilterAction(QLatin1String("transform:crop"), 1);
            QRect r = m_arg.toRect();
            action.addParameter(QLatin1String("x"),      r.x());
            action.addParameter(QLatin1String("y"),      r.y());
            action.addParameter(QLatin1String("width"),  r.width());
            action.addParameter(QLatin1String("height"), r.height());
            break;
        }

        case Resize:
        {
            action  = FilterAction(QLatin1String("transform:resize"), 1);
            QSize s = m_arg.toSize();
            action.addParameter(QLatin1String("width"),  s.width());
            action.addParameter(QLatin1String("height"), s.height());
            break;
        }

        case ConvertTo8Bit:
        case ConvertTo16Bit:
        {
            action = FilterAction(QLatin1String("transform:convertDepth"), 1);
            action.addParameter(QLatin1String("depth"), m_type == ConvertTo8Bit ? 8 : 16);
            break;
        }
    }

    action.setDisplayableName(displayableName());
    return action;
}

DImgBuiltinFilter DImgBuiltinFilter::reverseFilter() const
{
    switch (m_type)
    {
        case Rotate90:
            return DImgBuiltinFilter(Rotate270);

        case Rotate180:
            return DImgBuiltinFilter(Rotate180);

        case Rotate270:
            return DImgBuiltinFilter(Rotate90);

        case FlipHorizontally:
        case FlipVertically:
            return DImgBuiltinFilter(m_type);

        case Crop:
        case Resize:
        case ConvertTo8Bit:
        case ConvertTo16Bit:
        case NoOperation:
        default:
            return DImgBuiltinFilter();
    }
}

bool DImgBuiltinFilter::isReversible() const
{
    return reverseFilter().isValid();
}

QStringList DImgBuiltinFilter::supportedFilters()
{
    return QStringList() << QLatin1String("transform:rotate")
                         << QLatin1String("transform:flip")
                         << QLatin1String("transform:crop")
                         << QLatin1String("transform:resize")
                         << QLatin1String("transform:convertDepth");
}

QList<int> DImgBuiltinFilter::supportedVersions(const QString& filterIdentifier)
{
    QList<int> versions;

    // So far, all filters are at version 1
    if (isSupported(filterIdentifier))
    {
        versions << 1;
    }

    return versions;
}

QString DImgBuiltinFilter::i18nDisplayableName() const
{
    QByteArray latin1 = displayableName().toLatin1();
    return i18n(latin1.data());
}

QString DImgBuiltinFilter::displayableName() const
{
    switch (m_type)
    {
        case NoOperation:
            break;

        case Rotate90:
            return QString::fromUtf8(I18N_NOOP("Rotate Right"));

        case Rotate180:
            return QString::fromUtf8(I18N_NOOP("Rotate 180°"));

        case Rotate270:
            return QString::fromUtf8(I18N_NOOP("Rotate Left"));

        case FlipHorizontally:
            return QString::fromUtf8(I18N_NOOP("Flip Horizontally"));

        case FlipVertically:
            return QString::fromUtf8(I18N_NOOP("Flip Vertically"));

        case Crop:
            return QString::fromUtf8(I18N_NOOP("Crop"));

        case Resize:
            return QString::fromUtf8(I18N_NOOP("Resize"));

        case ConvertTo8Bit:
            return QString::fromUtf8(I18N_NOOP("Convert to 8 Bit"));

        case ConvertTo16Bit:
            return QString::fromUtf8(I18N_NOOP("Convert to 16 Bit"));
    }

    return QString();
}

QString DImgBuiltinFilter::filterIcon() const
{
    switch (m_type)
    {
        case NoOperation:
            break;

        case Rotate90:
            return QLatin1String("object-rotate-left");

        case Rotate180:
            return QLatin1String("transform-rotate");

        case Rotate270:
            return QLatin1String("object-rotate-right");

        case FlipHorizontally:
            return QLatin1String("object-flip-horizontal");

        case FlipVertically:
            return QLatin1String("object-flip-vertical");

        case Crop:
            return QLatin1String("transform-crop");

        case Resize:
            return QLatin1String("transform-scale");

        case ConvertTo8Bit:
            return QLatin1String("depth16to8");

        case ConvertTo16Bit:
            return QLatin1String("depth8to16");
    }

    return QString();
}

QString DImgBuiltinFilter::i18nDisplayableName(const QString& filterIdentifier)
{
    if (filterIdentifier == QLatin1String("transform:rotate"))
    {
        return i18nc("Rotate image", "Rotate");
    }
    else if (filterIdentifier == QLatin1String("transform:flip"))
    {
        return i18nc("Flip image", "Flip");
    }
    else if (filterIdentifier == QLatin1String("transform:crop"))
    {
        return i18nc("Crop image", "Crop");
    }
    else if (filterIdentifier == QLatin1String("transform:resize"))
    {
        return i18nc("Resize image", "Resize");
    }
    else if (filterIdentifier == QLatin1String("transform:convertDepth"))
    {
        return i18nc("Convert image bit depth", "Convert Depth");
    }

    return QString();
}

QString DImgBuiltinFilter::filterIcon(const QString& filterIdentifier)
{
    if (filterIdentifier == QLatin1String("transform:rotate"))
    {
        return QLatin1String("transform-rotate");
    }
    else if (filterIdentifier == QLatin1String("transform:flip"))
    {
        return QLatin1String("object-flip-horizontal");
    }
    else if (filterIdentifier == QLatin1String("transform:crop"))
    {
        return QLatin1String("transform-crop");
    }
    else if (filterIdentifier == QLatin1String("transform:resize"))
    {
        return QLatin1String("transform-scale");
    }
    else if (filterIdentifier == QLatin1String("transform:convertDepth"))
    {
        return QLatin1String("fill-color");
    }

    return QString();
}

bool DImgBuiltinFilter::isSupported(const QString& filterIdentifier)
{
    return filterIdentifier.startsWith(QLatin1String("transform:")) && supportedFilters().contains(filterIdentifier);
}

bool DImgBuiltinFilter::isSupported(const QString& filterIdentifier, int version)
{
    if (!isSupported(filterIdentifier))
    {
        return false;
    }

    // at the moment, all filters are at version 1
    return version == 1;
}

// -------------------------------------------------------------------------------------------------------------------

class DImgBuiltinThreadedFilter : public DImgThreadedFilter
{
public:

    explicit DImgBuiltinThreadedFilter(const DImgBuiltinFilter& filter, DImg* const orgImage, QObject* const parent = 0)
        : DImgThreadedFilter(orgImage, parent), m_filter(filter)
    {
    }

    explicit DImgBuiltinThreadedFilter(const DImgBuiltinFilter& filter, QObject* const parent = 0)
        : DImgThreadedFilter(parent), m_filter(filter)
    {
    }

    virtual QString filterIdentifier() const
    {
        return m_filter.filterAction().identifier();
    }

    virtual FilterAction filterAction()
    {
        return m_filter.filterAction();
    }

    void readParameters(const FilterAction& action)
    {
        m_filter = DImgBuiltinFilter(action);
    }

protected:

    void filterImage()
    {
        m_destImage = m_orgImage;
        m_filter.apply(m_destImage);
    }

    DImgBuiltinFilter m_filter;
};

DImgThreadedFilter* DImgBuiltinFilter::createThreadedFilter(DImg* const orgImage, QObject* const parent) const
{
    return new DImgBuiltinThreadedFilter(*this, orgImage, parent);
}

DImgThreadedFilter* DImgBuiltinFilter::createThreadedFilter(QObject* const parent) const
{
    return new DImgBuiltinThreadedFilter(*this, parent);
}

} // namespace Digikam
