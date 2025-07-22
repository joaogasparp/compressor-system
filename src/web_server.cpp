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

// Base64 encoding function
std::string base64Encode(const std::vector<uint8_t>& data) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;
    for (auto c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

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
            std::cerr << "Error creating socket" << std::endl;
            return false;
        }
        
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            std::cerr << "Error configuring socket" << std::endl;
            return false;
        }
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Error binding to port " << port << std::endl;
            return false;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Error listening" << std::endl;
            return false;
        }
        
        running = true;
        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Access: http://localhost:" << port << std::endl;
        
        return true;
    }
    
    std::string handleAlgorithmsList() {
        std::string jsonResponse = R"({"algorithms": ["lz77", "huffman", "rle"]})";
        return createCORSResponse("200 OK", "application/json", jsonResponse);
    }
    
    void run() {
        while (running) {
            struct sockaddr_in address;
            int addrlen = sizeof(address);
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            
            if (new_socket < 0) {
                if (running) {
                    std::cerr << "Error accepting connection" << std::endl;
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
        std::string request;
        char buffer[8192];
        ssize_t totalBytesRead = 0;
        int contentLength = 0;
        
        // First, read headers to get Content-Length
        while (true) {
            ssize_t bytesRead = read(socket, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) break;
            
            buffer[bytesRead] = '\0';
            request.append(buffer, bytesRead);
            totalBytesRead += bytesRead;
            
            // Check if we have complete headers
            size_t headerEnd = request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // Extract Content-Length
                size_t clPos = request.find("Content-Length: ");
                if (clPos != std::string::npos) {
                    clPos += 16; // Length of "Content-Length: "
                    size_t clEnd = request.find("\r\n", clPos);
                    if (clEnd != std::string::npos) {
                        contentLength = std::stoi(request.substr(clPos, clEnd - clPos));
                    }
                }
                
                // Calculate how much body we have
                int headerSize = headerEnd + 4;
                int bodyReceived = totalBytesRead - headerSize;
                
                // Read remaining body if needed
                while (bodyReceived < contentLength) {
                    bytesRead = read(socket, buffer, std::min(sizeof(buffer) - 1, (size_t)(contentLength - bodyReceived)));
                    if (bytesRead <= 0) break;
                    
                    buffer[bytesRead] = '\0';
                    request.append(buffer, bytesRead);
                    bodyReceived += bytesRead;
                }
                break;
            }
            
            // Safety check - don't read too much
            if (totalBytesRead > 20 * 1024 * 1024) break; // 20MB limit
        }
        std::string response;
        
        // Parse HTTP request
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        std::cout << method << " " << path << std::endl;
        
        if (method == "GET") {
            if (path == "/algorithms") {
                response = handleAlgorithmsList();
            } else if (path == "/" || path.find(".html") != std::string::npos ||
                path.find(".js") != std::string::npos || path.find(".css") != std::string::npos) {
                response = serveStaticFile(path);
            } else {
                response = createCORSResponse("404 Not Found", "text/plain", "Not Found");
            }
        } else if (method == "POST" && path == "/compress") {
            response = handleCompression(request);
        } else if (method == "POST" && path == "/decompress") {
            response = handleDecompression(request);
        } else if (method == "OPTIONS") {
            // Handle CORS preflight request
            response = createCORSResponse("200 OK", "text/plain", "OK");
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
                "<html><body><h1>404 - File not found</h1><p>React build not found. Run: cd web-app && npm run build</p></body></html>");
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
            std::cout << "Processing compression request..." << std::endl;
            
            // Debug: Print request headers
            size_t headerEnd = request.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                std::string headers = request.substr(0, headerEnd);
                std::cout << "Request headers:\n" << headers << std::endl;
            }
            
            // Parse multipart form data (simplified)
            size_t boundaryPos = request.find("boundary=");
            if (boundaryPos == std::string::npos) {
                std::cout << "Boundary not found in request" << std::endl;
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Boundary not found\"}");
            }
            
            std::string boundary = request.substr(boundaryPos + 9);
            // Remove quotes if present and find end of boundary
            if (boundary[0] == '"') {
                boundary = boundary.substr(1);
                size_t quotePos = boundary.find('"');
                if (quotePos != std::string::npos) {
                    boundary = boundary.substr(0, quotePos);
                }
            } else {
                size_t endPos = boundary.find_first_of("\r\n ");
                if (endPos != std::string::npos) {
                    boundary = boundary.substr(0, endPos);
                }
            }
            
            std::cout << "Using boundary: [" << boundary << "]" << std::endl;
            
            // Debug: Show all Content-Disposition headers
            std::cout << "All Content-Disposition headers found:" << std::endl;
            size_t searchPos = 0;
            while ((searchPos = request.find("Content-Disposition: form-data", searchPos)) != std::string::npos) {
                size_t lineEnd = request.find("\r\n", searchPos);
                if (lineEnd != std::string::npos) {
                    std::string line = request.substr(searchPos, lineEnd - searchPos);
                    std::cout << "  - " << line << std::endl;
                }
                searchPos = lineEnd + 1;
            }
            
            // Extract file data and algorithm
            std::string algorithm = extractFormField(request, "algorithm");
            std::vector<uint8_t> fileData = extractFileData(request, boundary);
            
            std::cout << "Algorithm extracted: [" << algorithm << "]" << std::endl;
            std::cout << "File data size: " << fileData.size() << " bytes" << std::endl;
            
            if (algorithm.empty()) {
                std::cout << "Algorithm field is empty" << std::endl;
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Algorithm field not found or empty\"}");
            }
            
            if (fileData.empty()) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"File not found\"}");
            }
            
            // Compress using selected algorithm
            auto compressor = compressor::AlgorithmFactory::create(algorithm);
            if (!compressor) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Invalid algorithm\"}");
            }
            
            auto start = std::chrono::high_resolution_clock::now();
            auto result = compressor->compress(fileData);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            if (!result.is_success()) {
                return createCORSResponse("500 Internal Server Error", "application/json", 
                    "{\"error\":\"Compression error: " + result.message() + "\"}");
            }
            
            // Verify compression by decompressing
            auto decompressResult = compressor->decompress(result.data());
            bool verified = decompressResult.is_success() && decompressResult.data() == fileData;
            
            // Encode compressed data in base64
            std::string base64Data = base64Encode(result.data());
            
            // Create JSON response
            std::string jsonResponse = "{";
            jsonResponse += "\"success\": true,";
            jsonResponse += "\"original_size\": " + std::to_string(fileData.size()) + ",";
            jsonResponse += "\"compressed_size\": " + std::to_string(result.data().size()) + ",";
            jsonResponse += "\"compression_ratio\": " + std::to_string((double)result.data().size() / fileData.size()) + ",";
            jsonResponse += "\"compression_time_ms\": " + std::to_string(duration.count()) + ",";
            jsonResponse += "\"algorithm\": \"" + algorithm + "\",";
            jsonResponse += "\"verified\": " + std::string(verified ? "true" : "false") + ",";
            jsonResponse += "\"compressed_data\": \"" + base64Data + "\"";
            jsonResponse += "}";
            
            std::cout << "Compression completed: " << fileData.size() << " -> " << result.data().size() 
                     << " bytes (" << std::fixed << std::setprecision(1) 
                     << ((double)result.data().size() / fileData.size() * 100) << "%)" << std::endl;
            
            return createCORSResponse("200 OK", "application/json", jsonResponse);
            
        } catch (const std::exception& e) {
            return createCORSResponse("500 Internal Server Error", "application/json", 
                "{\"error\":\"Internal error: " + std::string(e.what()) + "\"}");
        }
    }
    
    std::string handleDecompression(const std::string& request) {
        try {
            std::cout << "Processing decompression request..." << std::endl;
            
            // Parse multipart form data (simplified)
            size_t boundaryPos = request.find("boundary=");
            if (boundaryPos == std::string::npos) {
                std::cout << "Boundary not found in request" << std::endl;
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Boundary not found\"}");
            }
            
            std::string boundary = request.substr(boundaryPos + 9);
            // Remove quotes if present and find end of boundary
            if (boundary[0] == '"') {
                boundary = boundary.substr(1);
                size_t quotePos = boundary.find('"');
                if (quotePos != std::string::npos) {
                    boundary = boundary.substr(0, quotePos);
                }
            } else {
                size_t endPos = boundary.find_first_of("\r\n ");
                if (endPos != std::string::npos) {
                    boundary = boundary.substr(0, endPos);
                }
            }
            
            // Extract file data and algorithm
            std::string algorithm = extractFormField(request, "algorithm");
            std::vector<uint8_t> fileData = extractFileData(request, boundary);
            
            if (algorithm.empty() || fileData.empty()) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Missing algorithm or file data\"}");
            }
            
            std::cout << "Decompressing " << fileData.size() << " bytes using " << algorithm << std::endl;
            
            // Convert to ByteVector
            compressor::ByteVector compressedData(fileData.begin(), fileData.end());
            
            // Decompress using selected algorithm
            auto decompressor = compressor::AlgorithmFactory::create(algorithm);
            if (!decompressor) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Invalid algorithm: " + algorithm + "\"}");
            }
            
            auto result = decompressor->decompress(compressedData);
            
            if (!result.is_success()) {
                return createCORSResponse("400 Bad Request", "application/json", 
                    "{\"error\":\"Decompression error: " + result.message() + "\"}");
            }
            
            // Encode decompressed data in base64
            std::string encodedData = base64Encode(result.data());
            
            std::string jsonResponse = "{\"success\": true,";
            jsonResponse += "\"algorithm\": \"" + algorithm + "\",";
            jsonResponse += "\"decompressed_data\": \"" + encodedData + "\",";
            jsonResponse += "\"compressed_size\": " + std::to_string(compressedData.size()) + ",";
            jsonResponse += "\"decompressed_size\": " + std::to_string(result.data().size()) + ",";
            jsonResponse += "\"compression_ratio\": " + std::to_string((double)compressedData.size() / result.data().size()) + ",";
            jsonResponse += "\"decompression_time_ms\": " + std::to_string(result.stats().decompression_time_ms);
            jsonResponse += "}";
            
            std::cout << "Decompression completed: " << compressedData.size() << " -> " << result.data().size() 
                     << " bytes" << std::endl;
            
            return createCORSResponse("200 OK", "application/json", jsonResponse);
            
        } catch (const std::exception& e) {
            return createCORSResponse("500 Internal Server Error", "application/json", 
                "{\"error\":\"Internal error: " + std::string(e.what()) + "\"}");
        }
    }
    
    std::string createCORSResponse(const std::string& status, const std::string& contentType, const std::string& body) {
        std::string response = "HTTP/1.1 " + status + "\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
        response += "Access-Control-Max-Age: 86400\r\n";
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n\r\n";
        response += body;
        return response;
    }
    
    std::string extractFormField(const std::string& request, const std::string& fieldName) {
        std::cout << "Extracting form field: " << fieldName << std::endl;
        
        // Look for Content-Disposition: form-data; name="fieldName"
        std::string pattern = "Content-Disposition: form-data; name=\"" + fieldName + "\"";
        size_t pos = request.find(pattern);
        if (pos == std::string::npos) {
            std::cout << "Field not found: " << fieldName << std::endl;
            // Debug: show what patterns we do find
            size_t debugPos = request.find("Content-Disposition: form-data");
            if (debugPos != std::string::npos) {
                size_t lineEnd = request.find("\r\n", debugPos);
                if (lineEnd != std::string::npos) {
                    std::string foundPattern = request.substr(debugPos, lineEnd - debugPos);
                    std::cout << "Found pattern: " << foundPattern << std::endl;
                }
            }
            return "";
        }
        
        // Find the value after \r\n\r\n
        pos = request.find("\r\n\r\n", pos);
        if (pos == std::string::npos) {
            std::cout << "Field value start not found" << std::endl;
            return "";
        }
        pos += 4;
        
        size_t endPos = request.find("\r\n", pos);
        if (endPos == std::string::npos) {
            std::cout << "Field value end not found" << std::endl;
            return "";
        }
        
        std::string value = request.substr(pos, endPos - pos);
        std::cout << "Field " << fieldName << " = [" << value << "]" << std::endl;
        
        return value;
    }
    
    std::vector<uint8_t> extractFileData(const std::string& request, const std::string& boundary) {
        std::cout << "Extracting file data with boundary: [" << boundary << "]" << std::endl;
        
        // Look for file content after Content-Disposition header with name="file"
        std::string pattern = "Content-Disposition: form-data; name=\"file\"";
        size_t pos = request.find(pattern);
        if (pos == std::string::npos) {
            std::cout << "File form field not found" << std::endl;
            return {};
        }
        
        // Find the actual content after headers (after \r\n\r\n)
        pos = request.find("\r\n\r\n", pos);
        if (pos == std::string::npos) {
            std::cout << "Content start not found" << std::endl;
            return {};
        }
        pos += 4; // Skip \r\n\r\n
        
        // Find end of content (boundary with --)
        std::string endPattern = "\r\n--" + boundary;
        size_t endPos = request.find(endPattern, pos);
        if (endPos == std::string::npos) {
            // Try without \r\n prefix
            endPattern = "--" + boundary;
            endPos = request.find(endPattern, pos);
            if (endPos == std::string::npos) {
                std::cout << "Content end boundary not found" << std::endl;
                std::cout << "Looking for: [" << endPattern << "]" << std::endl;
                // Debug: show end of request
                if (request.size() > 100) {
                    std::cout << "End of request: [" << request.substr(request.size() - 100) << "]" << std::endl;
                }
                return {};
            }
        }
        
        std::string fileContent = request.substr(pos, endPos - pos);
        std::cout << "Extracted file data: " << fileContent.size() << " bytes" << std::endl;
        
        return std::vector<uint8_t>(fileContent.begin(), fileContent.end());
    }
};

// Global server instance
std::unique_ptr<WebServer> server;

void signalHandler(int sig) {
    std::cout << "\nStopping server..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "Starting Compressor Web Server..." << std::endl;
    
    server = std::make_unique<WebServer>();
    
    if (!server->start(8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Available algorithms:" << std::endl;
    for (const auto& algo : compressor::AlgorithmFactory::list_algorithms()) {
        std::cout << "   • " << algo << std::endl;
    }
    std::cout << std::endl;
    
    server->run();
    
    return 0;
}
