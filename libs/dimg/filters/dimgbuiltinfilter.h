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

#ifndef DIMGBUILTINFILTERS_H
#define DIMGBUILTINFILTERS_H

// Qt includes

#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;
class DImgThreadedFilter;
class FilterAction;

class DIGIKAM_EXPORT DImgBuiltinFilter
{
public:

    enum Type
    {
        NoOperation,
        Rotate90,
        Rotate180,
        Rotate270,
        FlipHorizontally,
        FlipVertically,
        Crop,             /// Argument: QRect
        Resize,           /// Argument: QSize
        ConvertTo8Bit,
        ConvertTo16Bit
    };

public:

    /// Create a filter performing no operation
    DImgBuiltinFilter();

    /**
     * Create a filter for the given action. If the action is not supported,
     * the filter will perform no operation.
     */
    explicit DImgBuiltinFilter(const FilterAction& action);

    /**
     * Create a filter of the given type.
     * See documentation of Type for required arguments.
     */
    explicit DImgBuiltinFilter(Type type, const QVariant& arg = QVariant());

    /// same as constructor
    void setAction(const FilterAction& action);
    void setAction(Type type, const QVariant& arg = QVariant());

    /**
     * Returns the reverse action of this filter.
     * If the current action is not revertible, returns an invalid filter.
     */
    DImgBuiltinFilter reverseFilter() const;
    bool isReversible()               const;

    /**
     * Checks that the action is supported and valid arguments are set
     */
    bool isValid() const;

    /// Apply the described change to the given image reference
    void apply(DImg& image) const;

    /**
     * NOTE: The following methods are also accessed by the more general
     * DImgFilterManager methods, so you usually do not need to call these directly.
     */

    /// Returns the FilterAction describing this filter.
    FilterAction filterAction() const;

    /// Returns a displayableName for this filter
    QString displayableName()     const;
    QString i18nDisplayableName() const;
    QString filterIcon()          const;

    /**
     * Returns a DImgThreadedFilter which executes this builtin action.
     */
    DImgThreadedFilter* createThreadedFilter(QObject* const parent = 0)                       const;
    DImgThreadedFilter* createThreadedFilter(DImg* const orgImage, QObject* const parent = 0) const;

public:

    static QString i18nDisplayableName(const QString& filterIdentifier);
    static QString filterIcon(const QString& filterIdentifier);

    static QStringList supportedFilters();

    /// Returns a list of supported versions of the given filter
    static QList<int> supportedVersions(const QString& filterIdentifier);

    /// Returns if the given filter and version are supported by DImgBuiltinFilter
    static bool isSupported(const QString& filterIdentifier);
    static bool isSupported(const QString& filterIdentifier, int version);

protected:

    Type     m_type;
    QVariant m_arg;
};

} // namespace Digikam

#endif // DIMGFILTERMANAGER_H
