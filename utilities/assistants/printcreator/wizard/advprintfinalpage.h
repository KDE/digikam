/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
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

#ifndef ADV_PRINT_FINAL_PAGE_H
#define ADV_PRINT_FINAL_PAGE_H

// Qt includes

#include <QString>
#include <QPainter>
#include <QList>
#include <QRect>
#include <QPrinter>
#include <QStringList>

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class AdvPrintPhoto;
class AdvPrintPhotoSize;

class AdvPrintFinalPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintFinalPage(QWizard* const dialog, const QString& title);
    ~AdvPrintFinalPage();

    void initializePage();
    bool isComplete() const;
    void cleanupPage();

    void printPhotos(const QList<AdvPrintPhoto*>& photos,
                     const QList<QRect*>& layouts,
                     QPrinter& printer);

    QStringList printPhotosToFile(const QList<AdvPrintPhoto*>& photos,
                                  const QString& baseFilename,
                                  AdvPrintPhotoSize* const layouts);

    bool paintOnePage(QPainter& p,
                      const QList<AdvPrintPhoto*>& photos,
                      const QList<QRect*>& layouts,
                      int& current,
                      bool cropDisabled,
                      bool useThumbnails = false);

    void removeGimpFiles();

private:

    void printCaption(QPainter& p,
                      AdvPrintPhoto* const photo,
                      int captionW,
                      int captionH,
                      const QString& caption);

    double getMaxDPI(const QList<AdvPrintPhoto*>& photos,
                     const QList<QRect*>& layouts,
                     int current);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_FINAL_PAGE_H
