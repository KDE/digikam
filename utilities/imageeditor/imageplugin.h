/* ============================================================
 * File  : imageplugin.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef IMAGEPLUGIN_H
#define IMAGEPLUGIN_H

#include <qobject.h>

#include <guiclient.h>

class KInstance;
class QWidget;

namespace Digikam
{

class ImagePlugin : public QObject, public GUIClient
{
public:
    
    ImagePlugin(QObject *parent, const char* name=0);
    virtual ~ImagePlugin();

    void setInstance(KInstance *instance);

    virtual void setEnabledSelectionActions(bool enable);

    void     setParentWidget(QWidget* parent);
    QWidget* parentWidget();
    
private:

    KInstance  *m_instance;
    QWidget    *m_parentWidget;
};

}

#endif /* IMAGEPLUGIN_H */
