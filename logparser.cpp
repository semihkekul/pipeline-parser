
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "logparser.h"


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
    if(m_pipelines.find(message->pipeline_id) == m_pipelines.end())
    {
        std::cerr << "Error: Pipeline ID " << message->pipeline_id << " not found." << std::endl;
        return;
    }

    auto& pipeline = m_pipelines[message->pipeline_id];

    pipeline.previous_map[message->next_id] = message->id; // The previous id of the next message is current id

    pipeline.messages_map[message->id] = std::move(message); // Add the message to the map

}

void LogParser::parseLog(std::istream& stream)
{
    std::string line;
    while (std::getline(stream, line)) 
    {
        auto log_message = parseLine(line);
        if (!log_message)
        {
            std::cerr << "Skipping malformed line: " << line << std::endl;
            continue; // Skip malformed lines
        }
        
        auto pipeline_iter = m_pipelines.find(log_message->pipeline_id);
        if(pipeline_iter == m_pipelines.end()) // add a new pipeline
        {
            Pipeline new_pipeline;
            m_pipelines[log_message->pipeline_id] = std::move(new_pipeline);
        } 

        addLogMessageToPipeline(std::move(log_message));
    }

    populateMessagesVector(); // Populate the messages vector after parsing all lines
}

void LogParser::parseLog(std::ifstream& file) 
{
    parseLog(static_cast<std::istream&>(file));
}

void LogParser::parseLog(const std::string& log)
{
    std::istringstream stream(log);
    parseLog(stream);
}


std::unique_ptr<LogMessage> LogParser::parseLine(const std::string& line) const
{

    if (line.empty()) 
    {
       return nullptr; // Return nullptr for empty lines
    }

    std::istringstream stream(line);
    auto message = std::make_unique<LogMessage>();

    if(!(stream >> message->pipeline_id >> message->id >> message->encoding))
    {
       std::cerr << "Malformed line: Missing required fields" << std::endl;
       return nullptr; 
    }

    if (message->encoding != ASCII_ENCODING && message->encoding != HEX_ENCODING) 
    {
        std::cerr << "Malformed line: Invalid encoding type" << std::endl;
        return nullptr; 
    }
    char opening_bracket=' ';
    if(!(stream >> opening_bracket))// Read the opening bracket '['
    {
       std::cerr << "Malformed line: Missing opening bracket"<< std::endl;
       return nullptr;
    }

    if (!std::getline(stream, message->body, ']')) // Read until the closing bracket
    {
        std::cerr << "Malformed line: Missing closing bracket"<< std::endl;
        return nullptr;
    }

    if(!(stream >> message->next_id))
    {
        std::cerr << "Malformed line: Missing next_id field"<< std::endl;
        return nullptr;
    }

    if (message->encoding == HEX_ENCODING) 
    {
        message->body = hexToAscii(message->body);
        if(message->body == "")
        {
            std::cerr << "Malformed line: Invalid hex encoding in body" << std::endl;
            return nullptr; // Return nullptr if hex conversion fails
        }
        
    }
    return message;
}

std::string LogParser::hexToAscii(const std::string& hex) const
{
    std::string ascii;
    for (size_t i = 0; i < hex.length(); i += 2) 
    {
        std::string byte = hex.substr(i, 2); // Extract two characters for each byte

        char ch = ' ';
        try
        {
            ch = std::stoi(byte, nullptr, 16); // Convert hex to decimal
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid hex byte: " << byte << std::endl;
            return "";
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Hex byte out of range: " << byte << std::endl;
            return "";
        }
        ascii.push_back(ch); // Append the character to the ASCII string
    }
    return ascii;
}

void LogParser::printLogMessages() const
{
     if (m_pipelines.empty()) 
     {
        throw std::runtime_error("No pipelines found. Please parse the log first.");
     }
    for (const auto& pipeline : m_pipelines) 
    {
        std::cout << "Pipeline " << pipeline.first << std::endl;
        const auto& messages = pipeline.second.messages_vector;
        for (const auto& msg : messages) 
        {
          std::cout << " " << msg->id <<"| "<< msg->body << "\n";
        }
        
    }
}

std::unique_ptr<std::string> LogParser::getLogMessagesPrintable() const
{
    if (m_pipelines.empty()) 
    {
        throw std::runtime_error("No pipelines found. Please parse the log first.");
    }

    auto result = std::make_unique<std::string>();
    for (const auto& pipeline : m_pipelines) 
    {
        *result += "Pipeline " + pipeline.first + "\n";
        const auto& messages = pipeline.second.messages_vector;
        for (const auto& msg : messages) 
        {
            *result += " " + msg->id + "| " + msg->body + "\n";
        }
    }
    return result;
}

