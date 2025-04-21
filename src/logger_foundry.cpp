#include "logger_foundry/logger_foundry.h"
#include "daemon_orchestrator.h"

#include <chrono>

namespace logger_foundry {
    logger_daemon::logger_daemon(const std::string& log_file_path, std::vector<socket_config::unix_socket_config> unix_socket_configs, std::vector<socket_config::web_socket_config> web_socket_configs, parser_strategy parsing_strategy, kill_logger_strategy kill_strategy) :
                                daemon_orchestrator_obj(std::make_unique<daemon_orchestrator::daemon_orch_obj>(log_file_path, std::move(unix_socket_configs), std::move(web_socket_configs), parsing_strategy)),
                                kill_strategy{ std::move(kill_strategy) }
                                { 
                                    daemon_orchestrator_obj->start_threads(); 
                                    
                                    kill_strategy_monitor = std::thread([this] {
                                        if (this->kill_strategy) {
                                            this->kill_strategy();
                                        } else {
                                            //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                                            //this->daemon_orchestrator_obj->wait_until_queues_empty();
                                        }

                                        this->daemon_orchestrator_obj->kill_threads();
                                    });
                                    
                            
                                        
                                }
    
    void logger_daemon::log_direct(std::string msg) {
        this->daemon_orchestrator_obj->log_direct(msg);
    }

    logger_daemon::~logger_daemon() {     
        //daemon_orchestrator_obj->kill_threads();             
                     
        if (kill_strategy_monitor.joinable()) {
            kill_strategy_monitor.join();
        } 
            
    }





    logger_daemon_builder& logger_daemon_builder::set_log_path(std::string log_file_path) {
        this->log_file_path = std::move(log_file_path);
        return *this;
    }

    logger_daemon_builder& logger_daemon_builder::add_unix_socket(std::string socket_path, uint16_t backlog) {
        this->unix_socket_configs.emplace_back(socket_config::unix_socket_config{std::move(socket_path), backlog});
        return *this;
    }

    logger_daemon_builder& logger_daemon_builder::add_web_socket(uint16_t port, uint16_t backlog, std::string host) {
        this->web_socket_configs.emplace_back(socket_config::web_socket_config{port, backlog, std::move(host)});
        return *this;
    }

    logger_daemon_builder& logger_daemon_builder::set_parser_strategy(parser_strategy parser_strategy_inst) {
        this->parser_strategy_inst = std::move(parser_strategy_inst);
        return *this;
    }

    logger_daemon_builder& logger_daemon_builder::set_kill_strategy(kill_logger_strategy kill_logger_strategy_inst) {
        this->kill_logger_strategy_inst = std::move(kill_logger_strategy_inst);
        return *this;
    }

    logger_daemon logger_daemon_builder::build() {
        return logger_daemon(log_file_path, std::move(unix_socket_configs), std::move(web_socket_configs), parser_strategy_inst, kill_logger_strategy_inst);
    }
}


