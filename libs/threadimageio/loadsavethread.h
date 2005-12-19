/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-12-17
 * Description : image file IO threaded interface.
 *
 * Copyright 2005 by Marcel Wiesweg, Gilles Caulier
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

#ifndef LOAD_SAVE_THREAD_H
#define LOAD_SAVE_THREAD_H

// Qt includes.

#include <qthread.h>
#include <qobject.h>
#include <qmutex.h>
#include <qptrlist.h>
#include <qwaitcondition.h>

// Digikam includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LoadSaveThread : public QObject, public QThread
{
    
    Q_OBJECT

public:

    LoadSaveThread();
    ~LoadSaveThread();
    
    void load(const QString& filePath);
    void save(DImg &image, const QString& filePath, const char* format);

signals:

    void signalImageLoaded(const QString& filePath, const DImg& img);
    void signalLoadingProgress(const QString& filePath, float progress);
    void signalImageSaved(const QString& filePath);
    void signalSavingProgress(const QString& filePath, float progress);
    
public:

    virtual void imageLoaded(const QString& filePath, const DImg& img)
            { emit signalImageLoaded(filePath, img); };
            
    virtual void loadingProgress(const QString& filePath, float progress)
            { emit signalLoadingProgress(filePath, progress); };
            
    virtual void imageSaved(const QString& filePath)
            { emit signalImageSaved(filePath); };
            
    virtual void savingProgress(const QString& filePath, float progress)
            { emit signalSavingProgress(filePath, progress); };

protected:
    
    virtual void run();
    virtual void customEvent(QCustomEvent *event);

private:

    QMutex               m_mutex;

    QWaitCondition       m_condVar;

    QPtrList<class Task> m_todo;

    bool                 m_running;
};

}      // namespace Digikam

#endif // LOAD_SAVE_THREAD_H
