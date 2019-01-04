/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : plugin to create video slideshow.
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

#include "videoslideshowplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"

namespace Digikam
{

VideoSlideShowPlugin::VideoSlideShowPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

QString VideoSlideShowPlugin::name() const
{
    return i18n("Video Slideshow");
}

QString VideoSlideShowPlugin::id() const
{
    return QLatin1String("VideoSlideShow");
}

QIcon VideoSlideShowPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("media-record"));
}

QString VideoSlideShowPlugin::description() const
{
    return i18n("A Tool to Create Video Slideshow.");
}

QString VideoSlideShowPlugin::details() const
{
    return i18n("<p>This tool permit to back-process image as frame to create video slide-show.</p>"
                "<p>Items to process can be selected one by one or by group through a selection of albums.</p>"
                "<p>Different visual effects can be apllied to images to make transitions while to render target video.</p>");
}

QList<DPluginAuthor> VideoSlideShowPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2012-2019"),
                             i18n("Author and Maintainer"))
            << DPluginAuthor(QLatin1String("A Janardhan Reddy"),
                             QLatin1String("annapareddyjanardhanreddy at gmail dot com"),
                             QLatin1String("(C) 2012"))
            ;
}

void VideoSlideShowPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create video slideshow..."));
    ac->setObjectName(QLatin1String("videoslideshow"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotVideoSlideShow()));

    addAction(ac);
}

void VideoSlideShowPlugin::slotVideoSlideShow()
{
    QPointer<VidSlideWizard> wzrd = new VidSlideWizard(0, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace Digikam
