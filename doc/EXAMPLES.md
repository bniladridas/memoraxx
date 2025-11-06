# Examples

Usage examples for memoraxx.

## Basic Usage

```bash
$ ./build/memoraxx
Waking up....
Welcome to memoraxx!
Ask anything. Type 'exit', 'quit', or 'clear' to manage memory.

> What is the capital of Japan?
memoraxx is thinking...
--- AI Response ---
The capital of Japan is Tokyo.
-------------------
[memoraxx: brain active...]
[Sat Jul 26 02:45:00 2025, took 2.41715s, CPU usage: 123.456 ms]
```

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

## Code Integration

### Custom Prompts

```cpp
#include <iostream>
#include <string>

int main() {
    LlamaStack llama;
    std::string prompt = "You are a helpful assistant.\n\nUser: Hello\nAssistant:";
    std::string response = llama.completion(prompt);
    std::cout << response << std::endl;
    return 0;
}
```

### Batch Processing

```cpp
#include <vector>

int main() {
    LlamaStack llama;
    std::vector<std::string> prompts = {"Hello", "How are you?"};

    for (const auto& prompt : prompts) {
        std::string response = llama.completion(prompt);
        std::cout << "Q: " << prompt << "\nA: " << response << "\n---" << std::endl;
    }
    return 0;
}
```

## Error Handling

```cpp
std::string response = llama.completion("Hello");
if (response.find("Error:") == 0) {
    std::cerr << "Error: " << response << std::endl;
}
```

## Integration

### Web Service (Conceptual)

```cpp
// Requires additional HTTP library
class AIService {
    LlamaStack llama;
public:
    std::string chat(const std::string& prompt) {
        return llama.completion(prompt);
    }
};
```

## Testing

```cpp
void test() {
    LlamaStack llama;
    std::string response = llama.completion("Test");
    assert(!response.empty());
}
```
