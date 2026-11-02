#include "GanttChart.h"

#include <QColor>
#include <QFont>
#include <QLocale>
#include <QPainter>
#include <QPalette>
#include <QPaintEvent>
#include <QPen>

#include <algorithm>

namespace {
// Mỗi đoạn cần tối thiểu chừng này pixel thì màu và ranh giới mới còn nhìn ra.
// Nếu giữ nguyên chiều rộng khung, 10.000 đoạn sẽ dồn thành 1 pixel mỗi đoạn.
// 12 px là mức đọc được khi kéo qua (một khung 1.138 px thấy ~95 đoạn).
constexpr int kMinSegmentWidth = 12;
// Trần chiều rộng nội dung: rộng hơn nữa thì thanh kéo chỉ còn là màn trình diễn,
// và Qt cũng giới hạn kích thước widget ở 16.777.215 px.
constexpr int kMaxContentWidth = 1000000;

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

    // Rộng theo số đoạn, không theo thời gian: burst trong đề chỉ 1-20 nên thang
    // thời gian thật sẽ cho ra những đoạn mỏng hơn 1 px.
    const qint64 needed = static_cast<qint64>(m_blocks.size()) * kMinSegmentWidth;
    m_contentWidth = static_cast<int>(std::min<qint64>(needed, kMaxContentWidth));
    setMinimumWidth(m_contentWidth);

    updateGeometry();
    update();
}

void GanttChart::paintEvent(QPaintEvent *event) {
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

    // Cha đặt widget này trong QScrollArea, nên khi cuộn chỉ một phần được vẽ.
    // Cắt theo event->rect() để không phải vẽ lại toàn bộ bề ngang.
    const QRect clipRect = event->rect();
    const auto timeAtX = [&](int x) {
        return static_cast<long long>(
            static_cast<long double>(x) * totalDuration / chartWidth);
    };

    const bool dense = m_blocks.size() > chartWidth;
    const int pixelsPerBlock =
        m_blocks.isEmpty() ? chartWidth : chartWidth / static_cast<int>(m_blocks.size());
    // Nhãn theo từng đoạn chỉ dùng được khi đoạn đủ rộng; nếu không phải đánh mốc
    // thời gian cách nhau một khoảng cố định.
    const bool useBoundaryLabels = !dense && pixelsPerBlock >= 40;

    if (dense) {
        QFont infoFont = painter.font();
        infoFont.setPointSize(8);
        painter.setFont(infoFont);
        painter.setPen(QColor(90, 90, 90));
        // Neo vào vùng đang nhìn thấy, không neo vào mép nội dung: khi đã cuộn
        // sang phải thì nhãn này vẫn phải đọc được.
        painter.drawText(QRect(clipRect.left() + marginX, 4,
                               std::max(0, clipRect.width() - 2 * marginX), 18),
                         Qt::AlignRight,
                         QStringLiteral("%1 đoạn — hiển thị thu gọn")
                             .arg(QLocale().toString(static_cast<qlonglong>(m_blocks.size()))));

        // Chỉ vẽ dải đang nhìn thấy; bắt đầu từ đoạn chứa mốc thời gian bên trái.
        const int xStart = std::max(0, clipRect.left() - marginX);
        const int xEnd = std::min(chartWidth, clipRect.right() - marginX + 1);
        int blockIndex = static_cast<int>(
            std::upper_bound(m_blocks.begin(), m_blocks.end(), timeAtX(xStart),
                             [](long long time, const GanttBlock &block) {
                                 return time < block.endTime;
                             }) -
            m_blocks.begin());
        painter.setPen(Qt::NoPen);
        for (int x = xStart; x < xEnd; ++x) {
            const long long time = timeAtX(x);
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
    } else {
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
            if (x1 > clipRect.right() || x1 + blockWidth < clipRect.left()) {
                continue;
            }

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

            if (useBoundaryLabels) {
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
    }

    if (useBoundaryLabels) {
        return;
    }

    // Trục thời gian khi từng đoạn quá hẹp để đặt nhãn riêng: đánh dấu cách nhau
    // khoảng 110 px, nên cuộn tới đâu cũng còn mốc thời gian để đối chiếu.
    QFont axisFont = painter.font();
    axisFont.setBold(false);
    axisFont.setPointSize(8);
    painter.setFont(axisFont);
    painter.setPen(QColor(60, 60, 60));

    constexpr int kTickStep = 110;
    const int labelWidth = 80;
    for (int x = 0; x <= chartWidth; x += kTickStep) {
        const int labelLeft = marginX + x - labelWidth / 2;
        if (clipRect.intersects(QRect(labelLeft, barTop + barHeight, labelWidth, 24))) {
            painter.drawLine(marginX + x, barTop + barHeight, marginX + x,
                             barTop + barHeight + 3);
            painter.drawText(QRect(labelLeft, barTop + barHeight + 4, labelWidth, 18),
                             Qt::AlignHCenter, QString::number(timeAtX(x)));
        }
    }

    // Mốc cuối luôn hiện, trừ khi nhãn kỳ trước đã đủ gần.
    const int lastTick = (chartWidth / kTickStep) * kTickStep;
    if (chartWidth - lastTick > labelWidth / 2) {
        const int labelLeft = marginX + chartWidth - labelWidth / 2;
        if (clipRect.intersects(QRect(labelLeft, barTop + barHeight, labelWidth, 24))) {
            painter.drawLine(marginX + chartWidth, barTop + barHeight, marginX + chartWidth,
                             barTop + barHeight + 3);
            painter.drawText(QRect(labelLeft, barTop + barHeight + 4, labelWidth, 18),
                             Qt::AlignHCenter, QString::number(totalDuration));
        }
    }
}
