/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef HTML_WIZARD_H
#define HTML_WIZARD_H

// Qt includes

#include <QWizard>

namespace Digikam
{

class GalleryInfo;
class DHistoryView;
class DProgressWdg;

/**
 * The wizard used by the user to select the various settings.
 */
class HTMLWizard : public QWizard
{
    Q_OBJECT

public:

    HTMLWizard(QWidget* const parent, GalleryInfo* const info);
    ~HTMLWizard();

    DHistoryView* progressView() const;
    DProgressWdg* progressBar()  const;

protected Q_SLOTS:

    virtual void accept();

private Q_SLOTS:

    void updateCollectionSelectorPageValidity();
    void slotThemeSelectionChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // HTML_WIZARD_H
