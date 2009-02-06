// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
This file is a part of digiKam project
http://www.digikam.org
Description : image editor printing interface.

Copyright 2009 Angelo Naselli <anaselli@linux.it>
- From  Gwenview code (Aurélien Gâteau)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef PRINTHELPER_H
#define PRINTHELPER_H


// Qt

// KDE

// Local
#include "digikam_export.h"
#include <dimg.h>

class QWidget;

namespace Digikam {


class PrintHelperPrivate;
class DIGIKAM_EXPORT PrintHelper {
public:
	PrintHelper(QWidget* parent);
	~PrintHelper();

	void print(DImg&);

private:
	PrintHelperPrivate* const d;
};


} // namespace

#endif /* PRINTHELPER_H */
