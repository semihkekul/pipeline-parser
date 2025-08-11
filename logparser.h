#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <fstream>
#include <memory>
#include <list>

constexpr char ASCII_ENCODING = '0';
constexpr char HEX_ENCODING = '1';
const std::string LAST_MESSAGE_ID("-1");
struct LogMessage
{
    std::string pipeline_id;
    std::string id;
    char encoding; 
    std::string body;
    std::string next_id;
};

struct Pipeline 
{
    std::unordered_map<std::string, std::unique_ptr<LogMessage>> messages_map;
    std::unordered_map<std::string, std::string> previous_map;
    std::unordered_map<std::string, std::string> next_map;
    std::list<std::string> messages_list;
};

class LogParser {
public:

    void printLogMessages() const;
    std::unique_ptr<std::string> getLogMessagesPrintable() const;
    void parseLog(std::ifstream& file);
    void parseLog(const std::string& log);

    std::unique_ptr<LogMessage> parseLine(const std::string& line) const;


private:

    std::unordered_map<std::string, Pipeline> m_pipelines;

    std::ifstream m_file;

    std::string hexToAscii(const std::string& hex) const;

    void parseLog(std::istream& stream);
    
    void populateMessagesVector();

    void addLogMessageToPipeline(std::unique_ptr<LogMessage> message);
};