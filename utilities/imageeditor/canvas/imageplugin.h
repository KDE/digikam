/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image plugins interface for image editor.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    explicit ImagePlugin(QObject* parent, const char* name=0);
    virtual ~ImagePlugin();

    virtual void setEnabledSelectionActions(bool enable);
    virtual void setEnabledActions(bool enable);

    void loadTool(EditorTool* tool);

    QString actionCategory() const;

protected:

    void setActionCategory(const QString& name);

private Q_SLOTS:

    void slotToolDone();

private:

    class ImagePluginPriv;
    ImagePluginPriv* const d;
};

}  //namespace Digikam

#endif /* IMAGEPLUGIN_H */
