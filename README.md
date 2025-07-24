<div style="text-align: center; font-size: 2.5rem;">
  💚
</div>
<div style="text-align: center;">
  <img src="https://img.shields.io/badge/Llama-AI-brightgreen" alt="Llama Badge">
  <img src="https://img.shields.io/badge/C++-Programming-blue" alt="C++ Badge">
  <img src="https://img.shields.io/badge/GPU-Inference-orange" alt="GPU Badge">
  <img src="https://img.shields.io/github/issues-pr/bniladridas/cpp_terminal_app?label=Pull%20Requests&color=yellow" alt="Pull Requests">
  <img src="https://img.shields.io/badge/PRs-Welcome-brightgreen" alt="PRs Welcome">
</div>

# IMPORTANT: Log In to the New Google Cloud Communities to Keep Your Profile

# Llama C++ Terminal Client

C++ client for local Llama 3.2. Shows performance metrics.

```cpp
LlamaStack llama;
std::string response = llama.completion("Hello");
```

## Documentation

[Setup](doc/README.md) | [Architecture](doc/ARCHITECTURE.md) | [API](doc/API.md) | [Examples](doc/EXAMPLES.md)

## Quick Start

### Prerequisites
C++11, CMake, libcurl, nlohmann/json, Ollama with llama3.2

### Build
```bash
cmake -S . -B build && cmake --build build
ollama serve && ./build/LlamaTerminalApp
```

### Output
```
Enter your message: Hello
Response: Hello! How can I assist you today?
CPU: 23ms | Duration: 6s
```

<details>
<summary>Key Code Highlights</summary>

**JSON Request Construction:**
```cpp
json json_payload = {
    {"model", "llama3.2"},
    {"prompt", user_input},
    {"stream", false}
};
```

**Error Handling Pattern:**
```cpp
std::string response = llama.completion(prompt);
if (response.find("Error:") == 0) {
    std::cerr << response << std::endl;
}
```

**Performance Metrics:**
```cpp
struct rusage usage;
getrusage(RUSAGE_SELF, &usage);
double cpu_ms = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0;
```

</details>

**Author:** [Niladri Das](https://security.googlecloudcommunity.com/members/niladridas-404568)  
**License:** Apache 2.0
