
#include <gtest/gtest.h>
#include "../logparser.h"


TEST(LogParserTest, ParseLogMessage)
{

    LogParser parser;
    std::string log_entry = "1 1 0 [Hello] 2";
    auto message = std::move(parser.parseLine(log_entry));

    ASSERT_NE(message, nullptr); 

    EXPECT_EQ(message->pipeline_id, std::string("1"));
    EXPECT_EQ(message->id, std::string("1"));
    EXPECT_EQ(message->encoding, ASCII_ENCODING);
    EXPECT_EQ(message->body, std::string("Hello"));
    EXPECT_EQ(message->next_id, std::string("2"));
}

TEST(LogParserTest, ParseMalformedLogMessages)
{

    LogParser parser;
    {
        std::string log_entry = "1 1 1 [Hello] 2"; // Wrong encoding type
        auto message = std::move(parser.parseLine(log_entry));

        EXPECT_EQ(message, nullptr);
    }
    {
        std::string log_entry = "1 0 [Hello] 2"; // Missing field
        auto message = std::move(parser.parseLine(log_entry));

        EXPECT_EQ(message, nullptr);
    }
    
    {
        std::string log_entry = "1 1 0 Hello 2"; // Missing brackets
        auto message = std::move(parser.parseLine(log_entry));

        EXPECT_EQ(message, nullptr);
    }
    {
        std::string log_entry = "1 1 0 [Hello]"; // Missing next_id
        auto message = std::move(parser.parseLine(log_entry));

        EXPECT_EQ(message, nullptr);
    }
    {
        std::string log_entry = ""; // Empty line
        auto message = std::move(parser.parseLine(log_entry));

        EXPECT_EQ(message, nullptr);
    }

    {
        std::string log_entry = "1 1 1 [4F4B] 2 \n 1 2 0 [Bye] -1"; // Multiple lines
        auto message = std::move(parser.parseLine(log_entry));

        ASSERT_NE(message, nullptr); 

        EXPECT_EQ(message->pipeline_id, std::string("1"));
        EXPECT_EQ(message->id, std::string("1"));
        EXPECT_EQ(message->encoding, HEX_ENCODING);
        EXPECT_EQ(message->body, std::string("OK"));
        EXPECT_EQ(message->next_id, std::string("2"));
    }


}

TEST(LogParserTest, ParseLogString)
{
    LogParser parser;
    std::string log_content = "2 3 1 [4F4B] -1\n"
                              "1 0 0 [some text] 1\n"
                              "1 1 0 [another text] 2\n"
                              "2 99 1 [4F4B] 3\n"
                              "1 2 1 [626F6479] -1\n";


    parser.parseLog(log_content);


    auto messages = std::move(parser.getLogMessagesPrintable());

    EXPECT_EQ(*messages, "Pipeline 2\n"
                         " 3| OK\n"
                         " 99| OK\n"
                         "Pipeline 1\n"
                         " 2| body\n"
                         " 1| another text\n"
                         " 0| some text\n");

}

TEST(LogParserTest, ParseMalformedLogStrings)
{
    {
        LogParser parser;
        //pipeline 2 doesn't have -1 ending
        std::string log_content = "1 0 0 [some text] 1\n"  
                                  "1 1 0 [another text] 2\n"
                                  "2 99 1 [4F4B] 3\n"
                                  "1 2 1 [626F6479] -1\n";


        parser.parseLog(log_content);


        auto messages = std::move(parser.getLogMessagesPrintable());

        EXPECT_EQ(*messages, "Pipeline 1\n"
                             " 2| body\n"
                             " 1| another text\n"
                             " 0| some text\n"
                             "Pipeline 2\n"
                             " 99| OK\n"
                             );
    }
    {
        LogParser parser; 
        std::string log_content = "2 3 1 [4F4B] -1\n"
                              "1 0 1 [some text] 1\n" //wrong encoding
                              "1 1 0 [another text] 2\n"
                              "2 99 1 [4F4B] 3\n"
                              "1 2 1 [626F6479] -1\n";


        parser.parseLog(log_content);


        auto messages = std::move(parser.getLogMessagesPrintable());

        EXPECT_EQ(*messages, "Pipeline 2\n"
                             " 3| OK\n"
                             " 99| OK\n"
                             "Pipeline 1\n"
                             " 2| body\n"
                             " 1| another text\n"
                             );
    }

    {
        LogParser parser; 
        std::string log_content = "2 3 1 [4F4B] -1\n"
                              "1 0 [some text] 1\n" //missing field
                              "1 1 0 [another text] 2\n"
                              "2 99 1 [4F4B] 3\n"
                              "1 2 1 [626F6479] -1\n";


        parser.parseLog(log_content);


        auto messages = std::move(parser.getLogMessagesPrintable());

        EXPECT_EQ(*messages, "Pipeline 2\n"
                             " 3| OK\n"
                             " 99| OK\n"
                             "Pipeline 1\n"
                             " 2| body\n"
                             " 1| another text\n"
                             );
    }



}

TEST(LogParserTest, ParseLogMalformedLogStringBrokenLinks)
{
     {
        LogParser parser;
        std::string log_content = 
                                  "1 0 0 [some text] 1\n"
                                  "1 2 1 [626F6479] 3\n"
                                  "1 4 0 [what another text] -1\n";
            


        parser.parseLog(log_content);


        auto messages = std::move(parser.getLogMessagesPrintable());

        EXPECT_EQ(*messages,"Pipeline 1\n"
                             " 4| what another text\n"
                             " 2| body\n"
                             " 0| some text\n");
    }
    
    {
        LogParser parser;
        std::string log_content =
                                  "1 5 0 [last] 7\n"  // 7 doesn't exists
                                  "1 0 0 [some text] 1\n"
                                  "1 2 1 [626F6479] 3\n";


        parser.parseLog(log_content);


        auto messages = std::move(parser.getLogMessagesPrintable());

        EXPECT_EQ(*messages,"Pipeline 1\n"
                             " 2| body\n"
                             " 0| some text\n"
                             " 5| last\n"
                             );
    }
}


TEST(LogParserTest, ParseLogMalformedLogStringLoopLinks)
{
    LogParser parser;
    std::string log_content = "2 3 1 [4F4B] -1\n"
                              "1 0 0 [some text] 1\n"
                              "1 1 0 [another text] 2\n"
                              "2 99 1 [4F4B] 3\n"
                              "1 2 1 [626F6479] 1\n";  // loop back to id 1


    parser.parseLog(log_content);


    auto messages = std::move(parser.getLogMessagesPrintable());

    EXPECT_EQ(*messages, "Pipeline 2\n"
                         " 3| OK\n"
                         " 99| OK\n"
                         "Pipeline 1\n"
                         " 2| body\n"
                         " 1| another text\n"
                         " 0| some text\n");
}

TEST(LogParserTest, ParserLogFile)
{
    LogParser parser;
    std::ifstream file(std::string(LOGS_DIR) + "/actual_log.log");
    parser.parseLog(file);

     auto messages = std::move(parser.getLogMessagesPrintable());

    EXPECT_EQ(*messages,    "Pipeline legacy-hex\n"
                            " 2| Morbi lobortis maximus viverra. Aliquam et hendrerit nulla\n"
                            " 1| Vivamus rutrum id erat nec vehicula. Donec fringilla lacinia eleifend.\n"
                            "Pipeline 2\n"
                            " 12| nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
                            " 30| commodo consequat. duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
                            " 10| Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea\n"
                            "Pipeline 1\n"
                            " 0| Lorem ipsum dolor sit amet, consectetur adipiscing elit\n"
                            "Pipeline 3\n"
                            " 1| sed do eiusmod tempor incididunt ut labore et dolore magna aliqua\n");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
