/*
    SPDX-FileCopyrightText: 2010 Volker Lanz <vl@fidra.de>
    SPDX-FileCopyrightText: 2014-2017 Andrius Štikonas <andrius@stikonas.eu>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(CONFIGUREOPTIONSDIALOG_H)

#define CONFIGUREOPTIONSDIALOG_H

#include <KConfigDialog>

class GeneralPageWidget;
class FileSystemColorsPageWidget;
class AdvancedPageWidget;
class OperationStack;

class ConfigureOptionsDialog : public KConfigDialog
{
public:
    ConfigureOptionsDialog(QWidget* parent, const OperationStack& ostack, const QString& name);
    ~ConfigureOptionsDialog();

protected:
    void updateSettings() override;
    void updateWidgetsDefault() override;
    bool hasChanged() override;
    bool isDefault() override;
    void onComboDefaultFileSystemActivated(int) {
        settingsChangedSlot();
    }
    void onShredSourceActivated() {
        settingsChangedSlot();
    }
    void onRaidConfigFilePathActivated() {
        settingsChangedSlot();
    }
    void onComboBackendActivated(int);

    GeneralPageWidget& generalPageWidget() {
        Q_ASSERT(m_GeneralPageWidget);
        return *m_GeneralPageWidget;
    }
    FileSystemColorsPageWidget& fileSystemColorsPageWidget() {
        Q_ASSERT(m_FileSystemColorsPageWidget);
        return *m_FileSystemColorsPageWidget;
    }
    AdvancedPageWidget& advancedPageWidget() {
        Q_ASSERT(m_AdvancedPageWidget);
        return *m_AdvancedPageWidget;
    }

    const OperationStack& operationStack() const {
        return m_OperationStack;
    }

private:
    GeneralPageWidget* m_GeneralPageWidget;
    FileSystemColorsPageWidget* m_FileSystemColorsPageWidget;
    AdvancedPageWidget* m_AdvancedPageWidget;
    const OperationStack& m_OperationStack;
};

#endif
