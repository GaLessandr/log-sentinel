# Log Sentinel

A modular, high-performance CLI log analyzer and real-time anomaly detector written in modern **C++17**. Built to parse web server access logs (Nginx/Apache Combined format), aggregate traffic metrics, and detect error spikes using a time-based sliding window.

## Architecture

The codebase separates public interfaces (`include/`), core implementation (`src/`), and sample datasets (`data/`) to support modular expansion and unit testing:

```text
log-sentinel/
├── CMakeLists.txt          # C++17 build configuration with strict compiler warnings
├── include/
│   ├── log_entry.hpp       # Strongly-typed representation of a parsed log record
│   ├── log_parser.hpp      # Zero-copy string_view parser interface & timestamp converter
│   └── analyzer.hpp        # Sliding-window anomaly detector & metrics aggregator interface
├── src/
│   ├── main.cpp            # CLI entry point and formatted report renderer
│   ├── log_parser.cpp      # Log line tokenization and UTC epoch parsing
│   └── analyzer.cpp        # Stateful ingestion engine and threshold alert logic
└── data/
    └── sample_access.log   # Sample dataset containing simulated 5xx traffic spikes
