
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "logparser.h"

namespace
{
constexpr char ASCII_ENCODING = '0';
constexpr char HEX_ENCODING = '1';
}

LogParser::LogParser(const std::string& filename)
:m_file(filename) 
{
    if (!m_file.is_open()) 
    {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
    else
    {
        parseLog();
        populateMessagesVector();
    }
}

void LogParser::populateMessagesVector()
{
    for (auto& pipeline : m_pipelines) 
    {
        auto& messages_map = pipeline.second.messages_map;
        auto& previous_map = pipeline.second.previous_map;
        auto& messages_vector = pipeline.second.messages_vector;

        const size_t len = messages_map.size();
        messages_vector.resize(len); // Resize the vector to hold all messages

        for (const auto& message_pair : messages_map) 
        {
            auto message_id = message_pair.first;
            
            if (previous_map.find(message_id) == previous_map.end()) // found the first message (the one without previous)
            {
                for(size_t i = 0; i < len; ++i) 
                {
                    const auto next_id = messages_map[message_id]->next_id;
                    auto iter_to_message = messages_map.find(message_id);
                    messages_vector[len - i - 1] = std::move(iter_to_message->second);
                    messages_map.erase(iter_to_message); // Remove the message from the map after moving it to the vector
                    message_id = next_id;
                }
                break;
            }
        }
    }
}
          

void LogParser::addLogMessageToPipeline(std::unique_ptr<LogMessage> message) 
{
    auto& pipeline = m_pipelines[message->pipeline_id];

    pipeline.previous_map[message->next_id] = message->id; // The previous id of the next message is current id

    pipeline.messages_map[message->id] = std::move(message); // Add the message to the map

}

void LogParser::parseLog() 
{
    std::string line;
    while (std::getline(m_file, line)) 
    {
        auto log_message = parseLine(line); 
        
        auto pipeline_iter = m_pipelines.find(log_message->pipeline_id);
        if(pipeline_iter == m_pipelines.end()) 
        {
            Pipeline new_pipeline;
            m_pipelines[log_message->pipeline_id] = std::move(new_pipeline);
        } 

        addLogMessageToPipeline(std::move(log_message));
    }
}

std::unique_ptr<LogParser::LogMessage> LogParser::parseLine(const std::string& line) const
{
    std::istringstream stream(line);
    auto message = std::make_unique<LogMessage>();

    stream >> message->pipeline_id >> message->id >> message->encoding;


    char opening_bracket;
    stream >> opening_bracket; // Read the opening bracket '['
    std::getline(stream, message->body, ']'); // Read until the closing bracket
    
    stream >> message->next_id;

    if (message->encoding == HEX_ENCODING) 
    {
        message->body = hexToAscii(message->body);
    }
    return message;
}

std::string LogParser::hexToAscii(const std::string& hex) const
{

    std::string ascii;
    for (size_t i = 0; i < hex.length(); i += 2) 
    {
        std::string byte = hex.substr(i, 2); // Extract two characters for each byte
        const char ch = std::stoi(byte, nullptr, 16); // Convert hex to decimal
        ascii.push_back(ch); // Append the character to the ASCII string
    }
    return ascii;
}

void LogParser::printLogMessages() const
{
 
    for (const auto& pipeline : m_pipelines) 
    {
        std::cout << "Pipeline: " << pipeline.first << std::endl;
        const auto& messages = pipeline.second.messages_vector;
        for (const auto& msg : messages) 
        {
          std::cout << "  " << msg->id <<"| "<< msg->body << "\n";
        }
        
    }
}

