# API Reference

This document describes the internal API of memoraxx, including the LlamaStack class and Ollama integration.

## LlamaStack Class

The core class for AI model interactions with memory persistence.

### Constructor

```cpp
LlamaStack(const std::string& url = "http://localhost:11434/api/generate",
           const std::string& model = "llama3.2",
           size_t memory_size = 5,
           const std::string& mem_file = "memory.json")
```

- `url`: API endpoint URL (default: localhost:11434)
- `model`: Model name (default: llama3.2)
- `memory_size`: Max stored interactions (default: 5)
- `mem_file`: Memory file path (default: memory.json)

Throws `std::runtime_error` on cURL failure.

```cpp
LlamaStack llama;  // Default config
LlamaStack llama("http://localhost:11434/api/generate", "llama3.2", 5, "memory.json");
```

### Destructor

```cpp
~LlamaStack()
```

Cleans up cURL resources and saves memory to file.

### Methods

#### completion

```cpp
std::string completion(const std::string& prompt)
```

Sends a prompt to the AI model and returns the response.

- **Parameters**: `prompt` - Input text
- **Returns**: AI response or error message
- **Errors**: cURL failures, JSON parse errors, missing response field

```cpp
LlamaStack llama;
std::string response = llama.completion("Hello");
if (response.find("Error:") == 0) {
    std::cerr << "Error: " << response << std::endl;
}
```

#### clear_memory

```cpp
void clear_memory()
```

Clears stored conversation memory.

## Ollama API Integration

### Endpoint

- **URL**: `http://localhost:11434/api/generate`
- **Method**: POST
- **Content-Type**: application/json

### Request Format

```json
{
    "model": "llama3.2",
    "prompt": "Contextual prompt with conversation history",
    "stream": false
}
```

**Fields:**
- `model`: AI model name
- `prompt`: Input text with context
- `stream`: Always false (non-streaming)

### Response Format

#### Success
```json
{
    "model": "llama3.2",
    "response": "AI-generated text",
    "done": true
}
```

**Key Field:** `response` - Extracted AI text

#### Error
```json
{
    "error": "Error message"
}
```

### Status Codes
- 200: Success
- 400: Bad request
- 404: Model not found
- 500: Server error

## Performance Metrics

### CPU Usage
Cross-platform measurement:
- Unix: `getrusage()`
- Windows: `GetProcessTimes()`
- Returns: CPU time in milliseconds

### Response Time
High-resolution timing with `std::chrono`:
- Measures total API call duration
- Returns: Time in seconds

### GPU Usage
Not implemented (placeholder)

## Error Handling

### Error Types

#### Network Errors (cURL)
- Connection failures
- Timeouts
- HTTP errors

#### JSON Errors
- Parse failures
- Missing response field

#### Initialization Errors
- cURL setup failures

## Usage Examples

### Basic Usage
```cpp
#include <iostream>
#include <string>

int main() {
    LlamaStack llama;
    std::string response = llama.completion("Hello, AI!");
    std::cout << "Response: " << response << std::endl;
    return 0;
}
```

### With Memory
```cpp
LlamaStack llama("http://localhost:11434/api/generate", "llama3.2", 5, "memory.json");
// Memory persists across runs
```

### Error Handling
```cpp
std::string response = llama.completion("Test");
if (response.find("Error:") == 0) {
    std::cerr << "Error: " << response << std::endl;
}
```

## Configuration

Configuration via constructor parameters:

- API URL
- Model name
- Memory size
- Memory file path

All parameters have defaults for easy setup.

## Thread Safety

Not thread-safe. Use separate instances per thread.

## Memory Usage

- Base: ~1-5 MB
- Per request: ~10-100 KB
- Memory persistence: JSON file storage
