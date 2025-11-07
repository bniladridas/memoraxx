#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <csignal>
#include <stdexcept>
#include <deque>
#include <fstream>
#include <algorithm> // For std::min, std::transform
#include <regex> // For std::regex, std::sregex_iterator
#ifndef _WIN32
#include <sys/resource.h> // For CPU usage
#endif
#include <functional> // For std::function
#include <vector> // For std::vector
#include <climits> // For INT_MAX

#ifdef _WIN32
#include <windows.h>
#endif

using json = nlohmann::json;

// Cross-platform CPU time function
double get_cpu_time() {
#ifdef _WIN32
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        ULARGE_INTEGER k, u;
        k.LowPart = kernel.dwLowDateTime;
        k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime;
        u.HighPart = user.dwHighDateTime;
        return (k.QuadPart + u.QuadPart) / 10000000.0; // seconds
    }
    return 0.0;
#else
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_stime.tv_sec + (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000000.0;
#endif
}

// Global flag for graceful shutdown
std::atomic<bool> g_shutdown{false};

// Signal handler for Ctrl+C
void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_shutdown = true;
    }
}

// Callback function to collect cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}

// Structure to hold prompt-response pairs
struct Interaction {
    std::string prompt;
    std::string response;
    int token_count;
};

// Structure for tools
struct Tool {
    std::string name;
    std::string description;
    json parameters;
};

// Improved token counter (word-based approximation).
// Estimates tokens as word count * 1.3 to account for subword tokenization.
// This is still a rough estimate and may not accurately reflect the tokenizer used by the LLM.
// This discrepancy could lead to either under-utilizing the context window or, more critically,
// exceeding the model's maximum token limit, which might cause API requests to fail.
// For production use, consider integrating a proper tokenizer library.
int count_tokens(const std::string& text) {
    std::istringstream iss(text);
    int word_count = std::distance(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{});
    return static_cast<int>(word_count * 1.3);
}

// Compute Levenshtein distance for fuzzy matching (space-optimized)
int levenshtein_distance(const std::string& s1, const std::string& s2) {
    size_t len1 = s1.size(), len2 = s2.size();

    // Ensure s1 is the shorter string for space optimization
    if (len1 > len2) {
        return levenshtein_distance(s2, s1);
    }

    std::vector<int> prev_row(len1 + 1);
    for (size_t i = 0; i <= len1; ++i) {
        prev_row[i] = i;
    }

    for (size_t j = 1; j <= len2; ++j) {
        int prev_val = prev_row[0];
        prev_row[0] = j;

        for (size_t i = 1; i <= len1; ++i) {
            int temp = prev_row[i];
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            prev_row[i] = std::min({prev_row[i] + 1, prev_row[i - 1] + 1, prev_val + cost});
            prev_val = temp;
        }
    }

    return prev_row[len1];
}

class LlamaStack {
private:
    std::string base_url;
    std::string model_name;
    CURL* curl;
    std::deque<Interaction> memory; // Memory to store recent interactions
    size_t max_tokens; // Maximum tokens to store
    size_t total_tokens; // Current total tokens
    std::string memory_file; // File for persistent memory (optional)
    std::vector<Tool> tools; // Available tools for agent

    // Initialize a new CURL handle for thread safety
    CURL* init_curl() {
        CURL* handle = curl_easy_init();
        if (!handle) {
            throw std::runtime_error("Failed to initialize cURL");
        }
        curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30L); // 30 seconds max for entire request
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 5L); // 5 seconds max to connect
        return handle;
    }

    // Build context from memory
    std::string build_context(const std::string& current_prompt) {
        // Build tools JSON
        json tools_json = json::array();
        for (const auto& tool : tools) {
            tools_json.push_back({
                {"name", tool.name},
                {"description", tool.description},
                {"parameters", tool.parameters}
            });
        }
        std::string tools_str = "You have access to the following tools:\n" + tools_json.dump(2) + "\n\nTo use a tool, respond with a JSON object like: {\"tool_call\": {\"name\": \"tool_name\", \"arguments\": {...}}}\n\n";

        std::string context = tools_str + "You are a highly knowledgeable and friendly AI assistant. Use tools when appropriate.\n\n"
                              "Use the following conversation history for context:\n\n";
        for (const auto& interaction : memory) {
            context += "User: " + interaction.prompt + "\nAssistant: " + interaction.response + "\n\n";
        }
        context += "User: " + current_prompt + "\nAssistant:";
        return context;
    }

    // Save memory to file
    void save_memory() {
        if (memory_file.empty()) return;
        try {
            json memory_json = json::array();
            for (const auto& interaction : memory) {
                memory_json.push_back({
                    {"prompt", interaction.prompt},
                    {"response", interaction.response},
                    {"token_count", interaction.token_count}
                });
            }
            std::ofstream ofs(memory_file);
            ofs << memory_json.dump(2);
            ofs.close();
        } catch (const std::exception& e) {
            std::cerr << "Failed to save memory: " << e.what() << std::endl;
        }
    }

    // Execute a tool
    std::string execute_tool(const std::string& name, const json& args) {
        if (name == "run_command") {
            if (!args.contains("command")) return "Error: Missing command argument";
            std::string cmd = args["command"].get<std::string>();
            FILE* pipe = popen(cmd.c_str(), "r");
            if (!pipe) return "Error: Failed to run command";
            char buffer[128];
            std::string result;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            pclose(pipe);
            return "Command output:\n" + result;
        }
        return "Unknown tool: " + name;
    }

    // Load memory from file
    void load_memory() {
        if (memory_file.empty()) return;
        try {
            std::ifstream ifs(memory_file);
            if (!ifs.is_open()) return;
            json memory_json;
            ifs >> memory_json;
            ifs.close();
            memory.clear();
            total_tokens = 0;
            for (const auto& item : memory_json) {
                if (item.contains("prompt") && item.contains("response")) {
                    // Store prompt and response to avoid repeated JSON lookups
                    const auto prompt = item["prompt"].get<std::string>();
                    const auto response = item["response"].get<std::string>();
                    int tokens = item.contains("token_count") ? item["token_count"].get<int>() : count_tokens(prompt + " " + response);
                    if (total_tokens + tokens <= max_tokens) {
                        memory.push_back({prompt, response, tokens});
                        total_tokens += tokens;
                    } else {
                        break; // Stop loading if would exceed
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to load memory: " << e.what() << std::endl;
        }
    }

public:
    LlamaStack(const std::string& url = "http://localhost:11434/api/generate",
                const std::string& model = "llama3.2",
                size_t max_tokens = 4096,
                const std::string& mem_file = "")
        : base_url(url), model_name(model), max_tokens(max_tokens), total_tokens(0), memory_file(mem_file) {
        curl = init_curl();
        if (!memory_file.empty()) {
            load_memory();
        }
        // Initialize tools
        Tool run_cmd = {
            "run_command",
            "Run a shell command and return the output",
            json{
                {"type", "object"},
                {"properties", {
                    {"command", json{{"type", "string"}, {"description", "The shell command to run"}}}
                }},
                {"required", json::array({"command"})}
            }
        };
        tools.push_back(run_cmd);
    }

    ~LlamaStack() {
        save_memory();
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }

    // Clear memory
    void clear_memory() {
        memory.clear();
        save_memory();
        std::cout << "Memory cleared.\n";
    }

    std::string completion(const std::string& prompt) {
        if (prompt.empty()) {
            return "Error: Empty prompt provided";
        }

        CURL* curl_handle = init_curl();
        std::string response_buffer;
        struct curl_slist* headers = nullptr;

        try {
            // Build prompt with context
            std::string full_prompt = build_context(prompt);

            // Construct JSON payload
            json json_payload = {
                {"model", model_name},
                {"prompt", full_prompt},
                {"stream", false}
            };
            std::string payload = json_payload.dump();

            // Set cURL options
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl_handle, CURLOPT_URL, base_url.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_buffer);

            // Perform the request with retry
            const int MAX_RETRIES = 3;
            int retries = MAX_RETRIES;
            bool success = false;
            for (int attempt = 0; attempt < retries; ++attempt) {
                if (attempt > 0) {
                    int delay = 1 << (attempt - 1); // 1, 2, 4 seconds
                    std::this_thread::sleep_for(std::chrono::seconds(delay));
                }

                CURLcode res = curl_easy_perform(curl_handle);
                if (res != CURLE_OK) {
                    if (attempt == retries - 1) {
                        throw std::runtime_error("cURL error: " + std::string(curl_easy_strerror(res)));
                    }
                    continue;
                }

                // Check HTTP status code
                long http_code = 0;
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code == 200) {
                    success = true;
                    break;
                } else if (http_code >= 500 && attempt < retries - 1) {
                    continue;
                } else {
                    throw std::runtime_error("HTTP error: " + std::to_string(http_code));
                }
            }

            // Parse JSON response
            json response_json = json::parse(response_buffer);
            if (!response_json.contains("response")) {
                throw std::runtime_error("No 'response' field in API output");
            }

            std::string result = response_json["response"].get<std::string>();

            // Check for tool call
            try {
                json response_parsed = json::parse(result);
                if (response_parsed.contains("tool_call")) {
                    std::string tool_name = response_parsed["tool_call"]["name"];
                    json tool_args = response_parsed["tool_call"]["arguments"];
                    std::string tool_output = execute_tool(tool_name, tool_args);
                    result = tool_output;
                }
            } catch (const json::exception&) {
                // Not a tool call, use as is
            }

            // Store interaction in memory
            int tokens = count_tokens(prompt + " " + result);
            memory.push_back({prompt, result, tokens});
            total_tokens += tokens;
            while (total_tokens > max_tokens && !memory.empty()) {
                total_tokens -= memory.front().token_count;
                memory.pop_front();
            }
            save_memory();

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl_handle);
            return result;
        } catch (const json::exception& e) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl_handle);
            return "JSON parse error: " + std::string(e.what());
        } catch (const std::exception& e) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl_handle);
            return "Error: " + std::string(e.what());
        }
    }
};

int main() {
    // Load config or use defaults
    std::string base_url = "http://localhost:11434/api/generate";
    std::string model = "llama3.2";
    size_t max_tokens = 4096;
    std::string memory_file = "memory.json";

    try {
        std::ifstream ifs("config.json");
        if (ifs.is_open()) {
            json config;
            ifs >> config;
            if (config.contains("base_url")) base_url = config["base_url"].get<std::string>();
            if (config.contains("model")) model = config["model"].get<std::string>();
            if (config.contains("max_tokens")) max_tokens = config["max_tokens"].get<size_t>();
            if (config.contains("memory_file")) memory_file = config["memory_file"].get<std::string>();
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Warning: Failed to parse config.json: " << e.what() << ". Using default settings." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to load config.json: " << e.what() << ". Using default settings." << std::endl;
    }

    std::signal(SIGINT, signal_handler);

    try {
        // Initialize with memory file for persistence
        LlamaStack llama(base_url, model, max_tokens, memory_file);

        // Startup animation
        std::cout << "Waking up";
        for (int i = 0; i < 4 && !g_shutdown; ++i) {
            std::cout << "." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (g_shutdown) {
            std::cout << "\nShutdown requested. Exiting.\n";
            return 0;
        }
        std::cout << "\n\033[1;32mWelcome to memoraxx!\033[0m\n";
        std::cout << "Ask anything. Type 'exit', 'quit', or 'clear' to manage memory.\n";

        std::string user_message;
        while (!g_shutdown) {
            std::cout << "\n> ";
            std::getline(std::cin, user_message);
            if (user_message.empty()) {
                std::cout << "Please enter a non-empty prompt.\n";
                continue;
            }

            // Convert input to lowercase for command matching
            std::string input_lower = user_message;
            std::transform(input_lower.begin(), input_lower.end(), input_lower.begin(), ::tolower);

            // Define commands and their actions
            struct Command {
                std::string name;
                std::function<void()> action;
            };

            std::vector<Command> commands = {
                {"exit", [&]() {
                    std::cout << "[memoraxx: shutting down";
                    for (int i = 0; i < 3; ++i) {
                        std::cout << "." << std::flush;
                        std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    }
                    std::cout << "]\nExiting. Goodbye!\n";
                    g_shutdown = true;
                }},
                {"quit", [&]() {
                    std::cout << "[memoraxx: shutting down";
                    for (int i = 0; i < 3; ++i) {
                        std::cout << "." << std::flush;
                        std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    }
                    std::cout << "]\nExiting. Goodbye!\n";
                    g_shutdown = true;
                }},
                {"clear", [&]() {
                    llama.clear_memory();
                }}
            };

            // Find the best matching command
            int min_distance = INT_MAX;
            int best_command_index = -1;

            for (size_t i = 0; i < commands.size(); ++i) {
                int distance = levenshtein_distance(input_lower, commands[i].name);
                if (distance < min_distance) {
                    min_distance = distance;
                    best_command_index = i;
                }
            }

            // Handle command based on minimum distance
            if (min_distance <= 2) {
                // Execute command if close enough
                commands[best_command_index].action();
                if (g_shutdown) break;
                continue;
            } else if (min_distance <= 3) {
                // Suggest command if somewhat close
                std::cout << "Did you mean '" << commands[best_command_index].name << "'? Try again.\n";
                continue;
            }

            // Get response with context
            auto start_time = std::chrono::high_resolution_clock::now();

            // Get CPU usage before operation
            double cpu_before = get_cpu_time();

            std::cout << "\rmemoraxx is thinking" << std::flush;
            std::atomic<bool> done{false};
            std::thread loader([&done]() {
                int count = 0;
                while (!done) {
                    std::cout << "\rmemoraxx is thinking" << std::string(count % 4, '.') << std::flush;
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    count++;
                }
            });

            std::string response = llama.completion(user_message);
            done = true;
            loader.join();
            std::cout << "\r" << std::string(20, ' ') << "\r"; // Clear line

            // Get CPU usage after operation
            double cpu_after = get_cpu_time();

            // Output results
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end_time - start_time;
            std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string current_time = std::ctime(&now);
            current_time = current_time.substr(0, current_time.length() - 1);

            // Calculate CPU usage difference for this operation
            double cpu_usage = (cpu_after - cpu_before) * 1000.0;

            std::cout << "\n--- AI Response ---\n" << response << "\n-------------------\n";
            std::cout << "[memoraxx: brain active";
            for (int i = 0; i < 3; ++i) {
                std::cout << "." << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            std::cout << "]\n[" << current_time << ", took " << duration.count() << "s, CPU usage: " << cpu_usage << " ms]\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
