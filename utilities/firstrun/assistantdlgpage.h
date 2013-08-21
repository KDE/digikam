/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ASSISTANT_DLG_PAGE_H
#define ASSISTANT_DLG_PAGE_H

// Qt includes

#include <QScrollArea>
#include <QString>
#include <QPixmap>

class KAssistantDialog;
class KPageWidgetItem;

namespace Digikam
{

class AssistantDlgPage : public QScrollArea
{

public:

    AssistantDlgPage(KAssistantDialog* const dlg, const QString& title);
    virtual ~AssistantDlgPage();

    KPageWidgetItem* page() const;

    void setPageWidget(QWidget* const w);
    void setLeftBottomPix(const QPixmap& pix);

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif /* ASSISTANT_DLG_PAGE_H */
