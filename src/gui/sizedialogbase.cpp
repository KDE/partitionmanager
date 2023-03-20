/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2018 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-FileCopyrightText: 2016 Chantara Tith <tith.chantara@gmail.com>
    SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/sizedialogbase.h"
#include "gui/sizedetailswidget.h"
#include "gui/sizedialogwidget.h"

#include "util/guihelpers.h"

#include <core/partitiontable.h>
#include <core/device.h>
#include <core/lvmdevice.h>
#include <core/partition.h>
#include <core/partitionalignment.h>

#include <gui/partresizerwidget.h>

#include <util/capacity.h>

#include <algorithm>

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QFrame>
#include <QtGlobal>
#include <QPushButton>
#include <QVBoxLayout>

#include <config.h>

static double sectorsToDialogUnit(const Device& d, qint64 v);
static qint64 dialogUnitToSectors(const Device& d, double v);

SizeDialogBase::SizeDialogBase(QWidget* parent, Device& d, Partition& part, qint64 minFirst, qint64 maxLast) :
    QDialog(parent),
    m_SizeDialogWidget(new SizeDialogWidget(this)),
    m_SizeDetailsWidget(new SizeDetailsWidget(this)),
    m_Device(d),
    m_Partition(part),
    m_MinimumFirstSector(minFirst),
    m_MaximumLastSector(maxLast),
    m_MinimumLength(-1),
    m_MaximumLength(-1),
    m_IsValidLVName(true)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->addWidget(&dialogWidget());
    QFrame* detailsBox = new QFrame(this);
    mainLayout->addWidget(detailsBox);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsBox);
    detailsLayout->addWidget(&detailsWidget());
    detailsWidget().hide();

    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(this);
    detailsButton = new QPushButton(dialogButtonBox);
    okButton = dialogButtonBox->addButton(QDialogButtonBox::Ok);
    cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
    detailsButton->setText(xi18nc("@action:button advanced settings button", "Advanced") + QStringLiteral(" >>"));
    dialogButtonBox->addButton(detailsButton, QDialogButtonBox::ActionRole);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(dialogButtonBox);

    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &SizeDialogBase::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &SizeDialogBase::reject);
    connect(detailsButton, &QPushButton::clicked, this, &SizeDialogBase::toggleDetails);
}

void SizeDialogBase::setupDialog()
{
    dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(device(), partition().firstSector() - minimumFirstSector()));
    dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(device(), maximumLastSector() - partition().lastSector()));

    dialogWidget().spinCapacity().setValue(Capacity(partition().capacity()).toDouble(preferredUnit()));

    dialogWidget().spinFreeBefore().setSuffix(QStringLiteral(" ") + Capacity::unitName(preferredUnit()));
    dialogWidget().spinFreeAfter().setSuffix(QStringLiteral(" ") + Capacity::unitName(preferredUnit()));
    dialogWidget().spinCapacity().setSuffix(QStringLiteral(" ") + Capacity::unitName(preferredUnit()));

    detailsWidget().spinFirstSector().setValue(partition().firstSector());
    detailsWidget().spinLastSector().setValue(partition().lastSector());

    detailsWidget().checkAlign().setChecked(Config::alignDefault());

    if (canGrow() || canShrink())
        dialogWidget().partResizerWidget().init(device(), partition(), minimumFirstSector(), maximumLastSector(), false, canMove());
    else
        dialogWidget().partResizerWidget().init(device(), partition(), minimumFirstSector(), maximumLastSector(), true, canMove());
    dialogWidget().partResizerWidget().setAlign(Config::alignDefault());

    if (device().type() == Device::Type::Disk_Device) {
        dialogWidget().lvName().hide();
        dialogWidget().textLVName().hide();
    }
    if (device().type() == Device::Type::LVM_Device) {
        dialogWidget().hideBeforeAndAfter();
        detailsWidget().checkAlign().setChecked(false);
        detailsWidget().checkAlign().setEnabled(false);
        detailsButton->hide();
        m_IsValidLVName = false;

        /* LVM logical volume name can consist of: letters numbers _ . - +
         * It cannot start with underscore _ and must not be equal to . or .. or any entry in /dev/
         * QLineEdit accepts QValidator::Intermediate, so we just disable . at the beginning */
        QRegularExpression re(QStringLiteral(R"(^(?!_|\.)[\w\-.+]+)"));
        QRegularExpressionValidator *validator = new QRegularExpressionValidator(re, this);
        dialogWidget().lvName().setValidator(validator);
    }
}

void SizeDialogBase::setupConstraints()
{
    // Do not allow moving first sector if moving partition is disabled
    bool moveAllowed = canMove();
    if (!moveAllowed)
        m_MinimumFirstSector = partition().firstSector();
    dialogWidget().spinFreeBefore().setEnabled(moveAllowed);
    dialogWidget().spinFreeAfter().setEnabled(moveAllowed);
    detailsWidget().spinFirstSector().setEnabled(moveAllowed);

    setMinimumLength(!canShrink() ? partition().length() : std::max(partition().sectorsUsed(), partition().minimumSectors()));
    setMaximumLength(!canGrow() ? partition().length() : std::min(maximumLastSector() - minimumFirstSector() + 1, partition().maximumSectors()));

    dialogWidget().partResizerWidget().setMinimumLength(minimumLength());
    dialogWidget().partResizerWidget().setMaximumLength(maximumLength());

    dialogWidget().labelMinSize().setText(Capacity::formatByteSize(minimumLength() * device().logicalSize()));
    dialogWidget().labelMaxSize().setText(Capacity::formatByteSize(maximumLength() * device().logicalSize()));

    dialogWidget().spinCapacity().setEnabled(canShrink() || canGrow());

    dialogWidget().partResizerWidget().setMaximumFirstSector(maximumFirstSector());
    dialogWidget().partResizerWidget().setMinimumLastSector(minimumLastSector());

    const qint64 totalCapacity = sectorsToDialogUnit(device(), maximumLastSector() - minimumFirstSector() + 1);

    const qint64 minCapacity = sectorsToDialogUnit(device(), minimumLength());
    const qint64 maxCapacity = sectorsToDialogUnit(device(), maximumLength());
    dialogWidget().spinCapacity().setRange(minCapacity, maxCapacity);
    minCapacity > maxCapacity ? okButton->setEnabled(false) : okButton->setEnabled(true);

    const qint64 maxFree = totalCapacity - minCapacity;

    dialogWidget().spinFreeBefore().setRange(0, maxFree);
    dialogWidget().spinFreeAfter().setRange(0, maxFree);

    detailsWidget().spinFirstSector().setRange(minimumFirstSector(), maximumLastSector());
    detailsWidget().spinLastSector().setRange(minimumFirstSector(), maximumLastSector());

    onAlignToggled(align());
}

void SizeDialogBase::setupConnections()
{
    connect(&dialogWidget().partResizerWidget(), &PartResizerWidget::firstSectorChanged, this, &SizeDialogBase::onResizerWidgetFirstSectorChanged);
    connect(&dialogWidget().partResizerWidget(), &PartResizerWidget::lastSectorChanged, this, &SizeDialogBase::onResizerWidgetLastSectorChanged);

    // TODO: Qt6 remove qOverload
    connect(&dialogWidget().spinFreeBefore(), qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeDialogBase::onSpinFreeBeforeChanged);
    connect(&dialogWidget().spinFreeAfter(), qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeDialogBase::onSpinFreeAfterChanged);
    connect(&dialogWidget().spinCapacity(), qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeDialogBase::onSpinCapacityChanged);

    connect(&detailsWidget().spinFirstSector(), qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeDialogBase::onSpinFirstSectorChanged);
    connect(&detailsWidget().spinLastSector(), qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SizeDialogBase::onSpinLastSectorChanged);
    connect(&detailsWidget().checkAlign(), &QCheckBox::toggled, this, &SizeDialogBase::onAlignToggled);

    connect(&dialogWidget().lvName(), &QLineEdit::textChanged, this, &SizeDialogBase::onLVNameChanged);
}

void SizeDialogBase::toggleDetails()
{
    const bool isVisible = detailsWidget().isVisible();
    detailsWidget().setVisible(!isVisible);
    detailsButton->setText(xi18nc("@action:button", "&Advanced") + (isVisible ? QStringLiteral(" >>") : QStringLiteral(" <<")));
}

void SizeDialogBase::onSpinFreeBeforeChanged(double newBefore)
{
    bool success = false;

    const double oldBefore = sectorsToDialogUnit(device(), partition().firstSector() - minimumFirstSector());
    const qint64 newFirstSector = minimumFirstSector() + dialogUnitToSectors(device(), newBefore);
    const qint64 deltaCorrection = newBefore > oldBefore
                                   ? PartitionAlignment::firstDelta(device(), partition(), newFirstSector)
                                   : 0;

    // We need different alignFirstSector parameters for moving the first sector (which
    // has to take into account min and max length of the partition) and for moving
    // the whole partition (which must NOT take min and max length into account since
    // the length is fixed in this case anyway)

    qint64 alignedFirstSector = align()
                                ? PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector + deltaCorrection, minimumFirstSector(), -1, -1, -1)
                                : newFirstSector;

    if (dialogWidget().partResizerWidget().movePartition(alignedFirstSector))
        success = true;
    else {
        alignedFirstSector = align()
                             ? PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector + deltaCorrection, minimumFirstSector(), -1, minimumLength(), maximumLength())
                             : newFirstSector;

        success = dialogWidget().partResizerWidget().updateFirstSector(alignedFirstSector);
    }

    if (success)
        setDirty();
    else
        // TODO: this is not the best solution: we should prevent the user from entering
        // illegal values with a validator
        updateSpinFreeBefore(dialogUnitToSectors(device(), oldBefore));
}

void SizeDialogBase::onSpinCapacityChanged(double newCapacity)
{
    bool success = false;

    qint64 newLength = qBound(
                           minimumLength(),
                           dialogUnitToSectors(device(), newCapacity),
                           std::min(maximumLastSector() - minimumFirstSector() + 1, maximumLength())
                       );

    if (newLength == partition().length())
        return;

    qint64 delta = newLength - partition().length();

    qint64 tmp = std::min(delta, maximumLastSector() - partition().lastSector());
    delta -= tmp;

    const bool signalState = dialogWidget().partResizerWidget().blockSignals(true);

    if (tmp != 0) {
        qint64 newLastSector = partition().lastSector() + tmp;

        if (align())
            newLastSector = PartitionAlignment::alignedLastSector(device(), partition(), newLastSector, minimumLastSector(), maximumLastSector(), minimumLength(), maximumLength());

        if (dialogWidget().partResizerWidget().updateLastSector(newLastSector)) {
            success = true;
            updateSpinFreeAfter(maximumLastSector() - newLastSector);
            updateSpinLastSector(newLastSector);
        }
    }

    tmp = std::min(delta, partition().firstSector() - minimumFirstSector());
    delta -= tmp;

    if (tmp != 0) {
        qint64 newFirstSector = partition().firstSector() - tmp;

        if (align())
            newFirstSector = PartitionAlignment::alignedFirstSector(device(), partition(), newFirstSector, minimumFirstSector(), maximumFirstSector(), minimumLength(), maximumLength());

        if (dialogWidget().partResizerWidget().updateFirstSector(newFirstSector)) {
            success = true;
            updateSpinFreeBefore(newFirstSector - minimumFirstSector());
            updateSpinFirstSector(newFirstSector);
        }
    }

    dialogWidget().partResizerWidget().blockSignals(signalState);

    if (success)
        setDirty();
}

void SizeDialogBase::onSpinFreeAfterChanged(double newAfter)
{
    bool success = false;
    const double oldAfter = sectorsToDialogUnit(device(), maximumLastSector() - partition().lastSector());
    const qint64 newLastSector = maximumLastSector() - dialogUnitToSectors(device(), newAfter);
    const qint64 deltaCorrection = newAfter > oldAfter
                                   ? PartitionAlignment::lastDelta(device(), partition(), newLastSector)
                                   : 0;

    // see onSpinFreeBeforeChanged on why this is as complicated as it is

    qint64 alignedLastSector = align()
                               ? PartitionAlignment::alignedLastSector(device(), partition(), newLastSector - deltaCorrection, -1, maximumLastSector(), -1, -1)
                               : newLastSector;

    if (dialogWidget().partResizerWidget().movePartition(alignedLastSector - partition().length() + 1))
        success = true;
    else {
        alignedLastSector = align()
                            ? PartitionAlignment::alignedLastSector(device(), partition(), newLastSector - deltaCorrection, -1, maximumLastSector(), minimumLength(), maximumLength())
                            : newLastSector;

        success = dialogWidget().partResizerWidget().updateLastSector(alignedLastSector);
    }

    if (success)
        setDirty();
    else
        // TODO: this is not the best solution: we should prevent the user from entering
        // illegal values with a validator
        updateSpinFreeAfter(dialogUnitToSectors(device(), oldAfter));
}

void SizeDialogBase::onSpinFirstSectorChanged(double newFirst)
{
    if (newFirst >= minimumFirstSector() && dialogWidget().partResizerWidget().updateFirstSector(newFirst))
        setDirty();
    else
        // TODO: this is not the best solution: we should prevent the user from entering
        // illegal values with a validator
        updateSpinFirstSector(partition().firstSector());
}

void SizeDialogBase::onSpinLastSectorChanged(double newLast)
{
    if (newLast <= maximumLastSector() && dialogWidget().partResizerWidget().updateLastSector(newLast))
        setDirty();
    else
        // TODO: this is not the best solution: we should prevent the user from entering
        // illegal values with a validator
        updateSpinLastSector(partition().lastSector());
}

void SizeDialogBase::onResizerWidgetFirstSectorChanged(qint64 newFirst)
{
    updateSpinFreeBefore(newFirst - minimumFirstSector());
    updateSpinFirstSector(newFirst);
    updateSpinCapacity(partition().length());
    setDirty();
}

void SizeDialogBase::onResizerWidgetLastSectorChanged(qint64 newLast)
{
    updateSpinFreeAfter(maximumLastSector() - newLast);
    updateSpinLastSector(newLast);
    updateSpinCapacity(partition().length());
    setDirty();
}

void SizeDialogBase::onAlignToggled(bool align)
{
    dialogWidget().partResizerWidget().setAlign(align);

    detailsWidget().spinFirstSector().setSingleStep(align ? PartitionAlignment::sectorAlignment(device()) : 1);
    detailsWidget().spinLastSector().setSingleStep(align ? PartitionAlignment::sectorAlignment(device()) : 1);

    const double capacityStep = align ? sectorsToDialogUnit(device(), PartitionAlignment::sectorAlignment(device())) : 1;

    dialogWidget().spinFreeBefore().setSingleStep(capacityStep);
    dialogWidget().spinFreeBefore().setSingleStep(capacityStep);
    dialogWidget().spinCapacity().setSingleStep(capacityStep);

    // if align is on, turn off keyboard tracking for all spin boxes to avoid the two clashing
    const auto children = dialogWidget().findChildren<QAbstractSpinBox*>() + detailsWidget().findChildren<QAbstractSpinBox*>();
    for (const auto &box : children)
        box->setKeyboardTracking(!align);

    if (align) {
        onSpinFirstSectorChanged(partition().firstSector());
        onSpinLastSectorChanged(partition().lastSector());
    }
}

void SizeDialogBase::onLVNameChanged(const QString& newName)
{
    partition().setPartitionPath(device().deviceNode() + QStringLiteral("/") + newName.trimmed());
    if ((dialogWidget().lvName().isVisible() &&
        dialogWidget().lvName().text().isEmpty()) ||
        (device().type() == Device::Type::LVM_Device &&
         dynamic_cast<LvmDevice&>(device()).partitionNodes().contains(partition().partitionPath())) ) {
        m_IsValidLVName = false;
    } else {
        m_IsValidLVName = true;
    }
    updateOkButtonStatus();
}

void SizeDialogBase::updateOkButtonStatus()
{
    okButton->setEnabled(isValidLVName());
}

void SizeDialogBase::updateSpinFreeBefore(qint64 sectorsFreeBefore)
{
    const bool signalState = dialogWidget().spinFreeBefore().blockSignals(true);
    dialogWidget().spinFreeBefore().setValue(sectorsToDialogUnit(device(), sectorsFreeBefore));
    dialogWidget().spinFreeBefore().blockSignals(signalState);
}

void SizeDialogBase::updateSpinCapacity(qint64 newLengthInSectors)
{
    bool state = dialogWidget().spinCapacity().blockSignals(true);
    dialogWidget().spinCapacity().setValue(sectorsToDialogUnit(device(), newLengthInSectors));
    dialogWidget().spinCapacity().blockSignals(state);
}

void SizeDialogBase::updateSpinFreeAfter(qint64 sectorsFreeAfter)
{
    const bool signalState = dialogWidget().spinFreeAfter().blockSignals(true);
    dialogWidget().spinFreeAfter().setValue(sectorsToDialogUnit(device(), sectorsFreeAfter));
    dialogWidget().spinFreeAfter().blockSignals(signalState);
}

void SizeDialogBase::updateSpinFirstSector(qint64 newFirst)
{
    const bool signalState = detailsWidget().spinFirstSector().blockSignals(true);
    detailsWidget().spinFirstSector().setValue(newFirst);
    detailsWidget().spinFirstSector().blockSignals(signalState);
}

void SizeDialogBase::updateSpinLastSector(qint64 newLast)
{
    const bool signalState = detailsWidget().spinLastSector().blockSignals(true);
    detailsWidget().spinLastSector().setValue(newLast);
    detailsWidget().spinLastSector().blockSignals(signalState);
}

const PartitionTable& SizeDialogBase::partitionTable() const
{
    Q_ASSERT(device().partitionTable());
    return *device().partitionTable();
}

bool SizeDialogBase::align() const
{
    return detailsWidget().checkAlign().isChecked();
}

qint64 SizeDialogBase::minimumLastSector() const
{
    return partition().minLastSector();
}

qint64 SizeDialogBase::maximumFirstSector() const
{
    return partition().maxFirstSector();
}

qint64 SizeDialogBase::minimumLength() const
{
    return m_MinimumLength;
}

qint64 SizeDialogBase::maximumLength() const
{
    return m_MaximumLength;
}

static double sectorsToDialogUnit(const Device& d, qint64 v)
{
    return Capacity(v * d.logicalSize()).toDouble(preferredUnit());
}

static qint64 dialogUnitToSectors(const Device& d, double v)
{
    return v * Capacity::unitFactor(Capacity::Unit::Byte, preferredUnit()) / d.logicalSize();
}

