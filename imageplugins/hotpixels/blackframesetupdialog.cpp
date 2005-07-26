/* ============================================================
* File  : blackframesetupdialog.cpp
* Author: Unai Garro <ugarro at users dot sourceforge dot net>
* Date  : 2005-06-19
* Description :  dialog to setup the black frame list
*
* Copyright 2005 by Unai Garro
*
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
#include <klocale.h>
#include <kimageio.h>
#include <iostream>
#include "blackframesetupdialog.h"

BlackFrameSetupDialog::BlackFrameSetupDialog(QWidget* parent)
: KDialogBase(Plain, i18n("Setup black frames"),
		Help|Ok|Cancel|User1, Ok,
		parent, 0, true, true,i18n("New black frame..."))
{
	//Setup the dialog
	mHBoxLayout=new QHBoxLayout(plainPage());
	mHBoxLayout->setAutoAdd(true);
	mBlackFrameListView=new BlackFrameListView(plainPage());
	mBlackFrameListView->addColumn(i18n("Preview"));
	mBlackFrameListView->addColumn(i18n("Size"));
	mBlackFrameListView->addColumn(i18n("Enabled"));
	
	//Connect signals & slots
	connect(this,SIGNAL(user1Clicked()),this,SLOT(selectBlackFrameFile()));
	
	
	
}

void BlackFrameSetupDialog::selectBlackFrameFile(void)
{
	
	KImageIO::registerFormats(); //Does one need to do this if digikam did so already?
	mFileSelectDialog=new KFileDialog(QString::null,KImageIO::pattern(),this,"",true);
	mFileSelectDialog->setCaption(i18n("Select a black frame image"));
	mFileSelectDialog->exec();
	
	//Load the selected file and insert into the list
	
	KURL url=mFileSelectDialog->selectedURL();
	BlackFrameListViewItem *it=new BlackFrameListViewItem(mBlackFrameListView,url);
}

#include "blackframesetupdialog.moc"