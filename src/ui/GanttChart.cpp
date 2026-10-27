#include "GanttChart.h"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPalette>
#include <QPaintEvent>
#include <QPen>

#include <algorithm>

namespace {
const QVector<QColor> &palette() {
    static const QVector<QColor> colors = {
        QColor(52, 152, 219),
        QColor(46, 204, 113),
        QColor(155, 89, 182),
        QColor(241, 196, 15),
        QColor(230, 126, 34),
        QColor(231, 76, 60),
        QColor(26, 188, 156),
        QColor(142, 68, 173),
    };
    return colors;
}

QColor colorForPid(int pid) {
    if (pid < 0) {
        return QColor(189, 195, 199);
    }
    const int index = (pid - 1) % palette().size();
    return palette().at(index);
}

QString labelForPid(int pid) {
    return pid < 0 ? QStringLiteral("IDLE") : QStringLiteral("P%1").arg(pid);
}
} // namespace

GanttChart::GanttChart(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(150);
}

void GanttChart::setScheduleData(const QVector<GanttBlock> &blocks) {
    m_blocks = blocks;
    update();
}

void GanttChart::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().color(QPalette::Base));

    if (m_blocks.isEmpty()) {
        painter.setPen(Qt::gray);
        QFont emptyFont = painter.font();
        emptyFont.setItalic(true);
        painter.setFont(emptyFont);
        painter.drawText(rect(), Qt::AlignCenter, "Chưa có dữ liệu biểu đồ Gantt.");
        return;
    }

    long long totalDuration = 0;
    for (const GanttBlock &block : m_blocks) {
        totalDuration = std::max(totalDuration, block.endTime);
    }
    if (totalDuration <= 0) {
        return;
    }

    const int marginX = 24;
    const int barTop = 34;
    const int chartWidth = width() - 2 * marginX;
    const int barHeight = std::max(36, height() - barTop - 44);
    if (chartWidth <= 0) {
        return;
    }

    const bool dense = m_blocks.size() > chartWidth;
    if (dense) {
        QFont infoFont = painter.font();
        infoFont.setPointSize(8);
        painter.setFont(infoFont);
        painter.setPen(QColor(90, 90, 90));
        painter.drawText(QRect(marginX, 4, chartWidth, 18), Qt::AlignRight,
                         QStringLiteral("%1 đoạn — hiển thị thu gọn").arg(m_blocks.size()));
    }

    if (dense) {
        int blockIndex = 0;
        painter.setPen(Qt::NoPen);
        for (int x = 0; x < chartWidth; ++x) {
            const long long time = static_cast<long long>(
                (static_cast<long double>(x) + 0.5L) * totalDuration / chartWidth);
            while (blockIndex < m_blocks.size() &&
                   m_blocks.at(blockIndex).endTime <= time) {
                ++blockIndex;
            }
            if (blockIndex >= m_blocks.size()) {
                break;
            }
            const GanttBlock &block = m_blocks.at(blockIndex);
            if (block.startTime <= time && time < block.endTime) {
                painter.fillRect(marginX + x, barTop, 1, barHeight,
                                 colorForPid(block.processId));
            }
        }

        QFont timeFont = painter.font();
        timeFont.setBold(false);
        timeFont.setPointSize(8);
        painter.setFont(timeFont);
        painter.setPen(QColor(60, 60, 60));
        painter.drawText(QRect(marginX - 24, barTop + barHeight + 4, 48, 18),
                         Qt::AlignHCenter, QString::number(0));
        painter.drawText(QRect(marginX + chartWidth - 24, barTop + barHeight + 4, 48, 18),
                         Qt::AlignHCenter, QString::number(totalDuration));
        return;
    }

    for (int i = 0; i < m_blocks.size(); ++i) {
        const GanttBlock &block = m_blocks.at(i);
        if (block.endTime <= block.startTime) {
            continue;
        }

        const double startRatio =
            static_cast<double>(block.startTime) / static_cast<double>(totalDuration);
        const double endRatio =
            static_cast<double>(block.endTime) / static_cast<double>(totalDuration);
        const int x1 = marginX + qRound(startRatio * chartWidth);
        const int x2 = marginX + qRound(endRatio * chartWidth);
        const int blockWidth = std::max(1, x2 - x1);

        painter.setPen(Qt::NoPen);
        painter.setBrush(colorForPid(block.processId));
        painter.drawRect(x1, barTop, blockWidth, barHeight);

        if (blockWidth >= 26) {
            QFont labelFont = painter.font();
            labelFont.setBold(true);
            labelFont.setPointSize(blockWidth >= 60 ? 10 : 8);
            painter.setFont(labelFont);
            painter.setPen(block.processId < 0 ? QColor(60, 60, 60) : Qt::white);
            painter.drawText(QRect(x1, barTop, blockWidth, barHeight),
                             Qt::AlignCenter, labelForPid(block.processId));
        }

        painter.setPen(QColor(60, 60, 60));
        QFont timeFont = painter.font();
        timeFont.setBold(false);
        timeFont.setPointSize(8);
        painter.setFont(timeFont);

        if (blockWidth >= 40 || i == 0) {
            painter.drawText(QRect(x1 - 24, barTop + barHeight + 4, 48, 18),
                             Qt::AlignHCenter, QString::number(block.startTime));
        }
        if (i == m_blocks.size() - 1) {
            painter.drawText(QRect(x2 - 24, barTop + barHeight + 4, 48, 18),
                             Qt::AlignHCenter, QString::number(block.endTime));
        }
    }
}
