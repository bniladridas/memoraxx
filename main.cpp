#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <curl/curl.h>
#include <sys/resource.h>
#include <sys/types.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include <nlohmann/json.hpp> // Requires JSON library (e.g., nlohmann/json)

using json = nlohmann::json;

// Callback function to collect cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}

class LlamaStack {
private:
    std::string base_url;
    CURL* curl;
    bool use_gpu;

public:
    LlamaStack(bool use_gpu = false) : use_gpu(use_gpu) {
        base_url = "http://localhost:11434/api/generate";
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize cURL");
        }
    }

    ~LlamaStack() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }

    std::string completion(const std::string& prompt) {
        if (!curl) {
            return "Error: cURL not initialized";
        }

        // Escape special characters for JSON
        json json_payload = {
            {"model", "llama3.2"},
            {"prompt", prompt},
            {"stream", false}
        };
        std::string payload = json_payload.dump();

        // Set up cURL options
        std::string response_buffer;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, base_url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            return "cURL error: " + std::string(curl_easy_strerror(res));
        }

        // Parse the response to extract the actual content
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
    }
};

int main() {
    try {
        LlamaStack llama(true);
        std::string user_message;

        std::cout << "Enter your message: ";
        std::getline(std::cin, user_message);

        std::string prompt = "You are a highly knowledgeable and friendly AI assistant. Please provide clear, concise, and engaging answers.\n\nUser: " + user_message + "\nAssistant:";

        // Start timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Get response from Llama API
        std::string response = llama.completion(prompt);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        // Get current time
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string current_time = std::ctime(&now);

        // Get resource usage
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        double cpu_usage = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0 +
                           (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000.0;

        // Placeholder for GPU usage (platform-specific, e.g., NVIDIA CUDA API or AMD ROCm)
        int gpu_usage = 0; // Implement GPU usage detection if needed

        // Hypothetical power consumption (arbitrary formula for demo)
        double power_consumption = cpu_usage * 0.5 + gpu_usage * 0.1;

        // Output results
        std::cout << "Response:" << std::endl;
        std::cout << "- Date and Time: " << current_time;
        std::cout << "- Reason for Response: The AI responded to the user's query." << std::endl;
        std::cout << "- Resource Consumption: CPU usage: " << cpu_usage << " ms, GPU usage: " << gpu_usage << "%" << std::endl;
        std::cout << "- Hypothetical Power Consumption: " << power_consumption << " units" << std::endl;
        std::cout << "- Duration: " << duration.count() << " seconds" << std::endl;
        std::cout << "- Response: " << response << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}