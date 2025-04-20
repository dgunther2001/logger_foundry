#include <logger_foundry/logger_foundry.h>
#include <chrono>

int main() {
    logger_foundry::logger_daemon orchestrator = logger_foundry::logger_daemon_builder()
        .set_log_path("logs/log1.log")
        .add_unix_socket("tmp/sock1.sock", 10)
        .add_unix_socket("tmp/sock2.sock", 15)
        .set_kill_strategy([] {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        })
        .build();

    return 0;
}