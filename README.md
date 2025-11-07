# memoraxx

A C++ terminal client for interacting with a local Llama-based AI language model server, featuring context-aware conversations with memory persistence and performance metrics.

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/bniladridas/memoraxx)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://isocpp.org/std/the-standard)

## Overview

memoraxx is a lightweight, terminal-based application designed to interact with a local Llama 3.2 model server (via Ollama). It sends user prompts to the server, maintains conversational context using a memory system, and reports CPU usage and response times for each interaction. Key features include:

- **Context-Aware Responses**: Stores up to 5 recent prompt-response pairs in memory, persisted to `memory.json` for cross-session continuity.
- **Performance Metrics**: Measures CPU usage (`getrusage`) and response duration for each query.
- **Agent Capabilities**: Supports tool calling for executing shell commands and extending functionality autonomously.
- **User-Friendly Interface**: Supports commands like `exit`, `quit`, and `clear` with fuzzy matching for typos (e.g., `quite` → `quit`).
- **Robust JSON Handling**: Uses `nlohmann/json` for reliable API communication.

## Documentation

- [API Reference](doc/API.md)
- [Architecture](doc/ARCHITECTURE.md)
- [Examples](doc/EXAMPLES.md)
- [Docker Setup](doc/DOCKER.md)
- [Contributing](CONTRIBUTING.md)

## Requirements

- **OS**: macOS (tested on Sequoia with AppleClang 16.0.0), Linux (Ubuntu 20.04+), Windows (10+)
- **Dependencies**:
  - `libcurl` (e.g., `libcurl4-openssl-dev` on Ubuntu, included in macOS SDK)
  - `nlohmann/json` (version ≥3.10)
  - CMake (version ≥3.10)
  - C++20 compiler (e.g., AppleClang, GCC, MSVC)
  - Ollama with Llama 3.2 model
- **Optional**: `libomp` for future parallel processing (not currently required)

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/bniladridas/memoraxx.git
   cd memoraxx
   ```

2. **Install Dependencies**:

   **macOS**:
   ```bash
   brew install curl nlohmann-json
   ```

   **Ubuntu**:
   ```bash
   sudo apt-get update
   sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev cmake build-essential
   ```

   **Windows (using vcpkg)**:
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

   > [!NOTE]
   > The build script automates the CMake configuration and build process.

    Or manually:
    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
    ```

### Using Docker

To build and run using Docker:
```bash
docker build -t memoraxx .
docker run -it memoraxx
```
> [!NOTE]
> This builds the application inside a container. For full functionality, Ollama must be running on the host or in a linked container.

5. **Run Tests**:
   ```bash
   ./e2e.sh
   ```
   This runs end-to-end tests, including build verification and integration tests (requires Ollama running).

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
  - **Agent Mode**: Ask the AI to use tools, e.g., "Run the command 'ls'" to execute shell commands.

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

 > Run the command 'echo hello'
 memoraxx is thinking...
 --- AI Response ---
 Command output:
 hello

 -------------------
 [memoraxx: brain active...]
 [Sat Jul 26 02:45:30 2025, took 1.5s, CPU usage: 100.0 ms]

 > quite
 [memoraxx: shutting down...]
 Exiting. Goodbye!
 ```

## Configuration

memoraxx can be configured via a `config.json` file in the project root. If the file is missing, default values are used.

Example `config.json`:
```json
{
    "base_url": "http://localhost:11434/api/generate",
    "model": "llama3.2",
    "max_tokens": 4096,
    "memory_file": "memory.json"
}
```

- `base_url`: URL of the Ollama API endpoint.
- `model`: The model name to use (e.g., "llama3.2").
- `max_tokens`: Maximum number of tokens to store in memory.
- `memory_file`: Path to the file for persisting conversation memory.

## Features

- **Memory System**: Stores up to 5 interactions in `memory.json` for context-aware responses across sessions.
- **Performance Monitoring**: Reports CPU usage (`getrusage`) and response time for each query.
- **Error Handling**: Robust cURL and JSON parsing with timeouts and HTTP status checks.
- **User Experience**: Loading animations, command suggestions, and graceful shutdown (Ctrl+C).

## Development

### Branches
- **main**: Stable branch with memory persistence and CPU metrics.

### Known Issues
- GPU usage measurement is not implemented (planned for future releases).
- `memory.json` is plain text; encryption is recommended for sensitive data.

## Frequently Asked Questions

### How do I install Ollama?
Follow the instructions at [ollama.ai](https://ollama.ai) to install Ollama, then run `ollama pull llama3.2` to get the model.

### How do I build the project?
Use `./build.sh` or follow the manual steps in Installation.

### What is the memory system?
The app stores up to 5 recent interactions in `memory.json` for context-aware responses.

## Acknowledgments
- **Meta AI**: For the Llama 3.2 model.
- **Ollama**: For the local model server.
- **nlohmann/json**: For robust JSON handling.
- **libcurl**: For HTTP communication.

## License
Apache 2.0 License. See [LICENSE](LICENSE) for details.
