# Llama C++ Terminal Client

<div style="text-align: center; font-size: 2.5rem;">
  💚
</div>
<div style="text-align: center;">
  <img src="https://img.shields.io/badge/Llama-AI-brightgreen" alt="Llama Badge">
  <img src="https://img.shields.io/badge/C++-Programming-blue" alt="C++ Badge">
  <img src="https://img.shields.io/badge/Memory-Context-orange" alt="Memory Badge">
  <img src="https://img.shields.io/github/issues-pr/bniladridas/cpp_terminal_app?label=Pull%20Requests&color=yellow" alt="Pull Requests">
  <img src="https://img.shields.io/badge/PRs-Welcome-brightgreen" alt="PRs Welcome">
</div>

> **IMPORTANT**: Log in to the [New Google Cloud Communities](https://security.googlecloudcommunity.com/members/niladridas-404568) to keep your profile.

C++ client for local Llama 3.2 with memory capabilities for context-aware responses and performance metrics.

```cpp
LlamaStack llama("http://localhost:11434/api/generate", "llama3.2", 5, "memory.json");
std::string response = llama.completion("Hello");
```

## Features
- **Context-Aware Responses**: Stores up to 5 recent prompt-response pairs (configurable) to maintain conversation context.
- **Persistent Memory**: Saves interactions to `memory.json` for cross-session continuity.
- **User Commands**: Type `clear` to reset memory, or `exit`/`quit` to close the app.
- **Performance Metrics**: Displays CPU usage and response time.
- **Robust Error Handling**: Manages cURL and JSON errors gracefully.

## Documentation

[Setup](doc/README.md) | [Architecture](doc/ARCHITECTURE.md) | [API](doc/API.md) | [Examples](doc/EXAMPLES.md)

## Quick Start

### Prerequisites
- C++17
- CMake
- `libcurl` (`sudo apt-get install libcurl4-openssl-dev` or `brew install curl`)
- `nlohmann/json` (`sudo apt-get install nlohmann-json3-dev` or download from [GitHub](https://github.com/nlohmann/json))
- Ollama with Llama 3.2 running locally (`ollama serve`)

### Build
```bash
cmake -S . -B build && cmake --build build
ollama serve && ./build/LlamaTerminalApp
```

### Output
```
Waking up....
Welcome to Memoraxx!
Ask anything. Type 'exit', 'quit', or 'clear' to manage memory.

> What is AI?
Memoraxx is thinking...
--- AI Response ---
Artificial Intelligence (AI) is the simulation of human intelligence in machines...
-------------------
[Memoraxx: brain active...]
[Sat Jul 26 00:51:23 2025, took 1.234s]

> What are its applications?
Memoraxx is thinking...
--- AI Response ---
Building on our previous discussion about AI, its applications include...
-------------------
[Memoraxx: brain active...]
[Sat Jul 26 00:51:25 2025, took 1.456s]

> clear
Memory cleared.
```

## Memory Capabilities
The client maintains a conversation history using a `std::deque` to store up to `max_memory_size` (default: 5) prompt-response pairs. These are included in API requests to provide context, enabling coherent and context-aware responses. Interactions can be persisted to `memory.json` for continuity across sessions. Use the `clear` command to reset memory.

**Example Memory File (`memory.json`):**
```json
[
  {
    "prompt": "What is AI?",
    "response": "Artificial Intelligence (AI) is the simulation of human intelligence..."
  },
  {
    "prompt": "What are its applications?",
    "response": "Building on our previous discussion about AI, its applications include..."
  }
]
```

<details>
<summary>Key Code Highlights</summary>

**Memory Structure and Context Building:**
```cpp
struct Interaction {
    std::string prompt;
    std::string response;
};

std::string build_context(const std::string& current_prompt) {
    std::string context = "You are a highly knowledgeable and friendly AI assistant. "
                         "Use the following conversation history for context:\n\n";
    for (const auto& interaction : memory) {
        context += "User: " + interaction.prompt + "\nAssistant: " + interaction.response + "\n\n";
    }
    context += "User: " + current_prompt + "\nAssistant:";
    return context;
}
```

**JSON Request with Context:**
```cpp
std::string full_prompt = build_context(prompt);
json json_payload = {
    {"model", model_name},
    {"prompt", full_prompt},
    {"stream", false}
};
```

**Memory Persistence:**
```cpp
void save_memory() {
    if (memory_file.empty()) return;
    json memory_json = json::array();
    for (const auto& interaction : memory) {
        memory_json.push_back({
            {"prompt", interaction.prompt},
            {"response", interaction.response}
        });
    }
    std::ofstream ofs(memory_file);
    ofs << memory_json.dump(2);
}
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
auto start_time = std::chrono::high_resolution_clock::now();
std::string response = llama.completion(prompt);
auto end_time = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> duration = end_time - start_time;
```

</details>

## Notes
- **Performance**: Tune `max_memory_size` based on API token limits to avoid truncation.
- **Security**: `memory.json` is plain text; consider encryption for sensitive data.
- **Extensibility**: Add features like memory summarization or keyword-based context filtering.

**Author:** [Niladri Das](https://security.googlecloudcommunity.com/members/niladridas-404568)  
**License:** Apache 2.0