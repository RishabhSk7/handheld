#include <exception>
#include <cstdlib>
#include <iostream>
#include <string>

#include "launcher/app.hpp"

int main(int argc, char** argv) {
    try {
        std::string configPath = "config.json";

        if (const char* envConfig = std::getenv("HANDHELD_CONFIG"); envConfig && *envConfig) {
            configPath = envConfig;
        }

        if (argc > 1 && argv[1] && *argv[1]) {
            configPath = argv[1];
        }

        std::cout << "[INFO] Using config: " << configPath << '\n';
        launcher::App app(configPath);
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Fatal error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
