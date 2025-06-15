#pragma once
#include <string>
#include <unordered_map>
#include <fstream>


class LogParser {
public:
    struct LogMessage
    {
        std::string pipeline_id;
        std::string id;
        char encoding; 
        std::string body;
        std::string next_id;
    };

    LogParser(const std::string& filename);

    void printLogMessages() const;
private:
    struct Pipeline 
    {
        std::unordered_map<std::string, std::unique_ptr<LogMessage>> messages_map;
        std::unordered_map<std::string, std::string> previous_map;
        std::vector<std::unique_ptr<LogMessage>> messages_vector; 
    };

    std::unordered_map<std::string, Pipeline> m_pipelines;

    std::ifstream m_file;

     std::unique_ptr<LogMessage> parseLine(const std::string& line) const;

    std::string hexToAscii(const std::string& hex) const;

    void parseLog();
    void populateMessagesVector();

    void addLogMessageToPipeline(std::unique_ptr<LogMessage> message);
};