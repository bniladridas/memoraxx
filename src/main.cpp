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
