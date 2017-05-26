#include "tuple.h"
#include <regex>

class PatternString {
    public:
        PatternString(std::string, bool);
        PatternString(Tuple);
        bool operator==(const std::string& rhs)const;
        bool operator==(const Tuple& rhs) const;
        Tuple get_tuple();
    private:
        std::string pattern_string;
        std::string[8] pattern_token;
        int size;
        bool search_string;
        
        void tokenize();
        bool compare(std::string, Tuple_Data);
        bool compare_string(std::string, char*);
        bool compare_int(std::string, int);
        bool compare_float(std::string, float);
        std::string tuple_to_string();    
        Tuple_Data token_to_data(std::string);
    
}
