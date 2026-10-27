#pragma once

#include <QVector>
#include <QWidget>

struct GanttBlock {
    int processId;
    long long startTime;
    long long endTime;
};

class GanttChart : public QWidget {
    Q_OBJECT

public:
    explicit GanttChart(QWidget *parent = nullptr);
    void setScheduleData(const QVector<GanttBlock> &blocks);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<GanttBlock> m_blocks;
};
