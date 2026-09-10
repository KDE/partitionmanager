/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/filesystempropertieswidget.h"

#include <fs/filesystem.h>

#include <util/capacity.h>

#include <QFrame>
#include <QGridLayout>
#include <QHash>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <KLocalizedString>

namespace
{
const FileSystemProperty::Group groupOrder[] = {
    FileSystemProperty::Group::UnitSizes,
    FileSystemProperty::Group::Capabilities,
    FileSystemProperty::Group::Reserved,
    FileSystemProperty::Group::Metadata,
    FileSystemProperty::Group::Journal,
    FileSystemProperty::Group::Specific,
};

QString displayName(const QString& id)
{
    static const QHash<QString, QString> names = {
        { QStringLiteral("block-size"),             xi18nc("@label", "Block size") },
        { QStringLiteral("sector-size"),            xi18nc("@label", "Sector size") },
        { QStringLiteral("metadata-node-size"),     xi18nc("@label", "Metadata node size") },
        { QStringLiteral("checksum-algorithm"),     xi18nc("@label", "Checksum algorithm") },
        { QStringLiteral("mft-record-size"),        xi18nc("@label", "MFT record size") },
        { QStringLiteral("inode-size"),             xi18nc("@label", "Inode size") },
        { QStringLiteral("inode-count"),            xi18nc("@label", "Inode count") },
        { QStringLiteral("fat-count"),              xi18nc("@label", "Number of FATs") },
        { QStringLiteral("filesystem-features"),    xi18nc("@label", "Enabled features") },
        { QStringLiteral("format-version"),         xi18nc("@label", "Format version") },
        { QStringLiteral("journal-size"),           xi18nc("@label", "Journal size") },
        { QStringLiteral("reserved-space"),         xi18nc("@label", "Reserved space") },
        { QStringLiteral("reserved-space-percent"), xi18nc("@label", "Reserved space, percent") },
        { QStringLiteral("reserved-block-count"),   xi18nc("@label", "Reserved block count") },
        { QStringLiteral("reserved-uid"),           xi18nc("@label", "Reserved space owner (UID)") },
        { QStringLiteral("reserved-gid"),           xi18nc("@label", "Reserved space owner (GID)") },
    };

    return names.value(id, id);
}

QString formatList(const QStringList& items)
{
    const int maxLineLength = 40;
    QStringList lines;
    QString current;

    for (const auto& item : items) {
        if (!current.isEmpty() && current.length() + 2 + item.length() > maxLineLength) {
            lines.append(current);
            current.clear();
        }
        current += current.isEmpty() ? item : QStringLiteral(", ") + item;
    }
    if (!current.isEmpty())
        lines.append(current);

    return lines.join(QLatin1Char('\n'));
}

QString formatValue(const FileSystemProperty& property)
{
    if (!property.value.isValid())
        return xi18nc("@item filesystem property whose value could not be read", "Unknown");

    switch (property.displayType) {
    case FileSystemProperty::DisplayType::Bytes:
        return Capacity::formatByteSize(property.value.toDouble());
    case FileSystemProperty::DisplayType::Number:
        return QLocale().toString(property.value.toLongLong());
    case FileSystemProperty::DisplayType::Percent:
        return xi18nc("@item value in percent", "%1%", QLocale().toString(property.value.toDouble(), 'f', 1));
    case FileSystemProperty::DisplayType::List:
        return formatList(property.value.toStringList());
    case FileSystemProperty::DisplayType::Text:
        break;
    }

    return property.value.toString();
}
}

FileSystemPropertiesWidget::FileSystemPropertiesWidget(QWidget* parent) :
    QWidget(parent),
    m_Layout(new QVBoxLayout(this)),
    m_Content(nullptr),
    m_IsEmpty(true)
{
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->setSizeConstraint(QLayout::SetMinimumSize);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}

void FileSystemPropertiesWidget::setFileSystem(const FileSystem& fs)
{
    delete m_Content;
    m_Content = new QWidget(this);
    m_Layout->addWidget(m_Content);

    QGridLayout* grid = new QGridLayout(m_Content);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 5);

    m_IsEmpty = true;
    int row = 0;

    const QList<FileSystemProperty>& properties = fs.properties();

    for (const auto& group : groupOrder) {
        bool firstInGroup = true;

        for (const auto& property : properties) {
            if (property.group != group)
                continue;

            // Already shown in the dedicated "Cluster size" row of the partition properties dialog.
            if (property.id == QLatin1String("cluster-size"))
                continue;

            if (firstInGroup) {
                QFrame* separator = new QFrame(m_Content);
                separator->setFrameShape(QFrame::HLine);
                separator->setFrameShadow(QFrame::Sunken);
                grid->addWidget(separator, row++, 0, 1, 2);

                firstInGroup = false;
                m_IsEmpty = false;
            }

            QLabel* label = new QLabel(xi18nc("@label", "%1:", displayName(property.id)), m_Content);
            label->setAlignment(Qt::AlignRight | Qt::AlignTop);

            QLabel* value = new QLabel(formatValue(property), m_Content);
            value->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

            grid->addWidget(label, row, 0, Qt::AlignRight | Qt::AlignTop);
            grid->addWidget(value, row, 1);
            ++row;
        }
    }

    m_Content->setVisible(!m_IsEmpty);
}
