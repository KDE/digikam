/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEEFFECT_SUPERIMPOSE_H
#define IMAGEEFFECT_SUPERIMPOSE_H

// KDE includes.

#include <kurl.h>

// Digikam includes.

#include "editortool.h"

class QPushButton;

namespace Digikam
{
class ThumbBarView;
class EditorToolSettings;
}

namespace DigikamSuperImposeImagesPlugin
{

class DirSelectWidget;
class SuperImposeWidget;

class SuperImposeTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    SuperImposeTool(QObject* parent);
    ~SuperImposeTool();

private slots:

    void slotTemplateDirChanged(const KURL& url);
    void slotRootTemplateDirChanged();
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void populateTemplates();
    void finalRendering();

private:

    KURL                         m_templatesUrl;
    KURL                         m_templatesRootUrl;

    Digikam::ThumbBarView       *m_thumbnailsBar;

    Digikam::EditorToolSettings *m_gboxSettings;

    SuperImposeWidget           *m_previewWidget;

    DirSelectWidget             *m_dirSelect;
};

}  // NameSpace DigikamSuperImposeImagesPlugin

#endif /* IMAGEEFFECT_SUPERIMPOSE_H */
