/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "expoblendingmanager.h"

// Local includes

#include "expoblendingwizard.h"
#include "expoblendingdlg.h"
#include "expoblendingthread.h"
#include "alignbinary.h"
#include "enfusebinary.h"

namespace Digikam
{

class ExpoBlendingManager::Private
{
public:

    explicit Private()
      : thread(0),
        wizard(0),
        dlg(0)
    {
    }

    QList<QUrl>             inputUrls;

    ExpoBlendingItemUrlsMap preProcessedUrlsMap;

    ExpoBlendingThread*     thread;

    AlignBinary             alignBinary;
    EnfuseBinary            enfuseBinary;

    ExpoBlendingWizard*     wizard;
    ExpoBlendingDlg*        dlg;
};

QPointer<ExpoBlendingManager> ExpoBlendingManager::internalPtr = QPointer<ExpoBlendingManager>();

ExpoBlendingManager::ExpoBlendingManager(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->thread = new ExpoBlendingThread(this);

    connect(&d->enfuseBinary, SIGNAL(signalEnfuseVersion(double)),
            this, SLOT(slotSetEnfuseVersion(double)));

    if (d->enfuseBinary.isValid())
    {
        slotSetEnfuseVersion(d->enfuseBinary.getVersion());
    }
}

ExpoBlendingManager::~ExpoBlendingManager()
{
    delete d->thread;
    delete d->wizard;
    delete d->dlg;
    delete d;
}

ExpoBlendingManager* ExpoBlendingManager::instance()
{
    if (ExpoBlendingManager::internalPtr.isNull())
    {
        ExpoBlendingManager::internalPtr = new ExpoBlendingManager();
    }

    return ExpoBlendingManager::internalPtr;
}

bool ExpoBlendingManager::isCreated()
{
    return (!internalPtr.isNull());
}

bool ExpoBlendingManager::checkBinaries()
{
    if (!d->alignBinary.recheckDirectories())
    {
        return false;
    }

    if (!d->enfuseBinary.recheckDirectories())
    {
        return false;
    }

    return true;
}

AlignBinary& ExpoBlendingManager::alignBinary() const
{
    return d->alignBinary;
}

EnfuseBinary& ExpoBlendingManager::enfuseBinary() const
{
    return d->enfuseBinary;
}

void ExpoBlendingManager::setItemsList(const QList<QUrl>& urls)
{
    d->inputUrls = urls;
}

QList<QUrl>& ExpoBlendingManager::itemsList() const
{
    return d->inputUrls;
}

void ExpoBlendingManager::setPreProcessedMap(const ExpoBlendingItemUrlsMap& urls)
{
    d->preProcessedUrlsMap = urls;
}

ExpoBlendingItemUrlsMap& ExpoBlendingManager::preProcessedMap() const
{
    return d->preProcessedUrlsMap;
}

ExpoBlendingThread* ExpoBlendingManager::thread() const
{
    return d->thread;
}

void ExpoBlendingManager::run()
{
    startWizard();
}

void ExpoBlendingManager::cleanUp()
{
    d->thread->cleanUpResultFiles();
}

void ExpoBlendingManager::startWizard()
{
    if (d->wizard && (d->wizard->isMinimized() || !d->wizard->isHidden()))
    {
        d->wizard->showNormal();
        d->wizard->activateWindow();
        d->wizard->raise();
    }
    else if (d->dlg && (d->dlg->isMinimized() || !d->dlg->isHidden()))
    {
        d->dlg->showNormal();
        d->dlg->activateWindow();
        d->dlg->raise();
    }
    else
    {
        delete d->wizard;
        delete d->dlg;
        d->dlg = 0;

        d->wizard = new ExpoBlendingWizard(this);

        connect(d->wizard, SIGNAL(accepted()),
                this, SLOT(slotStartDialog()));

        d->wizard->show();
    }
}

void ExpoBlendingManager::slotStartDialog()
{
    d->inputUrls = d->wizard->itemUrls();

    d->dlg = new ExpoBlendingDlg(this);
    d->dlg->show();
}

void ExpoBlendingManager::slotSetEnfuseVersion(double version)
{
    d->thread->setEnfuseVersion(version);
}

} // namespace Digikam
