/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PLECONFIGDIALOG_H
#define PLECONFIGDIALOG_H

#include <QDebug>

#include <kconfigdialog.h>

namespace PhotoLayoutsEditor
{
    class PLEConfigDialogPrivate;

    class PLEConfigDialog : public KConfigDialog
    {
            PLEConfigDialogPrivate* d;

        public:

            explicit PLEConfigDialog(QWidget * parent = 0);
            ~PLEConfigDialog();

            virtual void updateSettings();
            virtual void updateWidgets();

        friend class PLEConfigDialogPrivate;
    };
}

#endif // PLECONFIGDIALOG_H
