/* ============================================================
 * File  : tagfolderview.cpp
 * Author: Jörn Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-05
 * Copyright 2005 by Jörn Ahrens
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
 * ============================================================ */

#include <klocale.h>

#include "tagfolderview.h"

TagFolderView::TagFolderView(QWidget *parent)
    : QListView(parent)
{
    addColumn(i18n("My Tags"));
    setResizeMode(QListView::LastColumn);
}

#include "tagfolderview.moc"
