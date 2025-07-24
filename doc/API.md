# API Reference

How to use the LlamaStack class and what Ollama expects. Pretty straightforward stuff.

## Internal API

### LlamaStack Class

The core class that handles all AI model interactions.

#### Constructor

```cpp
LlamaStack(bool use_gpu = false)
```

- `use_gpu`: GPU acceleration (placeholder)
- Throws `std::runtime_error` on cURL failure

```cpp
LlamaStack llama;        // CPU mode
LlamaStack llama(true);  // GPU mode (future)
```

#### Destructor

```cpp
~LlamaStack()
```

Automatically cleans up cURL resources. No manual cleanup required.

#### completion Method

```cpp
std::string completion(const std::string& prompt)
```

Returns the AI response or an error message if something went wrong:
- `"Error: cURL not initialized"`
- `"cURL error: [description]"`  
- `"JSON parse error: [description]"`
- `"Error: No response field in API output"`

```cpp
LlamaStack llama;
std::string response = llama.completion("Hello");
if (response.find("Error:") == 0) {
    std::cerr << response << std::endl;
}
```

## External Ollama API

### Endpoint Configuration

- **Base URL**: `http://localhost:11434`
- **API Path**: `/api/generate`
- **Method**: `POST`
- **Content-Type**: `application/json`

### Request Format

```json
{
    "model": "llama3.2",
    "prompt": "Your input text here",
    "stream": false
}
```

**Fields:**
- `model` (string): The AI model to use. Currently hardcoded to "llama3.2"
- `prompt` (string): The input text for the AI to respond to
- `stream` (boolean): Whether to stream the response. Always `false` in current implementation

### Response Format

#### Successful Response
```json
{
    "model": "llama3.2",
    "created_at": "2025-07-24T23:18:25.123456Z",
    "response": "The AI-generated response text appears here.",
    "done": true,
    "context": [1, 2, 3, ...],
    "total_duration": 6015560000,
    "load_duration": 123456,
    "prompt_eval_count": 15,
    "prompt_eval_duration": 234567,
    "eval_count": 42,
    "eval_duration": 5678901
}
```

**Key Fields:**
- `response` (string): The main AI-generated text (this is what we extract)
- `done` (boolean): Indicates if the response is complete
- `total_duration` (integer): Total time in nanoseconds
- `eval_count` (integer): Number of tokens generated

#### Error Response
```json
{
    "error": "model 'llama3.2' not found"
}
```

### HTTP Status Codes

- **200 OK**: Successful request
- **400 Bad Request**: Invalid JSON or missing required fields
- **404 Not Found**: Model not found
- **500 Internal Server Error**: Server-side error

## Performance Metrics API

The application provides built-in performance monitoring through system calls.

### CPU Usage Measurement

```cpp
struct rusage usage;
getrusage(RUSAGE_SELF, &usage);

double cpu_usage = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0 +
                   (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000.0;
```

**Returns:** CPU time in milliseconds (user + system time)

### Response Time Measurement

```cpp
auto start_time = std::chrono::high_resolution_clock::now();
// ... API call ...
auto end_time = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> duration = end_time - start_time;
```

**Returns:** Wall clock time in seconds (double precision)

### Power Consumption Estimation

```cpp
double power_consumption = cpu_usage * 0.5 + gpu_usage * 0.1;
```

**Formula:** Hypothetical calculation for demonstration
- CPU factor: 0.5 units per millisecond
- GPU factor: 0.1 units per percent utilization
- **Note:** These are arbitrary units for demo purposes

## Error Handling Patterns

### Three-Tier Error Strategy

#### 1. Network Level (cURL)
```cpp
CURLcode res = curl_easy_perform(curl);
if (res != CURLE_OK) {
    return "cURL error: " + std::string(curl_easy_strerror(res));
}
```

**Common cURL Errors:**
- `CURLE_COULDNT_CONNECT`: Ollama server not running
- `CURLE_OPERATION_TIMEDOUT`: Request timeout
- `CURLE_HTTP_RETURNED_ERROR`: HTTP error status

#### 2. JSON Level (Parsing)
```cpp
try {
    json response_json = json::parse(response_buffer);
    if (response_json.contains("response")) {
        return response_json["response"].get<std::string>();
    } else {
        return "Error: No response field in API output";
    }
} catch (const json::exception& e) {
    return "JSON parse error: " + std::string(e.what());
}
```

**Common JSON Errors:**
- Parse errors: Malformed JSON response
- Missing fields: API response structure changed
- Type errors: Unexpected data types

#### 3. Application Level (Resources)
```cpp
if (!curl) {
    throw std::runtime_error("Failed to initialize cURL");
}
```

## Usage Examples

### Basic Usage
```cpp
#include "main.cpp"  // Or separate header file

int main() {
    try {
        LlamaStack llama;
        
        std::string prompt = "Explain quantum computing in simple terms.";
        std::string response = llama.completion(prompt);
        
        std::cout << "Response: " << response << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### With Performance Monitoring
```cpp
#include <chrono>
#include <sys/resource.h>

int main() {
    LlamaStack llama;
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Make API call
    std::string response = llama.completion("Hello, AI!");
    
    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    );
    
    // Get resource usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    std::cout << "Response: " << response << std::endl;
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "CPU Time: " << usage.ru_utime.tv_sec << "s" << std::endl;
    
    return 0;
}
```

### Error Handling Example
```cpp
int main() {
    try {
        LlamaStack llama;
        
        std::string response = llama.completion("Test prompt");
        
        // Check for API errors
        if (response.find("Error:") == 0) {
            std::cerr << "API Error: " << response << std::endl;
            return 1;
        }
        
        // Check for cURL errors
        if (response.find("cURL error:") == 0) {
            std::cerr << "Network Error: " << response << std::endl;
            std::cerr << "Is Ollama running? Try: ollama serve" << std::endl;
            return 1;
        }
        
        std::cout << "Success: " << response << std::endl;
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Initialization Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Configuration Options

### Compile-Time Configuration

Currently, configuration is hardcoded but can be modified in the source:

```cpp
// In LlamaStack constructor
base_url = "http://localhost:11434/api/generate";  // API endpoint

// In completion method
{"model", "llama3.2"},  // AI model name
{"stream", false}       // Streaming mode
```

### Future Configuration API

Planned configuration system:

```cpp
struct LlamaConfig {
    std::string api_endpoint = "http://localhost:11434/api/generate";
    std::string model_name = "llama3.2";
    bool enable_streaming = false;
    int timeout_seconds = 30;
    bool enable_gpu = false;
};

LlamaStack llama(config);
```

## Thread Safety

**Current Status:** Not thread-safe
- Single cURL handle per instance
- No synchronization mechanisms
- Designed for single-threaded use

**Recommendations:**
- Use separate LlamaStack instances per thread
- Implement mutex protection for shared instances
- Consider connection pooling for high-concurrency scenarios

## Memory Usage

### Typical Memory Footprint
- **Base application**: ~1-5 MB
- **cURL library**: ~100 KB
- **JSON processing**: ~10-100 KB per request
- **Response buffering**: Variable (depends on response length)

### Memory Management
- **RAII pattern**: Automatic resource cleanup
- **STL containers**: Automatic memory management
- **No manual memory allocation**: Reduces leak potential