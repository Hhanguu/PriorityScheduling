#include "MainWindow.h"
#include "GanttChart.h"

#include "core/Metrics.h"
#include "core/NonPreemptiveScheduler.h"
#include "core/PreemptiveScheduler.h"
#include "io/CsvHandler.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace {
constexpr int kMaxTableRows = 10000;

QString formatAverage(double value) {
    return QString::number(value, 'f', 2);
}

QTableWidgetItem *readOnlyItem(const QString &text) {
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    resetResults();
}

void MainWindow::setupUI() {
    setWindowTitle("Priority Scheduling - OS Project");
    resize(1180, 900);

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    auto *topLayout = new QHBoxLayout();
    btnOpen = new QPushButton("Chọn file CSV", this);
    btnOpen->setObjectName("btnOpen");
    comboMode = new QComboBox(this);
    comboMode->setObjectName("comboMode");
    comboMode->addItem("Chỉ Non-Preemptive", 0);
    comboMode->addItem("Chỉ Preemptive", 1);
    comboMode->addItem("Cả hai + so sánh", 2);
    comboMode->setCurrentIndex(2);
    comboAlgorithm = new QComboBox(this);
    comboAlgorithm->setObjectName("comboAlgorithm");
    comboAlgorithm->addItem("Non-Preemptive Priority", 0);
    comboAlgorithm->addItem("Preemptive Priority", 1);
    btnRun = new QPushButton("Chạy lập lịch", this);
    btnRun->setObjectName("btnRun");
    btnExport = new QPushButton("Xuất file kết quả", this);
    btnExport->setObjectName("btnExport");
    btnExport->setEnabled(false);

    topLayout->addWidget(btnOpen);
    topLayout->addWidget(new QLabel("Chế độ chạy:", this));
    topLayout->addWidget(comboMode);
    topLayout->addWidget(new QLabel("Hiển thị:", this));
    topLayout->addWidget(comboAlgorithm);
    topLayout->addWidget(btnRun);
    topLayout->addWidget(btnExport);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);

    auto *inputGroup = new QGroupBox("Danh sách tiến trình đầu vào", splitter);
    auto *inputLayout = new QVBoxLayout(inputGroup);
    tableInput = new QTableWidget(inputGroup);
    tableInput->setObjectName("tableInput");
    tableInput->setColumnCount(4);
    tableInput->setHorizontalHeaderLabels({"PID", "Arrival Time", "Burst Time", "Priority"});
    tableInput->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableInput->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableInput->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputLayout->addWidget(tableInput);
    splitter->addWidget(inputGroup);

    auto *resultGroup = new QGroupBox("Kết quả thực thi chi tiết", splitter);
    auto *resultLayout = new QVBoxLayout(resultGroup);
    tableResult = new QTableWidget(resultGroup);
    tableResult->setObjectName("tableResult");
    tableResult->setColumnCount(8);
    tableResult->setHorizontalHeaderLabels(
        {"PID", "Arrival Time", "Burst Time", "Priority", "CT", "TAT", "WT", "RT"});
    tableResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableResult->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultLayout->addWidget(tableResult);
    splitter->addWidget(resultGroup);

    auto *ganttGroup = new QGroupBox("Biểu đồ Gantt", splitter);
    auto *ganttLayout = new QVBoxLayout(ganttGroup);
    ganttChart = new GanttChart(ganttGroup);
    ganttChart->setObjectName("ganttChart");
    ganttLayout->addWidget(ganttChart);
    splitter->addWidget(ganttGroup);

    auto *comparisonGroup = new QGroupBox("So sánh hai thuật toán", splitter);
    auto *comparisonLayout = new QVBoxLayout(comparisonGroup);
    tableComparison = new QTableWidget(comparisonGroup);
    tableComparison->setObjectName("tableComparison");
    tableComparison->setColumnCount(3);
    tableComparison->setHorizontalHeaderLabels(
        {"Chỉ số", "Non-Preemptive", "Preemptive"});
    tableComparison->verticalHeader()->setVisible(false);
    tableComparison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableComparison->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableComparison->setRowCount(3);
    tableComparison->setItem(0, 0, readOnlyItem("Avg Waiting Time"));
    tableComparison->setItem(1, 0, readOnlyItem("Avg Turnaround Time"));
    tableComparison->setItem(2, 0, readOnlyItem("Avg Response Time"));
    comparisonLayout->addWidget(tableComparison);
    splitter->addWidget(comparisonGroup);

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    splitter->setStretchFactor(2, 2);
    splitter->setStretchFactor(3, 1);
    mainLayout->addWidget(splitter, 1);

    lblLimit = new QLabel(this);
    lblLimit->setObjectName("lblLimit");
    lblLimit->setStyleSheet("color: #b36b00;");
    lblLimit->setVisible(false);
    mainLayout->addWidget(lblLimit);

    auto *metricsLayout = new QHBoxLayout();
    lblAvgWaiting = new QLabel(this);
    lblAvgTurnaround = new QLabel(this);
    lblAvgResponse = new QLabel(this);
    lblAvgWaiting->setObjectName("lblAvgWaiting");
    lblAvgTurnaround->setObjectName("lblAvgTurnaround");
    lblAvgResponse->setObjectName("lblAvgResponse");
    lblElapsed = new QLabel(this);
    lblElapsed->setObjectName("lblElapsed");
    lblElapsed->setStyleSheet("color: #79c0ff; font-weight: bold;");
    metricsLayout->addWidget(lblAvgWaiting);
    metricsLayout->addWidget(lblAvgTurnaround);
    metricsLayout->addWidget(lblAvgResponse);
    metricsLayout->addStretch();
    metricsLayout->addWidget(lblElapsed);
    mainLayout->addLayout(metricsLayout);

    setCentralWidget(centralWidget);
    statusBar()->showMessage("Chọn file CSV để bắt đầu.");

    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::handleOpenFile);
    connect(btnRun, &QPushButton::clicked, this, &MainWindow::handleRunSchedule);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::handleExportOutput);
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleModeChanged);
    connect(comboAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleAlgorithmChanged);
}

void MainWindow::handleOpenFile() {
    QString startDir = QDir::current().filePath("input");
    if (!QDir(startDir).exists()) {
        startDir = QDir::currentPath();
    }

    const QString filePath = QFileDialog::getOpenFileName(
        this, "Chọn file CSV đầu vào", startDir, "CSV Files (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    QString error;
    if (!loadCsv(filePath, &error)) {
        QMessageBox::critical(this, "Lỗi đọc file CSV", error);
        return;
    }

    statusBar()->showMessage(
        QString("Đã nạp %1 tiến trình từ %2").arg(inputProcesses.size()).arg(filePath));
}

void MainWindow::handleRunSchedule() {
    QString error;
    if (!runSchedules(&error)) {
        QMessageBox::warning(this, "Cảnh báo", error);
        return;
    }

    const auto fmt = [](qint64 ms) {
        if (ms <= 0) {
            return QStringLiteral("<1 ms");
        }
        return QLocale(QLocale::Vietnamese).toString(ms) + QStringLiteral(" ms");
    };

    if (npReady && pReady) {
        statusBar()->showMessage(
            QString("Đã chạy xong cả hai thuật toán: NP %1, P %2.")
                .arg(fmt(npElapsedMs), fmt(pElapsedMs)));
        lblElapsed->setText(
            QString("Benchmark: NP %1 | P %2").arg(fmt(npElapsedMs), fmt(pElapsedMs)));
    } else if (npReady) {
        statusBar()->showMessage(
            QString("Đã chạy xong Non-Preemptive trong %1.").arg(fmt(npElapsedMs)));
        lblElapsed->setText(QString("Benchmark: NP %1").arg(fmt(npElapsedMs)));
    } else {
        statusBar()->showMessage(
            QString("Đã chạy xong Preemptive trong %1.").arg(fmt(pElapsedMs)));
        lblElapsed->setText(QString("Benchmark: P %1").arg(fmt(pElapsedMs)));
    }
}

void MainWindow::handleExportOutput() {
    if (!hasResults()) {
        return;
    }

    QString startDir = QDir::current().filePath("output");
    if (!QDir(startDir).exists()) {
        startDir = QDir::currentPath();
    }
    const int fileCount = (npReady ? 2 : 0) + (pReady ? 2 : 0)
                        + (npReady && pReady ? 1 : 0);
    const QString dirPath = QFileDialog::getExistingDirectory(
        this, QString("Chọn thư mục lưu %1 file kết quả").arg(fileCount), startDir);
    if (dirPath.isEmpty()) {
        return;
    }

    QString error;
    if (!exportResults(dirPath, &error)) {
        QMessageBox::critical(this, "Lỗi ghi file", error);
        return;
    }

    QStringList files;
    if (npReady) {
        files << "np_result.csv" << "np_gantt.csv";
    }
    if (pReady) {
        files << "p_result.csv" << "p_gantt.csv";
    }
    if (npReady && pReady) {
        files << "comparison.csv";
    }

    QMessageBox::information(
        this, "Thành công",
        QString("Đã xuất %1 file vào:\n%2\n\n%3")
            .arg(files.size()).arg(dirPath).arg(files.join('\n')));
    statusBar()->showMessage(QString("Đã xuất %1 file CSV kết quả.").arg(files.size()));
}

void MainWindow::handleAlgorithmChanged(int index) {
    Q_UNUSED(index);
    if (!hasResults()) {
        return;
    }
    updateResultTable();
    updateAverages();
    updateGantt();
    updateTableLimitNotice();
}

void MainWindow::handleModeChanged(int index) {
    QSignalBlocker blocker(comboAlgorithm);
    comboAlgorithm->setEnabled(index == 2);
    if (index == 0) {
        comboAlgorithm->setCurrentIndex(0);
    } else if (index == 1) {
        comboAlgorithm->setCurrentIndex(1);
    }
    resetResults();
    statusBar()->showMessage("Đã đổi chế độ chạy. Bấm Chạy lập lịch để chạy lại.");
}

void MainWindow::resetResults() {
    npResult = ScheduleResult{};
    pResult = ScheduleResult{};
    npReady = false;
    pReady = false;
    resultsReady = false;
    npElapsedMs = 0;
    pElapsedMs = 0;
    if (lblElapsed != nullptr) {
        lblElapsed->clear();
    }
    if (btnExport != nullptr) {
        btnExport->setEnabled(false);
    }
    updateResultTable();
    updateComparisonTable();
    updateAverages();
    updateGantt();
    updateTableLimitNotice();
}

bool MainWindow::loadCsv(const QString &path, QString *errorMessage) {
    std::string error;
    std::vector<Process> loadedProcesses;
    if (!CsvHandler::readProcesses(path.toStdString(), loadedProcesses, error)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromStdString(error);
        }
        return false;
    }

    inputProcesses = std::move(loadedProcesses);
    resetResults();
    updateInputTable();
    updateTableLimitNotice();
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::runSchedules(QString *errorMessage) {
    if (inputProcesses.empty()) {
        if (errorMessage != nullptr) {
            *errorMessage = "Chưa có dữ liệu tiến trình. Vui lòng mở file CSV trước.";
        }
        return false;
    }

    const int mode = comboMode->currentIndex();
    npResult = ScheduleResult{};
    pResult = ScheduleResult{};
    npReady = false;
    pReady = false;

    if (mode == 0 || mode == 2) {
        QElapsedTimer timer;
        timer.start();
        npResult = NonPreemptiveScheduler::schedule(inputProcesses);
        npElapsedMs = timer.elapsed();
        Metrics::fillAverages(npResult);
        npReady = true;
    }
    if (mode == 1 || mode == 2) {
        QElapsedTimer timer;
        timer.start();
        pResult = PreemptiveScheduler::schedule(inputProcesses);
        pElapsedMs = timer.elapsed();
        Metrics::fillAverages(pResult);
        pReady = true;
    }

    {
        QSignalBlocker blocker(comboAlgorithm);
        comboAlgorithm->setEnabled(mode == 2);
        comboAlgorithm->setCurrentIndex(mode == 1 ? 1 : 0);
    }

    resultsReady = npReady || pReady;
    btnExport->setEnabled(resultsReady);
    updateTables();
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::exportResults(const QString &dirPath, QString *errorMessage) {
    if (!hasResults()) {
        if (errorMessage != nullptr) {
            *errorMessage = "Chưa có kết quả để xuất. Vui lòng chạy lập lịch trước.";
        }
        return false;
    }

    const QDir outputDir(dirPath);
    std::string error;
    bool ok = true;

    if (ok && npReady) {
        ok = CsvHandler::writeProcessResults(
                 outputDir.filePath("np_result.csv").toStdString(), npResult, error) &&
             CsvHandler::writeGantt(
                 outputDir.filePath("np_gantt.csv").toStdString(), npResult, error);
    }
    if (ok && pReady) {
        ok = CsvHandler::writeProcessResults(
                 outputDir.filePath("p_result.csv").toStdString(), pResult, error) &&
             CsvHandler::writeGantt(
                 outputDir.filePath("p_gantt.csv").toStdString(), pResult, error);
    }
    if (ok && npReady && pReady) {
        ok = CsvHandler::writeComparison(
            outputDir.filePath("comparison.csv").toStdString(), npResult, pResult, error);
    }

    if (!ok) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromStdString(error);
        }
        return false;
    }

    if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::hasResults() const {
    return resultsReady;
}

void MainWindow::updateTables() {
    updateInputTable();
    updateResultTable();
    updateComparisonTable();
    updateAverages();
    updateGantt();
    updateTableLimitNotice();
}

void MainWindow::updateInputTable() {
    const int visibleRows =
        std::min(static_cast<int>(inputProcesses.size()), kMaxTableRows);
    tableInput->setRowCount(visibleRows);
    for (int i = 0; i < visibleRows; ++i) {
        const Process &process = inputProcesses[static_cast<std::size_t>(i)];
        tableInput->setItem(i, 0, readOnlyItem(QStringLiteral("P%1").arg(process.pid)));
        tableInput->setItem(i, 1, readOnlyItem(QString::number(process.arrivalTime)));
        tableInput->setItem(i, 2, readOnlyItem(QString::number(process.burstTime)));
        tableInput->setItem(i, 3, readOnlyItem(QString::number(process.priority)));
    }
}

void MainWindow::updateResultTable() {
    const ScheduleResult &result = activeResult();
    const int visibleRows =
        std::min(static_cast<int>(result.processes.size()), kMaxTableRows);
    tableResult->setRowCount(visibleRows);
    for (int i = 0; i < visibleRows; ++i) {
        const ProcessResult &row = result.processes[static_cast<std::size_t>(i)];
        const Process *input = findInputProcess(row.pid);
        const long long arrival = input != nullptr ? input->arrivalTime : 0;
        const long long burst = input != nullptr ? input->burstTime : 0;
        const int priority = input != nullptr ? input->priority : 0;

        tableResult->setItem(i, 0, readOnlyItem(QStringLiteral("P%1").arg(row.pid)));
        tableResult->setItem(i, 1, readOnlyItem(QString::number(arrival)));
        tableResult->setItem(i, 2, readOnlyItem(QString::number(burst)));
        tableResult->setItem(i, 3, readOnlyItem(QString::number(priority)));
        tableResult->setItem(i, 4, readOnlyItem(QString::number(row.completionTime)));
        tableResult->setItem(i, 5, readOnlyItem(QString::number(row.turnaroundTime)));
        tableResult->setItem(i, 6, readOnlyItem(QString::number(row.waitingTime)));
        tableResult->setItem(i, 7, readOnlyItem(QString::number(row.responseTime)));
    }
}

void MainWindow::updateComparisonTable() {
    const Comparison comparison = Metrics::compare(npResult, pResult);
    const QString npDash = npReady ? formatAverage(comparison.waitingTime.np) : "—";
    const QString pDash = pReady ? formatAverage(comparison.waitingTime.p) : "—";
    tableComparison->setItem(0, 1, readOnlyItem(npDash));
    tableComparison->setItem(0, 2, readOnlyItem(pDash));
    tableComparison->setItem(
        1, 1, readOnlyItem(npReady ? formatAverage(comparison.turnaroundTime.np) : "—"));
    tableComparison->setItem(
        1, 2, readOnlyItem(pReady ? formatAverage(comparison.turnaroundTime.p) : "—"));
    tableComparison->setItem(
        2, 1, readOnlyItem(npReady ? formatAverage(comparison.responseTime.np) : "—"));
    tableComparison->setItem(
        2, 2, readOnlyItem(pReady ? formatAverage(comparison.responseTime.p) : "—"));
}

void MainWindow::updateAverages() {
    const ScheduleResult &result = activeResult();
    lblAvgWaiting->setText(
        QString("Avg Waiting Time: %1").arg(formatAverage(result.averageWaitingTime)));
    lblAvgTurnaround->setText(
        QString("Avg Turnaround Time: %1").arg(formatAverage(result.averageTurnaroundTime)));
    lblAvgResponse->setText(
        QString("Avg Response Time: %1").arg(formatAverage(result.averageResponseTime)));
}

void MainWindow::updateGantt() {
    QVector<GanttBlock> blocks;
    const ScheduleResult &result = activeResult();
    blocks.reserve(static_cast<int>(result.gantt.size()));
    for (const GanttSegment &segment : result.gantt) {
        blocks.append(GanttBlock{segment.pid, segment.startTime, segment.endTime});
    }
    ganttChart->setScheduleData(blocks);
}

void MainWindow::updateTableLimitNotice() {
    QStringList notices;
    if (inputProcesses.size() > static_cast<std::size_t>(kMaxTableRows)) {
        notices << QString("input: %1/%2 dòng")
                       .arg(kMaxTableRows)
                       .arg(inputProcesses.size());
    }
    const std::size_t resultRows = activeResult().processes.size();
    if (resultRows > static_cast<std::size_t>(kMaxTableRows)) {
        notices << QString("kết quả: %1/%2 dòng")
                       .arg(kMaxTableRows)
                       .arg(resultRows);
    }

    if (notices.isEmpty()) {
        lblLimit->setVisible(false);
        return;
    }

    lblLimit->setText(
        QString("Bảng chỉ hiển thị %1; file CSV xuất vẫn đủ toàn bộ.")
            .arg(notices.join(", ")));
    lblLimit->setVisible(true);
}

const Process *MainWindow::findInputProcess(int pid) const {
    for (const Process &process : inputProcesses) {
        if (process.pid == pid) {
            return &process;
        }
    }
    return nullptr;
}

const ScheduleResult &MainWindow::activeResult() const {
    return comboAlgorithm->currentIndex() == 0 ? npResult : pResult;
}
