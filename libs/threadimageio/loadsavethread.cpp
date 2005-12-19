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

    LoadedEvent(const QString &filePath, DImg &img)
        : m_filePath(filePath), m_img(img)
        {};
        
    virtual void notify(LoadSaveThread *thread)
        { thread->imageLoaded(m_filePath, m_img); };
        
private:
    
    QString m_filePath;
    DImg    m_img;
};

//---------------------------------------------------------------------------------------------------

class LoadingTask : public Task
{
public:
    
    LoadingTask(const QString &filePath)
        : m_filePath(filePath)
        {}

    virtual void execute(LoadSaveThread *thread)
    {
        DImg img(m_filePath);
        QApplication::postEvent(thread, new LoadedEvent(m_filePath, img));
    };
    
private:

    QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class SavedEvent : public NotifyEvent
{
public:
    
    SavedEvent(const QString &filePath)
        : m_filePath(filePath)
        {};
    
    virtual void notify(LoadSaveThread *thread)
    {
        thread->imageSaved(m_filePath);
    };
    
private:

    QString m_filePath;
};

//---------------------------------------------------------------------------------------------------

class SavingTask : public Task
{
public:

    SavingTask(DImg &img, const QString &filePath, const char* format)
        : m_img(img), m_filePath(filePath), m_format(format)
        {};
    
    virtual void execute(LoadSaveThread *thread)
    {
        m_img.save(m_filePath, m_format);
        QApplication::postEvent(thread, new SavedEvent(m_filePath));
    };
    
private:

    DImg        m_img;
    QString     m_filePath;
    const char* m_format;
};

//---------------------------------------------------------------------------------------------------

LoadSaveThread::LoadSaveThread()
{
    m_running = true;
    start();
}

LoadSaveThread::~LoadSaveThread()
{
    m_running = false;
    {
        QMutexLocker lock(&m_mutex);
        m_condVar.wakeAll();
    }
    
    // put some sort of stop function into DImg loading?
    wait();
}

void LoadSaveThread::load(const QString& filePath)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new LoadingTask(filePath));
    m_condVar.wakeAll();
}

void LoadSaveThread::save(DImg &image, const QString& filePath, const char* format)
{
    QMutexLocker lock(&m_mutex);
    m_todo.append(new SavingTask(image, filePath, format));
    m_condVar.wakeAll();
}

void LoadSaveThread::run()
{
    while (m_running)
    {
        Task *task = 0;
        {
            QMutexLocker lock(&m_mutex);
            task = m_todo.getFirst();
            if (task)
                m_todo.removeFirst();
            else
                m_condVar.wait(1000);
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
