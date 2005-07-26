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

#ifndef BLACKFRAMESETUPDIALOG_H
#define BLACKFRAMESETUPDIALOG_H

#include <qlayout.h>

#include <kdialogbase.h>
#include <kfiledialog.h>
#include "blackframelistview.h"

class BlackFrameSetupDialog:public KDialogBase
{
Q_OBJECT

public:
	BlackFrameSetupDialog(QWidget *parent);
	~BlackFrameSetupDialog(){}
private: 
	QHBoxLayout *mHBoxLayout;
	BlackFrameListView *mBlackFrameListView;
	KFileDialog *mFileSelectDialog;
private slots:
	void selectBlackFrameFile(void);
	
};

#endif
