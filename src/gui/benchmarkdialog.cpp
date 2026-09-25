/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gui/benchmarkdialog.h"

#include <core/device.h>
#include <util/capacity.h>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <algorithm>
#include <cmath>

static constexpr qint64 MiB = 1024 * 1024;
static constexpr int accessTimeBatchSize = 64;

class BenchmarkGraphWidget : public QWidget
{
public:
    explicit BenchmarkGraphWidget(QWidget* parent) :
        QWidget(parent)
    {
        setBackgroundRole(QPalette::Base);
        setAutoFillBackground(true);
        setMinimumSize(560, 260);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void clear() {
        m_Rates.clear();
        m_AccessTimes.clear();
        update();
    }

    void addRate(qreal position, qreal bytesPerSecond) {
        m_Rates.append(QPointF(position, bytesPerSecond));
        update();
    }

    void addAccessTime(qreal position, qreal seconds) {
        m_AccessTimes.append(QPointF(position, seconds));
        update();
    }

    const QList<QPointF>& rates() const {
        return m_Rates;
    }

    const QList<QPointF>& accessTimes() const {
        return m_AccessTimes;
    }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QList<QPointF> m_Rates;
    QList<QPointF> m_AccessTimes;
};

static qreal niceCeiling(qreal value)
{
    if (value <= 0)
        return 1;

    const qreal magnitude = std::pow(10.0, std::floor(std::log10(value)));
    const qreal fraction = value / magnitude;
    const qreal nice = fraction <= 1 ? 1 : fraction <= 2 ? 2 : fraction <= 5 ? 5 : 10;
    return nice * magnitude;
}

static QString axisLabel(qreal value, qreal maximum)
{
    return QLocale().toString(value, 'f', maximum >= 10 ? 0 : maximum >= 1 ? 1 : 2);
}

void BenchmarkGraphWidget::paintEvent(QPaintEvent*)
{
    constexpr int divisions = 5;

    qreal maxRate = 0;
    for (const QPointF& p : std::as_const(m_Rates))
        maxRate = std::max(maxRate, p.y() / MiB);
    qreal maxTime = 0;
    for (const QPointF& p : std::as_const(m_AccessTimes))
        maxTime = std::max(maxTime, p.y() * 1000);
    maxRate = niceCeiling(maxRate * 1.1);
    maxTime = niceCeiling(maxTime * 1.1);

    QStringList rateLabels;
    QStringList timeLabels;
    for (int i = 0; i <= divisions; ++i) {
        rateLabels.append(xi18nc("@label graph axis, read rate", "%1 MiB/s", axisLabel(maxRate * i / divisions, maxRate)));
        timeLabels.append(xi18nc("@label graph axis, access time", "%1 ms", axisLabel(maxTime * i / divisions, maxTime)));
    }

    const QFontMetrics metrics = fontMetrics();
    int leftWidth = 0;
    int rightWidth = 0;
    for (int i = 0; i <= divisions; ++i) {
        leftWidth = std::max(leftWidth, metrics.horizontalAdvance(rateLabels[i]));
        rightWidth = std::max(rightWidth, metrics.horizontalAdvance(timeLabels[i]));
    }

    const int spacing = metrics.height() / 2;
    const QRectF plot(leftWidth + 2 * spacing, 2 * metrics.height() + spacing,
                      width() - leftWidth - rightWidth - 4 * spacing, height() - 4 * metrics.height() - 3 * spacing);
    if (plot.width() <= 0 || plot.height() <= 0)
        return;

    const QColor textColor = palette().color(QPalette::Text);
    QColor gridColor = textColor;
    gridColor.setAlphaF(0.15);
    const QColor rateColor = palette().color(QPalette::Highlight);
    const QColor timeColor = palette().color(QPalette::LinkVisited);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i <= divisions; ++i) {
        const qreal y = plot.bottom() - plot.height() * i / divisions;
        painter.setPen(gridColor);
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter.setPen(textColor);
        painter.drawText(QRectF(0, y - metrics.height() / 2.0, leftWidth + spacing, metrics.height()),
                         Qt::AlignRight | Qt::AlignVCenter, rateLabels[i]);
        painter.drawText(QRectF(plot.right() + spacing, y - metrics.height() / 2.0, rightWidth + spacing, metrics.height()),
                         Qt::AlignLeft | Qt::AlignVCenter, timeLabels[i]);
    }

    for (int percent = 0; percent <= 100; percent += 10) {
        const qreal x = plot.left() + plot.width() * percent / 100;
        painter.setPen(gridColor);
        painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
        if (percent % 20 == 0) {
            painter.setPen(textColor);
            const QString label = QLocale().toString(percent) + QLocale().percent();
            const qreal labelWidth = metrics.horizontalAdvance(label);
            painter.drawText(QRectF(x - labelWidth / 2, plot.bottom() + spacing / 2.0, labelWidth, metrics.height()),
                             Qt::AlignCenter, label);
        }
    }

    const QString positionLabel = xi18nc("@label graph axis", "Position on device");
    painter.drawText(QRectF(plot.left(), plot.bottom() + metrics.height() + spacing / 2.0, plot.width(), metrics.height()),
                     Qt::AlignCenter, positionLabel);

    auto toPoint = [&plot] (const QPointF& p, qreal maximum) {
        return QPointF(plot.left() + plot.width() * p.x(), plot.bottom() - plot.height() * std::min(p.y() / maximum, 1.0));
    };

    painter.setPen(Qt::NoPen);
    painter.setBrush(timeColor);
    for (const QPointF& p : std::as_const(m_AccessTimes))
        painter.drawEllipse(toPoint(QPointF(p.x(), p.y() * 1000), maxTime), 1.5, 1.5);

    if (!m_Rates.isEmpty()) {
        QPainterPath path(toPoint(QPointF(m_Rates.first().x(), m_Rates.first().y() / MiB), maxRate));
        for (const QPointF& p : std::as_const(m_Rates))
            path.lineTo(toPoint(QPointF(p.x(), p.y() / MiB), maxRate));
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(rateColor, 2));
        painter.drawPath(path);
    }

    const QString rateLegend = xi18nc("@label graph legend", "Read rate");
    const QString timeLegend = xi18nc("@label graph legend", "Access time");
    const qreal legendY = spacing / 2.0;
    qreal legendX = plot.left();
    painter.setPen(QPen(rateColor, 2));
    painter.drawLine(QPointF(legendX, legendY + metrics.height() / 2.0), QPointF(legendX + 2 * spacing, legendY + metrics.height() / 2.0));
    legendX += 3 * spacing;
    painter.setPen(textColor);
    painter.drawText(QPointF(legendX, legendY + metrics.ascent()), rateLegend);
    legendX += metrics.horizontalAdvance(rateLegend) + 2 * spacing;
    painter.setPen(Qt::NoPen);
    painter.setBrush(timeColor);
    painter.drawEllipse(QPointF(legendX + spacing, legendY + metrics.height() / 2.0), 3, 3);
    legendX += 3 * spacing;
    painter.setPen(textColor);
    painter.drawText(QPointF(legendX, legendY + metrics.ascent()), timeLegend);
}

BenchmarkDialog::BenchmarkDialog(QWidget* parent, Device& d) :
    QDialog(parent),
    m_Device(d),
    m_Benchmark(new DeviceReadBenchmark(d.deviceNode(), this)),
    m_Graph(new BenchmarkGraphWidget(this))
{
    setWindowTitle(xi18nc("@title:window", "Benchmark: <filename>%1</filename>", device().deviceNode()));

    setupDialog();
    setupConnections();

    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("benchmarkDialog"));
    restoreGeometry(kcg.readEntry<QByteArray>("Geometry", QByteArray()));
    m_TransferRateSamples->setValue(kcg.readEntry("TransferRateSamples", 100));
    m_SampleSize->setValue(kcg.readEntry("SampleSize", 10));
    m_AccessTimeSamples->setValue(kcg.readEntry("AccessTimeSamples", 1000));
}

BenchmarkDialog::~BenchmarkDialog()
{
    KConfigGroup kcg(KSharedConfig::openConfig(), QStringLiteral("benchmarkDialog"));
    kcg.writeEntry("Geometry", saveGeometry());
    kcg.writeEntry("TransferRateSamples", m_TransferRateSamples->value());
    kcg.writeEntry("SampleSize", m_SampleSize->value());
    kcg.writeEntry("AccessTimeSamples", m_AccessTimeSamples->value());
}

void BenchmarkDialog::setupDialog()
{
    m_TransferRateSamples = new QSpinBox(this);
    m_TransferRateSamples->setRange(2, 1000);

    m_SampleSize = new QSpinBox(this);
    m_SampleSize->setRange(1, 64);
    m_SampleSize->setSuffix(xi18nc("@item:valuesuffix", " MiB"));

    m_AccessTimeSamples = new QSpinBox(this);
    m_AccessTimeSamples->setRange(0, 10000);

    QFormLayout* settingsLayout = new QFormLayout;
    settingsLayout->addRow(xi18nc("@label:spinbox", "Transfer rate samples:"), m_TransferRateSamples);
    settingsLayout->addRow(xi18nc("@label:spinbox", "Sample size:"), m_SampleSize);
    settingsLayout->addRow(xi18nc("@label:spinbox", "Access time samples:"), m_AccessTimeSamples);

    m_AverageRate = new QLabel(this);
    m_MinimumRate = new QLabel(this);
    m_MaximumRate = new QLabel(this);
    m_AverageAccessTime = new QLabel(this);

    QFormLayout* resultsLayout = new QFormLayout;
    resultsLayout->addRow(xi18nc("@label", "Average read rate:"), m_AverageRate);
    resultsLayout->addRow(xi18nc("@label", "Minimum read rate:"), m_MinimumRate);
    resultsLayout->addRow(xi18nc("@label", "Maximum read rate:"), m_MaximumRate);
    resultsLayout->addRow(xi18nc("@label", "Average access time:"), m_AverageAccessTime);

    QHBoxLayout* formsLayout = new QHBoxLayout;
    formsLayout->addLayout(settingsLayout);
    formsLayout->addSpacing(fontMetrics().height());
    formsLayout->addLayout(resultsLayout);
    formsLayout->addStretch();

    m_Progress = new QProgressBar(this);
    m_Progress->setRange(0, 1);
    m_Progress->setValue(0);

    m_Status = new QLabel(xi18nc("@info", "The device is only read, no data is written. "
                                          "Other activity on the device affects the results."), this);
    m_Status->setWordWrap(true);

    m_ButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_StartButton = m_ButtonBox->addButton(xi18nc("@action:button", "Start Benchmark"), QDialogButtonBox::ActionRole);
    m_StartButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_StopButton = m_ButtonBox->addButton(xi18nc("@action:button", "Stop"), QDialogButtonBox::ActionRole);
    m_StopButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formsLayout);
    mainLayout->addWidget(m_Graph, 1);
    mainLayout->addWidget(m_Progress);
    mainLayout->addWidget(m_Status);
    mainLayout->addWidget(m_ButtonBox);

    setRunning(false);
    updateResults();
}

void BenchmarkDialog::setupConnections()
{
    connect(m_ButtonBox, &QDialogButtonBox::rejected, this, &BenchmarkDialog::reject);
    connect(m_StartButton, &QPushButton::clicked, this, &BenchmarkDialog::start);
    connect(m_StopButton, &QPushButton::clicked, this, &BenchmarkDialog::stop);
    connect(m_Benchmark, &DeviceReadBenchmark::finished, this, &BenchmarkDialog::onBenchmarkFinished);
}

void BenchmarkDialog::setRunning(bool running)
{
    m_TransferRateSamples->setEnabled(!running);
    m_SampleSize->setEnabled(!running);
    m_AccessTimeSamples->setEnabled(!running);
    m_StartButton->setEnabled(!running);
    m_StopButton->setEnabled(running);
}

void BenchmarkDialog::start()
{
    m_Graph->clear();
    m_StopRequested = false;
    m_SampleCount = m_TransferRateSamples->value();
    m_AccessCount = m_AccessTimeSamples->value();
    m_SamplesDone = 0;
    m_AccessesDone = 0;
    updateResults();

    m_Progress->setRange(0, m_SampleCount + m_AccessCount);
    m_Progress->setValue(0);
    m_Status->setText(xi18nc("@info:status", "Preparing the benchmark…"));

    m_Phase = Phase::Probing;
    setRunning(true);
    if (!m_Benchmark->request({}, 0))
        finish(xi18nc("@info:status", "Could not connect to the KPMcore helper."));
}

void BenchmarkDialog::stop()
{
    m_StopRequested = true;
    m_StopButton->setEnabled(false);
    m_Status->setText(xi18nc("@info:status", "Stopping after the current read request finishes…"));
}

void BenchmarkDialog::finish(const QString& message)
{
    m_Phase = Phase::Idle;
    setRunning(false);
    m_Status->setText(message);
}

void BenchmarkDialog::onBenchmarkFinished(const DeviceReadBenchmark::Result& result)
{
    switch (result.status) {
    case DeviceReadBenchmark::Status::Success:
        break;
    case DeviceReadBenchmark::Status::Unsupported:
        finish(xi18nc("@info:status", "Direct I/O is not supported for this device. The benchmark was not run, "
                                      "because cached reads would not show the speed of the device."));
        return;
    case DeviceReadBenchmark::Status::InvalidRequest:
        finish(xi18nc("@info:status", "The benchmark request was rejected as invalid."));
        return;
    case DeviceReadBenchmark::Status::Failed:
        finish(xi18nc("@info:status", "Reading from the device failed or access was denied."));
        return;
    }

    switch (m_Phase) {
    case Phase::Idle:
        return;
    case Phase::Probing:
        if (!prepare(result))
            return;
        m_Phase = Phase::TransferRate;
        requestTransferRateSample();
        return;
    case Phase::TransferRate: {
        qint64 elapsedNs = 0;
        for (const qint64 ns : result.elapsedNs)
            elapsedNs += ns;
        const qint64 sampleLength = m_ReadLength * m_ReadsPerSample;
        const qreal position = static_cast<qreal>(m_SampleOffset + sampleLength / 2) / m_DeviceSize;
        m_Graph->addRate(position, sampleLength * 1e9 / std::max<qint64>(elapsedNs, 1));
        ++m_SamplesDone;
        updateResults();
        requestTransferRateSample();
        return;
    }
    case Phase::AccessTime:
        for (int i = 0; i < result.elapsedNs.size() && i < m_AccessOffsets.size(); ++i)
            m_Graph->addAccessTime(static_cast<qreal>(m_AccessOffsets[i]) / m_DeviceSize, result.elapsedNs[i] / 1e9);
        m_AccessesDone += m_AccessOffsets.size();
        updateResults();
        requestAccessTimeBatch();
        return;
    }
}

bool BenchmarkDialog::prepare(const DeviceReadBenchmark::Result& result)
{
    m_DeviceSize = result.deviceSize;
    m_Alignment = std::max(result.logicalBlockSize, result.physicalBlockSize);
    if (m_DeviceSize <= 0 || m_Alignment <= 0) {
        finish(xi18nc("@info:status", "Could not determine the size of the device."));
        return false;
    }

    auto alignDown = [this] (qint64 value) {
        return value - value % m_Alignment;
    };

    const qint64 sampleSize = std::min(m_SampleSize->value() * MiB, m_DeviceSize);
    m_ReadsPerSample = static_cast<int>((sampleSize + DeviceReadBenchmark::maxReadLength - 1) / DeviceReadBenchmark::maxReadLength);
    m_ReadLength = alignDown(sampleSize / m_ReadsPerSample);
    if (m_ReadLength <= 0)
        m_SampleCount = 0;

    m_AccessBlockSize = ((std::max<qint64>(m_Alignment, 4096) + m_Alignment - 1) / m_Alignment) * m_Alignment;
    if (m_AccessBlockSize > m_DeviceSize)
        m_AccessCount = 0;

    m_Progress->setRange(0, std::max(m_SampleCount + m_AccessCount, 1));
    return true;
}

void BenchmarkDialog::requestTransferRateSample()
{
    if (m_StopRequested) {
        finish(xi18nc("@info:status", "Benchmark stopped."));
        return;
    }

    if (m_SamplesDone >= m_SampleCount) {
        m_Phase = Phase::AccessTime;
        requestAccessTimeBatch();
        return;
    }

    const qint64 sampleLength = m_ReadLength * m_ReadsPerSample;
    const qint64 span = m_DeviceSize - sampleLength;
    m_SampleOffset = span * m_SamplesDone / std::max(m_SampleCount - 1, 1);
    m_SampleOffset -= m_SampleOffset % m_Alignment;

    QList<qint64> offsets;
    for (int i = 0; i < m_ReadsPerSample; ++i)
        offsets.append(m_SampleOffset + i * m_ReadLength);

    m_Status->setText(xi18nc("@info:status", "Measuring transfer rate: sample %1 of %2…", m_SamplesDone + 1, m_SampleCount));
    if (!m_Benchmark->request(offsets, m_ReadLength))
        finish(xi18nc("@info:status", "Could not connect to the KPMcore helper."));
}

void BenchmarkDialog::requestAccessTimeBatch()
{
    if (m_StopRequested) {
        finish(xi18nc("@info:status", "Benchmark stopped."));
        return;
    }

    if (m_AccessesDone >= m_AccessCount) {
        finish(xi18nc("@info:status", "Benchmark finished."));
        return;
    }

    const qint64 blocks = m_DeviceSize / m_AccessBlockSize;
    const int batchSize = std::min(accessTimeBatchSize, m_AccessCount - m_AccessesDone);
    m_AccessOffsets.clear();
    for (int i = 0; i < batchSize; ++i)
        m_AccessOffsets.append(QRandomGenerator::global()->bounded(blocks) * m_AccessBlockSize);

    m_Status->setText(xi18nc("@info:status", "Measuring access time: %1 of %2 reads…", m_AccessesDone, m_AccessCount));
    if (!m_Benchmark->request(m_AccessOffsets, m_AccessBlockSize))
        finish(xi18nc("@info:status", "Could not connect to the KPMcore helper."));
}

void BenchmarkDialog::updateResults()
{
    const QList<QPointF>& rates = m_Graph->rates();
    const QList<QPointF>& accessTimes = m_Graph->accessTimes();
    const QString unknown = xi18nc("@label benchmark result not measured yet", "—");

    if (rates.isEmpty()) {
        m_AverageRate->setText(unknown);
        m_MinimumRate->setText(unknown);
        m_MaximumRate->setText(unknown);
    } else {
        qreal inverseSum = 0;
        qreal minimum = rates.first().y();
        qreal maximum = rates.first().y();
        for (const QPointF& p : rates) {
            inverseSum += 1 / p.y();
            minimum = std::min(minimum, p.y());
            maximum = std::max(maximum, p.y());
        }
        auto formatRate = [] (qreal bytesPerSecond) {
            return xi18nc("@label read rate, %1 is a size such as 120.5 MiB", "%1/s", Capacity::formatByteSize(bytesPerSecond, 1));
        };
        m_AverageRate->setText(formatRate(rates.size() / inverseSum));
        m_MinimumRate->setText(formatRate(minimum));
        m_MaximumRate->setText(formatRate(maximum));
    }

    if (accessTimes.isEmpty())
        m_AverageAccessTime->setText(unknown);
    else {
        qreal sum = 0;
        for (const QPointF& p : accessTimes)
            sum += p.y();
        m_AverageAccessTime->setText(xi18nc("@label access time in milliseconds", "%1 ms",
                                            QLocale().toString(sum / accessTimes.size() * 1000, 'f', 2)));
    }

    m_Progress->setValue(m_SamplesDone + m_AccessesDone);
}

#include "moc_benchmarkdialog.cpp"
