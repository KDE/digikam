/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGFILTERMANAGER_H
#define DIMGFILTERMANAGER_H

#include <QStringList>
#include <QList>
#include <QString>

#include "dimgfiltergenerator.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgFilterManager : public DImgFilterGenerator
{
public:

    static DImgFilterManager* instance();

    QStringList supportedFilters();
    QList<int> supportedVersions(const QString& filterIdentifier);
    QString displayableName(const QString& filterIdentifier);

    QString getFilterIcon(const QString& filterIdentifier);

    bool isSupported(const QString& filterIdentifier);
    bool isSupported(const QString& filterIdentifier, int version);

    DImgThreadedFilter* createFilter(const QString& filterIdentifier, int version);

    /**
     * Registers all filter provided by this generator.
     */
    void addGenerator(DImgFilterGenerator* generator);
    void removeGenerator(DImgFilterGenerator* generator);

private:

    DImgFilterManager();
    ~DImgFilterManager();

private:

    friend class DImgFilterManagerCreator;

    class DImgFilterManagerPriv;
    DImgFilterManagerPriv* const d;
};

} // namespace Digikam

#endif // DIMGFILTERMANAGER_H
