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

    // Chiều rộng tối thiểu để mỗi đoạn còn phân biệt được bằng mắt. Bằng 0 khi
    // dữ liệu đủ thưa để vẽ vừa khung. MainWindow đặt widget này trong
    // QScrollArea, nên giá trị khác 0 sẽ sinh thanh kéo ngang.
    int contentWidth() const { return m_contentWidth; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<GanttBlock> m_blocks;
    int m_contentWidth = 0;
};
