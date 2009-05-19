/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CHANNELMIXERTOOL_H
#define CHANNELMIXERTOOL_H

// Local includes

#include "editortool.h"

namespace Digikam
{
class DColor;
}

namespace DigikamChannelMixerImagesPlugin
{

class ChannelMixerToolPriv;

class ChannelMixerTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    ChannelMixerTool(QObject *parent);
    ~ChannelMixerTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void adjustSliders();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetCurrentChannel();
    void slotResetSettings();
    void slotEffect();
    void slotChannelChanged();
    void slotGainsChanged();
    void slotMonochromeActived(bool mono);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);

private:

    ChannelMixerToolPriv* const d;
};

}  // namespace DigikamChannelMixerImagesPlugin

#endif /* CHANNELMIXERTOOL_H */
