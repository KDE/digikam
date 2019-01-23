/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_EXPO_BLENDING_MANAGER_H
#define DIGIKAM_EXPO_BLENDING_MANAGER_H

// Qt includes

#include <QObject>
#include <QPointer>
#include <QUrl>

// Local includes

#include "dplugingeneric.h"
#include "expoblendingactions.h"
#include "digikam_export.h"

using namespace Digikam;

namespace GenericExpoBlendingPlugin
{

class ExpoBlendingThread;
class AlignBinary;
class EnfuseBinary;

class DIGIKAM_EXPORT ExpoBlendingManager : public QObject
{
    Q_OBJECT

public:

    explicit ExpoBlendingManager(QObject* const parent = 0);
    ~ExpoBlendingManager();

    static QPointer<ExpoBlendingManager> internalPtr;
    static ExpoBlendingManager*          instance();
    static bool                          isCreated();

    bool checkBinaries();

    void setItemsList(const QList<QUrl>& urls);
    QList<QUrl>& itemsList() const;

    void setPlugin(DPlugin* const plugin);

    void setPreProcessedMap(const ExpoBlendingItemUrlsMap& urls);
    ExpoBlendingItemUrlsMap& preProcessedMap() const;

    ExpoBlendingThread* thread() const;
    AlignBinary&  alignBinary()  const;
    EnfuseBinary& enfuseBinary() const;

    void run();

    /**
     * Clean up all temporary files produced so far.
     */
    void cleanUp();

private Q_SLOTS:

    void slotStartDialog();
    void slotSetEnfuseVersion(double version);

private:

    void startWizard();

private:

    class Private;
    Private* const d;
};

} // namespace GenericExpoBlendingPlugin

#endif // DIGIKAM_EXPO_BLENDING_MANAGER_H
