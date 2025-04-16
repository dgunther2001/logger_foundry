#include "input_socket.h"

namespace input_socket {
    void input_socket_obj::enqueue_buffer_parser(std::string msg) {
        enqueue_to_buffer_parser_callback(msg);
    }

    void input_socket_obj::init_socket() {
        //SOCKET_PATH = std::getenv("PYROXENE_LOG_SOCKET_PATH");
        unlink(SOCKET_PATH.c_str());
    
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("socket");
            return;
        }
    
        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH.c_str(), sizeof(addr.sun_path) - 1);
    
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind");
            return;
        }
    
        if (listen(sockfd, 20) == -1) {
            perror("listen");
            return;
        }
    
        std::cout << "Waiting for connection on Logger backend socket " << SOCKET_PATH << "...\n";

    }

    void input_socket_obj::read_socket() {

    }

    void input_socket_obj::close_socket() {
        if (sockfd != -1) {
            close(sockfd);
            sockfd = -1;
        }
        unlink(SOCKET_PATH.c_str()); 
    
        std::cout << "Socket cleaned up\n";
    }

    void input_socket_obj::run_thread() {
        
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

        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        if (writing_thread.joinable()) {
            writing_thread.join();
        }
    }

}