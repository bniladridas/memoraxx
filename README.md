# memoraxx

A C++ terminal client for interacting with a local Llama-based AI language model server, featuring context-aware conversations with memory persistence and performance metrics.

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/bniladridas/memoraxx)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://isocpp.org/std/the-standard)

## Overview

memoraxx is a lightweight, terminal-based application designed to interact with a local Llama 3.2 model server (via Ollama). It sends user prompts to the server, maintains conversational context using a memory system, and reports CPU usage and response times for each interaction. Key features include:

- **Context-Aware Responses**: Stores up to 5 recent prompt-response pairs in memory, persisted to `memory.json` for cross-session continuity.
- **Performance Metrics**: Measures CPU usage (`getrusage`) and response duration for each query.
- **User-Friendly Interface**: Supports commands like `exit`, `quit`, and `clear` with fuzzy matching for typos (e.g., `quite` → `quit`).
- **Robust JSON Handling**: Uses `nlohmann/json` for reliable API communication.

The project is actively developed on the `main` branch. Contributions are welcome at [github.com/bniladridas/memoraxx](https://github.com/bniladridas/memoraxx).

## Installation

### Prerequisites
- **OS**: macOS (tested on Sequoia with AppleClang 16.0.0), Linux (Ubuntu 20.04+), Windows (10+)
- **Dependencies**:
  - `libcurl` (e.g., `libcurl4-openssl-dev` on Ubuntu, included in macOS SDK)
  - `nlohmann/json` (version ≥3.10)
  - CMake (version ≥3.10)
  - C++20 compiler (e.g., AppleClang, GCC, MSVC)
  - Ollama with Llama 3.2 model
- **Optional**: `libomp` for future parallel processing (not currently required)

### Setup
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/bniladridas/memoraxx.git
   cd memoraxx
   ```

2. **Install Dependencies** (macOS):
   ```bash
   brew install curl nlohmann-json
   ```

   On Ubuntu:
   ```bash
   sudo apt-get update
   sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev cmake build-essential
   ```

   On Windows (using vcpkg):
   ```bash
   vcpkg install curl nlohmann-json
   ```
   Ensure vcpkg is integrated with Visual Studio.

3. **Install Ollama**:
   Follow instructions at [ollama.ai](https://ollama.ai) and pull the Llama 3.2 model:
   ```bash
   ollama pull llama3.2
   ```

4. **Build the Project**:
   Use the build script:
   ```bash
   ./build.sh
   ```
   
   Or manually:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

## Usage

1. **Start the Ollama Server**:
   ```bash
   ollama serve
   ```

2. **Run memoraxx**:
   ```bash
   ./build/memoraxx
   ```

3. **Interact**:
   - Enter prompts at the `>` cursor.
   - Use commands:
     - `exit` or `quit`: Exit the application.
     - `clear`: Reset conversation memory.
   - Typos are handled (e.g., `quite` → `quit`).

**Example Interaction**:
```
Waking up....
Welcome to memoraxx!
Ask anything. Type 'exit', 'quit', or 'clear' to manage memory.

> What is AI?
memoraxx is thinking...
--- AI Response ---
Artificial Intelligence (AI) is the simulation of human intelligence in machines...
-------------------
[memoraxx: brain active...]
[Sat Jul 26 02:45:00 2025, took 2.41715s, CPU usage: 123.456 ms]

> What's its history?
memoraxx is thinking...
--- AI Response ---
Building on our discussion about AI, its history began in the 1950s...
-------------------
[memoraxx: brain active...]
[Sat Jul 26 02:45:15 2025, took 2.83422s, CPU usage: 134.789 ms]

> quite
[memoraxx: shutting down...]
Exiting. Goodbye!
```

## Features

- **Memory System**: Stores up to 5 interactions in `memory.json` for context-aware responses across sessions.
- **Performance Monitoring**: Reports CPU usage (`getrusage`) and response time for each query.
- **Error Handling**: Robust cURL and JSON parsing with timeouts and HTTP status checks.
- **User Experience**: Loading animations, command suggestions, and graceful shutdown (Ctrl+C).

## Development

### Branches
- **main**: Stable branch with memory persistence and CPU metrics.

### Conventional Commits
This project enforces conventional commit standards for clean and consistent commit messages.

- Commit messages must start with a type: `feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `test:`, `chore:`, `perf:`, `ci:`, `build:`, `revert:`
- First line must be lowercase and ≤30 characters.
- To enable validation, copy the hook: `cp scripts/commit-msg .git/hooks/ && chmod +x .git/hooks/commit-msg`
- To rewrite messages, use `scripts/rewrite_msg.sh`

### Contributing
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/your-feature`).
3. Commit changes (`git commit -m 'Add your feature'`).
4. Push to the branch (`git push origin feature/your-feature`).
5. Open a pull request.

### Known Issues
- GPU usage measurement is not implemented (planned for future releases).
- `memory.json` is plain text; encryption is recommended for sensitive data.

## Acknowledgments
- **Meta AI**: For the Llama 3.2 model.
- **Ollama**: For the local model server.
- **nlohmann/json**: For robust JSON handling.
- **libcurl**: For HTTP communication.

## License
Apache 2.0 License. See [LICENSE](LICENSE) for details.

## Author
Niladri Das