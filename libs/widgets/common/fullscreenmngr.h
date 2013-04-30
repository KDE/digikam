/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen manager for digiKam XML GUI windows
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

#ifndef FULLSCREENMNGR_H
#define FULLSCREENMNGR_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"

class QAction;

class KXmlGuiWindow;
class KToggleFullScreenAction;

namespace Digikam
{

/** Data container to use in managed window.
 */
class DIGIKAM_EXPORT FullScreenMngr
{
public:

    explicit FullScreenMngr();
    virtual ~FullScreenMngr();

    /** Set instance of managed window
     */
    void setManagedWindow(KXmlGuiWindow* const win);

    /** Create Full-screen action to action collection instance from managed window
     *  set through setManagedWindow(). This action must be connected to relevant slot in managed window instance.
     *  Use fullScreenAction() to get instance of created action.
     *  'name' is action name used in KDE UI rc file.
     */
    QAction* createFullScreenAction(const QString& name);

    /** Read and write settings from/to KDE config file
     */
    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

    /** Switch Window 'win' to full screen mode.
     *  'set' argument will set or reset the fullscreen state of window.
     */
    void switchWindowToFullScreen(bool set);

    /** Active full-screen action from managed window. Called typically when Escape key is pressed.
     */
    void escapePressed();

public:

    /** Settigns taken from managed window configuration to handle toolbar visibility  in full-screen mode
     */
    bool m_fullScreenHideToolBar;

    /** Settigns taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool m_fullScreenHideThumbBar;

private:

    class Private;
    Private* const d;
};

// -------------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT FullScreenSettings : public QWidget
{

public:

    enum FullScreenOptions
    {
        TOOLBAR  = 0x00000001,
        THUMBBAR = 0x00000002
    };

public:

    explicit FullScreenSettings(int options, QWidget* const parent);
    virtual ~FullScreenSettings();

    void readSettings(const KConfigGroup& group);
    void saveSettings(KConfigGroup& group);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* FULLSCREENMNGR_H */
