# Checkpoint 02 Report — Team Progress & Comparison with Checkpoint 01

## CPU Priority Scheduling Simulation

### Group 10 — Course Code: 012012500105
### Instructor: Mai Ngọc Châu
### Date: November 11, 2026

---

## 1) Executive summary

This document summarizes our progress from **Checkpoint 01 → Checkpoint 02** for the Priority Scheduling OS project.

Compared to Checkpoint 01, Checkpoint 02 delivers:

- **Full correctness coverage**: unit + integration + GUI tests are implemented and executed via `ctest` (3/3 suites).
- **Major performance improvements**: Preemptive priority scheduling becomes **event-based** and runs efficiently up to **1,000,000 processes**.
- **Scalability in GUI**: tables and Gantt visualization are adapted for very large datasets (including horizontal scroll for dense segments).

---

## 2) Comparison with Checkpoint 01

### 2.1 Checkpoint 01 (baseline)

Checkpoint 01 reported the following baseline status (extracted from `CheckPoint/Checkpoint01_10/Checkpoint01_10_Report.pdf`):

**Functional scope (demo-ready):**

- Load valid CSV; run **Non-preemptive** and **Preemptive** variants (NP-only, P-only, or both).
- Display CT/TAT/WT/RT; show averages and NP/P comparison.
- Render **Gantt chart** (including IDLE segments) and export CSV results.

**Testing:**

- `ctest`: **3/3 suites pass**.
- **63 test cases, 3408 assertions** total.
  - `unit_tests`: 53 cases / 3311 assertions (Pass)
  - `integration_tests`: 1 case / 1 assertion (Pass, **stub**)
  - `gui_tests`: 9 cases / 96 assertions (Pass)

**Benchmark (reference, on developer’s i5-11400F, Release O2):**

- Non-preemptive:
  - 100,000: **29 ms**
  - 1,000,000: **0.86 s**
  - 2,000,000: **2.00 s**
- Preemptive:
  - 100,000: **311.35 s**
  - 1,000,000 / 2,000,000: **not measured** (P estimated to take hours)
  - Reported complexity concern: **O(n × total burst)** for Preemptive.

**GUI limitations:**

- GUI results table limited to **10,000 rows**.
- For datasets around **1,000,000 processes**, the GUI could not show everything; **Gantt chart was compressed**.

**Main remaining issues / planned work (Checkpoint 01 → 02):**

- Convert Preemptive implementation to **event-based + heap**.
- Add **benchmark runner** (CSV + reproducible logs).
- Improve GUI scalability (virtual table + Gantt adapted to viewport).
- Strengthen integration test to verify CSV → scheduler → output end-to-end.

---

### 2.2 Checkpoint 02 (current state)

Checkpoint 02 is fully implemented in the repository and validated by the automated test suite.

**Testing (now fully integrated):**

- `ctest`: **3/3 suites pass**
- Total: **99 test cases / 112,740 assertions**
  - `unit_tests`: 79 / 112,207
  - `integration_tests`: 10 / 318
  - `gui_tests`: 10 / 215

**Performance benchmarks (release configuration):**

- **Non-preemptive (NP):**
  - 100,000: **13.7 ms**
  - 1,000,000: **242 ms**
  - 2,000,000: **577 ms**
- **Preemptive (P):**
  - 100,000: **27 ms**
  - 500,000: **242 ms**
  - 1,000,000: **572 ms**

**Benchmark methodology:**

- NP: measured as mean/median over 5 runs.
- P: measured for representative input sizes (single-run in our runner) after moving to event-based simulation.

**GUI scalability & Gantt improvements (1,000,000-row file):**

- `loadCsv`: **272 ms**
- Run NP only: **251 ms**
- Run both NP+P: **835 ms**
- Peak RSS: **247 MiB**
- Gantt rendering draws only visible segments and supports **horizontal scroll** when the dataset contains many segments.

---

### 2.3 Summary table — What improved?

| Area | Checkpoint 01 | Checkpoint 02 |
|---|---|---|
| Testing coverage | 63 cases / 3408 assertions, integration end-to-end mostly missing (stub) | 99 cases / 112,740 assertions, integration verifies CSV → scheduler → export |
| Preemptive performance | 100k ~311.35 s; complexity concern O(n × total burst) | Event-based simulation: 1,000,000 ~572 ms |
| GUI scalability | Table limited to ~10,000 rows; 1M view not feasible; Gantt compressed | Virtual `QTableView` + viewport-based Gantt; 1M rows viewable; horizontal scroll for dense segments |
| Benchmark reproducibility | Benchmark data measured manually / temporary tools | `benchmark_runner` + `benchmark_np` + report logs integrated |
| CSV error handling | Error reporting stopped early / limited visibility (as reported) | Error collection aggregates invalid lines up to a controlled limit |

---

## 3) Team progress (Week-by-week milestones)

| Week | Workstream | Deliverables | Status |
|---|---|---|---|
| 1 | Project foundation | CMake, conventions, frozen API contract | Done |
| 2 | CSV module | `CsvHandler`, CSV validation + unit tests (C01–C25) | Done |
| 3 | Non-preemptive Priority | `NonPreemptiveScheduler` + tests (N01–N18) | Done |
| 4 | Preemptive Priority | `PreemptiveScheduler` + tests (P01–P18) | Done |
| 5 | GUI skeleton + integration | `MainWindow`, `GanttChart`, GUI wiring + tests (U01–U13) | Done |
| 6 | GUI scalability | Virtual `QTableView` model + horizontal-scroll Gantt | Done (merge PR #40) |
| 7 | Benchmark + report data | `benchmark_runner`, `benchmark_np`, `generate_perf` | Done |
| 8 | Verification + final docs | `ctest` suites, final report + demo reference | Code ready; report finalized |

---

## 4) Member responsibility

Responsibility is aligned with Checkpoint 01’s member list, and updated to reflect the actual deliverables merged in the final implementation.

| # | Member | MSSV | Responsibility |
|---|---|---|---|
| 1 | Võ Nguyễn Minh Hưng (Leader) | 087207010338 | Architecture, integration, benchmark orchestration, final QA/report |
| 2 | Huỳnh Tuấn Kiệt | 066207016352 | Non-preemptive scheduler implementation + unit tests |
| 3 | Nguyễn Trường Vĩ | 095207004773 | Preemptive scheduler implementation + unit tests |
| 4 | Lê Đình Minh Hưng | 045207004732 | Qt GUI implementation (MainWindow/Gantt/virtual table) + GUI tests |
| 5 | Nguyễn Kim Hoàng Lan | 068307000700 | CSV I/O, validation rules, integration-level IO testing |

**Concrete deliverables (merged into the codebase):**

- `Kiệt NP`: `src/core/NonPreemptiveScheduler.{h,cpp}`, `tests/unit/non-preemptive/*`
- `Vĩ P`: `src/core/PreemptiveScheduler.{h,cpp}`, `tests/unit/preemptive/*`
- `Minh Hưng GUI`: `src/ui/MainWindow.{h,cpp}`, `src/ui/GanttChart.{h,cpp}`, `src/ui/ProcessTableModel.{h,cpp}`, `tests/gui/gui_test.cpp`
- `Lan CSV`: `src/io/CsvHandler.{h,cpp}`, `tests/unit/csv/csv_handler_test.cpp`
- `Hưng (Leader)`: integration glue, benchmark tools, final report documentation
