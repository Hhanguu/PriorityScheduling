#pragma once

#include <QMainWindow>
#include <vector>

#include "core/Process.h"
#include "core/ScheduleResult.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class GanttChart;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    bool loadCsv(const QString &path, QString *errorMessage = nullptr);
    bool runSchedules(QString *errorMessage = nullptr);
    bool exportResults(const QString &dirPath, QString *errorMessage = nullptr);
    bool hasResults() const;

private slots:
    void handleOpenFile();
    void handleRunSchedule();
    void handleExportOutput();
    void handleAlgorithmChanged(int index);
    void handleModeChanged(int index);

private:
    void setupUI();
    void resetResults();
    void updateTables();
    void updateInputTable();
    void updateResultTable();
    void updateComparisonTable();
    void updateAverages();
    void updateGantt();
    void updateTableLimitNotice();
    const Process *findInputProcess(int pid) const;
    const ScheduleResult &activeResult() const;

    QPushButton *btnOpen = nullptr;
    QPushButton *btnRun = nullptr;
    QPushButton *btnExport = nullptr;
    QComboBox *comboMode = nullptr;
    QComboBox *comboAlgorithm = nullptr;

    QTableWidget *tableInput = nullptr;
    QTableWidget *tableResult = nullptr;
    QTableWidget *tableComparison = nullptr;
    GanttChart *ganttChart = nullptr;

    QLabel *lblLimit = nullptr;
    QLabel *lblAvgWaiting = nullptr;
    QLabel *lblAvgTurnaround = nullptr;
    QLabel *lblAvgResponse = nullptr;
    QLabel *lblElapsed = nullptr;

    std::vector<Process> inputProcesses;
    ScheduleResult npResult;
    ScheduleResult pResult;
    bool npReady = false;
    bool pReady = false;
    bool resultsReady = false;
    qint64 npElapsedMs = 0;
    qint64 pElapsedMs = 0;
};
