#pragma once

#include <QAbstractTableModel>
#include <vector>

#include "core/Process.h"
#include "core/ScheduleResult.h"

// Model chỉ đọc cho hai bảng lớn của MainWindow.
//
// QTableWidget dựng một đối tượng cho mỗi ô, nên với 1.000.000 tiến trình là
// 8.000.000 đối tượng và gần như treo máy. Model này không giữ dữ liệu riêng:
// nó đọc thẳng vector đầu vào và ScheduleResult, chỉ sinh chuỗi cho những ô
// QTableView thật sự hỏi tới.
class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum class Kind {
        Input,  // PID, AT, BT, Priority
        Result  // PID, AT, BT, Priority, CT, TAT, WT, RT
    };

    explicit ProcessTableModel(Kind kind, QObject *parent = nullptr);

    // Dùng cho bảng đầu vào.
    void setInput(const std::vector<Process> *input);

    // Dùng cho bảng kết quả. Vẫn cần input để hiện lại AT/BT/Priority vì
    // ScheduleResult không lưu ba trường này.
    //
    // Điều kiện: result->processes[i] là kết quả của input[i] (cả hai scheduler
    // đều ghi theo đúng thứ tự đầu vào; unit test đã kiểm điều này).
    void setResult(const std::vector<Process> *input, const ScheduleResult *result);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    Kind m_kind;
    const std::vector<Process> *m_input = nullptr;
    const ScheduleResult *m_result = nullptr;
};
