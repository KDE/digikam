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

    if (action.identifier() == "transform:rotate" && action.version() == 1)
    {
        int angle = action.parameter("angle").toInt();

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
    else if (action.identifier() == "transform:flip" && action.version() == 1)
    {
        QString direction = action.parameter("direction").toString();

        if (direction == "vertical")
        {
            m_type = FlipVertically;
        }
        else
        {
            m_type = FlipHorizontally;
        }
    }
    else if (action.identifier() == "transform:crop" && action.version() == 1)
    {
        m_type     = Crop;
        int x      = action.parameter("x").toInt();
        int y      = action.parameter("y").toInt();
        int width  = action.parameter("width").toInt();
        int height = action.parameter("height").toInt();
        m_arg      = QRect(x, y, width, height);
    }
    else if (action.identifier() == "transform:resize" && action.version() == 1)
    {
        m_type     = Resize;
        int width  = action.parameter("width").toInt();
        int height = action.parameter("height").toInt();
        m_arg      = QSize(width, height);
    }
    else if (action.identifier() == "transform:convertDepth" && action.version() == 1)
    {
        int depth = action.parameter("depth").toInt();

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
            action = FilterAction("transform:rotate", 1);
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

            action.addParameter("angle", angle);
            break;
        }

        case FlipHorizontally:
        case FlipVertically:
        {
            action = FilterAction("transform:flip", 1);
            action.addParameter("direction", m_type == FlipHorizontally ? "horizontal" : "vertical");
            break;
        }

        case Crop:
        {
            action  = FilterAction("transform:crop", 1);
            QRect r = m_arg.toRect();
            action.addParameter("x",      r.x());
            action.addParameter("y",      r.y());
            action.addParameter("width",  r.width());
            action.addParameter("height", r.height());
            break;
        }

        case Resize:
        {
            action  = FilterAction("transform:resize", 1);
            QSize s = m_arg.toSize();
            action.addParameter("width",  s.width());
            action.addParameter("height", s.height());
            break;
        }

        case ConvertTo8Bit:
        case ConvertTo16Bit:
        {
            action = FilterAction("transform:convertDepth", 1);
            action.addParameter("depth", m_type == ConvertTo8Bit ? 8 : 16);
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
    return QStringList() << "transform:rotate"
                         << "transform:flip"
                         << "transform:crop"
                         << "transform:resize"
                         << "transform:convertDepth";
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
            return I18N_NOOP("Rotate Right");

        case Rotate180:
            return I18N_NOOP("Rotate 180°");

        case Rotate270:
            return I18N_NOOP("Rotate Left");

        case FlipHorizontally:
            return I18N_NOOP("Flip Horizontally");

        case FlipVertically:
            return I18N_NOOP("Flip Vertically");

        case Crop:
            return I18N_NOOP("Crop");

        case Resize:
            return I18N_NOOP("Resize");

        case ConvertTo8Bit:
            return I18N_NOOP("Convert to 8 Bit");

        case ConvertTo16Bit:
            return I18N_NOOP("Convert to 16 Bit");
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
            return "object-rotate-left";

        case Rotate180:
            return "transform-rotate";

        case Rotate270:
            return "object-rotate-right";

        case FlipHorizontally:
            return "object-flip-horizontal";

        case FlipVertically:
            return "object-flip-vertical";

        case Crop:
            return "transform-crop";

        case Resize:
            return "transform-scale";

        case ConvertTo8Bit:
            return "depth16to8";

        case ConvertTo16Bit:
            return "depth8to16";
    }

    return QString();
}

QString DImgBuiltinFilter::i18nDisplayableName(const QString& filterIdentifier)
{
    if (filterIdentifier == "transform:rotate")
    {
        return i18nc("Rotate image", "Rotate");
    }
    else if (filterIdentifier == "transform:flip")
    {
        return i18nc("Flip image", "Flip");
    }
    else if (filterIdentifier == "transform:crop")
    {
        return i18nc("Crop image", "Crop");
    }
    else if (filterIdentifier == "transform:resize")
    {
        return i18nc("Resize image", "Resize");
    }
    else if (filterIdentifier == "transform:convertDepth")
    {
        return i18nc("Convert image bit depth", "Convert Depth");
    }

    return QString();
}

QString DImgBuiltinFilter::filterIcon(const QString& filterIdentifier)
{
    if (filterIdentifier == "transform:rotate")
    {
        return "transform-rotate";
    }
    else if (filterIdentifier == "transform:flip")
    {
        return "object-flip-horizontal";
    }
    else if (filterIdentifier == "transform:crop")
    {
        return "transform-crop";
    }
    else if (filterIdentifier == "transform:resize")
    {
        return "transform-scale";
    }
    else if (filterIdentifier == "transform:convertDepth")
    {
        return "fill-color";
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
