/* ============================================================
 * File  : camerapropsplugin.h
 * Author: Jörn Ahrens <kde@jokele.de>
 * Date  : 2004-07-23
 * Description : 
 * This class adds additional tab to KPropertiesDialog to show
 * the extended file attributes from the digikamcamera kioslave. 
 *
 * Copyright 2004 by Jörn Ahrens

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

#include <klocale.h>
#include <kdialogbase.h>

#include <kio/global.h>
#include <kio/job.h>

#include <qvaluelist.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include "camerapropsplugin.h"

using namespace KIO;

CameraPropsPlugin::CameraPropsPlugin(KPropertiesDialog *_props ) :
	KPropsDlgPlugin(_props)
{
	QFrame *frame = properties->addPage(i18n("&Camera Info"));
	
	QVBoxLayout *vbox = new QVBoxLayout(frame);
	vbox->setSpacing(KDialog::spacingHint());
	
	QGroupBox *statusgroup = new QGroupBox(1, Qt::Horizontal, 
		i18n("Downloaded Status"), frame);
	QGroupBox *imagegroup = new QGroupBox(1, Qt::Horizontal, 
		i18n("Image Size"), frame);
	vbox->addWidget(statusgroup);
	vbox->addWidget(imagegroup);
	vbox->addStretch(1);

	QFrame *statusframe = new QFrame(statusgroup);
	QGridLayout *statusgrid = new QGridLayout(statusframe, 1, 3, 0, 
		KDialog::spacingHint());
	statusgrid->setColStretch(2,1);
	statusgrid->addWidget(new QLabel(i18n("Status:"), statusframe), 0, 0);
	QLabel *status = new QLabel(i18n("unknown"), statusframe);
	statusgrid->addWidget(status, 0, 1);
	
	QFrame *imageframe = new QFrame(imagegroup);
	QGridLayout *imagegrid = new QGridLayout(imageframe, 2, 3, 0, 
		KDialog::spacingHint());
	imagegrid->setColStretch(2,1);
	imagegrid->addWidget(new QLabel(i18n("Width:"), imageframe), 0, 0);
	QLabel *width = new QLabel(i18n("unknown"), imageframe);
	imagegrid->addWidget(width, 0, 1);
	imagegrid->addWidget(new QLabel(i18n("Height:"), imageframe), 1, 0);
	QLabel *height = new QLabel(i18n("unknown"), imageframe);
	imagegrid->addWidget(height, 1, 1);
	
	UDSEntry uds = properties->item()->entry();
	QValueListIterator<UDSAtom> iter;
	for(iter = uds.begin(); iter != uds.end(); iter++) 
	{
		if((*iter).m_uds == KIO::UDS_XML_PROPERTIES) 
		{
			QStringList proplist = QStringList::split("/", (*iter).m_str);
			QStringList::iterator propiter = proplist.begin();
			if(propiter != proplist.end() && *propiter != "0")
				width->setText(*propiter);
			propiter++;	
			if(propiter != proplist.end() && *propiter != "0")
				height->setText(*propiter);
			propiter++;
			if(propiter != proplist.end() && *propiter != "-1")
				status->setText(*propiter);
		}
	}
}

CameraPropsPlugin::~CameraPropsPlugin()
{
}

