#include "input_socket.h"

namespace input_socket {
    void input_socket_obj::enqueue_to_consumer(std::string msg) {
        enqueue_to_consumer_callback(msg);
    }

    void input_socket_obj::init_socket() {
        switch (socket_type) {
            case util::socket_type::UNIX: {
                socket_tracer->start_time = std::chrono::steady_clock::now();

                //SOCKET_PATH = std::getenv("PYROXENE_LOG_SOCKET_PATH");
                unlink(socket_path.c_str());
            
                sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (sockfd == -1) {
                    perror("socket");
                    return;
                }
            
                sockaddr_un addr;
                memset(&addr, 0, sizeof(addr));
                addr.sun_family = AF_UNIX;
                strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
            
                if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                    perror("bind");
                    return;
                }
            
                if (listen(sockfd, backlog) == -1) {
                    perror("listen");
                    return;
                }
            
                std::cout << "Waiting for connection on Logger backend socket " << socket_path << "...\n";
                break;
            }
            case util::socket_type::WEB: {
                socket_tracer->start_time = std::chrono::steady_clock::now();
                sockfd = socket(AF_INET6, SOCK_STREAM, 0);
                if (sockfd == -1) {
                    perror("socket(AF_INET6)");
                    return;
                }


                //int disable_only_v6 = 0;
                int on = 1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                               (char*)&on, sizeof(on)) < 0) {
                    perror("setsockopt(IPV6_V6ONLY)");
                }

                sockaddr_in6 server_addr{};
                server_addr.sin6_family = AF_INET6;
                server_addr.sin6_port = htons(port);

                if (host_path.empty()) {
                    server_addr.sin6_addr = in6addr_any;
                } else if (inet_pton(AF_INET6, host_path.c_str(), &server_addr.sin6_addr) != 1) {
                    perror("inet_pton(AF_INET6)");
                    return;
                }

                if (bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                    perror("bind(AF_INET6)");
                    return;
                }

                if (listen(sockfd, backlog) < 0) {
                    perror("listen(AF_INET6)");
                    return;
                }
                std::cout << "Waiting for connection on port " << port << "...\n";
                break;
            }
            default:
                std::cout << "Invalid socket type passed.\n";
        }

    }

    void input_socket_obj::read_socket() {

    }

    void input_socket_obj::close_socket() {
        if (sockfd != -1) {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            sockfd = -1;
        }
        switch (socket_type) {
            case util::socket_type::UNIX: {
                socket_tracer->end_time = std::chrono::steady_clock::now();
                unlink(socket_path.c_str()); 
                break;
            }
            case util::socket_type::WEB: {
                socket_tracer->end_time = std::chrono::steady_clock::now();
                break;
            }

        }

    }

    void input_socket_obj::run_thread() {
        switch (socket_type) {
            case util::socket_type::UNIX: {      
                while (is_thread_running) {
                    int client_fd = accept(sockfd, nullptr, nullptr);
                    
                    if (client_fd == -1) {
                        if (errno == EINTR) continue; 
                        //perror("accept");
                        break;
                    }
                    

                    char buffer[1024];
                    ssize_t bytes_read;

                    while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
                        std::string message(buffer, bytes_read); 
                        socket_tracer->bytes_received += bytes_read;
                        socket_tracer->connections_received += 1;
                        enqueue_to_consumer(std::move(message)); 
                    }

                    close(client_fd);
                    
                }
                break;
            }   
            case util::socket_type::WEB: {
                while (is_thread_running) {
                    sockaddr_storage client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    int client_fd = accept(sockfd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
                    if (client_fd == -1) {
                        if (errno == EINTR) 
                            continue;  
                        break;       
                    }
                    char buffer[1024];
                    ssize_t bytes_read;
                    while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
                        std::string message(buffer, bytes_read);
                        socket_tracer->bytes_received += bytes_read;
                        socket_tracer->connections_received += 1;
                        enqueue_to_consumer(std::move(message));
                    }
            

                    close(client_fd);
                }


                break;
            }
            default:
                std::cout << "Invalid socket type passed.\n";
        }
         
    }

    void input_socket_obj::init_thread() {
        if (!parser_thread_active_callback()) {
            // ADD ERROR HANDLING
        }
        is_thread_running = true;
        writing_thread = std::thread(&input_socket_obj::run_thread, this);
    }

    void input_socket_obj::stop_thread() {
        switch (socket_type) {
            case util::socket_type::UNIX: {
                break;
            }
            case util::socket_type::WEB: {
                break;
            }
            default:
                std::cout << "Invalid socket type passed.\n";
        }

        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            is_thread_running = false;
            thread_active_condition_var.notify_all();
        }

        if (writing_thread.joinable()) {
            writing_thread.join();
        }
    }





    input_socket_builder& input_socket_builder::set_log_path(std::string log_file_path) {
        this->log_file_path = std::move(log_file_path);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_backlog(uint16_t num_connections) {
        if (num_connections > 40) {
            this->backlog = 40;
            return *this;
        }
        this->backlog = num_connections;
        return *this;
    }

    input_socket_builder& input_socket_builder::set_socket_type(util::socket_type sock_type) {
        this->sock_type = sock_type;
        return *this;
    }


    input_socket_builder& input_socket_builder::set_socket_path(std::string socket_path) {
        this->socket_path = std::move(socket_path);
        return *this;
    }


    input_socket_builder& input_socket_builder::set_host_path(std::string host_path) {
        this->host_path = std::move(host_path);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_port(uint16_t port) {
        this->port = port;
        return *this;
    }

    input_socket_builder& input_socket_builder::set_enqueue_to_consumer_callback(std::function<void(std::string)> enqueue_to_consumer_callback) {
        this->enqueue_to_consumer_callback = std::move(enqueue_to_consumer_callback);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_log_diagnostic_callback(std::function<void(std::string)> log_diagnostic_callback) {
        this->log_diagnostic_callback = std::move(log_diagnostic_callback);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_parser_active_callback(std::function<bool()> parser_thread_active_callback) {
        this->parser_thread_active_callback = std::move(parser_thread_active_callback);
        return *this;
    }

    std::unique_ptr<input_socket_obj> input_socket_builder::build() {
        return std::make_unique<input_socket_obj>(enqueue_to_consumer_callback, log_diagnostic_callback, parser_thread_active_callback,
                                host_path, socket_path, port, backlog, sock_type);
    }

}

namespace input_socket::util {
    std::string construct_eot_socket_diagnostic_msg(std::vector<std::optional<input_socket::util::socket_tracer_health_snapshot>>& socket_tracers) 
    {
        std::ostringstream eot_diagnostic_msg;
        std::stringstream header_intermediate;
        eot_diagnostic_msg << "\n+------------------------------------------END-OF-TEST-DIAGNOSTICS------------------------------------------+\n";
        header_intermediate << std::left;
        header_intermediate << "| " << std::setw(10) << "Type";
        header_intermediate << "| " << std::setw(40) << "Socket Path/Port";
        header_intermediate << "| " << std::setw(16) << "Uptime";
        header_intermediate << "| " << std::setw(16) << "# Connections";
        header_intermediate << "| " << std::setw(16) << "Bytes Rec.";
        header_intermediate << "|\n";

        header_intermediate
            << "|" << std::setfill('-') << std::setw(11) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(41) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|\n";

        eot_diagnostic_msg << header_intermediate.str();

        for (auto const& tracer_struct : socket_tracers) {
            if (tracer_struct) {
                eot_diagnostic_msg << format_diagnostics(tracer_struct->socket_or_port, tracer_struct->connections_received, tracer_struct->bytes_received, tracer_struct->socket_uptime().count(), tracer_struct->socket_type);
            }
        }

        eot_diagnostic_msg << "+-----------------------------------------------------------------------------------------------------------+\n";
        return eot_diagnostic_msg.str();
    }

    std::string format_diagnostics(const std::string& socket_or_port, uint32_t connections_received, uint32_t bytes_received, double uptime, socket_type sock_type) {
        std::stringstream output_msg;
        output_msg << std::left;

        std::stringstream uptime_msg;
        switch (sock_type) {
            case socket_type::UNIX: {
                output_msg << "| " << std::setw(10) << "Unix ";
                output_msg << "| " << std::setw(40) << input_socket::util::format_socket_path(socket_or_port);
                break;
            }
            case socket_type::WEB: {
                output_msg << "| " << std::setw(10) << "IPv6 ";
                output_msg << "| " << std::setw(40) << socket_or_port;
                break;
            }
        }

        uptime_msg << std::fixed << std::setprecision(2) << uptime << "s";
        output_msg << "| " << std::setw(16) << uptime_msg.str();
        output_msg << "| " << std::setw(16) << connections_received;
        output_msg << "| " << std::setw(16) << bytes_received;

        output_msg << "|\n";
        return (std::move(output_msg.str()));
    }

    std::string construct_health_monitor_diagnostic(std::vector<input_socket::util::socket_tracer_delta>& snapshot_deltas, uint64_t health_interval) {
        std::stringstream output_msg;
        std::stringstream header_intermediate;


        output_msg << "\n+-----------------------------------------------------------------------------------------------------------------------------------+\n";
        output_msg << "|";
        output_msg << center_string(("Health Report @ " + std::format("{:%T}", time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())) + " (Interval = " + std::to_string(health_interval) + "s)"), 131, ' ');
        output_msg << "|\n";
        output_msg << "+-----------------------------------------------------------------------------------------------------------------------------------+\n";

        header_intermediate << std::left;
        header_intermediate << "| " << std::setw(10) << "Type";
        header_intermediate << "| " << std::setw(40) << "Socket Path/Port";
        header_intermediate << "| " << std::setw(16) << "Uptime";
        header_intermediate << "| " << std::setw(16) << "# Connections";
        header_intermediate << "| " << std::setw(11) << "Δ Conn";
        header_intermediate << "| " << std::setw(16) << "Bytes Rec.";
        header_intermediate << "| " << std::setw(11) << "Δ Bytes";
        header_intermediate << "|\n";

        header_intermediate
            << "|" << std::setfill('-') << std::setw(11) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(41) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(11) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(17) << "" << std::setfill(' ')
            << "|" << std::setfill('-') << std::setw(11) << "" << std::setfill(' ')
            << "|\n";

        output_msg << header_intermediate.str();

        for (auto const& snapshot_delta : snapshot_deltas) {
            output_msg << format_health_monitor_diagnostics(snapshot_delta);
        }

        output_msg << "+-----------------------------------------------------------------------------------------------------------------------------------+\n\n";
        return output_msg.str();
    }

    std::string format_health_monitor_diagnostics(const input_socket::util::socket_tracer_delta& snapshot_delta) {
        std::stringstream output_msg;
        output_msg << std::left;
        std::stringstream uptime_msg;

        switch (snapshot_delta.socket_type) {
            case socket_type::UNIX: {
                output_msg << "| " << std::setw(10) << "Unix ";
                output_msg << "| " << std::setw(40) << input_socket::util::format_socket_path(snapshot_delta.socket_or_port);
                break;
            }
            case socket_type::WEB: {
                output_msg << "| " << std::setw(10) << "IPv6 ";
                output_msg << "| " << std::setw(40) << snapshot_delta.socket_or_port;
                break;
            }
        }

        uptime_msg << std::fixed << std::setprecision(2) << snapshot_delta.uptime << "s";
        output_msg << "| " << std::setw(16) << uptime_msg.str();
        output_msg << "| " << std::setw(16) << snapshot_delta.connections_received;
        output_msg << "| " << std::setw(10) << snapshot_delta.connections_delta;
        output_msg << "| " << std::setw(16) << snapshot_delta.bytes_received;
        output_msg << "| " << std::setw(10) << snapshot_delta.bytes_delta;

        output_msg << "|\n";

        return output_msg.str();
    }



    std::string format_socket_path(const std::string& socket_path) {
        std::vector<std::string> path_components;
        std::stringstream socket_path_as_stream(socket_path);
        std::string path_component;
        while (std::getline(socket_path_as_stream, path_component, '/')) {
            if (!path_component.empty())
                path_components.push_back(path_component);
        }
        if (path_components.size() < 4) {
            return std::move(socket_path);
        }
        std::ostringstream output_string;

        output_string << path_components.at(0);
        
        output_string << "/.../";
        output_string << path_components.at(path_components.size() - 2) << "/";
        output_string << path_components.at(path_components.size() - 1);

        return output_string.str();
    }

    std::string center_string(const std::string& string, uint8_t width, char filler) {
        if (string.size() > width) return string;
        uint8_t padding = width - string.size();
        uint8_t left = padding / 2;
        uint8_t right = padding - left;
        return (std::string(left, filler) + string + std::string(right, filler));
    }
}