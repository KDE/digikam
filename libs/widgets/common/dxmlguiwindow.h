/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DXMLGUIWINDOW_H
#define DXMLGUIWINDOW_H

// Qt includes

#include <QWidget>
#include <QObject>

// KDE includes

#include <kxmlguiwindow.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"

class QEvent;

class KToolBar;

namespace Digikam
{

/** Optional parts which can be hiddedn or not from managed window configuration panel
 */
enum FullScreenOptions
{
    FS_TOOLBARS = 0x00000001,
    FS_THUMBBAR = 0x00000002,
    FS_SIDEBARS = 0x00000004,
    FS_NONE     = 0x00000008,
    FS_DEFAULT  = FS_TOOLBARS | FS_THUMBBAR    // Image Editor config.
};

static const QString s_configFullScreenHideToolBarsEntry("FullScreen Hide ToolBars");
static const QString s_configFullScreenHideThumbBarEntry("FullScreen Hide ThumbBar");
static const QString s_configFullScreenHideSideBarsEntry("FullScreen Hide SideBars");

/** Data container to use in managed window.
 */
class DIGIKAM_EXPORT DXmlGuiWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:

    explicit DXmlGuiWindow(QWidget* const parent=0, Qt::WindowFlags f=KDE_DEFAULT_WINDOWFLAGS);
    virtual ~DXmlGuiWindow();

    /** Set full-screen options to managed window
     */
    void setFullScreenOptions(int options);

    /** Create Full-screen action to action collection instance from managed window
     *  set through setManagedWindow(). This action is connected to slotToggleFullScreen() slot.
     *  'name' is action name used in KDE UI rc file.
     */
    void createFullScreenAction(const QString& name);

    /** Read full-screen settings from KDE config file.
     */
    void readFullScreenSettings(const KConfigGroup& group);

    /** Switch Window 'win' to full screen mode.
     *  'set' argument will set or reset the fullscreen state of window.
     */
    void switchWindowToFullScreen(bool set);

    /** Return true if managed window is currently in Full Screen Mode
     */
    bool fullScreenIsActive() const;

protected:

    bool eventFilter(QObject* obj, QEvent* ev);
    void keyPressEvent(QKeyEvent* e);

    /** Re-implement this method if you want to manage sidebars visibility in full-screen mode.
     *  By default this method do nothing.
     */
    virtual void showSideBars(bool visible);

    /** Re-implement this method if you want to manage thumbbar visibility in full-screen mode.
     *  By default this method do nothing.
     */
    virtual void showThumbBar(bool visible);

    /** Re-implement this method if you want to manage customized view visibility in full-screen mode.
     *  By default this method do nothing.
     */
    virtual void showCustomizedView(bool visible);

    /** Re-implement this method if managed window has a thumbbar. This must return visibility state of it.
     */
    virtual bool thumbbarVisibility() const;

private Q_SLOTS:

    void slotToggleFullScreen(bool);

private:

    /** Used by slotToggleFullScreen() to switch tool-bar visibility in managed window
     */
    void showToolBars(bool visible);

    /** Return main tool bar instance created in managed window.
     */
    KToolBar* mainToolBar() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* DXMLGUIWINDOW_H */
