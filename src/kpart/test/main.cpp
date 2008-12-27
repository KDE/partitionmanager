/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "kparttest.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char* argv[])
{
	KAboutData aboutData(
		"kparttest",
		NULL,
		ki18n("KDE Partition Manager KPart"), "0.1",
		ki18n("A test application for KDE Partition Manager's KPart."),
		KAboutData::License_GPL,
		ki18n("Copyright (c) 2008 Volker Lanz")
	);

	KCmdLineArgs::init(argc, argv, &aboutData);
	
	KApplication app;

	KPartTest* widget = new KPartTest();
	widget->show();

	return app.exec();
}
