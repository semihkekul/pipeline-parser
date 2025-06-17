
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "logparser.h"
#include <unordered_set>


void LogParser::populateMessagesVector()
{
    for (auto& pipeline : m_pipelines) 
    {
        auto& messages_map = pipeline.second.messages_map;
        auto& previous_map = pipeline.second.previous_map;
        auto& next_map = pipeline.second.next_map;
        auto& messages_list = pipeline.second.messages_list;

        std::unordered_set<std::string> visited_ids; // To keep track of visited message IDs

        for (const auto& message_pair : messages_map) {
            const auto& msg = message_pair.second;

            if(visited_ids.find(msg->id) != visited_ids.end()) // stop if there is loop or already visited
            {
                continue; 
            }

            if (previous_map.find(msg->id) == previous_map.end()) 
            {
                std::string current_id = msg->id;

                while (visited_ids.find(current_id) == visited_ids.end()) // stop if there is loop
                {
                    if(messages_map.find(current_id) == messages_map.end())
                    {
                        break;
                    }

                    visited_ids.insert(current_id); // Mark the current message ID as visited
                    
                    messages_list.push_back(current_id);

                    auto next_id_iter = next_map.find(current_id);
                    if(next_id_iter == next_map.end())
                    {
                        break; // No next message found, exit the loop
                    }
                    current_id = next_id_iter->second;

                }

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

    if(message->next_id != LAST_MESSAGE_ID) // this is the last message so no next message
    {
        pipeline.previous_map[message->next_id] = message->id; // The previous id of the next message is current id
        pipeline.next_map[message->id] = message->next_id; // The next id of the current message is next_id
    }
    

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
        const auto& messages_list = pipeline.second.messages_list;
        const auto& messages_map = pipeline.second.messages_map;
        for (auto it = messages_list.rbegin(); it != messages_list.rend(); ++it) 
        {
         const std::string& id = *it;

          const auto& msg = messages_map.find(id)->second;
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
        const auto& messages_list = pipeline.second.messages_list;
        const auto& messages_map = pipeline.second.messages_map;

        // Reverse traverse the messages_list
        for (auto it = messages_list.rbegin(); it != messages_list.rend(); ++it) 
        {
            const std::string& id = *it;

            const auto& msg = messages_map.find(id)->second;

            *result += " " + msg->id + "| " + msg->body + "\n";
        }
    }
    return result;
}

