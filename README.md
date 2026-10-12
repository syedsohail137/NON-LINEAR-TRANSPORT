<!-- PROJECT BANNER -->
<br />
<div align="center">
  <h2 align="center">🧮 Quantitative Transport & Volatility Engine</h2>

  <p align="center">
    A self-calibrating C++ engine integrating optimal transport distances and Garman-Klass localized realized volatility matrices.
    <br />
    <br />
    <a href="#-core-components">View Components</a>
    ·
    <a href="#-mathematical-framework">Mathematical Core</a>
    ·
    <a href="#-getting-started">Run Baseline</a>
  </p>
</div>

<!-- BADGES -->
<div align="center">

[![Language](https://shields.io)](https://cppreference.com)
[![Dependencies](https://shields.io)](https://cppreference.com)
[![License](https://shields.io)](https://choosealicense.com)

</div>

---

## 📖 Table of Contents
- [About the Project](#-about-the-project)
- [Core Components](#-core-components)
- [Mathematical Framework](#-mathematical-framework)
- [Getting Started](#-getting-started)
- [Execution Output](#-execution-output)

---

## 🧐 About the Project

This repository contains a zero-dependency, native cross-platform quantitative execution environment. Built entirely in pure C++, the architecture ingests historical text matrices to parse asset trends, self-calibrates via **Ordinary Least Squares (OLS)** regressions, and maps derivative surface structures using optimized **Optimal Transport** distances. 

---

## 📁 Core Components

All key entities are bundled systematically within the `generated/` workspace folder:

* **`engine.cpp`** — The production execution script. Written in pure C++ using only standard headers (`<vector>`, `<cmath>`, `<fstream>`). It controls the historical parser, OLS matrix solvers, and optimal transport loops.
* **`market_history.txt`** — Clean, structured historical dataset containing Open, High, Low, Close, and Volume (OHLCV) fields to seed the localized volatility routines.
* **`option_chain.txt`** — Multi-strike option pricing chain accompanied by structural evaluation headers and structural expiry dates.
* **`output.txt`** — Real-world terminal readout verifying exact parameter extractions and robust target values.
* **`idea.pdf`** — Academic research paper detailing the mathematical physics-to-finance integration framework with modern academic citations.

---

## 🧮 Mathematical Framework

The core engine relies on structural financial and statistical paradigms:
1. **Garman-Klass Volatility:** Utilizes high, low, open, and close historical intervals to extract a minimal-variance realized volatility baseline.
2. **OLS Calibration:** Solves historical linear text matrices via standard Ordinary Least Squares mapping to establish system parameters.
3. **Optimal Transport:** Computes geometric distance metrics over distribution matrices to isolate structural targets (\(V^{*}\)).

---

## 🚀 Getting Started

### Prerequisites
* Any standard **C++17 compatible compiler** (GCC, Clang, or MSVC).
* No external third-party matrix packages required.

### Compilation & Execution

1. Open your terminal and navigate to the project directory:
   ```bash
   cd generated
   ```

2. Compile the engine using native flag optimizations:
   ```bash
   g++ -O3 -std=c++17 engine.cpp -o quant_engine
   ```

3. Run the compiled executable:
   ```bash
   ./quant_engine
   ```

---

## 📊 Execution Output

Upon completion, the engine logs data directly to the console and mirrors the runtime telemetry inside **`output.txt`**:

```text
[INFO] Parsing market_history.txt ... Complete.
[INFO] Initializing Garman-Klass volatility matrix routines.
[INFO] Resolving OLS Matrix System Self-Calibration...
[INFO] Computing Optimal Transport distance layers...
----------------------------------------------------------------
>> Parameter Extraction: Successful
>> Targeted Robust Value (V*): [Validated Target Metric]
----------------------------------------------------------------
[SUCCESS] Logs safely exported to output.txt
```

---

## 📄 License

Distributed under the MIT License. See project root for full licensing text.
