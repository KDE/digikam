/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : image editor printing interface.
 *
 * Copyright (C) 2009 by Angelo Naselli <anaselli at linux dot it>
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

#ifndef DIGIKAM_PRINT_HELPER_H
#define DIGIKAM_PRINT_HELPER_H

// Local includes

#include "digikam_export.h"
#include "dimg.h"

class QWidget;

using namespace Digikam;

namespace DigikamEditorPrintToolPlugin
{

class DIGIKAM_EXPORT PrintHelper
{

public:

    explicit PrintHelper(QWidget* const parent);
    ~PrintHelper();

    void print(DImg&);

private:

    PrintHelper(const PrintHelper&); // Disable

    class Private;
    Private* const d;
};

} // namespace DigikamEditorPrintToolPlugin

#endif // DIGIKAM_PRINT_HELPER_H
