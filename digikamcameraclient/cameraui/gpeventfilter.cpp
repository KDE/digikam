/* ============================================================
 * File  : gpeventfilter.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <qevent.h>

#include "camerauiview.h"
#include "gpevents.h"
#include "gpeventfilter.h"


GPEventFilter::GPEventFilter(QObject* parent)
    : QObject(parent)
{
    parent->installEventFilter(this);
    view_ = static_cast<CameraUIView *>(parent);
}

GPEventFilter::~GPEventFilter()
{
    
}

bool GPEventFilter::eventFilter(QObject *, QEvent *e)
{

    if (e->type() < QCustomEvent::User) {
        return false;
    }

    
    switch (e->type()) {

    case(GPEvent::Init): {
        view_->cameraInitialized(true);
        break;
    }

    case (GPEvent::Error): {
        GPEventError *event(static_cast<GPEventError *>(e));
        view_->cameraErrorMsg(event->errorMsg());
        break;
    }

    case (GPEvent::GetSubFolders): {
        GPEventGetSubFolders
            *event(static_cast<GPEventGetSubFolders *>(e));

        QString folder(event->folder());
        MTList<QString> subFolderList(event->subFolderList());

        for (int i=0; i<subFolderList.count(); i++) {
            view_->cameraSubFolder(folder, subFolderList[i]);
        }

        break;
    }

    case (GPEvent::GetItemsInfo): {
        GPEventGetItemsInfo
            *event(static_cast<GPEventGetItemsInfo *>(e));

        QString folder(event->folder());
        MTList<GPFileItemInfo> mtList(event->infoList());

        GPFileItemInfoList infoList;

        GPFileItemInfoList::const_iterator it;
        for (it = mtList.begin(); it != mtList.end(); it++)
            infoList.append(*it);

        view_->cameraNewItems(folder, infoList);

        break;
    }

    case (GPEvent::GetAllItemsInfo): {
        GPEventGetAllItemsInfo
            *event(static_cast<GPEventGetAllItemsInfo *>(e));

        MTList<GPFileItemInfo> mtList(event->infoList());

        GPFileItemInfoList infoList;

        GPFileItemInfoList::const_iterator it;
        for (it = mtList.begin(); it != mtList.end(); it++)
            infoList.append(*it);

        view_->cameraNewItems(infoList);

        break;
    }

    case(GPEvent::GetThumbnail): {
        GPEventGetThumbnail
            *event(static_cast<GPEventGetThumbnail *>(e));

        view_->cameraNewThumbnail(event->folder(),
                               event->imageName(),
                               event->thumbnail());
        break;
    }

    case(GPEvent::DownloadItem): {
        GPEventDownloadItem
            *event(static_cast<GPEventDownloadItem *>(e));

        view_->cameraDownloadedItem(event->folder(),
                                    event->itemName());
        break;
    }

    case(GPEvent::DeleteItem): {
        GPEventDeleteItem
            *event(static_cast<GPEventDeleteItem *>(e));

        view_->cameraDeletedItem(event->folder(),
                                 event->itemName());
        break;
    }

    case(GPEvent::DeleteAllItems): {
        view_->cameraDeletedAllItems();
        break;
    }

    case(GPEvent::OpenItem): {
        GPEventOpenItem
            *event(static_cast<GPEventOpenItem *>(e));

        view_->cameraOpenedItem(event->openFile());
        break;
    }

    case(GPEvent::OpenItemWithService): {
        GPEventOpenItemWithService
            *event(static_cast<GPEventOpenItemWithService *>(e));

        view_->cameraOpenedItem(event->openFile(),
                                event->serviceName());
        break;
    }

    case(GPEvent::ExifInfo): {
        GPEventExifInfo
            *event(static_cast<GPEventExifInfo *>(e));

        view_->cameraExifInfo(event->folder(),
                              event->itemName(),
                              event->data(),
                              event->size());
        break;
    }
        
    case(GPEvent::StatusMsg): {
        GPEventStatusMsg
            *event(static_cast<GPEventStatusMsg *>(e));

        emit signalStatusMsg(event->msg());
        break;
    }

    case(GPEvent::Progress): {
        GPEventProgress
            *event(static_cast<GPEventProgress *>(e));

        emit signalProgressVal(event->val());
        break;
    }

    case(GPEvent::Busy): {
        GPEventBusy
            *event(static_cast<GPEventBusy *>(e));

        emit signalBusy(event->busy());
        break;
    }

    default: {
        qWarning("Event Filter: Unknown Event");
        break;
    }
    }
        
        
    
    // eat this event

    return true;
    
    
}

#include "gpeventfilter.moc"
