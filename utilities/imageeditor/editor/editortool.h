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

#include <qobject.h>
#include <qstring.h>
#include <qpixmap.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class EditorToolSettings;
class EditorToolPriv;

class DIGIKAM_EXPORT EditorTool : public QObject
{
    Q_OBJECT

public:

    EditorTool(QObject *parent);
    virtual ~EditorTool();

    QString             toolName() const;
    QPixmap             toolIcon() const;
    QWidget*            toolView() const;
    EditorToolSettings* toolSettings() const;

signals:

    void okClicked();
    void cancelClicked();

protected:

    void setToolName(const QString& name);
    void setToolIcon(const QPixmap& icon);
    void setToolView(QWidget *view);
    void setToolSettings(EditorToolSettings *settings);

    virtual void readSettings();
    virtual void writteSettings();
    virtual void finalRendering(){};
    virtual void setBusy(bool){};

protected slots:

    void slotTimer();
    void slotOk();
    void slotCancel();

    virtual void slotLoadSettings(){};
    virtual void slotSaveAsSettings(){};
    virtual void slotResetSettings();
    virtual void slotEffect(){};

private:

    EditorToolPriv *d;
};

}  //namespace Digikam

#endif /* IMAGEPLUGIN_H */

