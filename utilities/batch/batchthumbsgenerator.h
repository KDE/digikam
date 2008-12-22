/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : batch thumbnails generator
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BATCHTHUMBSGENERATOR_H
#define BATCHTHUMBSGENERATOR_H

// Local includes.

#include "dprogressdlg.h"

class QWidget;
class QPixmap;

class KURL;

namespace Digikam
{

class BatchThumbsGeneratorPriv;

class BatchThumbsGenerator : public DProgressDlg
{
    Q_OBJECT

public:

    BatchThumbsGenerator(QWidget* parent);
    ~BatchThumbsGenerator();

signals:

    void signalRebuildThumbsDone();
    void signalRebuildAllThumbsDone();

private:

    void rebuildAllThumbs(int size);
    void abort();

protected:
    
    void closeEvent(QCloseEvent *e);

protected slots:

    void slotCancel();
    
private slots:

    void slotRebuildThumbs128();
    void slotRebuildThumbs256();
    void slotRebuildThumbDone(const KURL& url, const QPixmap& pix=QPixmap());
    void slotRebuildAllThumbComplete();

private:

    BatchThumbsGeneratorPriv *d;
};

}  // namespace Digikam

#endif /* BATCHTHUMBSGENERATOR_H */
