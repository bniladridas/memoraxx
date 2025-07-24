# Examples

Some ways to use this. Started with basic examples, then got carried away and added more complex stuff.

## Basic Usage Examples

### Basic usage

```bash
$ ./build/LlamaTerminalApp
Enter your message: What is the capital of Japan?

Response:
- CPU usage: 25.3 ms, Duration: 3.2 seconds  
- Response: The capital of Japan is Tokyo.
```

Pretty basic but it works. The timing info is actually useful for seeing how fast your setup is.

### Creative Writing

```bash
Enter your message: Write a short poem about coding
Response: Lines of code dance across the screen,
Logic flows like a digital stream...
```

### Technical Explanation

```bash
Enter your message: Explain REST APIs in simple terms
Response: REST APIs are like restaurant waiters. You request food (data) using a menu (endpoints)...
```

## Code Integration Examples

### 1. Custom Prompt Engineering

```cpp
#include <iostream>
#include <string>
// ... other includes from main.cpp

std::string createSystemPrompt(const std::string& role, const std::string& context) {
    return "You are a " + role + ". " + context + "\n\nUser: ";
}

int main() {
    try {
        LlamaStack llama(true);
        
        // Create specialized prompts
        std::string system_prompt = createSystemPrompt(
            "helpful programming tutor",
            "Explain concepts clearly with examples. Use simple language."
        );
        
        std::string user_input;
        std::cout << "Ask a programming question: ";
        std::getline(std::cin, user_input);
        
        std::string full_prompt = system_prompt + user_input + "\nTutor:";
        std::string response = llama.completion(full_prompt);
        
        std::cout << "Programming Tutor: " << response << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 2. Batch Processing Multiple Queries

```cpp
#include <vector>
#include <fstream>
// ... other includes

int main() {
    try {
        LlamaStack llama;
        
        // Read questions from file
        std::vector<std::string> questions = {
            "What is machine learning?",
            "Explain neural networks",
            "What is deep learning?",
            "How does backpropagation work?"
        };
        
        std::ofstream output_file("ai_responses.txt");
        
        for (size_t i = 0; i < questions.size(); ++i) {
            std::cout << "Processing question " << (i + 1) << "/" << questions.size() << std::endl;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            std::string response = llama.completion(questions[i]);
            auto end_time = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                end_time - start_time
            );
            
            // Write to file
            output_file << "Q" << (i + 1) << ": " << questions[i] << std::endl;
            output_file << "A" << (i + 1) << ": " << response << std::endl;
            output_file << "Time: " << duration.count() << " seconds" << std::endl;
            output_file << "---" << std::endl;
        }
        
        output_file.close();
        std::cout << "Batch processing complete. Results saved to ai_responses.txt" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 3. Interactive Chat Loop

```cpp
#include <iostream>
#include <string>
#include <vector>
// ... other includes

int main() {
    try {
        LlamaStack llama;
        std::vector<std::string> conversation_history;
        
        std::cout << "=== Interactive Chat (type 'quit' to exit) ===" << std::endl;
        
        while (true) {
            std::string user_input;
            std::cout << "\nYou: ";
            std::getline(std::cin, user_input);
            
            if (user_input == "quit" || user_input == "exit") {
                break;
            }
            
            // Build context from conversation history
            std::string context = "Previous conversation:\n";
            for (const auto& msg : conversation_history) {
                context += msg + "\n";
            }
            
            std::string full_prompt = context + "User: " + user_input + "\nAssistant:";
            
            std::cout << "AI: ";
            std::string response = llama.completion(full_prompt);
            std::cout << response << std::endl;
            
            // Update conversation history
            conversation_history.push_back("User: " + user_input);
            conversation_history.push_back("Assistant: " + response);
            
            // Keep only last 10 exchanges to manage context length
            if (conversation_history.size() > 20) {
                conversation_history.erase(conversation_history.begin(), conversation_history.begin() + 2);
            }
        }
        
        std::cout << "Chat ended. Goodbye!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Performance Monitoring Examples

### 1. Detailed Performance Analysis

```cpp
#include <iomanip>
// ... other includes

struct PerformanceMetrics {
    double response_time_seconds;
    double cpu_time_ms;
    long memory_usage_kb;
    std::string timestamp;
};

PerformanceMetrics measurePerformance(LlamaStack& llama, const std::string& prompt) {
    PerformanceMetrics metrics;
    
    // Get initial resource usage
    struct rusage usage_before, usage_after;
    getrusage(RUSAGE_SELF, &usage_before);
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Make API call
    std::string response = llama.completion(prompt);
    
    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    getrusage(RUSAGE_SELF, &usage_after);
    
    // Calculate metrics
    auto duration = std::chrono::duration<double>(end_time - start_time);
    metrics.response_time_seconds = duration.count();
    
    double cpu_before = (usage_before.ru_utime.tv_sec + usage_before.ru_stime.tv_sec) * 1000.0 +
                       (usage_before.ru_utime.tv_usec + usage_before.ru_stime.tv_usec) / 1000.0;
    double cpu_after = (usage_after.ru_utime.tv_sec + usage_after.ru_stime.tv_sec) * 1000.0 +
                      (usage_after.ru_utime.tv_usec + usage_after.ru_stime.tv_usec) / 1000.0;
    
    metrics.cpu_time_ms = cpu_after - cpu_before;
    metrics.memory_usage_kb = usage_after.ru_maxrss;
    
    // Get timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    metrics.timestamp = std::ctime(&time_t);
    
    return metrics;
}

int main() {
    try {
        LlamaStack llama;
        
        std::vector<std::string> test_prompts = {
            "Hello",
            "Explain quantum physics in one paragraph",
            "Write a 500-word essay about climate change",
            "What is 2+2?"
        };
        
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Performance Analysis Results:" << std::endl;
        std::cout << "=============================" << std::endl;
        
        for (size_t i = 0; i < test_prompts.size(); ++i) {
            std::cout << "\nTest " << (i + 1) << ": " << test_prompts[i].substr(0, 30) << "..." << std::endl;
            
            PerformanceMetrics metrics = measurePerformance(llama, test_prompts[i]);
            
            std::cout << "  Response Time: " << metrics.response_time_seconds << " seconds" << std::endl;
            std::cout << "  CPU Time: " << metrics.cpu_time_ms << " ms" << std::endl;
            std::cout << "  Memory Usage: " << metrics.memory_usage_kb << " KB" << std::endl;
            std::cout << "  Timestamp: " << metrics.timestamp;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 2. Performance Logging to CSV

```cpp
#include <fstream>
#include <sstream>
// ... other includes

void logPerformanceToCSV(const std::string& prompt, const PerformanceMetrics& metrics) {
    std::ofstream csv_file("performance_log.csv", std::ios::app);
    
    // Check if file is empty (write header)
    csv_file.seekp(0, std::ios::end);
    if (csv_file.tellp() == 0) {
        csv_file << "Timestamp,Prompt,ResponseTime(s),CPUTime(ms),Memory(KB)" << std::endl;
    }
    
    // Escape commas in prompt
    std::string escaped_prompt = prompt;
    std::replace(escaped_prompt.begin(), escaped_prompt.end(), ',', ';');
    
    csv_file << metrics.timestamp.substr(0, 24) << ","
             << escaped_prompt << ","
             << metrics.response_time_seconds << ","
             << metrics.cpu_time_ms << ","
             << metrics.memory_usage_kb << std::endl;
    
    csv_file.close();
}
```

## Error Handling Examples

### 1. Comprehensive Error Handling

```cpp
#include <iostream>
#include <string>
// ... other includes

enum class ErrorType {
    NONE,
    CURL_ERROR,
    JSON_ERROR,
    API_ERROR,
    INIT_ERROR
};

ErrorType classifyError(const std::string& response) {
    if (response.find("cURL error:") == 0) return ErrorType::CURL_ERROR;
    if (response.find("JSON parse error:") == 0) return ErrorType::JSON_ERROR;
    if (response.find("Error:") == 0) return ErrorType::API_ERROR;
    return ErrorType::NONE;
}

void handleError(ErrorType error_type, const std::string& error_message) {
    switch (error_type) {
        case ErrorType::CURL_ERROR:
            std::cerr << "Network Error: " << error_message << std::endl;
            std::cerr << "Suggestions:" << std::endl;
            std::cerr << "  - Check if Ollama is running: ollama serve" << std::endl;
            std::cerr << "  - Verify network connectivity" << std::endl;
            std::cerr << "  - Check firewall settings" << std::endl;
            break;
            
        case ErrorType::JSON_ERROR:
            std::cerr << "Data Format Error: " << error_message << std::endl;
            std::cerr << "Suggestions:" << std::endl;
            std::cerr << "  - Update Ollama to latest version" << std::endl;
            std::cerr << "  - Check API compatibility" << std::endl;
            break;
            
        case ErrorType::API_ERROR:
            std::cerr << "API Error: " << error_message << std::endl;
            std::cerr << "Suggestions:" << std::endl;
            std::cerr << "  - Verify model is installed: ollama list" << std::endl;
            std::cerr << "  - Pull model if missing: ollama pull llama3.2" << std::endl;
            break;
            
        case ErrorType::INIT_ERROR:
            std::cerr << "Initialization Error: " << error_message << std::endl;
            std::cerr << "Suggestions:" << std::endl;
            std::cerr << "  - Reinstall libcurl development libraries" << std::endl;
            std::cerr << "  - Check system dependencies" << std::endl;
            break;
            
        case ErrorType::NONE:
            // No error
            break;
    }
}

int main() {
    try {
        LlamaStack llama;
        
        std::string user_input;
        std::cout << "Enter your message: ";
        std::getline(std::cin, user_input);
        
        std::string response = llama.completion(user_input);
        
        ErrorType error_type = classifyError(response);
        if (error_type != ErrorType::NONE) {
            handleError(error_type, response);
            return 1;
        }
        
        std::cout << "Response: " << response << std::endl;
        
    } catch (const std::runtime_error& e) {
        handleError(ErrorType::INIT_ERROR, e.what());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Integration Examples

### 1. Web Server Integration (Conceptual)

```cpp
// This is a conceptual example - would require additional libraries
#include <httplib.h>  // cpp-httplib library
// ... other includes

class AIWebService {
private:
    LlamaStack llama;
    
public:
    AIWebService() : llama(true) {}
    
    std::string handleRequest(const std::string& prompt) {
        auto start_time = std::chrono::high_resolution_clock::now();
        std::string response = llama.completion(prompt);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration<double>(end_time - start_time);
        
        // Create JSON response
        json json_response = {
            {"response", response},
            {"processing_time", duration.count()},
            {"timestamp", std::time(nullptr)}
        };
        
        return json_response.dump();
    }
};

int main() {
    AIWebService service;
    httplib::Server server;
    
    server.Post("/api/chat", [&service](const httplib::Request& req, httplib::Response& res) {
        try {
            json request_json = json::parse(req.body);
            std::string prompt = request_json["prompt"];
            
            std::string response = service.handleRequest(prompt);
            
            res.set_content(response, "application/json");
        } catch (const std::exception& e) {
            json error_response = {{"error", e.what()}};
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
        }
    });
    
    std::cout << "Starting web server on http://localhost:8080" << std::endl;
    server.listen("localhost", 8080);
    
    return 0;
}
```

### 2. Configuration File Support

```cpp
#include <fstream>
// ... other includes

struct AppConfig {
    std::string api_endpoint = "http://localhost:11434/api/generate";
    std::string model_name = "llama3.2";
    bool enable_gpu = false;
    int timeout_seconds = 30;
    bool verbose_output = false;
};

AppConfig loadConfig(const std::string& config_file) {
    AppConfig config;
    std::ifstream file(config_file);
    
    if (!file.is_open()) {
        std::cout << "Config file not found, using defaults." << std::endl;
        return config;
    }
    
    json config_json;
    file >> config_json;
    
    if (config_json.contains("api_endpoint")) {
        config.api_endpoint = config_json["api_endpoint"];
    }
    if (config_json.contains("model_name")) {
        config.model_name = config_json["model_name"];
    }
    if (config_json.contains("enable_gpu")) {
        config.enable_gpu = config_json["enable_gpu"];
    }
    if (config_json.contains("timeout_seconds")) {
        config.timeout_seconds = config_json["timeout_seconds"];
    }
    if (config_json.contains("verbose_output")) {
        config.verbose_output = config_json["verbose_output"];
    }
    
    return config;
}

int main() {
    try {
        AppConfig config = loadConfig("config.json");
        
        std::cout << "Using configuration:" << std::endl;
        std::cout << "  API Endpoint: " << config.api_endpoint << std::endl;
        std::cout << "  Model: " << config.model_name << std::endl;
        std::cout << "  GPU Enabled: " << (config.enable_gpu ? "Yes" : "No") << std::endl;
        
        LlamaStack llama(config.enable_gpu);
        
        // Rest of application logic...
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Testing Examples

### 1. Unit Test Framework (using simple assertions)

```cpp
#include <cassert>
// ... other includes

void testBasicFunctionality() {
    std::cout << "Testing basic functionality..." << std::endl;
    
    try {
        LlamaStack llama;
        std::string response = llama.completion("Hello");
        
        // Basic assertions
        assert(!response.empty());
        assert(response.find("Error:") != 0);  // Should not start with "Error:"
        
        std::cout << "✓ Basic functionality test passed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Basic functionality test failed: " << e.what() << std::endl;
    }
}

void testErrorHandling() {
    std::cout << "Testing error handling..." << std::endl;
    
    // This test assumes Ollama is not running
    try {
        LlamaStack llama;
        std::string response = llama.completion("Test");
        
        // Should get a cURL error if Ollama is not running
        if (response.find("cURL error:") == 0) {
            std::cout << "✓ Error handling test passed (expected cURL error)" << std::endl;
        } else {
            std::cout << "✓ Error handling test passed (Ollama is running)" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "✓ Error handling test passed (exception caught)" << std::endl;
    }
}

int main() {
    std::cout << "Running LlamaTerminalApp Tests" << std::endl;
    std::cout << "==============================" << std::endl;
    
    testBasicFunctionality();
    testErrorHandling();
    
    std::cout << "\nAll tests completed." << std::endl;
    return 0;
}
```

These examples demonstrate various ways to use and extend the LlamaTerminalApp, from basic usage to advanced integration scenarios. Each example includes error handling and performance considerations appropriate for production use.