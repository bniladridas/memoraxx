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
#include <sys/resource.h> // For CPU usage

using json = nlohmann::json;

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
};

// Compute Levenshtein distance for fuzzy matching
int levenshtein_distance(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));
    for (size_t i = 0; i <= len1; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= len2; ++j) dp[0][j] = j;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }
    return dp[len1][len2];
}

class LlamaStack {
private:
    std::string base_url;
    std::string model_name;
    CURL* curl;
    std::deque<Interaction> memory; // Memory to store recent interactions
    size_t max_memory_size; // Maximum number of interactions to store
    std::string memory_file; // File for persistent memory (optional)

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
        std::string context = "You are a highly knowledgeable and friendly AI assistant. "
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
                    {"response", interaction.response}
                });
            }
            std::ofstream ofs(memory_file);
            ofs << memory_json.dump(2);
            ofs.close();
        } catch (const std::exception& e) {
            std::cerr << "Failed to save memory: " << e.what() << std::endl;
        }
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
            for (const auto& item : memory_json) {
                if (item.contains("prompt") && item.contains("response")) {
                    memory.push_back({item["prompt"].get<std::string>(), item["response"].get<std::string>()});
                    if (memory.size() >= max_memory_size) {
                        memory.pop_front();
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
               size_t memory_size = 5,
               const std::string& mem_file = "")
        : base_url(url), model_name(model), max_memory_size(memory_size), memory_file(mem_file) {
        curl = init_curl();
        if (!memory_file.empty()) {
            load_memory();
        }
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

            // Perform the request
            CURLcode res = curl_easy_perform(curl_handle);
            if (res != CURLE_OK) {
                throw std::runtime_error("cURL error: " + std::string(curl_easy_strerror(res)));
            }

            // Check HTTP status code
            long http_code = 0;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code != 200) {
                throw std::runtime_error("HTTP error: " + std::to_string(http_code));
            }

            // Parse JSON response
            json response_json = json::parse(response_buffer);
            if (!response_json.contains("response")) {
                throw std::runtime_error("No 'response' field in API output");
            }

            std::string result = response_json["response"].get<std::string>();

            // Store interaction in memory
            memory.push_back({prompt, result});
            if (memory.size() > max_memory_size) {
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
    std::signal(SIGINT, signal_handler);

    try {
        // Initialize with memory file for persistence
        LlamaStack llama("http://localhost:11434/api/generate", "llama3.2", 5, "memory.json");

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
        std::cout << "\n\033[1;32mWelcome to Memoraxx!\033[0m\n";
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

            // Fuzzy matching for commands
            if (levenshtein_distance(input_lower, "exit") <= 2 || input_lower == "exit") {
                std::cout << "[Memoraxx: shutting down";
                for (int i = 0; i < 3; ++i) {
                    std::cout << "." << std::flush;
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                }
                std::cout << "]\nExiting. Goodbye!\n";
                break;
            }
            if (levenshtein_distance(input_lower, "quit") <= 2 || input_lower == "quit") {
                std::cout << "[Memoraxx: shutting down";
                for (int i = 0; i < 3; ++i) {
                    std::cout << "." << std::flush;
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                }
                std::cout << "]\nExiting. Goodbye!\n";
                break;
            }
            if (levenshtein_distance(input_lower, "clear") <= 2 || input_lower == "clear") {
                llama.clear_memory();
                continue;
            }

            // Suggest commands for unrecognized input
            if (levenshtein_distance(input_lower, "exit") <= 3 || levenshtein_distance(input_lower, "quit") <= 3 || levenshtein_distance(input_lower, "clear") <= 3) {
                std::cout << "Did you mean 'exit', 'quit', or 'clear'? Try again.\n";
                continue;
            }

            // Get response with context
            auto start_time = std::chrono::high_resolution_clock::now();
            std::cout << "\rMemoraxx is thinking" << std::flush;
            std::atomic<bool> done{false};
            std::thread loader([&done]() {
                int count = 0;
                while (!done) {
                    std::cout << "\rMemoraxx is thinking" << std::string(count % 4, '.') << std::flush;
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    count++;
                }
            });

            std::string response = llama.completion(user_message);
            done = true;
            loader.join();
            std::cout << "\r" << std::string(20, ' ') << "\r"; // Clear line

            // Output results
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end_time - start_time;
            std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string current_time = std::ctime(&now);
            current_time = current_time.substr(0, current_time.length() - 1);

            // CPU usage
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            double cpu_usage = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0 +
                               (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000.0;

            std::cout << "\n--- AI Response ---\n" << response << "\n-------------------\n";
            std::cout << "[Memoraxx: brain active";
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