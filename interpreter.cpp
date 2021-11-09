/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <chrono>

#include <phpcpp.h>

using namespace std;
using namespace std::chrono;

vector<string> explode(string s, string delim)
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
    while(str.compare(0,1," ")==0)
        str.erase(str.begin()); // remove leading whitespaces
    while(str.size()>0 && str.compare(str.size()-1,1," ")==0)
        str.erase(str.end()-1); // remove trailing whitespaces

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

Php::Value calc(Php::Parameters &params){
    /*auto $events = params[0];
    auto $code = params[1];

    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};

    map<string, string> $predefined_vars = {{"tid", "typeId"}, {"qid", "qualifierId"}};

    map<string, string> $operands = {{"!=", "!="}, {">=", ">="}, {"<=", "<="}, {"=", "=="}, {">", ">"}, {"<", "<"}};

    vector<map<string, string>> $collection = {};
    Php::Value $lol;
    $lol[0] = "lol";*/

    return params[0];
}

int main()
{
    auto start = high_resolution_clock::now();
    cout<<eval("true and (true and   (not false and not true))");
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << duration.count() << endl;

    return 0;
}

