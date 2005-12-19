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

// Qt includes.

#include <qapplication.h>

// Locale includes.

#include "loadsavethread.h"

namespace Digikam
{

class Task
{
public:

    virtual void execute(LoadSaveThread* thread) = 0;
};

//---------------------------------------------------------------------------------------------------

class NotifyEvent : public QCustomEvent
{
public:

    static QEvent::Type notifyEventId()
        { return QEvent::User; };
    
    NotifyEvent() : QCustomEvent(notifyEventId()) {};
    
    virtual void notify(LoadSaveThread *thread) = 0;
};

//---------------------------------------------------------------------------------------------------

class LoadedEvent : public NotifyEvent
{
public:

    LoadedEvent(const QString &filePath, DImg &img) : filePath(filePath), img(img) {};
        
    virtual void notify(LoadSaveThread *thread)
        { thread->imageLoaded(filePath, img); };
        
private:
    
    QString filePath;
    DImg    img;
};

//---------------------------------------------------------------------------------------------------

class LoadingTask : public Task
{
public:
    
    LoadingTask(const QString &filePath) : filePath(filePath) {}

    virtual void execute(LoadSaveThread *thread)
    {
        DImg img(filePath);
        QApplication::postEvent(thread, new LoadedEvent(filePath, img));
    };
    
private:

    QString filePath;
};

//---------------------------------------------------------------------------------------------------

class SavedEvent : public NotifyEvent
{
public:
    
    SavedEvent(const QString &filePath) : filePath(filePath) {};
    
    virtual void notify(LoadSaveThread *thread)
    {
        thread->imageSaved(filePath);
    };
    
private:

    QString filePath;
};

//---------------------------------------------------------------------------------------------------

class SavingTask : public Task
{
public:

    SavingTask(DImg &img, const QString &filePath, const char* format) : img(img), filePath(filePath), format(format) {};
    
    virtual void execute(LoadSaveThread *thread)
    {
        img.save(filePath, format);
        QApplication::postEvent(thread, new SavedEvent(filePath));
    };
    
private:

    DImg        img;
    QString     filePath;
    const char* format;
};

//---------------------------------------------------------------------------------------------------

LoadSaveThread::LoadSaveThread()
{
    running = true;
    start();
}

LoadSaveThread::~LoadSaveThread()
{
    running = false;
    {
        QMutexLocker lock(&mutex);
        condVar.wakeAll();
    }
    
    // put some sort of stop function into DImg loading?
    wait();
}

void LoadSaveThread::load(const QString& filePath)
{
    QMutexLocker lock(&mutex);
    todo.append(new LoadingTask(filePath));
    condVar.wakeAll();
}

void LoadSaveThread::save(DImg &image, const QString& filePath, const char* format)
{
    QMutexLocker lock(&mutex);
    todo.append(new SavingTask(image, filePath, format));
    condVar.wakeAll();
}

void LoadSaveThread::run()
{
    while (running)
    {
        Task *task = 0;
        {
            QMutexLocker lock(&mutex);
            task = todo.getFirst();
            if (task)
                todo.removeFirst();
            else
                condVar.wait(1000);
        }
        if (task)
        {
            task->execute(this);
            delete task;
        }
    }
}

void LoadSaveThread::customEvent(QCustomEvent *event)
{
    if (event->type() == NotifyEvent::notifyEventId())
    {
        ((NotifyEvent *)event)->notify(this);
    }
}

}   // namespace Digikam

#include "loadsavethread.moc"
