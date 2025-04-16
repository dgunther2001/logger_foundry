#include "logger_foundry/logger_foundry.h"
#include "daemon_orchestrator.h"

#include <chrono>

namespace logger_foundry {
    logger_daemon::logger_daemon(const char* log_file_path, const char* socket_path, parser_strategy parsing_strategy, kill_logger_strategy kill_strategy) :
                                daemon_orchestrator(std::make_unique<daemon_orchestrator::daemon_orch_obj>(log_file_path, socket_path, parsing_strategy)),
                                kill_strategy{ std::move(kill_strategy) }
                                { 
                                    daemon_orchestrator->start_threads(); 
                                    
                                    kill_strategy_monitor = std::thread([this] {
                                        if (this->kill_strategy) {
                                            this->kill_strategy();
                                        } else {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                                            //this->daemon_orchestrator->wait_until_queues_empty();
                                        }

                                        this->daemon_orchestrator->kill_threads();
                                    });
                                    
                            
                                        
                                }

    logger_daemon::logger_daemon(const char* log_file_path, const char* socket_path, kill_logger_strategy kill_strategy) :
        logger_daemon(log_file_path, socket_path, nullptr, std::move(kill_strategy))
        {}

    logger_daemon::~logger_daemon() {     
        //daemon_orchestrator->kill_threads();             
                     
        if (kill_strategy_monitor.joinable()) {
            kill_strategy_monitor.join();
        } 
            
    }
}


