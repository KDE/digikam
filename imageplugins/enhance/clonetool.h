/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-12
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-07-12 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef CLONETOOL_H
#define CLONETOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class CloneTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    CloneTool(QObject* parent);
    ~CloneTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotSettingsChanged();
    void slotStrokeOver();

private:

    void readSettings();
    void writeSettings();
    //void prepareEffect();//FIXME
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

private:

    class CloneToolPriv;
    CloneToolPriv* const d;
};

} // namespace DigikamEnhanceImagePlugin

#endif // CLONETOOL_H
