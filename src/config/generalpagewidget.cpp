/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "config/generalpagewidget.h"

#include <backend/corebackendmanager.h>
#include <fs/filesystemfactory.h>

#include <util/helpers.h>
#include "util/guihelpers.h"

#include <config.h>

GeneralPageWidget::GeneralPageWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    setupDialog();
}

FileSystem::Type GeneralPageWidget::defaultFileSystem() const
{
    return FileSystem::typeForName(comboDefaultFileSystem().currentText());
}

void GeneralPageWidget::setDefaultFileSystem(FileSystem::Type t)
{
    const int idx = comboDefaultFileSystem().findText(FileSystem::nameForType(t));
    comboDefaultFileSystem().setCurrentIndex(idx != -1 ? idx : 0);
}

void GeneralPageWidget::setupDialog()
{
    QStringList fsNames;
    for (const auto &fs : FileSystemFactory::map())
        if (fs->supportCreate() != FileSystem::cmdSupportNone && fs->type() != FileSystem::Type::Extended && fs->type() != FileSystem::Type::Luks)
            fsNames.append(fs->name());

    std::sort(fsNames.begin(), fsNames.end(), caseInsensitiveLessThan);

    for (const auto &fsName : qAsConst(fsNames))
        comboDefaultFileSystem().addItem(createFileSystemColor(FileSystem::typeForName(fsName), 8), fsName);

    setDefaultFileSystem(GuiHelpers::defaultFileSystem());

    kcfg_shredSource->setId(radioButton, 0);
    kcfg_shredSource->setId(radioButton_2, 1);
    radioButton->setChecked(Config::shredSource() == Config::EnumShredSource::random);
    radioButton_2->setChecked(Config::shredSource() == Config::EnumShredSource::zeros);
}
