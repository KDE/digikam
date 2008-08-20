/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EDITORTOOL_H
#define EDITORTOOL_H

// Qt includes.

#include <QObject>
#include <QString>
#include <QPixmap>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class EditorToolPriv;

class DIGIKAM_EXPORT EditorTool : public QObject
{
    Q_OBJECT

public:

    EditorTool(QObject *parent);
    virtual ~EditorTool();

    QString  toolName() const;
    QPixmap  toolIcon() const;
    QWidget* toolView() const;
    QWidget* toolSettings() const;

signals:

    void okClicked();
    void cancelClicked();

protected:

    void setToolName(const QString& name);
    void setToolIcon(const QPixmap& icon);
    void setToolView(QWidget *view);
    void setToolSettings(QWidget *settings);

    virtual void readSettings();
    virtual void saveSettings();
    virtual void setBusy(bool)=0;

private:

    EditorToolPriv *d;
};

}  //namespace Digikam

#endif /* IMAGEPLUGIN_H */
