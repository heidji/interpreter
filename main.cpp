#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <chrono>

#include <phpcpp.h>

using namespace std;
using namespace std::chrono;

void trim(string str){
    while(str.compare(0,1," ")==0)
        str.erase(str.begin()); // remove leading whitespaces
    while(str.size()>0 && str.compare(str.size()-1,1," ")==0)
        str.erase(str.end()-1); // remove trailing whitespaces
}

vector<string> explode(string delim, string s)
{
    vector<string> result;

    int temp;
    while (s.find(delim) != string::npos)
    {
        temp = s.find(delim);
        result.push_back(s.substr(0, temp));
        s.replace(0, temp+delim.length(), "");
    }
    result.push_back(s);

    return result;
}

vector<string> trim_explode(string delim, string s)
{
    vector<string> result;

    int temp;
    while (s.find(delim) != string::npos)
    {
        temp = s.find(delim);
        result.push_back(s.substr(0, temp));
        trim(s.replace(0, temp+delim.length(), ""));
    }
    result.push_back(s);

    return result;
}

string eval_exp(string str){
    bool res;
    auto or_exp = explode(str, "or");
    for(string s_or : or_exp){
        auto and_exp = explode(s_or, "and");
        res = true;
        for(string s_and : and_exp){
            if(s_and.find("false") != string::npos){
                if(s_and.find("not") == string::npos){
                    res = false;
                    break;
                }
            }else if(s_and.find("true") != string::npos){
                if(s_and.find("not") != string::npos){
                    res = false;
                    break;
                }
            }
        }
        if(res)
            return "true";
    }
    return "false";
}

void clean(string &str){
    // to lower
    locale loc;
    string temp = "";
    for(auto elem : str)
        temp += tolower(elem,loc);
    str = temp;

    // trim
    trim(str);

    // remove excess whitespaces
    size_t pos = str.find("  ");
    while(pos != string::npos) {
        str.replace(pos, 2, " " );
        pos = str.find("  ");
    }
}

string eval(string str){
    clean(str);

    while(str.find("(") != string::npos){
        // find last "("
        size_t pos_start = str.find_last_of("(");
        // find the following ")"
        size_t pos_end = str.find(")",pos_start+1);
        if(pos_end == string::npos)
            throw std::invalid_argument("invalid expression: missing closing braces");
        str.replace(pos_start, pos_end-pos_start+1, " "+eval_exp(str.substr(pos_start+1, pos_end-pos_start-1)+" "));
        clean(str);
    }

    return eval_exp(str);
}

Php::Value interpreter(Php::Parameters &params)
{
    auto $events = params[0];
    auto $code = params[1];

    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};

    map<string, string> $predefined_vars = {{"tid", "typeId"}, {"qid", "qualifierId"}};

    map<string, string> $operands = {{"!=", "!="}, {">=", ">="}, {"<=", "<="}, {"=", "=="}, {">", ">"}, {"<", "<"}};

    Php::Value $collection;
    Php::Value $empty;
    Php::Value $temp;

    for (auto&& [$step, $event] : $events) {
        for (auto&& [$i_event_name, $code_blocks] : $code){
            for (auto&& [$block, $instruction] : $code_blocks){
                for (auto&& [$c, $condition] : $instruction["conditions"]){
                    Php::Value $args = trim_explode(",", $condition);
                    for (auto&& [$x, $arg] : $args){

                    }
                }
            }
        }
    }


    //how to loop in C i guess
     /*for ( int i = 0; i < $collection.size( ) ; i++ ) {
        $temp = $collection[i];
        for (auto&& [key, val] : $temp)
        {
            $temp[key] = "LOL";
            $collection[i] = $temp;
        }
    }*/

    return $collection;
}

// Symbols are exported according to the "C" language
extern "C"
{
    // export the "get_module" function that will be called by the Zend engine
    PHPCPP_EXPORT void *get_module()
    {
        // create extension
        static Php::Extension extension("interpreter","1.0");

        // add function, with defined numeric parameters, to extension
        extension.add<interpreter>("interpreter");

        // return the extension module
        return extension.module();
    }
}
