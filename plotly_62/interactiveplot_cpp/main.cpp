#include <iostream>
#include <string>
#include <zmq.hpp>
#include "KPI/b_data_storage/kpi_config_storage.h"
#include "KPI/a_persistence_layer/hdf_wrapper.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting KPI Server (C++ Implementation)" << std::endl;
    
    // Initialize KPI configurations
    kpi::data_storage::initialize_all_kpi_configs();
    
    // Initialize ZMQ context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    
    try {
        socket.bind("tcp://*:5555");
        std::cout << "KPI Server listening on port 5555" << std::endl;
        
        while (true) {
            zmq::message_t request;
            
            // Wait for next request from client
            auto result = socket.recv(request, zmq::recv_flags::none);
            if (!result) {
                continue;
            }
            
            std::string request_str(static_cast<char*>(request.data()), request.size());
            std::cout << "Received request: " << request_str << std::endl;
            
            // Parse request (simplified JSON parsing)
            // In production, use proper JSON library
            std::string response;
            
            if (request_str.find("sensor_id") != std::string::npos) {
                try {
                    // Extract parameters from request
                    std::string sensor_id = "sensor1"; // Parse from request
                    std::string input_file = "/path/to/input.h5"; // Parse from request
                    std::string output_file = "/path/to/output.h5"; // Parse from request
                    std::string output_dir = "/path/to/output"; // Parse from request
                    std::string base_name = "test_run"; // Parse from request
                    
                    // Process KPI request
                    std::string html_report = kpi::persistence::parse_for_kpi(
                        sensor_id, input_file, output_dir, base_name, "kpi", output_file
                    );
                    
                    response = "{\"status\": \"success\", \"html_report\": \"" + html_report + "\"}";
                } catch (const std::exception& e) {
                    response = "{\"status\": \"error\", \"message\": \"" + std::string(e.what()) + "\"}";
                }
            } else {
                response = "{\"status\": \"error\", \"message\": \"Invalid request format\"}";
            }
            
            // Send reply back to client
            zmq::message_t reply(response.size());
            memcpy(reply.data(), response.c_str(), response.size());
            socket.send(reply, zmq::send_flags::none);
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
