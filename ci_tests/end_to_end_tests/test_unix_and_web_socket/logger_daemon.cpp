#include <logger_foundry/logger_foundry.h>
#include <chrono>

int main() {
    logger_foundry::logger_daemon orchestrator = logger_foundry::logger_daemon_builder()
        .set_log_path("logs/log1.log")
        .add_unix_socket("tmp/sock1.sock", 10)
        .add_unix_socket("tmp/sock2.sock", 15)
        .add_web_socket(50051, 20)
        .add_web_socket(50052, 20)
        .build();

    return 0;
}