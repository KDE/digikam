/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-14
 * Description : a dialog to display image file save options.
 *
 * Copyright (C) 2009 by David Eriksson <meldavid at acc umu se>
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

#include "filesaveoptionsdlg.h"

// KDE includes

#include <klocale.h>

// Local includes

namespace Digikam
{

FileSaveOptionsDlg::FileSaveOptionsDlg(QWidget* const parent, FileSaveOptionsBox* const options)
    : KDialog(parent)
{
    setCaption(i18n("Settings for Saving Image File"));
    setButtons(Cancel|Ok);
    setDefaultButton(Ok);
    setMainWidget(options);
}

FileSaveOptionsDlg::~FileSaveOptionsDlg()
{
}

}  // namespace Digikam
