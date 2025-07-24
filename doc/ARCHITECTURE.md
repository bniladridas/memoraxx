# Architecture

How I structured this thing. Kept it simple - one main class that talks to Ollama and tracks some basic metrics.

## System Flow

```
Client ──HTTP/JSON──► Ollama:11434 ──► Llama3.2
  │                                        │
  └── Performance Monitor ◄───────────────┘
      (CPU, Time, Memory)
```

## Component Design

### 1. LlamaStack Class

**Purpose**: Core API client for Ollama communication

**Responsibilities**:
- HTTP request/response handling
- JSON payload construction and parsing
- cURL session management
- Error handling and recovery

**Key Methods**:
```cpp
class LlamaStack {
private:
    std::string base_url;        // API endpoint
    CURL* curl;                  // cURL handle
    bool use_gpu;               // GPU preference flag

public:
    LlamaStack(bool use_gpu);   // Constructor with GPU option
    ~LlamaStack();              // Cleanup cURL resources
    std::string completion(const std::string& prompt);
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

### 3. Performance Monitoring System

**Metrics Collected**:
- **CPU Usage**: User + system time via `getrusage()`
- **Wall Clock Time**: High-resolution timing with `std::chrono`
- **Memory Usage**: Available through `rusage.ru_maxrss`
- **Power Estimation**: Calculated heuristic based on CPU usage

**Implementation**:
```cpp
// Timing measurement
auto start_time = std::chrono::high_resolution_clock::now();
// ... API call ...
auto end_time = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> duration = end_time - start_time;

// Resource usage
struct rusage usage;
getrusage(RUSAGE_SELF, &usage);
double cpu_usage = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0;
```

### 4. Error Handling Strategy

**Three-Layer Error Handling**:

1. **cURL Level**: Network and HTTP errors
```cpp
CURLcode res = curl_easy_perform(curl);
if (res != CURLE_OK) {
    return "cURL error: " + std::string(curl_easy_strerror(res));
}
```

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

### Request Flow
1. **User Input** → Terminal input capture
2. **Prompt Construction** → Add system context
3. **JSON Serialization** → Create API payload
4. **HTTP Request** → POST to Ollama server
5. **Response Handling** → Parse JSON response
6. **Output Formatting** → Display with metrics

### Response Processing
```cpp
// Input: Raw HTTP response
std::string response_buffer;

// Processing: JSON extraction
json response_json = json::parse(response_buffer);
std::string ai_response = response_json["response"];

// Output: Formatted display with metrics
std::cout << "- Response: " << ai_response << std::endl;
```

## Memory Management

### RAII Pattern
- **cURL Handle**: Automatic cleanup in destructor
- **JSON Objects**: Stack-allocated, automatic cleanup
- **String Buffers**: STL containers with automatic memory management

### Resource Lifecycle
```cpp
LlamaStack::LlamaStack() {
    curl = curl_easy_init();  // Acquire resource
    if (!curl) throw std::runtime_error("Init failed");
}

LlamaStack::~LlamaStack() {
    if (curl) {
        curl_easy_cleanup(curl);  // Release resource
    }
}
```

## Threading Model

**Current**: Single-threaded synchronous design
- Simple and predictable
- Suitable for terminal interaction
- No race conditions or synchronization complexity

**Future Considerations**:
- Async request handling for better UX
- Background performance monitoring
- Concurrent model loading

## Security Considerations

### Current Security Measures
- **Local-only communication**: No external network access
- **Input validation**: JSON structure validation
- **Resource limits**: Bounded by system resources
- **Error containment**: Graceful error handling without crashes

### Security Boundaries
```
User Input → JSON Validation → HTTP Client → Local Server
     ↓              ↓              ↓            ↓
 Sanitized → Structured → Encrypted → Processed
```

## Performance Characteristics

### Latency Profile
- **Network**: ~1-5ms (localhost)
- **JSON Processing**: ~0.1-1ms
- **Model Inference**: 1-10 seconds (model dependent)
- **Output Formatting**: ~0.1ms

### Memory Usage
- **Base Application**: ~1-5MB
- **cURL Overhead**: ~100KB
- **JSON Processing**: ~10-100KB per request
- **Response Buffering**: Variable (response size dependent)

### Scalability Limits
- **Single Request**: Limited by model inference time
- **Memory**: Limited by available system RAM
- **CPU**: Single-threaded, one core utilization

## Extension Points

### 1. Model Support
```cpp
// Current: Hardcoded model
{"model", "llama3.2"}

// Future: Configurable models
{"model", config.model_name}
```

### 2. GPU Integration
```cpp
// Placeholder for GPU metrics
int gpu_usage = 0;

// Future: CUDA/ROCm integration
int gpu_usage = get_gpu_utilization();
```

### 3. Streaming Support
```cpp
// Current: Batch responses
{"stream", false}

// Future: Real-time streaming
{"stream", true}
// + WebSocket or SSE handling
```

### 4. Configuration System
```cpp
// Future: External configuration
struct Config {
    std::string api_endpoint;
    std::string model_name;
    bool enable_gpu;
    int timeout_seconds;
};
```

## Build System

### CMake Configuration
```cmake
# Minimum requirements
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

# Dependencies
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Linking
target_link_libraries(LlamaTerminalApp 
    PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json
)
```

### Dependency Management
- **System Libraries**: libcurl (HTTP client)
- **Header-only Libraries**: nlohmann/json (JSON processing)
- **Build Tools**: CMake (cross-platform builds)

## Testing Strategy

### Current Testing
- **Manual Testing**: Interactive terminal usage
- **Build Verification**: CMake compilation success
- **Basic Functionality**: Single request/response cycle

### Future Testing Framework
```cpp
// Unit tests for core components
TEST(LlamaStackTest, BasicCompletion) {
    LlamaStack llama;
    std::string response = llama.completion("test");
    ASSERT_FALSE(response.empty());
}

// Integration tests
TEST(IntegrationTest, EndToEndFlow) {
    // Test full request/response cycle
}

// Performance tests
TEST(PerformanceTest, ResponseTime) {
    // Measure and validate response times
}
```