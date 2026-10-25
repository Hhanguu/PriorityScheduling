
#include "MainWindow.h"
#include "GanttChart.h"

#include "core/Process.h"
#include "core/NonPreemptiveScheduler.h"
#include "core/PreemptiveScheduler.h"
#include "core/ScheduleResult.h"
#include "core/Metrics.h"
#include "io/CsvHandler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <string>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    setWindowTitle("Priority Scheduling - OS Project");
    resize(1000, 750);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *topLayout = new QHBoxLayout();

    btnOpen = new QPushButton("Chon file CSV", this);

    comboAlgorithm = new QComboBox(this);
    comboAlgorithm->addItem("Non-Preemptive Priority", 0);
    comboAlgorithm->addItem("Preemptive Priority", 1);

    btnRun = new QPushButton("Chay lap lich", this);
    btnExport = new QPushButton("Xuat file ket qua", this);
    btnExport->setEnabled(false);

    topLayout->addWidget(btnOpen);
    topLayout->addWidget(new QLabel("Thuat toan:", this));
    topLayout->addWidget(comboAlgorithm);
    topLayout->addWidget(btnRun);
    topLayout->addWidget(btnExport);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    mainLayout->addWidget(
        new QLabel("Danh sach tien trinh dau vao:", this)
    );

    tableInput = new QTableWidget(this);
    tableInput->setColumnCount(4);

    tableInput->setHorizontalHeaderLabels({
        "PID", "Arrival Time", "Burst Time", "Priority"
    });

    tableInput->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );

    mainLayout->addWidget(tableInput);

    mainLayout->addWidget(
        new QLabel("Ket qua thuc thi chi tiet:", this)
    );

    tableResult = new QTableWidget(this);
    tableResult->setColumnCount(8);

    tableResult->setHorizontalHeaderLabels({
        "PID", "Arrival Time", "Burst Time", "Priority",
        "CT", "TAT", "WT", "RT"
    });

    tableResult->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );

    mainLayout->addWidget(tableResult);

    QHBoxLayout *metricsLayout = new QHBoxLayout();

    lblAvgWaiting = new QLabel("Avg Waiting Time: 0.00", this);
    lblAvgTurnaround = new QLabel("Avg Turnaround Time: 0.00", this);
    lblAvgResponse = new QLabel("Avg Response Time: 0.00", this);

    metricsLayout->addWidget(lblAvgWaiting);
    metricsLayout->addWidget(lblAvgTurnaround);
    metricsLayout->addWidget(lblAvgResponse);

    mainLayout->addLayout(metricsLayout);

    setCentralWidget(centralWidget);

    connect(
        btnOpen, &QPushButton::clicked,
        this, &MainWindow::handleOpenFile
    );

    connect(
        btnRun, &QPushButton::clicked,
        this, &MainWindow::handleRunSchedule
    );

    connect(
        btnExport, &QPushButton::clicked,
        this, &MainWindow::handleExportOutput
    );
}

void MainWindow::handleOpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Chon file CSV dau vao",
        "",
        "CSV Files (*.csv)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    std::string error;
    std::vector<Process> loadedProcesses;

    bool success = CsvHandler::readProcesses(
        filePath.toStdString(),
        loadedProcesses,
        error
    );

    if (!success) {
        QMessageBox::critical(
            this,
            "Loi doc file CSV",
            QString::fromStdString(error)
        );
        return;
    }

    inputProcesses = loadedProcesses;

    hasResult = false;
    btnExport->setEnabled(false);

    updateTables();
}

void MainWindow::updateTables()
{
    tableInput->setRowCount(
        static_cast<int>(inputProcesses.size())
    );

    for (size_t i = 0; i < inputProcesses.size(); ++i) {
        const auto& p = inputProcesses[i];

        tableInput->setItem(
            static_cast<int>(i), 0,
            new QTableWidgetItem(
                "P" + QString::number(p.pid)
            )
        );

        tableInput->setItem(
            static_cast<int>(i), 1,
            new QTableWidgetItem(
                QString::number(p.arrivalTime)
            )
        );

        tableInput->setItem(
            static_cast<int>(i), 2,
            new QTableWidgetItem(
                QString::number(p.burstTime)
            )
        );

        tableInput->setItem(
            static_cast<int>(i), 3,
            new QTableWidgetItem(
                QString::number(p.priority)
            )
        );
    }

    tableResult->setRowCount(0);

    lblAvgWaiting->setText("Avg Waiting Time: 0.00");
    lblAvgTurnaround->setText("Avg Turnaround Time: 0.00");
    lblAvgResponse->setText("Avg Response Time: 0.00");
}

void MainWindow::handleRunSchedule()
{
    if (inputProcesses.empty()) {
        QMessageBox::warning(
            this,
            "Canh bao",
            "Chua co du lieu tien trinh! Vui long mo file CSV truoc."
        );
        return;
    }

    int algoIndex = comboAlgorithm->currentIndex();

    if (algoIndex == 0) {
        currentResult =
            NonPreemptiveScheduler::schedule(inputProcesses);
    } else {
        currentResult =
            PreemptiveScheduler::schedule(inputProcesses);
    }

    Metrics::fillAverages(currentResult);

    tableResult->setRowCount(
        static_cast<int>(currentResult.processes.size())
    );

    for (size_t i = 0; i < currentResult.processes.size(); ++i) {
        const auto& res = currentResult.processes[i];

        long long at = 0;
        long long bt = 0;
        int pr = 0;

        for (const auto& p : inputProcesses) {
            if (p.pid == res.pid) {
                at = p.arrivalTime;
                bt = p.burstTime;
                pr = p.priority;
                break;
            }
        }

        int row = static_cast<int>(i);

        tableResult->setItem(
            row, 0,
            new QTableWidgetItem(
                "P" + QString::number(res.pid)
            )
        );

        tableResult->setItem(
            row, 1,
            new QTableWidgetItem(QString::number(at))
        );

        tableResult->setItem(
            row, 2,
            new QTableWidgetItem(QString::number(bt))
        );

        tableResult->setItem(
            row, 3,
            new QTableWidgetItem(QString::number(pr))
        );

        tableResult->setItem(
            row, 4,
            new QTableWidgetItem(
                QString::number(res.completionTime)
            )
        );

        tableResult->setItem(
            row, 5,
            new QTableWidgetItem(
                QString::number(res.turnaroundTime)
            )
        );

        tableResult->setItem(
            row, 6,
            new QTableWidgetItem(
                QString::number(res.waitingTime)
            )
        );

        tableResult->setItem(
            row, 7,
            new QTableWidgetItem(
                QString::number(res.responseTime)
            )
        );
    }

    lblAvgWaiting->setText(
        QString("Avg Waiting Time: %1")
            .arg(currentResult.averageWaitingTime, 0, 'f', 2)
    );

    lblAvgTurnaround->setText(
        QString("Avg Turnaround Time: %1")
            .arg(currentResult.averageTurnaroundTime, 0, 'f', 2)
    );

    lblAvgResponse->setText(
        QString("Avg Response Time: %1")
            .arg(currentResult.averageResponseTime, 0, 'f', 2)
    );

    hasResult = true;
    btnExport->setEnabled(true);
}

void MainWindow::handleExportOutput()
{
    if (!hasResult) {
        return;
    }

    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "Chon thu muc luu ket qua"
    );

    if (dirPath.isEmpty()) {
        return;
    }

    std::string error;

    bool isNonPreemptive =
        (comboAlgorithm->currentIndex() == 0);

    std::string resPath =
        dirPath.toStdString() +
        (isNonPreemptive
            ? "/np_result.csv"
            : "/p_result.csv");

    std::string ganttPath =
        dirPath.toStdString() +
        (isNonPreemptive
            ? "/np_gantt.csv"
            : "/p_gantt.csv");

    if (!CsvHandler::writeProcessResults(
            resPath, currentResult, error) ||
        !CsvHandler::writeGantt(
            ganttPath, currentResult, error)) {

        QMessageBox::critical(
            this,
            "Loi ghi file",
            QString::fromStdString(error)
        );

        return;
    }

    QMessageBox::information(
        this,
        "Thanh cong",
        "Da xuat file ket qua thanh cong!"
    );
}
