# Architecture

memoraxx is a C++ terminal client for interacting with local Llama AI models via Ollama, featuring memory persistence and performance monitoring.

## System Overview

```
Terminal ──HTTP/JSON──► Ollama Server ──► Llama Model
    │                           │
    └── Memory System ◄─────────┘
        (JSON persistence)
```

## Core Components

### LlamaStack Class

**Purpose**: Manages AI model interactions with conversation memory.

**Key Features**:
- HTTP communication with Ollama
- JSON request/response handling
- Conversation context storage
- Cross-platform performance monitoring

**Interface**:
```cpp
class LlamaStack {
private:
    std::string base_url;
    std::string model_name;
    CURL* curl;
    std::deque<Interaction> memory;
    size_t max_memory_size;
    std::string memory_file;

public:
    LlamaStack(const std::string& url, const std::string& model,
               size_t mem_size, const std::string& mem_file);
    ~LlamaStack();
    std::string completion(const std::string& prompt);
    void clear_memory();
};
```

### 2. HTTP Communication Layer

**Protocol**: HTTP POST to `/api/generate`
**Content-Type**: `application/json`
**Payload Structure**:
```json
{
    "model": "llama3.2",
    "prompt": "user input text",
    "stream": false
}
```

**Response Structure**:
```json
{
    "response": "AI generated text",
    "done": true,
    "context": [...],
    "total_duration": 1234567890,
    "load_duration": 123456,
    "prompt_eval_count": 10,
    "eval_count": 25
}
```

### Performance Monitoring

**Metrics**:
- CPU time (cross-platform)
- Response duration
- Memory usage (future)

**Implementation**:
```cpp
// Cross-platform CPU measurement
double cpu_before = get_cpu_time();
// ... API call ...
double cpu_after = get_cpu_time();
double cpu_usage = (cpu_after - cpu_before) * 1000.0; // ms

// Timing
auto start = std::chrono::high_resolution_clock::now();
// ... 
auto end = std::chrono::high_resolution_clock::now();
double duration = std::chrono::duration<double>(end - start).count();
```

### Error Handling

**Layers**:
1. Network (cURL errors)
2. JSON parsing
3. Application logic

**Strategy**: Return descriptive error strings for user feedback.

2. **JSON Level**: Parsing and structure validation
```cpp
try {
    json response_json = json::parse(response_buffer);
    if (response_json.contains("response")) {
        return response_json["response"].get<std::string>();
    }
} catch (const json::exception& e) {
    return "JSON parse error: " + std::string(e.what());
}
```

3. **Application Level**: Resource initialization and cleanup
```cpp
if (!curl) {
    throw std::runtime_error("Failed to initialize cURL");
}
```

## Data Flow

### Request Processing
1. User input → Fuzzy command matching
2. Build context from memory
3. JSON payload → HTTP POST to Ollama
4. Parse response → Store in memory
5. Display with performance metrics

### Memory Management
- Deque stores last N interactions
- JSON serialization for persistence
- Automatic cleanup on overflow

## Memory Management

### RAII Pattern
- cURL handles: Automatic cleanup
- STL containers: Automatic memory management
- Memory persistence: JSON file I/O

### Conversation Memory
- Deque for efficient FIFO storage
- Configurable size limit
- File-based persistence

## Threading Model

Single-threaded with async UI elements (loading animations).

Future: Potential async HTTP requests.

## Security

- Localhost-only communication
- Input sanitization
- JSON validation
- Graceful error handling

## Performance

### Latency
- Network: ~1-5ms
- JSON: ~0.1-1ms
- Model inference: 1-10s
- Total: Model-dependent

### Memory
- Base: ~1-5MB
- Per request: ~10-100KB
- Memory file: Persistent storage

## Future Extensions

- Configurable models
- GPU acceleration
- Streaming responses
- External configuration files

## Build System

### CMake Configuration
```cmake
cmake_minimum_required(VERSION 3.10)
project(memoraxx LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(CURL REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)

add_executable(memoraxx src/main.cpp)
target_link_libraries(memoraxx PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
```

### Dependencies
- libcurl: HTTP client
- nlohmann/json: JSON processing
- CMake: Build system

## CI/CD

### GitHub Actions Workflow

The project uses GitHub Actions for automated testing and building.

- **Workflow File**: `.github/workflows/ci.yml`
- **Environment**: Ubuntu latest
- **Triggers**: Push and pull requests to `main` branch
- **Validation**: YAML syntax is validated using yamllint in CI
- **Steps**:
  1. Checkout code
  2. Lint YAML files with yamllint
  3. Install dependencies (libcurl4-openssl-dev, nlohmann-json3-dev)
  4. Set up CMake
  5. Configure and build project
  6. Run end-to-end tests
  7. Build and push Docker image to GHCR (on push)

### Build Scripts

- **`build.sh`**: Automates the build process using CMake
- **`e2e.sh`**: Runs comprehensive tests including build verification and integration tests with Ollama

## Infrastructure Requirements

### Local Development

- **Ollama Server**: Must be running locally on `http://localhost:11434`
- **Model**: llama3.2 (or compatible Llama model)
- **Dependencies**: libcurl, nlohmann/json (installed via apt on Ubuntu)

### Deployment

- Self-contained executable
- No external services required beyond Ollama
- Cross-platform (Linux/macOS/Windows via CMake)

## Testing

### Current
- Manual testing via terminal
- Build verification
- Basic functionality checks

### Future
- Unit tests for LlamaStack
- Integration tests
- Performance benchmarks