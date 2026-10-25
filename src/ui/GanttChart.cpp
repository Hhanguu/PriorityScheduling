#include "GanttChart.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QFont>

GanttChart::GanttChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
}

void GanttChart::setScheduleData(const QVector<GanttBlock> &blocks) {
    m_blocks = blocks;
    update(); 
}

void GanttChart::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_blocks.isEmpty()) {
        painter.setPen(Qt::gray);
        QFont font("Segoe UI", 10);
font.setItalic(true);
painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Chưa có dữ liệu biểu đồ Gantt.");
        return;
    }

    int totalDuration = 0;
    for (const auto &block : m_blocks) {
        totalDuration = qMax(totalDuration, block.endTime);
    }

    if (totalDuration <= 0) return;

    int chartWidth = width() - 40;
    int chartHeight = 50;
    int startX = 20;
    int startY = 40;

  
    const QVector<QColor> colors = {
        QColor(52, 152, 219),  
        QColor(46, 204, 113),  
        QColor(155, 89, 182), 
        QColor(241, 196, 15),  
        QColor(230, 126, 34),  
        QColor(231, 76, 60)    
    };
    for (int i = 0; i < m_blocks.size(); ++i) {
        const auto &block = m_blocks[i];
        
        double startRatio = static_cast<double>(block.startTime) / totalDuration;
        double durationRatio = static_cast<double>(block.endTime - block.startTime) / totalDuration;

        int x = startX + static_cast<int>(startRatio * chartWidth);
        int w = static_cast<int>(durationRatio * chartWidth);

        QColor color = colors[i % colors.size()];
        painter.setBrush(QBrush(color));
        painter.setPen(QPen(Qt::black, 1));

        painter.drawRect(x, startY, w, chartHeight);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
        painter.drawText(QRect(x, startY, w, chartHeight), Qt::AlignCenter, QString::number(block.processId));

        painter.setPen(Qt::black);
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(x - 10, startY + chartHeight + 20, QString::number(block.startTime));

        if (i == m_blocks.size() - 1) {
            int endX = x + w;
            painter.drawText(endX - 10, startY + chartHeight + 20, QString::number(block.endTime));
        }
    }
}