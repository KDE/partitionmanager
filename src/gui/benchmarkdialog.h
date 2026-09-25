/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#if !defined(BENCHMARKDIALOG_H)

#define BENCHMARKDIALOG_H

#include <util/devicereadbenchmark.h>

#include <QDialog>
#include <QList>

class Device;
class BenchmarkGraphWidget;

class QDialogButtonBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;

class BenchmarkDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(BenchmarkDialog)

public:
    BenchmarkDialog(QWidget* parent, Device& d);
    ~BenchmarkDialog() override;

private:
    enum class Phase {
        Idle,
        Probing,
        TransferRate,
        AccessTime
    };

    void setupDialog();
    void setupConnections();

    void start();
    void stop();
    void finish(const QString& message);
    void setRunning(bool running);

    void onBenchmarkFinished(const DeviceReadBenchmark::Result& result);
    bool prepare(const DeviceReadBenchmark::Result& result);
    void requestTransferRateSample();
    void requestAccessTimeBatch();
    void updateResults();

    Device& device() {
        return m_Device;
    }

private:
    Device& m_Device;
    DeviceReadBenchmark* m_Benchmark;
    BenchmarkGraphWidget* m_Graph;

    QSpinBox* m_TransferRateSamples;
    QSpinBox* m_SampleSize;
    QSpinBox* m_AccessTimeSamples;

    QLabel* m_AverageRate;
    QLabel* m_MinimumRate;
    QLabel* m_MaximumRate;
    QLabel* m_AverageAccessTime;
    QLabel* m_Status;
    QProgressBar* m_Progress;

    QDialogButtonBox* m_ButtonBox;
    QPushButton* m_StartButton;
    QPushButton* m_StopButton;

    Phase m_Phase = Phase::Idle;
    bool m_StopRequested = false;

    qint64 m_DeviceSize = 0;
    qint64 m_Alignment = 0;
    qint64 m_ReadLength = 0;
    int m_ReadsPerSample = 0;
    qint64 m_AccessBlockSize = 0;

    int m_SampleCount = 0;
    int m_AccessCount = 0;
    int m_SamplesDone = 0;
    int m_AccessesDone = 0;
    qint64 m_SampleOffset = 0;
    QList<qint64> m_AccessOffsets;
};

#endif
