/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MAIL_FINAL_PAGE_H
#define MAIL_FINAL_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class MailFinalPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit MailFinalPage(QWizard* const dialog, const QString& title);
    ~MailFinalPage();

    void initializePage();
    bool isComplete() const;
    void cleanupPage();

private Q_SLOTS:

    void slotDone();
    void slotProcess();
    void slotMessage(const QString&, bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // MAIL_FINAL_PAGE_H
