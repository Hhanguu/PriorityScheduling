#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void handleOpenFile();
    void handleRunSchedule();
    void handleExportOutput();

private:
    void setupUI();
    void updateTables();

    QPushButton *btnOpen;
    QPushButton *btnRun;
    QPushButton *btnExport;
    QComboBox *comboAlgorithm;
    
    QTableWidget *tableInput;
    QTableWidget *tableResult;
    
    QLabel *lblAvgWaiting;
    QLabel *lblAvgTurnaround;
    QLabel *lblAvgResponse;

    std::vector<Process> inputProcesses;
    ScheduleResult currentResult;
    bool hasResult = false;
};