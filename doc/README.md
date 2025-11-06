# memoraxx Setup Guide

This guide provides detailed instructions for setting up and running memoraxx on your machine.

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

### Install Ollama
Follow instructions at [ollama.ai](https://ollama.ai) to install Ollama, then pull the Llama 3.2 model:
```bash
ollama pull llama3.2
```

### Install Dependencies

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

## Build

Clone the repository and build:
```bash
git clone https://github.com/bniladridas/memoraxx.git
cd memoraxx
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

## Usage

### Basic Usage
```bash
# Start Ollama server
ollama serve

# Run memoraxx
./build/memoraxx
```

Enter prompts at the `>` cursor. Use commands like `exit`, `quit`, or `clear`.

### Example Interaction
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

> exit
[memoraxx: shutting down...]
Exiting. Goodbye!
```

## API Reference

### LlamaStack Class

#### Constructor
```cpp
LlamaStack(const std::string& url = "http://localhost:11434/api/generate",
           const std::string& model = "llama3.2",
           size_t memory_size = 5,
           const std::string& mem_file = "memory.json")
```
- `url`: API endpoint URL
- `model`: Model name
- `memory_size`: Max interactions to store
- `mem_file`: File for persistent memory

#### Methods
```cpp
std::string completion(const std::string& prompt)
```
- **Parameters**: `prompt` - The input text to send to the AI model
- **Returns**: AI-generated response as string
- **Throws**: `std::runtime_error` on failure

```cpp
void clear_memory()
```
- Clears stored conversation memory

### Configuration
- **API Endpoint**: `http://localhost:11434/api/generate`
- **Model**: `llama3.2`
- **Content-Type**: `application/json`
- **Streaming**: Disabled

## Performance Monitoring

The application measures performance metrics for each query:

### CPU Usage
- Measured using `getrusage()` on Unix or `GetProcessTimes()` on Windows
- Reports CPU time in milliseconds

### Response Time
- High-resolution timing using `std::chrono`
- Measures total duration from request to response

### GPU Usage
- Not implemented (placeholder for future releases)

## Troubleshooting

### Common Issues

#### Build fails with missing dependencies
Ensure all dependencies are installed as per the Requirements section.

#### Connection errors
```bash
# Ensure Ollama is running
ollama serve

# Check model availability
ollama list

# Pull model if needed
ollama pull llama3.2
```

#### Compiler errors
Ensure your compiler supports C++20. Update CMakeLists.txt if needed.

### Debug Mode
To enable verbose cURL output, modify the code to set `CURLOPT_VERBOSE`.

### Performance Tips
- Use smaller models for faster responses
- Ensure adequate RAM for the model
- Local inference avoids network latency

## Contributing

### Code Style
- Follow C++20 standards
- Use meaningful variable names
- Add error handling
- Include performance measurements

### Testing
```bash
# Build and test
./build.sh
echo "test" | ./build/memoraxx
```

## License

Apache 2.0 License. See [LICENSE](../LICENSE) for details.
