/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


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

    void closeEvent(QCloseEvent *e);

protected Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotDetectFaces();
    void slotGotImagePreview(const LoadingDescription&, const DImg&);


private:

    BatchFaceDetectorPriv* const d;

};

}
#endif // BATCHFACEDETECTOR_H
