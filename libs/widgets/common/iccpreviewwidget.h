/* ============================================================
 * Author: Francisco J. Cruz <fj.cruz@supercable.es>
 * Date  : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 * 
 * Copyright 2006 by Francisco J. Cruz
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

#ifndef ICCPREVIEWWIDGET_H
#define ICCPREVIEWWIDGET_H

// KDE includes.

#include <kpreviewwidgetbase.h>
#include <kurl.h>

// Local includes.

#include "digikam_export.h"

#include LCMS_HEADER

namespace Digikam
{

class ICCPreviewWidgetPriv;

class DIGIKAM_EXPORT ICCPreviewWidget : public KPreviewWidgetBase
{

Q_OBJECT

public:

    ICCPreviewWidget(QWidget *parent);
    ~ICCPreviewWidget();

public slots:

    virtual void showPreview(const KURL &url);
    virtual void clearPreview();

protected:

    virtual void virtual_hook(int, void*){};

private:

    void getICCData(const KURL &url);

private :

    ICCPreviewWidgetPriv* d;

};

} // namespace Digikam

#endif /* ICCPREVIEWWIDGET_H */
