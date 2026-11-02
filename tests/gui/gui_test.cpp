#include "doctest.h"

#include "ui/GanttChart.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMap>
#include <QScrollArea>
#include <QScrollBar>
#include <QTableView>
#include <QTableWidget>
#include <QTextStream>
#include <QTemporaryDir>

namespace {
QString dataPath(const QString &relative) {
    return QDir(QString::fromUtf8(PS_TEST_DATA_DIR)).filePath(relative);
}

QString readText(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

// Bảng so sánh vẫn là QTableWidget (3 dòng cố định).
QTableWidget *table(MainWindow &window, const char *name) {
    return window.findChild<QTableWidget *>(QString::fromLatin1(name));
}

// Hai bảng lớn dùng QTableView + model ảo, nên đọc ô qua model chứ không qua item.
QTableView *view(MainWindow &window, const char *name) {
    return window.findChild<QTableView *>(QString::fromLatin1(name));
}

QString cell(QTableView *tableView, int row, int column) {
    if (tableView == nullptr || tableView->model() == nullptr) {
        return {};
    }
    return tableView->model()->index(row, column).data().toString();
}

QLabel *label(MainWindow &window, const char *name) {
    return window.findChild<QLabel *>(QString::fromLatin1(name));
}

QComboBox *algorithmBox(MainWindow &window) {
    return window.findChild<QComboBox *>("comboAlgorithm");
}

QComboBox *modeBox(MainWindow &window) {
    return window.findChild<QComboBox *>("comboMode");
}
} // namespace

TEST_CASE("GUI nạp CSV và giữ đúng thứ tự") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    CHECK(error.isEmpty());

    QTableView *input = view(window, "tableInput");
    REQUIRE(input != nullptr);
    REQUIRE(input->model() != nullptr);
    CHECK(input->model()->rowCount() == 10);
    CHECK(cell(input, 0, 0) == "P1");
    CHECK(cell(input, 0, 1) == "0");
    CHECK(cell(input, 0, 2) == "5");
    CHECK(cell(input, 0, 3) == "3");
    CHECK(cell(input, 9, 0) == "P10");
}

TEST_CASE("GUI báo lỗi CSV và giữ dữ liệu cũ") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    QTableView *input = view(window, "tableInput");
    REQUIRE(input != nullptr);
    REQUIRE(input->model() != nullptr);
    REQUIRE(input->model()->rowCount() == 10);

    CHECK_FALSE(window.loadCsv(dataPath("gui/err_bad_header.csv"), &error));
    CHECK(error == QStringLiteral(
        "Dòng 1: header phải là pid,arrival,burst,priority"));
    CHECK(input->model()->rowCount() == 10);
}

TEST_CASE("GUI chạy hai thuật toán, hiện averages và so sánh") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    REQUIRE(window.runSchedules(&error));
    CHECK(error.isEmpty());
    CHECK(window.hasResults());
    window.show();
    CHECK(!window.grab().isNull());

    CHECK(label(window, "lblAvgWaiting")->text() == "Avg Waiting Time: 8.00");
    CHECK(label(window, "lblAvgTurnaround")->text() == "Avg Turnaround Time: 11.50");
    CHECK(label(window, "lblAvgResponse")->text() == "Avg Response Time: 8.00");

    QTableWidget *comparison = table(window, "tableComparison");
    REQUIRE(comparison != nullptr);
    CHECK(comparison->item(0, 1)->text() == "8.00");
    CHECK(comparison->item(0, 2)->text() == "7.80");
    CHECK(comparison->item(1, 1)->text() == "11.50");
    CHECK(comparison->item(1, 2)->text() == "11.30");
    CHECK(comparison->item(2, 1)->text() == "8.00");
    CHECK(comparison->item(2, 2)->text() == "5.30");

    // 10 đoạn thì không được có thanh kéo ngang: bảng phải vừa khung.
    auto *gantt = window.findChild<GanttChart *>("ganttChart");
    REQUIRE(gantt != nullptr);
    CHECK(gantt->contentWidth() == 120);
    auto *scroll = window.findChild<QScrollArea *>("ganttScroll");
    REQUIRE(scroll != nullptr);
    CHECK(scroll->horizontalScrollBar()->maximum() == 0);

    QComboBox *combo = algorithmBox(window);
    REQUIRE(combo != nullptr);
    combo->setCurrentIndex(1);
    CHECK(label(window, "lblAvgWaiting")->text() == "Avg Waiting Time: 7.80");
    CHECK(label(window, "lblAvgTurnaround")->text() == "Avg Turnaround Time: 11.30");
    CHECK(label(window, "lblAvgResponse")->text() == "Avg Response Time: 5.30");
}

TEST_CASE("GUI bảng kết quả khớp file expected") {
    MainWindow window;
    QComboBox *mode = modeBox(window);
    REQUIRE(mode != nullptr);
    mode->setCurrentIndex(0);
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    REQUIRE(window.runSchedules(&error));

    // pid -> "arrival,burst,priority" từ file đầu vào, để kiểm 3 cột lấy từ input.
    QMap<int, QStringList> inputRow;
    const QStringList inputLines = readText(dataPath("gui/sample.csv")).split('\n', Qt::SkipEmptyParts);
    for (int i = 1; i < inputLines.size(); ++i) {
        const QStringList fields = inputLines.at(i).split(',');
        if (fields.size() == 4) {
            inputRow.insert(fields.at(0).toInt(),
                            {fields.at(1), fields.at(2), fields.at(3)});
        }
    }

    QTableView *result = view(window, "tableResult");
    REQUIRE(result != nullptr);
    REQUIRE(result->model() != nullptr);

    const QStringList expected = readText(dataPath("ket_qua/gui/sample_np_result.csv"))
                                     .split('\n', Qt::SkipEmptyParts);
    REQUIRE(expected.size() > 1);
    const int rows = expected.size() - 1;
    REQUIRE(result->model()->rowCount() == rows);
    for (int i = 0; i < rows; ++i) {
        const QStringList fields = expected.at(i + 1).split(',');
        REQUIRE(fields.size() == 5);
        const int pid = fields.at(0).toInt();
        CHECK(cell(result, i, 0) == QStringLiteral("P%1").arg(pid));
        REQUIRE(inputRow.contains(pid));
        CHECK(cell(result, i, 1) == inputRow.value(pid).at(0));
        CHECK(cell(result, i, 2) == inputRow.value(pid).at(1));
        CHECK(cell(result, i, 3) == inputRow.value(pid).at(2));
        CHECK(cell(result, i, 4) == fields.at(1));
        CHECK(cell(result, i, 5) == fields.at(2));
        CHECK(cell(result, i, 6) == fields.at(3));
        CHECK(cell(result, i, 7) == fields.at(4));
    }
}

TEST_CASE("GUI xuất đủ 5 file và khớp expected") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    REQUIRE(window.runSchedules(&error));

    QTemporaryDir temp;
    REQUIRE(temp.isValid());
    REQUIRE(window.exportResults(temp.path(), &error));
    CHECK(error.isEmpty());

    const QStringList files = {
        "np_result.csv", "np_gantt.csv", "p_result.csv", "p_gantt.csv", "comparison.csv"};
    for (const QString &name : files) {
        CHECK(QFile::exists(QDir(temp.path()).filePath(name)));
        CHECK(readText(QDir(temp.path()).filePath(name)) ==
              readText(dataPath("ket_qua/gui/sample_" + name)));
    }
}

TEST_CASE("GUI chạy độc lập Non-Preemptive") {
    MainWindow window;
    QComboBox *mode = modeBox(window);
    REQUIRE(mode != nullptr);
    mode->setCurrentIndex(0);
    CHECK_FALSE(algorithmBox(window)->isEnabled());

    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    REQUIRE(window.runSchedules(&error));
    CHECK(label(window, "lblAvgWaiting")->text() == "Avg Waiting Time: 8.00");

    QTableWidget *comparison = table(window, "tableComparison");
    REQUIRE(comparison != nullptr);
    CHECK(comparison->item(0, 1)->text() == "8.00");
    CHECK(comparison->item(0, 2)->text() == "—");

    QTemporaryDir temp;
    REQUIRE(temp.isValid());
    REQUIRE(window.exportResults(temp.path(), &error));
    CHECK(QFile::exists(QDir(temp.path()).filePath("np_result.csv")));
    CHECK(QFile::exists(QDir(temp.path()).filePath("np_gantt.csv")));
    CHECK_FALSE(QFile::exists(QDir(temp.path()).filePath("p_result.csv")));
    CHECK_FALSE(QFile::exists(QDir(temp.path()).filePath("comparison.csv")));
}

TEST_CASE("GUI chạy độc lập Preemptive") {
    MainWindow window;
    QComboBox *mode = modeBox(window);
    REQUIRE(mode != nullptr);
    mode->setCurrentIndex(1);
    CHECK_FALSE(algorithmBox(window)->isEnabled());

    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    REQUIRE(window.runSchedules(&error));
    CHECK(label(window, "lblAvgWaiting")->text() == "Avg Waiting Time: 7.80");

    QTableWidget *comparison = table(window, "tableComparison");
    REQUIRE(comparison != nullptr);
    CHECK(comparison->item(0, 1)->text() == "—");
    CHECK(comparison->item(0, 2)->text() == "7.80");

    QTemporaryDir temp;
    REQUIRE(temp.isValid());
    REQUIRE(window.exportResults(temp.path(), &error));
    CHECK(QFile::exists(QDir(temp.path()).filePath("p_result.csv")));
    CHECK(QFile::exists(QDir(temp.path()).filePath("p_gantt.csv")));
    CHECK_FALSE(QFile::exists(QDir(temp.path()).filePath("np_result.csv")));
    CHECK_FALSE(QFile::exists(QDir(temp.path()).filePath("comparison.csv")));
}

TEST_CASE("GUI chưa có dữ liệu thì không chạy scheduler") {
    MainWindow window;
    QString error;
    CHECK_FALSE(window.runSchedules(&error));
    CHECK_FALSE(error.isEmpty());
    CHECK_FALSE(window.hasResults());
}

TEST_CASE("GUI hiển thị đủ 10005 dòng và vẽ Gantt thu gọn") {
    QTemporaryDir temp;
    REQUIRE(temp.isValid());
    const QString csvPath = QDir(temp.path()).filePath("large.csv");
    QFile file(csvPath);
    REQUIRE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << "pid,arrival,burst,priority\n";
    for (int pid = 1; pid <= 10005; ++pid) {
        out << pid << ',' << (pid % 100) << ',' << ((pid % 20) + 1) << ','
            << ((pid % 10) + 1) << '\n';
    }
    file.close();

    MainWindow window;
    window.resize(1180, 900);
    QString error;
    REQUIRE(window.loadCsv(csvPath, &error));
    QTableView *input = view(window, "tableInput");
    REQUIRE(input != nullptr);
    REQUIRE(input->model() != nullptr);
    // Không còn cắt ở 10.000 dòng: model phải giữ đủ toàn bộ file.
    CHECK(input->model()->rowCount() == 10005);
    CHECK(cell(input, 10004, 0) == "P10005");

    QLabel *count = label(window, "lblLimit");
    REQUIRE(count != nullptr);
    CHECK_FALSE(count->isHidden());
    CHECK(count->text() == "Đầu vào: 10.005 dòng");

    QComboBox *mode = modeBox(window);
    REQUIRE(mode != nullptr);
    mode->setCurrentIndex(0);
    REQUIRE(window.runSchedules(&error));
    CHECK(view(window, "tableResult")->model()->rowCount() == 10005);
    window.show();
    QApplication::processEvents();

    auto *gantt = window.findChild<GanttChart *>("ganttChart");
    REQUIRE(gantt != nullptr);

    // 10.005 đoạn: mỗi đoạn giữ tối thiểu 12 px nên widget rộng hơn khung và
    // QScrollArea phải có thanh kéo ngang.
    CHECK(gantt->contentWidth() == 10005 * 12);
    auto *scroll = window.findChild<QScrollArea *>("ganttScroll");
    REQUIRE(scroll != nullptr);
    CHECK(scroll->horizontalScrollBar()->maximum() > 0);

    const QImage image = gantt->grab().toImage();
    int colored = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).saturation() > 80) {
                ++colored;
            }
        }
    }
    CHECK(colored > 1000);
}

TEST_CASE("GUI nạp được 500 tiến trình") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/gui_large_500.csv"), &error));
    QTableView *input = view(window, "tableInput");
    REQUIRE(input != nullptr);
    REQUIRE(input->model() != nullptr);
    CHECK(input->model()->rowCount() == 500);
}
