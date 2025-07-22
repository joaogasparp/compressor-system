#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

#include "core/algorithm.hpp"
#include "utils/crc.hpp"

class WebServer {
private:
    int server_fd;
    bool running;
    
public:
    WebServer() : server_fd(-1), running(false) {}
    
    ~WebServer() {
        stop();
    }
    
    bool start(int port = 8080) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            std::cerr << "Erro ao criar socket" << std::endl;
            return false;
        }
        
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            std::cerr << "Erro ao configurar socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Erro ao fazer bind na porta " << port << std::endl;
            return false;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Erro ao escutar" << std::endl;
            return false;
        }
        
        running = true;
        std::cout << "🚀 Servidor iniciado na porta " << port << std::endl;
        std::cout << "📱 Acesse: http://localhost:" << port << std::endl;
        
        return true;
    }
    
    void run() {
        while (running) {
            struct sockaddr_in address;
            int addrlen = sizeof(address);
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            
            if (new_socket < 0) {
                if (running) {
                    std::cerr << "Erro ao aceitar conexão" << std::endl;
                }
                continue;
            }
            
            std::thread(&WebServer::handleRequest, this, new_socket).detach();
        }
    }
    
    void stop() {
        running = false;
        if (server_fd != -1) {
            close(server_fd);
            server_fd = -1;
        }
    }
    
private:
    void handleRequest(int socket) {
        char buffer[8192] = {0};
        read(socket, buffer, 8192);
        
        std::string request(buffer);
        std::string response;
        
        // Parse HTTP request
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        std::cout << "📨 " << method << " " << path << std::endl;
        
        if (method == "GET") {
            if (path == "/" || path.find(".html") != std::string::npos ||
                path.find(".js") != std::string::npos || path.find(".css") != std::string::npos) {
                response = serveStaticFile(path);
            } else {
                response = createCORSResponse("404 Not Found", "text/plain", "Not Found");
            }
        } else if (method == "POST" && path == "/api/compress") {
            response = handleCompression(request);
        } else if (method == "OPTIONS") {
            response = createCORSResponse("200 OK", "text/plain", "");
        } else {
            response = createCORSResponse("405 Method Not Allowed", "text/plain", "Method Not Allowed");
        }
        
        send(socket, response.c_str(), response.length(), 0);
        close(socket);
    }
    
    std::string serveStaticFile(std::string path) {
        if (path == "/") path = "/index.html";
        
        // Serve React build files
        std::string fullPath = "web-app/build" + path;
        std::ifstream file(fullPath, std::ios::binary);
        
        if (!file.good()) {
            return createCORSResponse("404 Not Found", "text/html", 
                "<html><body><h1>404 - Arquivo não encontrado</h1><p>Build da aplicação React não encontrado. Execute: cd web-app && npm run build</p></body></html>");
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        std::string contentType = "text/html";
        if (path.find(".js") != std::string::npos) contentType = "application/javascript";
        else if (path.find(".css") != std::string::npos) contentType = "text/css";
        else if (path.find(".json") != std::string::npos) contentType = "application/json";
        
        return createCORSResponse("200 OK", contentType, content);
    }
    
    std::string handleCompression(const std::string& request) {
        try {
            // Parse multipart form data (simplified)
            size_t boundaryPos = request.find("boundary=");
            if (boundaryPos == std::string::npos) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Boundary não encontrado\"}");
            }
            
            std::string boundary = request.substr(boundaryPos + 9);
            boundary = boundary.substr(0, boundary.find("\\r\\n"));
            
            // Extract file data and algorithm
            std::string algorithm = extractFormField(request, "algorithm");
            std::vector<uint8_t> fileData = extractFileData(request, boundary);
            
            if (fileData.empty()) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Arquivo não encontrado\"}");
            }
            
            // Compress using selected algorithm
            auto compressor = compressor::AlgorithmFactory::create(algorithm);
            if (!compressor) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Algoritmo inválido\"}");
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = compressor->compress(fileData);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            if (!result.is_success()) {
                return createCORSResponse("500 Internal Server Error", "application/json", 
                    "{\"error\":\"Erro na compressão: " + result.message() + "\"}");
            }
            
            // Verify compression by decompressing
            auto decompressResult = compressor->decompress(result.data());
            bool verified = decompressResult.is_success() && decompressResult.data() == fileData;
            
            // Create response with compressed data
            std::string response = "HTTP/1.1 200 OK\\r\\n";
            response += "Access-Control-Allow-Origin: *\\r\\n";
            response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\\r\\n";
            response += "Access-Control-Allow-Headers: Content-Type\\r\\n";
            response += "Content-Type: application/octet-stream\\r\\n";
            response += "Content-Disposition: attachment; filename=\\\"compressed.dat\\\"\\r\\n";
            response += "X-Original-Size: " + std::to_string(fileData.size()) + "\\r\\n";
            response += "X-Compressed-Size: " + std::to_string(result.data().size()) + "\\r\\n";
            response += "X-Compression-Ratio: " + std::to_string((double)result.data().size() / fileData.size() * 100) + "\\r\\n";
            response += "X-Compression-Time: " + std::to_string(duration.count()) + "\\r\\n";
            response += "X-Verified: " + std::string(verified ? "true" : "false") + "\\r\\n";
            response += "X-Algorithm: " + algorithm + "\\r\\n";
            response += "Content-Length: " + std::to_string(result.data().size()) + "\\r\\n\\r\\n";
            
            response.append(reinterpret_cast<const char*>(result.data().data()), result.data().size());
            
            std::cout << "✅ Compressão concluída: " << fileData.size() << " → " << result.data().size() 
                     << " bytes (" << std::fixed << std::setprecision(1) 
                     << ((double)result.data().size() / fileData.size() * 100) << "%)" << std::endl;
            
            return response;
            
        } catch (const std::exception& e) {
            return createCORSResponse("500 Internal Server Error", "application/json", 
                "{\"error\":\"Erro interno: " + std::string(e.what()) + "\"}");
        }
    }
    
    std::string createCORSResponse(const std::string& status, const std::string& contentType, const std::string& body) {
        std::string response = "HTTP/1.1 " + status + "\\r\\n";
        response += "Access-Control-Allow-Origin: *\\r\\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\\r\\n";
        response += "Access-Control-Allow-Headers: Content-Type\\r\\n";
        response += "Content-Type: " + contentType + "\\r\\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\\r\\n\\r\\n";
        response += body;
        return response;
    }
    
    std::string extractFormField(const std::string& request, const std::string& fieldName) {
        std::string pattern = "name=\\\"" + fieldName + "\\\"\\r\\n\\r\\n";
        size_t pos = request.find(pattern);
        if (pos == std::string::npos) return "";
        
        pos += pattern.length();
        size_t endPos = request.find("\\r\\n", pos);
        if (endPos == std::string::npos) return "";
        
        return request.substr(pos, endPos - pos);
    }
    
    std::vector<uint8_t> extractFileData(const std::string& request, const std::string& boundary) {
        std::string pattern = "Content-Type: application/octet-stream\\r\\n\\r\\n";
        size_t pos = request.find(pattern);
        if (pos == std::string::npos) {
            // Try other content types
            pattern = "Content-Type: "; 
            pos = request.find(pattern);
            if (pos != std::string::npos) {
                pos = request.find("\\r\\n\\r\\n", pos);
                if (pos != std::string::npos) pos += 4;
            }
        } else {
            pos += pattern.length();
        }
        
        if (pos == std::string::npos) return {};
        
        std::string endPattern = "\\r\\n--" + boundary;
        size_t endPos = request.find(endPattern, pos);
        if (endPos == std::string::npos) return {};
        
        std::string fileContent = request.substr(pos, endPos - pos);
        return std::vector<uint8_t>(fileContent.begin(), fileContent.end());
    }
};

// Global server instance
std::unique_ptr<WebServer> server;

void signalHandler(int sig) {
    std::cout << "\\n🛑 Parando servidor..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "🌐 Iniciando Compressor Web Server..." << std::endl;
    
    server = std::make_unique<WebServer>();
    
    if (!server->start(8080)) {
        std::cerr << "❌ Falha ao iniciar servidor" << std::endl;
        return 1;
    }
    
    std::cout << "📋 Algoritmos disponíveis:" << std::endl;
    for (const auto& algo : compressor::AlgorithmFactory::list_algorithms()) {
        std::cout << "   • " << algo << std::endl;
    }
    std::cout << std::endl;
    
    server->run();
    
    return 0;
}
