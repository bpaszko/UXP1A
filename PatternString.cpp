#include "PatternString.h"

bool PatternString::operator==(const std::string& rhs) const
{
    return this->pattern_string == rhs;
}

bool PatternString::operator==(const Tuple& rhs) const
{
    for(int i = 0; i < this->size; ++i)
        if(!compare(this->pattern_token[i], rhs.data[i]))
            return false;
    return true;        
}

PatternString::PatternString(std::string pattern)
{
    this->pattern_string = pattern;
    tokenize();
}

PatternString::PatternString(Tuple tuple)
{
    this->pattern_string = tuple_to_string(tuple);
}

void PatternString::tokenize()
{
    bool flag = false;
    int tokens = 0;
    for(int i=0; i<s.size() && tokens < 8; ++i)
    {
        if(s[i]=='\"')
        {
            flag = !flag;
            continue;
        }
        if(s[i]==',' && !flag)
            tokens++;
        else
            pattern_token[tokens] += s[i];
    }
    for(int i = 0; i < tokens; ++i)
        if(pattern_token[i].size() > 65) //truncate too long strings
            pattern_token[i] = pattern_token[i].substr(0, 65);
}

bool PatternString::compare(std::string token, Tuple_Data to_compare)
{
    switch(token[0])
    {
        case 's':
            if(to_compare.data_type != data_type::DATA_STRING)
                return false;
            return compare_string(token.substr(2), to_compare.data_union.data_string);
        case 'i':
            if(to_compare.data_type != data_type::DATA_INT)
                return false;
            return compare_int(token.substr(2), to_compare.data_union.data_int);
        case 'f':
            if(to_compare.data_type != data_type::DATA_FLOAT)
                return false;
            return compare_int(token.substr(2), to_compare.data_union.data_float);
        default:
            return false;
    }
}

bool PatternString::compare_string(std::string pattern, const char* to_compare)
{
    std::regex regex(pattern)
    return std::regex_match(to_compare, regex);
}

bool PatternString::compare_int(std::string pattern, const int to_compare)
{
    switch(pattern[0])
    {
        case '*': case '\0':
            return true;
        case '>':
            return std::stoi(pattern.substr(1)) > to_compare;
        case '<':
            return std::stoi(pattern.substr(1)) < to_compare;
        default:
            return std::stoi(pattern) == to_compare; 
    }
}

bool PatternString::compare_float(std::string pattern, const float to_compare)
{
    switch(pattern[0])
    {
        case '*': case '\0':
            return true;
        case '>':
            return std::stof(pattern.substr(1)) > to_compare;
        case '<':
            return std::stof(pattern.substr(1)) < to_compare;
        default:
            return std::stof(pattern) == to_compare; 
    }
}

Tuple PatternString::get_tuple()
{
    Tuple to_return;
    for(int i = 0; i < size; ++i)
    {
        to_return.data[i] = token_to_data(pattern_token[i]);
    }
    return to_return;
}


Tuple_Data PatternString::token_to_data(std::string token)
{
    Tuple_Data to_return;
    switch(token[0])
    {
        case 's':
            to_return.type = data_type::DATA_STRING;
            // remember that it will need a "conversion" to shared memory string!
            to_return.data_union.data_string = token.substr(2).c_str();
        case 'i':
            to_return.type = data_type::DATA_INT;
            to_retun.data_union.data_int = std::stoi(token.substr(2));
        case 'f':
            to_return.type = data_type::DATA_FLOAT;
            to_retun.data_union.data_int = std::stof(token.substr(2));            
        default:
            to_return.type = data_type::NO_DATA;
    }
    return to_return;
}

std::string PatternString::tuple_to_string(Tuple tuple)
{
	
	int tuple_it = 0;
	while(tuple.data[tuple_it].type != data_type::NO_DATA && tuple_it < 8)
	{
	    std::string token;
		switch(tuple.data[tuple_it].type)
		{
			case data_type::DATA_INT:
			    token = "i:";
			    token += std::to_string(tuple.data[tuple_it].data_union.data_int);
			break;
			    case data_type::DATA_FLOAT:
			    token = "f:";
			    token += std::to_string(tuple.data[tuple_it].data_union.data_float);
			    break;			
			case data_type::DATA_STRING:
			    token = "s:";
			    token += tuple.data[tuple_it].data_union.data_string;
			    break;
			default:
			    token = "";
		}
		tuple_it++;
		this->token_pattern[tuple_it] = token;
		if(tuple.data[tuple_it].type != data_type::NO_DATA)
			pattern += "," + token; 
	}
	return pattern
}

friend ostream& operator<<(ostream& os, const PatternString& ps)
{
    os << ps.pattern_string;
}
