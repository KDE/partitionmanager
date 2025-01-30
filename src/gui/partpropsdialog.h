/*
    SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(PARTPROPSDIALOG_H)

#define PARTPROPSDIALOG_H

#include <fs/filesystem.h>

#include <core/partition.h>
#include <core/partitiontable.h>

#include <QDialog>

class Device;
class PartPropsWidget;

class QDialogButtonBox;
class QVBoxLayout;
class QWidget;
class QString;


/** Show Partition properties.

    Dialog that shows a Partition's properties and allows the user to change (or recreate)
    the Partition's FileSystem, its label and its flags.

    @author Volker Lanz <vl@fidra.de>
*/
class PartPropsDialog : public QDialog
{
    Q_DISABLE_COPY(PartPropsDialog)

public:
    PartPropsDialog(QWidget* parent, Device& d, Partition& p);
    ~PartPropsDialog();

public:
    QString newPartitionLabel() const;
    QString newLabel() const;
    PartitionTable::Flags newFlags() const;
    FileSystem::Type newFileSystemType() const;
    bool forceRecreate() const {
        return m_ForceRecreate;    /**< @return true if user wants to recreate the FileSystem on the Partition */
    }

protected:
    void setupDialog();
    void setupConnections();
    void setupFileSystemComboBox();
    void setupFlagsList();
    void updateHideAndShow();

    bool warnFileSystemChange() const {
        return m_WarnFileSystemChange;
    }
    void setWarnFileSystemChange() {
        m_WarnFileSystemChange = true;
    }

    Device& device() {
        return m_Device;
    }
    const Device& device() const {
        return m_Device;
    }

    Partition& partition() {
        return m_Partition;
    }
    const Partition& partition() const {
        return m_Partition;
    }

    PartPropsWidget& dialogWidget() {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }
    const PartPropsWidget& dialogWidget() const {
        Q_ASSERT(m_DialogWidget);
        return *m_DialogWidget;
    }

    bool isReadOnly() const {
        return m_ReadOnly;
    }
    void setForceRecreate(bool b) {
        m_ForceRecreate = b;
    }

    void updatePartitionFileSystem();

protected:
    void setDirty(void *unused = nullptr);
    void onFilesystemChanged(int idx);
    void onRecreate(int);

private:
    // m_Device and m_Partition cannot be const because the PartResizerWidget takes
    // both as nonconst even in read only mode (which is a bad design flaw and should be
    // fixed by splitting it in a PartDisplayWidget that is read-only and a PartResizerWidget
    // subclassed from that, maybe)
    Device& m_Device;
    // m_Partition is also not a reference because we want to be able to change it and
    // forget the changes if the user cancels the dialog
    Partition m_Partition;
    bool m_WarnFileSystemChange;
    PartPropsWidget* m_DialogWidget;
    bool m_ReadOnly;
    bool m_ForceRecreate;

    QDialogButtonBox* dialogButtonBox;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QVBoxLayout *mainLayout;
};

#endif
