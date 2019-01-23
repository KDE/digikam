/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : a tool to show image using an OpenGL interface.
 *
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_GLVIEWERPLUGIN_GLVIEWERHELPDLG_H
#define DIGIKAM_GLVIEWERPLUGIN_GLVIEWERHELPDLG_H

// Local includes

#include "dplugin.h"
#include "dplugindialog.h"

using namespace Digikam;

namespace GenericGLViewerPlugin
{

class GLViewerHelpDlg : public DPluginDialog
{

public:

    explicit GLViewerHelpDlg(DPlugin* const plugin);
    ~GLViewerHelpDlg();
};

} // namespace GenericGLViewerPlugin

#endif // DIGIKAM_GLVIEWERPLUGIN_GLVIEWERHELPDLG_H
