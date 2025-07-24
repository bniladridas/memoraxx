# Setup Guide

How to get this thing running on your machine. I tested it on macOS, should work on Linux too.

## What you need

- macOS/Linux (haven't tested Windows but WSL might work)
- Any decent C++ compiler with C++11 support
- CMake 3.10+ (probably works with older versions too)
- libcurl, nlohmann/json, and Ollama running

## Installation

### Get Ollama running
```bash
brew install ollama                              # macOS
curl -fsSL https://ollama.ai/install.sh | sh    # Linux
ollama serve && ollama pull llama3.2            # this takes a while
```

### Install Dependencies
```bash
# macOS
brew install cmake curl nlohmann-json

# Ubuntu/Debian  
sudo apt-get update && sudo apt-get install -y cmake libcurl4-openssl-dev nlohmann-json3-dev build-essential
```

## Build

```bash
git clone <repo-url> && cd cpp_terminal_app
cmake -S . -B build && cmake --build build
```

## Usage

### Basic Usage
```bash
# Run the application
./build/LlamaTerminalApp

# Enter your message when prompted
Enter your message: Hello, how are you?
```

### Example Output
```
Response:
- Date and Time: Thu Jul 24 23:18:25 2025
- Reason for Response: The AI responded to the user's query.
- Resource Consumption: CPU usage: 23.172 ms, GPU usage: 0%
- Hypothetical Power Consumption: 11.586 units
- Duration: 6.01556 seconds
- Response: Hello! It's great to meet you...
```

## API Reference

### LlamaStack Class

#### Constructor
```cpp
LlamaStack(bool use_gpu = false)
```
- `use_gpu`: Enable GPU acceleration (placeholder for future implementation)

#### Methods
```cpp
std::string completion(const std::string& prompt)
```
- **Parameters**: `prompt` - The input text to send to the AI model
- **Returns**: AI-generated response as string
- **Throws**: `std::runtime_error` if cURL initialization fails

### Configuration
- **API Endpoint**: `http://localhost:11434/api/generate`
- **Model**: `llama3.2`
- **Content-Type**: `application/json`
- **Streaming**: Disabled (`"stream": false`)

## Performance Monitoring

The application tracks several performance metrics:

### CPU Usage
- Measured using `getrusage()` system call
- Reports user and system time in milliseconds
- Formula: `(user_time + system_time) * 1000`

### Response Time
- High-resolution timing using `std::chrono`
- Measures total API call duration
- Includes network latency and model inference time

### Power Consumption (Estimated)
- Hypothetical calculation: `cpu_usage * 0.5 + gpu_usage * 0.1`
- Units are arbitrary for demonstration purposes
- Can be customized for specific hardware profiling

### GPU Usage
- Currently placeholder (returns 0%)
- Ready for integration with NVIDIA CUDA or AMD ROCm APIs

## Troubleshooting

### Common Issues

#### 1. "Failed to initialize cURL"
```bash
# Install libcurl development headers
# Ubuntu/Debian:
sudo apt-get install libcurl4-openssl-dev

# macOS:
brew install curl
```

#### 2. "Could not find nlohmann_json"
```bash
# Ubuntu/Debian:
sudo apt-get install nlohmann-json3-dev

# macOS:
brew install nlohmann-json
```

#### 3. "Connection refused" or API errors
```bash
# Ensure Ollama is running
ollama serve

# Verify model is available
ollama list

# Pull model if missing
ollama pull llama3.2
```

#### 4. Build fails with C++11 errors
```bash
# Ensure your compiler supports C++11
g++ --version
clang++ --version

# Update CMakeLists.txt if needed:
set(CMAKE_CXX_STANDARD 17)  # Use C++17 instead
```

### Debug Mode
To enable verbose output for debugging:
```cpp
// Add to main.cpp for debugging
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
```

### Performance Optimization
- **Model Selection**: Smaller models (7B) respond faster than larger ones (70B)
- **Hardware**: SSD storage improves model loading times
- **Memory**: Ensure sufficient RAM for model size
- **Network**: Local inference eliminates network latency

## Contributing

### Code Style
- Follow C++11 standards
- Use meaningful variable names
- Add error handling for all external API calls
- Include performance measurements for new features

### Testing
```bash
# Build and run basic functionality test
cmake --build build
echo "test message" | ./build/LlamaTerminalApp
```

## License

Apache 2.0 - See [LICENSE](../LICENSE)