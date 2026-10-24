#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

const std::vector<long long> kDefaultSizes = {
    1,      10,     50,      100,     150,     200,     500,
    1000,   5000,   10000,   50000,   100000,  500000,  1000000,
};

bool generate(const std::filesystem::path& path, long long count, std::uint32_t seed) {
    std::mt19937 rng(seed + static_cast<std::uint32_t>(count));
    std::uniform_int_distribution<long long> arrival(0, count > 1 ? count : 1);
    std::uniform_int_distribution<int> burst(1, 20);
    std::uniform_int_distribution<int> priority(1, 10);

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Khong mo duoc file: " << path.string() << '\n';
        return false;
    }

    out << "pid,arrival,burst,priority\n";
    for (long long pid = 1; pid <= count; ++pid) {
        out << pid << ',' << arrival(rng) << ',' << burst(rng) << ',' << priority(rng)
            << '\n';
    }
    return out.good();
}

} // namespace

int main(int argc, char** argv) {
    std::string outDir = "samples";
    std::uint32_t seed = 42;
    std::vector<long long> sizes;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--out" && i + 1 < argc) {
            outDir = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--sizes") {
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                sizes.push_back(std::stoll(argv[++i]));
            }
        } else {
            std::cerr << "Tham so khong hop le: " << arg << '\n';
            return 1;
        }
    }

    if (sizes.empty()) {
        sizes = kDefaultSizes;
    }

    std::error_code error;
    std::filesystem::create_directories(outDir, error);
    if (error) {
        std::cerr << "Khong tao duoc thu muc: " << outDir << '\n';
        return 1;
    }

    for (const long long count : sizes) {
        if (count <= 0) {
            std::cerr << "Kich thuoc phai > 0\n";
            return 1;
        }

        const std::filesystem::path path =
            std::filesystem::path(outDir) / ("perf_" + std::to_string(count) + ".csv");
        if (!generate(path, count, seed)) {
            return 1;
        }

        const double sizeMb =
            static_cast<double>(std::filesystem::file_size(path)) / (1024.0 * 1024.0);
        std::cout << path.string() << ": " << count << " dong, " << sizeMb << " MB\n";
    }

    return 0;
}
