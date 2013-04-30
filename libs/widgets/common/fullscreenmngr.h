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

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_export.h"

class KXmlGuiWindow;
class KToggleFullScreenAction;

namespace Digikam
{

/** Data container to use in managed window.
 */
class DIGIKAM_EXPORT FullScreenMngr
{
public:

    FullScreenMngr();
    virtual ~FullScreenMngr();

    /** Set instance of managed window
     */
    void setManagedWindow(KXmlGuiWindow* const win);

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
    bool                     m_fullScreenHideToolBar;

    /** Settigns taken from managed window configuration to handle thumbbar visibility in full-screen mode
     */
    bool                     m_fullScreenHideThumbBar;

    /** Action from window used to sxitch fullscreen state*/
    KToggleFullScreenAction* m_fullScreenAction;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* FULLSCREENMNGR_H */
