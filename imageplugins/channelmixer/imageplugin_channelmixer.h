/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_CHANNELMIXER_H
#define IMAGEPLUGIN_CHANNELMIXER_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class ImagePlugin_ChannelMixer : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_ChannelMixer(QObject *parent, const QVariantList& args);
    ~ImagePlugin_ChannelMixer();

    void setEnabledActions(bool enable);

private Q_SLOTS:

    void slotChannelMixer();

private:

    KAction *m_channelMixerAction;
};

#endif /* IMAGEPLUGIN_CHANNELMIXER_H */
