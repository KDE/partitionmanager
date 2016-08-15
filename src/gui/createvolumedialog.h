/*************************************************************************
 *  Copyright (C) 2016 by Chantara Tith <tith.chantara@gmail.com>        *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(CREATEVOLUMEDIALOG__H)

#define CREATEVOLUMEDIALOG__H

#include "gui/volumedialog.h"

class CreateVolumeDialog : public VolumeDialog
{
    Q_DISABLE_COPY(CreateVolumeDialog)

public:
    CreateVolumeDialog(QWidget* parent, QString& vgname, QStringList& pvlist, qint32& pesize);

protected:
    void accept() override;
    void setupDialog() override;
    void setupConnections() override;

protected:
    void onVGNameChanged(const QString& vgname);
    void onSpinPESizeChanged(int newsize);

    qint32& peSize() {
        return m_PESize;
    }

    qint32& m_PESize;
};

#endif
