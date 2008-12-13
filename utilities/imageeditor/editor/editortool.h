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

class DImgThreadedFilter;
class EditorToolSettings;
class EditorToolPriv;

class DIGIKAM_EXPORT EditorTool : public QObject
{
    Q_OBJECT

public:

    EditorTool(QObject *parent);
    virtual ~EditorTool();

    void                init();

    QString             toolHelp() const;
    QString             toolName() const;
    QPixmap             toolIcon() const;
    QWidget*            toolView() const;
    EditorToolSettings* toolSettings() const;

signals:

    void okClicked();
    void cancelClicked();

protected:

    void setToolHelp(const QString& anchor);
    void setToolName(const QString& name);
    void setToolIcon(const QPixmap& icon);

    virtual void setToolView(QWidget *view);
    virtual void setToolSettings(EditorToolSettings *settings);
    virtual void setBusy(bool);
    virtual void readSettings();
    virtual void writeSettings();
    virtual void finalRendering(){};

protected slots:

    void slotTimer();

    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotInit();
    virtual void slotLoadSettings(){};
    virtual void slotSaveAsSettings(){};
    virtual void slotResetSettings();
    virtual void slotEffect(){};

private:

    EditorToolPriv *d;
};

// -----------------------------------------------------------------

class EditorToolThreadedPriv;

class DIGIKAM_EXPORT EditorToolThreaded : public EditorTool
{
    Q_OBJECT

public:

    enum RenderingMode
    {
        NoneRendering=0,
        PreviewRendering,
        FinalRendering
    };

public:

    EditorToolThreaded(QObject *parent);
    virtual ~EditorToolThreaded();

    /** Set the small text to show in editor status progress bar during 
        tool computation. If it's not set, tool name is used instead.
     */
    void setProgressMessage(const QString& mess);

    /** return the current tool rendering mode.
     */
    RenderingMode renderingMode() const;

public slots:

    virtual void slotAbort();

protected:

    DImgThreadedFilter* filter() const;
    void setFilter(DImgThreadedFilter *filter);

    void customEvent(QCustomEvent *event);

    virtual void setToolView(QWidget *view);
    virtual void prepareEffect(){};
    virtual void prepareFinal(){};
    virtual void putPreviewData(){};
    virtual void putFinalData(){};
    virtual void renderingFinished(){};

protected slots:

    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotEffect();

private slots:

    void slotResized();

private:

    EditorToolThreadedPriv *d;
};

}  //namespace Digikam

#endif /* IMAGEPLUGIN_H */
