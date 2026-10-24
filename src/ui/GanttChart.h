#pragma once
#include <QWidget>
#include <vector>
#include "core/GanttSegment.h"

class GanttChart : public QWidget {
    Q_OBJECT
public:
    explicit GanttChart(QWidget *parent = nullptr);
    void setData(const std::vector<GanttSegment>& segments); 
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<GanttSegment> segments;
};