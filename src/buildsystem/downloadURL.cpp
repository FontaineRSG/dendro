//
// Created by cv2 on 28.02.2025.
//

#include <curl/curl.h>
#include "Buildsystem.h"

static size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

bool BuildSystem::downloadUrl(const std::string& url, const std::string& outputFile) {
    std::cout << "downloading source." << std::endl;
    std::cout << url << std::endl;
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    FILE* fp = fopen(outputFile.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: DendroBuildSystem");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    std::cout << res << std::endl;

    curl_slist_free_all(headers);
    fclose(fp);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}
