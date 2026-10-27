#include "doctest.h"

#include "ui/GanttChart.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QLabel>
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

QTableWidget *table(MainWindow &window, const char *name) {
    return window.findChild<QTableWidget *>(QString::fromLatin1(name));
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

    QTableWidget *input = table(window, "tableInput");
    REQUIRE(input != nullptr);
    CHECK(input->rowCount() == 10);
    CHECK(input->item(0, 0)->text() == "P1");
    CHECK(input->item(0, 1)->text() == "0");
    CHECK(input->item(0, 2)->text() == "5");
    CHECK(input->item(0, 3)->text() == "3");
    CHECK(input->item(9, 0)->text() == "P10");
}

TEST_CASE("GUI báo lỗi CSV và giữ dữ liệu cũ") {
    MainWindow window;
    QString error;
    REQUIRE(window.loadCsv(dataPath("gui/sample.csv"), &error));
    QTableWidget *input = table(window, "tableInput");
    REQUIRE(input != nullptr);
    REQUIRE(input->rowCount() == 10);

    CHECK_FALSE(window.loadCsv(dataPath("gui/err_bad_header.csv"), &error));
    CHECK(error == QStringLiteral(
        "Dòng 1: header phải là pid,arrival,burst,priority"));
    CHECK(input->rowCount() == 10);
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

    QComboBox *combo = algorithmBox(window);
    REQUIRE(combo != nullptr);
    combo->setCurrentIndex(1);
    CHECK(label(window, "lblAvgWaiting")->text() == "Avg Waiting Time: 7.80");
    CHECK(label(window, "lblAvgTurnaround")->text() == "Avg Turnaround Time: 11.30");
    CHECK(label(window, "lblAvgResponse")->text() == "Avg Response Time: 5.30");
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

TEST_CASE("GUI giới hạn bảng và vẽ Gantt thu gọn với 10k dòng") {
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
    QTableWidget *input = table(window, "tableInput");
    REQUIRE(input != nullptr);
    CHECK(input->rowCount() == 10000);

    QLabel *limit = label(window, "lblLimit");
    REQUIRE(limit != nullptr);
    CHECK_FALSE(limit->isHidden());
    CHECK(limit->text().contains("10000/10005"));

    QComboBox *mode = modeBox(window);
    REQUIRE(mode != nullptr);
    mode->setCurrentIndex(0);
    REQUIRE(window.runSchedules(&error));
    CHECK(table(window, "tableResult")->rowCount() == 10000);
    window.show();
    QApplication::processEvents();

    auto *gantt = window.findChild<GanttChart *>("ganttChart");
    REQUIRE(gantt != nullptr);
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
    QTableWidget *input = table(window, "tableInput");
    REQUIRE(input != nullptr);
    CHECK(input->rowCount() == 500);
}
