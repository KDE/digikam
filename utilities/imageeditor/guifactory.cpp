/* ============================================================
 * File  : guifactory.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-11
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

#include <qptrlist.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qcstring.h>

#include <kaction.h>
#include <ktoolbar.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <klocale.h>

#include "guiclient.h"
#include "guifactory.h"

namespace Digikam
{

class GUIElement
{
public:

    enum Type {
        TopLevel=0,
        Menu,
        Merge,
        Action,
        Separator
    };
    
    GUIElement(GUIElement *parent, Type type, const QString& text,
               KAction *action=0) {

        m_parent = parent;
        m_type   = type;
        m_text   = text;
        m_action = action;

        m_next   = 0;
        m_prev   = 0;
        m_firstChild = 0;
        m_lastChild  = 0;
        m_clearing   = false;
        
        if (parent) 
            parent->insertChild(this);
    }

    GUIElement(GUIElement *parent, GUIElement *before, Type type,
               const QString& text, KAction *action=0) {

        m_parent = parent;
        m_type   = type;
        m_text   = text;
        m_action = action;

        m_next   = 0;
        m_prev   = 0;
        m_firstChild = 0;
        m_lastChild  = 0;
        m_clearing   = false;
        
        if (parent)
            if (!before)
                parent->insertChild(this);
            else
                parent->insertChildBefore(this, before);
    }

    ~GUIElement() {
        if (m_parent)
            m_parent->removeChild(this);
        clear();
    }

    void insertChild(GUIElement *child) {
        if (!child)
            return;

        if (!m_firstChild) {
            m_firstChild = child;
            m_lastChild  = child;
            child->m_next = 0;
            child->m_prev = 0;
        }
        else {
            m_lastChild->m_next = child;
            child->m_prev  = m_lastChild;
            child->m_next  = 0;
            m_lastChild    = child;
        }
    }
    
    void insertChildBefore(GUIElement *child, GUIElement *before) {

        if (!child || !before)
            return;

        if (!m_firstChild) {
            m_firstChild = child;
            m_lastChild  = child;
            child->m_next = 0;
            child->m_prev = 0;
        }
        else {

            if (before == m_firstChild) {
                m_firstChild->m_prev = child;
                child->m_next  = m_firstChild;
                child->m_prev  = 0;
                m_firstChild   = child;
            }
            else {
                
                before->m_prev->m_next = child;
                child->m_prev = before->m_prev;

                child->m_next  = before;
                before->m_prev = child;
            }
        }
    }

    void removeChild(GUIElement *child) {

        if (!child || m_clearing)
            return;

        if (child == m_firstChild) {
            m_firstChild = m_firstChild->m_next;
            if (m_firstChild)
                m_firstChild->m_prev = 0;
            else
                m_firstChild = m_lastChild = 0;
        }
        else if (child == m_lastChild) {
            m_lastChild = m_lastChild->m_prev;
            if (m_lastChild)
                m_lastChild->m_next = 0;
            else
                m_firstChild = m_lastChild = 0;
        }
        else {
            GUIElement* c = child;
            if (c->m_prev)
                c->m_prev->m_next = c->m_next;
            if (c->m_next)
                c->m_next->m_prev = c->m_prev;
        }
    }

    void clear() {

        m_clearing = true;
        
        GUIElement* child = m_firstChild;
        GUIElement* nextChild;
        while (child) {
            nextChild = child->m_next;
            delete child;
            child = nextChild;
        }

        m_action = 0;
        m_firstChild = 0;
        m_lastChild  = 0;

        m_clearing = false;
    }        

    GUIElement* findChild(Type type, const QString& text, bool create=true) {

        GUIElement* child = m_firstChild;
        GUIElement* nextChild;
        while (child) {
            if (child->m_type == type && child->m_text == text)
                return child;
            nextChild = child->m_next;
            child = nextChild;
        }

        if (create)
            return new GUIElement(this, type, text);
        else
            return 0;
    }

    Type        m_type;
    QString     m_text;
    GUIElement *m_parent;
    GUIElement *m_firstChild;
    GUIElement *m_lastChild;
    GUIElement *m_next;
    GUIElement *m_prev;
    bool        m_clearing;
    KAction    *m_action;

private:

    GUIElement() {}
    GUIElement(const GUIElement&) {}
};

class GUIFactoryPriv
{
public:

    QPtrList<GUIClient> clientList;
    bool                dirty;
    GUIElement         *menuBarGUI;
    GUIElement         *toolBarGUI;
    GUIElement         *popMenuGUI;
};


GUIFactory::GUIFactory()
{
    d = new GUIFactoryPriv;
    
    d->menuBarGUI = new GUIElement(0, GUIElement::TopLevel, "Root");
    d->toolBarGUI = new GUIElement(0, GUIElement::TopLevel, "Root");
    d->popMenuGUI = new GUIElement(0, GUIElement::TopLevel, "Root");
    d->dirty      = false;
}

GUIFactory::~GUIFactory()
{
    delete d->menuBarGUI;
    delete d->toolBarGUI;
    delete d->popMenuGUI;
    delete d;    
}

void GUIFactory::insertClient(GUIClient *client)
{
    if (d->clientList.find(client) == -1) {
        d->clientList.append(client);
        d->dirty = true;
    }
}

void GUIFactory::removeClient(GUIClient *client)
{
    if (d->clientList.find(client) != -1) {
        d->clientList.remove(client);
        d->dirty = true;
    }
}

void GUIFactory::buildGUI(QWidget *w)
{
    if (d->dirty) 
        parseDefinition();

    if (w->inherits("QMainWindow")) {
        QMainWindow *p = static_cast<QMainWindow*>(w);

        {
            QMenuBar* menuBar = p->menuBar();
            menuBar->clear();
            
            GUIElement *gui = d->menuBarGUI->m_firstChild;
            if (gui)
                buildGUI(gui, menuBar);
        }

        {
             QObject* obj = p->child("toolbar","KToolBar");
             KToolBar* toolBar = 0;
             if (obj)
                 toolBar = static_cast<KToolBar*>(obj);
             else
                 toolBar = new KToolBar(p,  Qt::DockTop);
             toolBar->clear();

            GUIElement *gui = d->toolBarGUI->m_firstChild;
            if (gui) 
                buildGUI(gui, toolBar);
        }
    }
    else if (w->inherits("QPopupMenu")) {

        QPopupMenu *p = static_cast<QPopupMenu*>(w);
        p->clear();
            
        GUIElement *gui = d->popMenuGUI->m_firstChild;
        if (gui)
            buildGUI(gui, p);
    }
}

void GUIFactory::buildGUI(GUIElement *g, QWidget *widget)
{
    if (!g || !widget)
        return;
    
    GUIElement *gui = g;
    GUIElement *next = 0;

    
    while (gui) {

        next = gui->m_next;

        QString i18nText;
        QCString text = gui->m_text.utf8();
        if (!text.isEmpty())
            i18nText = i18n( text );
        else
            i18nText = i18n( "No text!" );
        
        if (gui->m_type == GUIElement::Action) {
            if (gui->m_action)
                gui->m_action->plug(widget);
        }
        else if (gui->m_type == GUIElement::Menu) {
            QPopupMenu* popMenu = new QPopupMenu(widget);
            if (widget->inherits("QMenuBar")) {
                QMenuBar *mb = static_cast<QMenuBar*>(widget);
                mb->insertItem(i18nText, popMenu);
                buildGUI(gui->m_firstChild, popMenu);
            }
            else if (widget->inherits("QPopupMenu")) {
                QPopupMenu *pm = static_cast<QPopupMenu*>(widget);
                pm->insertItem(i18nText, popMenu);
                buildGUI(gui->m_firstChild, popMenu);
            }
        }
        else if (gui->m_type == GUIElement::Separator) {
            if (widget->inherits("QMenuBar")) {
                QMenuBar *mb = static_cast<QMenuBar*>(widget);
                mb->insertSeparator();
            }
            else if (widget->inherits("QPopupMenu")) {
                QPopupMenu *pm = static_cast<QPopupMenu*>(widget);
                pm->insertSeparator();
            }
        }
        
        gui = next;
    }
}

void GUIFactory::parseDefinition()
{
    d->menuBarGUI->clear();
    d->toolBarGUI->clear();
    d->popMenuGUI->clear();

    for (GUIClient* client = d->clientList.first(); client;
         client = d->clientList.next()) {

        QStringList guiDef(client->guiDefinition());
        if (guiDef.empty())
            continue;

        for(QStringList::Iterator iter = guiDef.begin();
            iter != guiDef.end(); ++iter) {

            QStringList gd = QStringList::split(QString("/"),
                                                *iter, true);
            if (gd.count() < 1)
                continue;

            QString tag(gd.first());

            gd.pop_front();
            
            if (tag.lower() == "menubar")
                buildTree(client, d->menuBarGUI, gd);
            else if (tag.lower() == "toolbar")
                buildTree(client, d->toolBarGUI, gd);
            else if (tag.lower() == "popupmenu")
                buildTree(client, d->popMenuGUI, gd);
            else
                kdWarning() << "GUIFactory: unknown tag: " << tag << endl;
        }
    }

    d->dirty = false;
}

void GUIFactory::buildTree(GUIClient *client, GUIElement *parent,
                           QStringList gd)
{
    if (gd.count() < 3)
        return;
    
    QString tag(gd.first().lower());
    gd.pop_front();

    QString name(gd.first());
    gd.pop_front();

    QString group(gd.first());
    gd.pop_front();

    if (tag == "menu") {

        GUIElement *gui;
        
        if (group.isEmpty())
            gui = parent->findChild(GUIElement::Menu, name);
        else {
            GUIElement *guiMerge = parent->findChild(GUIElement::Merge,
                                                     group, false);
            if (guiMerge) {
                gui = parent->findChild(GUIElement::Menu, name, false);
                if (!gui)
                    gui = new GUIElement(parent, guiMerge, GUIElement::Menu,
                                         name);
            }
            else {
                gui = parent->findChild(GUIElement::Menu, name);
            }
        }
            
        buildTree(client, gui, gd);

    }
    else if (tag == "action") {

        GUIElement *gui;
            
        if (group.isEmpty())
            gui = parent->findChild(GUIElement::Action, name);
        else {
            GUIElement *guiMerge = parent->findChild(GUIElement::Merge,
                                                     group, false);
            if (guiMerge)
                gui = new GUIElement(parent, guiMerge, GUIElement::Action,
                                     name);
            else
                gui = parent->findChild(GUIElement::Action, name);
        }

        KAction *action = client->actionCollection()->action(name.latin1());
        if (action)
            gui->m_action = action;
        else
            delete gui;
    }
    else if (tag == "definegroup") {
        parent->findChild(GUIElement::Merge, name);
    }
    else if (tag == "separator") {
        new GUIElement(parent, GUIElement::Separator, "separator");
    }
    else {
        kdWarning() << "GUIFactory: unknown tag: " << tag << endl;
    }
}

}
