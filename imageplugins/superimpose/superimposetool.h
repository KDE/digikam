/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SUPERIMPOSETOOL_H
#define SUPERIMPOSETOOL_H

// Local includes

#include "editortool.h"

class KUrl;

namespace DigikamSuperImposeImagesPlugin
{

class SuperImposeToolPriv;

class SuperImposeTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    SuperImposeTool(QObject* parent);
    ~SuperImposeTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotTemplateDirChanged(const KUrl& url);
    void slotRootTemplateDirChanged(void);

private:

    void readSettings();
    void writeSettings();
    void populateTemplates(void);
    void finalRendering();

private:

    SuperImposeToolPriv* const d;
};

}  // namespace DigikamSuperImposeImagesPlugin

#endif /* SUPERIMPOSETOOL_H */
