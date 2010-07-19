/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef BATCHFACEDETECTOR_H
#define BATCHFACEDETECTOR_H

// Qt includes

#include <QCloseEvent>

// Local includes

#include "dprogressdlg.h"

class QWidget;
class KUrl;
class QPixmap;

namespace Digikam
{

class DImg;
class LoadingDescription;
class BatchFaceDetectorPriv;

class BatchFaceDetector : public DProgressDlg
{
    Q_OBJECT

public:

    explicit BatchFaceDetector(QWidget* parent, bool rebuildAll);
    ~BatchFaceDetector();

Q_SIGNALS:

    void signalDetectAllFacesDone();

private:

    void abort();
    void complete();
    void processOne();

protected:

    void closeEvent(QCloseEvent* e);

protected Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotDetectFaces();
    void slotGotImagePreview(const LoadingDescription&, const DImg&);

private:

    BatchFaceDetectorPriv* const d;
};

} // namespace Digikam

#endif // BATCHFACEDETECTOR_H
