#include "input_socket.h"

namespace input_socket {
    void input_socket_obj::enqueue_buffer_parser(std::string msg) {
        enqueue_to_buffer_parser_callback(msg);
    }

    void input_socket_obj::init_socket() {
        switch (socket_type) {
            case socket_type::UNIX: {
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
            case socket_type::WEB: {

                sockfd = socket(AF_INET6, SOCK_STREAM, 0);
                if (sockfd == -1) {
                    perror("socket(AF_INET6)");
                    return;
                }

                int disable_only_v6 = 0;
                if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY,
                               &disable_only_v6, sizeof(disable_only_v6)) == -1) {
                    perror("setsockopt(IPV6_V6ONLY)");
                }

                sockaddr_in6 addr6{};
                addr6.sin6_family = AF_INET6;
                addr6.sin6_port   = htons(port);

                if (host_path.empty()) {
                    addr6.sin6_addr = in6addr_any;
                } else if (inet_pton(AF_INET6, host_path.c_str(), &addr6.sin6_addr) != 1) {
                    perror("inet_pton(AF_INET6)");
                    return;
                }

                if (bind(sockfd, (sockaddr*)&addr6, sizeof(addr6)) == -1) {
                    perror("bind(AF_INET6)");
                    return;
                }

                if (listen(sockfd, backlog) == -1) {
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
        if (socket_type ==socket_type::UNIX) {
            unlink(socket_path.c_str()); 
        }
    }

    void input_socket_obj::run_thread() {
        switch (socket_type) {
            case socket_type::UNIX: {      
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
                        enqueue_buffer_parser(std::move(message)); 
                    }

                    close(client_fd);
                    
                }
                break;
            }   
            case socket_type::WEB: {
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
                        enqueue_buffer_parser(std::move(message));
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
        {
            std::lock_guard<std::mutex> lock(thread_active_mutex);
            is_thread_running = false;
            thread_active_condition_var.notify_all();
        }

        switch (socket_type) {
            case socket_type::UNIX: {
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);
                break;
            }
            case socket_type::WEB: {
                break;
            }
            default:
                std::cout << "Invalid socket type passed.\n";
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

    input_socket_builder& input_socket_builder::set_socket_type(socket_type sock_type) {
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

    input_socket_builder& input_socket_builder::set_enqueue_buffer_parser_callback(std::function<void(std::string)> enqueue_to_buffer_parser_callback) {
        this->enqueue_to_buffer_parser_callback = std::move(enqueue_to_buffer_parser_callback);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_direct_log_callback(std::function<void(std::string)> log_self_callback) {
        this->log_self_callback = std::move(log_self_callback);
        return *this;
    }

    input_socket_builder& input_socket_builder::set_parser_active_callback(std::function<bool()> parser_thread_active_callback) {
        this->parser_thread_active_callback = std::move(parser_thread_active_callback);
        return *this;
    }

    std::unique_ptr<input_socket_obj> input_socket_builder::build() {
        return std::make_unique<input_socket_obj>(enqueue_to_buffer_parser_callback, log_self_callback, parser_thread_active_callback,
                                host_path, socket_path, port, backlog, sock_type);
    }

}