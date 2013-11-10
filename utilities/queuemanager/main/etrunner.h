/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2010-11-15
 * Description : a kipi plugin to export images to Yandex.Fotki web service
 *
 * Copyright (C) 2010 by Roman Tsisyk <roman at tsisyk dot com>
 *
 * GUI based on PicasaWeb KIPI Plugin
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ET_RUNNER_H
#define ET_RUNNER_H

#include <QEventLoop>

#include <kprocess.h>

#include "etwidget.h"

class ETRunner : public QObject
{
    Q_OBJECT

public:

    ETRunner(QObject* parent, const QString& tool, const QList<KUrl>& urls);
    ~ETRunner();

    Q_SLOT bool run();
    Q_SLOT void terminate();
    void wait();

    Q_SIGNAL void error(const QString& title, const QString& text);
    Q_SIGNAL void started();
    Q_SIGNAL void finished();

    const ETConfig::Ptr& config() const
    {
        return toolcfg_;
    }

private Q_SLOTS:

    void onStarted();
    void onError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    ETConfig::Ptr toolcfg_;
    QList<KUrl> urls_;
    KProcess* process_;
};



#endif /* ET_DIALOG_H */
