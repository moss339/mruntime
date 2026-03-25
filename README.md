# mruntime

Runtime foundation library for MOSS project, providing process priority management, CPU core affinity, and time acquisition features.

## Features

- Process priority management (nice value)
- CPU core affinity binding
- System uptime measurement (nanoseconds/microseconds)
- Epoch-based timestamp (milliseconds)
- High-precision sleep functions

## Architecture

- `Runtime` - Main entry point with configuration
- `Process` - Process-level management (priority, CPU affinity)
- `Clock` - Time acquisition and sleep functions

## Usage

```cpp
#include <mruntime/mruntime.h>

int main() {
    mruntime::Runtime::Config config;
    config.priority = -10;
    config.cpu_affinity = {0, 1, 2, 3};

    auto runtime = mruntime::Runtime::create(config);

    // Use clock
    uint64_t uptime = mruntime::Clock::uptime_ns();
    mruntime::Clock::sleep_ms(100);

    return 0;
}
```

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
