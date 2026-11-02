#include "ProcessTableModel.h"

#include <QString>

#include <cstddef>

namespace {
const char *const kInputHeaders[] = {"PID", "Arrival Time", "Burst Time", "Priority"};
const char *const kResultHeaders[] = {"PID",         "Arrival Time", "Burst Time", "Priority",
                                      "CT",          "TAT",          "WT",         "RT"};
} // namespace

ProcessTableModel::ProcessTableModel(Kind kind, QObject *parent)
    : QAbstractTableModel(parent), m_kind(kind) {}

void ProcessTableModel::setInput(const std::vector<Process> *input) {
    beginResetModel();
    m_input = input;
    endResetModel();
}

void ProcessTableModel::setResult(const std::vector<Process> *input,
                                  const ScheduleResult *result) {
    beginResetModel();
    m_input = input;
    m_result = result;
    endResetModel();
}

int ProcessTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    if (m_kind == Kind::Input) {
        return m_input == nullptr ? 0 : static_cast<int>(m_input->size());
    }
    return m_result == nullptr ? 0 : static_cast<int>(m_result->processes.size());
}

int ProcessTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_kind == Kind::Input ? 4 : 8;
}

QVariant ProcessTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }

    if (m_kind == Kind::Input) {
        if (m_input == nullptr || index.row() >= static_cast<int>(m_input->size())) {
            return {};
        }
        const Process &process = (*m_input)[static_cast<std::size_t>(index.row())];
        switch (index.column()) {
        case 0:
            return QStringLiteral("P%1").arg(process.pid);
        case 1:
            return QString::number(process.arrivalTime);
        case 2:
            return QString::number(process.burstTime);
        case 3:
            return QString::number(process.priority);
        default:
            return {};
        }
    }

    if (m_result == nullptr ||
        index.row() >= static_cast<int>(m_result->processes.size())) {
        return {};
    }
    const ProcessResult &row = m_result->processes[static_cast<std::size_t>(index.row())];

    // Ba cột đầu lấy từ đầu vào; ScheduleResult không lưu AT/BT/Priority.
    // Kết quả được ghi theo đúng thứ tự đầu vào, nên dùng chung chỉ số dòng.
    if (index.column() <= 3) {
        const std::size_t resultRow = static_cast<std::size_t>(index.row());
        if (m_input == nullptr || resultRow >= m_input->size()) {
            return index.column() == 0 ? QStringLiteral("P%1").arg(row.pid)
                                       : QStringLiteral("—");
        }
        const Process &process = (*m_input)[resultRow];
        if (process.pid != row.pid) {
            // Không nên xảy ra; nếu có thì báo "—" còn hơn hiện số của tiến
            // trình khác.
            return index.column() == 0 ? QStringLiteral("P%1").arg(row.pid)
                                       : QStringLiteral("—");
        }
        switch (index.column()) {
        case 0:
            return QStringLiteral("P%1").arg(process.pid);
        case 1:
            return QString::number(process.arrivalTime);
        case 2:
            return QString::number(process.burstTime);
        default:
            return QString::number(process.priority);
        }
    }

    switch (index.column()) {
    case 4:
        return QString::number(row.completionTime);
    case 5:
        return QString::number(row.turnaroundTime);
    case 6:
        return QString::number(row.waitingTime);
    case 7:
        return QString::number(row.responseTime);
    default:
        return {};
    }
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    const char *const *headers = m_kind == Kind::Input ? kInputHeaders : kResultHeaders;
    const int count = m_kind == Kind::Input ? 4 : 8;
    if (section < 0 || section >= count) {
        return {};
    }
    return QString::fromLatin1(headers[section]);
}
