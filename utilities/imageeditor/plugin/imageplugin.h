/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins interface for image editor.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_H
#define IMAGEPLUGIN_H

// Qt includes

#include <QtCore/QObject>

// KDE includes

#include <kxmlguiclient.h>

// Local includes

#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class EditorTool;

class DIGIKAM_EXPORT ImagePlugin : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:

    explicit ImagePlugin(QObject* const parent, const char* name = 0);
    virtual ~ImagePlugin();

    virtual void setEnabledSelectionActions(bool enable);
    virtual void setEnabledActions(bool enable);

    void loadTool(EditorTool* const tool);

    QString actionCategory() const;

protected:

    /** Used to set actions category into KDE config shortcuts dialog for Image EditorTool.
     *  One plugin can host more than one tool with more than one action shortcuts.
     *  Define a category, will group actions by plugin name in shortcuts list.
     */
    void setActionCategory(const QString& cat);

private Q_SLOTS:

    void slotToolDone();
};

}  //namespace Digikam

#endif /* IMAGEPLUGIN_H */
