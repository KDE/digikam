/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to apply film grain
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "filmgrainplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "filmgrain.h"

namespace DigikamBqmFilmGrainPlugin
{

FilmGrainPlugin::FilmGrainPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

FilmGrainPlugin::~FilmGrainPlugin()
{
}

QString FilmGrainPlugin::name() const
{
    return i18n("Film Grain");
}

QString FilmGrainPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FilmGrainPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("filmgrain"));
}

QString FilmGrainPlugin::description() const
{
    return i18n("A tool to add film grain");
}

QString FilmGrainPlugin::details() const
{
    return i18n("<p>This Batch Queue Manager tool can add film grain over images.</p>");
}

QList<DPluginAuthor> FilmGrainPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2019"))
            ;
}

void FilmGrainPlugin::setup(QObject* const parent)
{
    FilmGrain* const tool = new FilmGrain(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmFilmGrainPlugin
