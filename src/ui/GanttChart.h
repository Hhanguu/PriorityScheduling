#pragma once
#include <QWidget>
#include <QVector>
#include <QString>

struct GanttBlock {
    int processId;
    int startTime;
    int endTime;
    QString colorHex;
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