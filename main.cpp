#include <cstdio>
#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>
#include <fstream>
#include <mutex>
#include <map>
#include <iterator>

#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/rapidjson/filereadstream.h"
#include "include/rapidjson/encodedstream.h"

using namespace rapidjson;

using namespace std;
using namespace std::chrono;

mutex m;

string bool2str(bool x)
{
    return x ? "true" : "false";
}

string k(int c)
{
    string s = "";
    for (int i = 0; i < c; i++)
    {
        s += "\t";
    }
    return s;
}

struct result_t
{
    string event_type, event_id, id, time, noten_context;
    map<string, string> values;

    string toString(int c = 0){
        string s = k(c) + "event_type: " + event_type + "\n" + k(c) + "event_id: " + event_id + "\n" + k(c)+
        "id: " + id + "\n" + k(c) + "time: " + time + "\n" + k(c) + "noten_context: " + noten_context +
        "\n" + k(c) + "values: {\n";
        for (auto &&[key, v] : values)
        {
            s += k(c+1) + key + ": " + v + "\n";
        }
        s += k(c) + "}\n";
        return s;
    }

    string toJson(){
        string s = "{\"event_type\":\"" + event_type + "\",\"event_id\":\"" + event_id +
                   "\",\"id\":\"" + id + "\",\"time\":\"" + time + "\",\"noten_context\":\"" + noten_context +
                   "\",\"values\": {";
        int x = 0;
        for (auto &&[key, v] : values)
        {
            s += "\"" + key + "\":\"" + v + "\"";
            x++;
            if(x < values.size())
                s += ",";
        }
        s += "}}";
        return s;
    }
};

struct qualifier_t
{
    map<string, string> params;

    string toString(int c = 0){
        string s = "";
        for(auto&& [key, v] : params){
            s += k(c) + key + ": " + v + "\n";
        }
        return s;
    }
};

struct event_t
{
    int time = 0;
    map<string, string> params;
    vector<qualifier_t> qualifier;

    string toString(int c = 0)
    {
        string s = k(c) + "time: "+to_string(time)+"\n"+k(c)+"params: {\n";
        for (auto &&[key, v] : params)
        {
            s += k(c+1) + key + ": " + v + "\n";
        }
        s += k(c) + "}\n" + k(c) + "qualifier: [\n";
        for (qualifier_t &v : qualifier)
        {
            s += v.toString(c+1) + "\n";
        }
        s += "\n" + k(c) + "]";
        return s;
    }
};

struct rule_side_t
{
    string event = "";
    string qualifier = "";
    string var = "";
    bool abstract = false;
    bool constant = false;
    bool numeric = false;

    string toString(int c = 0)
    {
        return k(c) + "event: " + event + "\n" + k(c) + "qualifier: " + qualifier + "\n" + k(c) + "var: " + var + "\n" + k(c) + "abstract: " +
               bool2str(abstract) + "\n" + k(c) + "constant: " + bool2str(constant) + "\n" + k(c) + "numeric: " + bool2str(numeric) + "\n";
    }
};

struct rule_t
{
    bool isNot = false;
    string op = "";
    rule_side_t left;
    rule_side_t right;

    string toString(int c = 0)
    {
        return k(c) + "isNot: " + bool2str(isNot) + "\n" + k(c) + "op: " + op + "\n" + k(c) + "left: {\n" + left.toString(c + 1) + "\n" + k(c) + "}\n" + k(c) + "right: {\n" + right.toString(c + 1) + "\n" + k(c) + "}";
    }
};

struct condition_t
{
    bool isEvent = false;
    string iterator = "";
    int within = 0;
    string withinType = "";
    int times = 0;
    int direction = 0;
    vector<string> skips;
    vector<rule_t> rules;

    string toString(int c = 0)
    {
        string s = k(c) + "isEvent: " + bool2str(isEvent) + "\n" + k(c) + "iterator: " + iterator +
        "\n" + k(c) + "within: " + to_string(within) + "\n" + k(c) + "withinType: " + withinType + "\n" +
        k(c) + "times: " + to_string(times) + "\n" + k(c) + "direction: " + to_string(direction) + "\n" + k(c) + "skips: [\n";
        for (string &skip : skips)
        {
            s += k(c+1) + skip + "\n";
        }
        s += "\n" + k(c) + "]\n";
        s += k(c) + "rules: [\n";
        for (rule_t &rule : rules)
        {
            s += k(c) +"{\n" + rule.toString(c + 1) + "\n" + k(c) + "},\n";
        }
        s += "\n" + k(c) + "]\n";
        return s;
    }
};

struct formula_connected_t
{
    string op = "";
    string query = "";
    rule_side_t left;
    rule_side_t right;

    string toString(int c = 0){
        return k(c) + "op: " + op + "\n" + k(c) + "query: " + query + "\n" + k(c) + "left: {\n" + left.toString(c + 1) + "\n" + k(c) + "}\n" + k(c) + "right: {\n" + right.toString(c + 1) + "\n" + k(c) + "}\n";
    }
};

struct logic_gate_t
{
    bool isNot = false;
    string query;

    string toString(int c = 0){
        return k(c) + "isNot: " + bool2str(isNot) + ", query: " + query;
    }
};

struct formula_t
{
    map<string, string> expanded;
    map<string, vector<vector<logic_gate_t>>> logic;
    vector<rule_side_t> simple;
    vector<formula_connected_t> connected;

    string toString(int c = 0){
        string s = k(c) + "expanded: [\n";
        for (auto &&[key, v] : expanded)
        {
            s += k(c + 1) + key + ": " + v + "\n";
        }
        s += k(c) + "]\n" + k(c) + "logic: {\n";
        for(auto &&[key, v] : logic){
            s += k(c+1) + key + ": [\n";
            for(vector<logic_gate_t> &v1 : v){
                s += k(c+2) + "[\n";
                for(logic_gate_t &v2 : v1){
                    s += k(c+3) + "{" + v2.toString() + "}\n";
                }
                s += k(c+2) + "]\n";
            }
            s += k(c+1) + "]\n";
        }
        s += k(c) + "}\n" + k(c) + "simple: [\n";
        for (rule_side_t &side : simple)
        {
            s += k(c + 1) + "{\n" + side.toString(c + 2) + "\n" + k(c + 1)+ "}\n";
        }
        s += k(c) + "]\n";
        s += k(c) + "connected: [\n";
        for (formula_connected_t &side : connected)
        {
            s += k(c + 1) + "{\n" + side.toString(c + 2) + "\n" + k(c + 1) + "}\n";
        }
        s += k(c) + "]\n";
        return s;
    }
};

struct cpp_t
{
    formula_t formula;
    map<string, condition_t> conditions;
    map<string, rule_side_t> values;

    string toString(int c = 0)
    {
        string s = k(c) + "formula:\n " + formula.toString(c+1) + "\n" + k(c) + "conditions: {\n";
        for (auto &&[key, v] : conditions)
        {
            s += k(c + 1) + key + ":{ \n" + v.toString(c + 2) + "\n" + k(c + 1) + "},\n";
        }
        s += k(c) + "}" + k(c) + "\n" + k(c) + "values: {\n";
        for (auto &&[key, v] : values)
        {
            s += k(c + 1) + key + ":{ \n" + v.toString(c + 2) + "\n" + k(c + 1) + "},\n";
        }
        s += k(c) + "}" + k(c) + "\n";
        return s;
    }
};

struct e_q_t
{
    event_t event;
    map<string, bool> evals;
    map<string, qualifier_t> qualifier;

    string toString(int c = 0){
        string s = k(c) + "event: {\n"+ event.toString(c+1)+"\n"+k(c)+"}\n";
        s += k(c) + "qualifier: [\n";
        for (auto &&[key, v] : qualifier)
        {
            s += k(c + 1) + key + ": {\n" + v.toString(c + 2) + "\n" + k(c + 1) + "}\n";
        }
        s += k(c) + "]\n";
        return s;
    }
};

struct q_t
{
    map<string, e_q_t> eq;
    map<string, bool> evals;
    map<string, string> test;

    string toString(int c = 0){
        string s = k(c) + "eq: {\n";
        for (auto &&[key, v] : eq)
        {
            s += k(c + 1) + key + ": {\n" + v.toString(c+2) + "\n" + k(c+1) + "}\n";
        }
        s += k(c) + "}\n" + k(c) + "evals: {\n";
        for (auto &&[key, v] : evals)
        {
            s += k(c + 1) + key + ": "+ bool2str(v) +"\n";
        }
        s += k(c) + "}\n" + k(c) + "test: [\n";
        for (auto &&[key, v] : test)
        {
            s += k(c + 1) + key + ": " + v + "\n";
        }
        s += k(c) + "]\n";
        return s;
    }
};

struct instruction_t
{
    string primary = "";
    map<string, string> conditions;
    string formula = "";
    map<string, string> values;
    map<string, map<string, map<string, map<string, vector<map<string, vector<string>>>>>>> skips;
    string noten_context = "";
    string optTid = "";
    cpp_t cpp;

    string toString(int c = 0)
    {
        string s;
        s = k(c) + "primary: " + primary + "\n" + k(c) + "formula: " + formula + "\n" + 
        k(c) + "noten_context: " + noten_context + "\n" + k(c) + "optTid: " + optTid + "\n" + k(c) + "conditions: {\n";
        for (auto &&[key, v] : conditions)
        {
            s += k(c + 1) + key + ": " + v + "\n";
        }
        s += "\n" + k(c) + "}\n";
        s += k(c) + "skips: {\n";
        // s1
        for (auto &&[key1, v1] : skips)
        {
            s += k(c + 1) + key1 + ": { \n";
            // skip / stop / count
            for (auto &&[key2, v2] : v1)
            {
                s += k(c + 2) + key2 + ": { \n";
                // team / teamvs
                for (auto &&[key3, v3] : v2)
                {
                    s += k(c + 3) + key3 + ": { \n";
                    // team / teamvs
                    for (auto &&[key4, v4] : v3)
                    {
                        s += k(c + 4) + key4 + ": [ \n";
                        // is / is_not
                        for (map<string, vector<string>> v5 : v4)
                        {
                            s += k(c + 5) + "{ \n";
                            // elems
                            for (auto &&[key6, v6] : v5)
                            {
                                s += k(c + 6) + key6 + ": [ \n";
                                // values
                                for (string &v7 : v6)
                                {
                                    s += k(c + 7) + v7 + "\n";
                                }
                                s += k(c + 6) + "]\n";
                            }
                            s += k(c + 5) + "}\n";
                        }
                        s += k(c + 4) + "]\n";
                    }
                    s += k(c + 3) + "}\n";
                }
                s += k(c + 2) + "}\n";
            }
            s += k(c + 1) + "}\n";
        }
        s += "\n" + k(c) + "}\n";
        s += k(c) + "cpp: {\n" + cpp.toString(c + 1) + k(c + 1) + "},\n";
        return s;
    }
};

string code2string(map<string, vector<instruction_t>> &m, int c = 0)
{
    string s;
    s = k(c) + "{\n";
    for (auto &&[key, v] : m)
    {
        s += k(c + 1) + key + ": " + "[\n";
        for (instruction_t &i : v)
        {
            s += i.toString(c + 2) + ",\n";
        }
        s += "\n" + k(c + 1) + "]\n";
    }
    s += "\n" + k(c) + "}\n";
    return s;
}

string events2string(vector<event_t> &m, int c = 0)
{
    string s;
    s += k(c) + "[\n";
    for (event_t &i : m)
    {
        s += i.toString(c + 1) + ",\n";
    }
    s += "\n" + k(c) + "]\n";
    return s;
}

string result2string(vector<result_t> &m, int c = 0)
{
    string s;
    s += k(c) + "[\n";
    for (result_t &i : m)
    {
        s += i.toString(c + 1) + ",\n";
    }
    s += "\n" + k(c) + "]\n";
    return s;
}

string result2json(vector<result_t> &m, int c = 0)
{
    string s;
    s += k(c) + "[";
    int x = 0;
    for (result_t &i : m)
    {
        s += i.toJson();
        x++;
        if(x < m.size())
            s += ",";
    }
    s += "]";
    return s;
}

bool is_numeric(string s)
{
    return !s.empty() && s.find_first_not_of("-.0123456789") == std::string::npos;
}

int strtotime(string date)
{
    stringstream yearValue(date.substr(0, 4));
    int year;
    yearValue >> year;
    stringstream monthValue(date.substr(5, 2));
    int month;
    monthValue >> month;
    stringstream dayValue(date.substr(8, 2));
    int day;
    dayValue >> day;
    stringstream hourValue(date.substr(11, 2));
    int hour;
    hourValue >> hour;
    stringstream minValue(date.substr(14, 2));
    int min;
    minValue >> min;
    stringstream secValue(date.substr(17, 2));
    int sec;
    secValue >> sec;

    // approximation for comparison purposes only
    return (((((year - 2000) * 12 + month) * 30 + day) * 24 + hour) * 60 + min) * 60 + sec;
}

string trim(string str)
{
    while (str.compare(0, 1, " ") == 0)
        str.erase(str.begin()); // remove leading whitespaces
    while (str.size() > 0 && str.compare(str.size() - 1, 1, " ") == 0)
        str.erase(str.end() - 1); // remove trailing whitespaces

    return str;
}

vector<string> explode(string delim, string s)
{
    vector<string> result;

    if (s.empty())
        return result;

    int temp;
    while (s.find(delim) != string::npos && !delim.empty())
    {
        temp = s.find(delim);
        result.push_back(s.substr(0, temp));
        s.replace(0, temp + delim.length(), "");
    }
    result.push_back(s);

    return result;
}

vector<string> trim_explode(string delim, string s)
{
    vector<string> result;

    if (s.empty())
        return result;

    int temp;
    while (s.find(delim) != string::npos && !delim.empty())
    {
        temp = s.find(delim);
        result.push_back(trim(s.substr(0, temp)));
        s.replace(0, temp + delim.length(), "");
    }
    result.push_back(trim(s));

    return result;
}

string eval_exp(string str)
{
    bool res;
    auto or_exp = explode("or", str);
    for (string &s_or : or_exp)
    {
        auto and_exp = explode("and", s_or);
        res = true;
        for (string &s_and : and_exp)
        {
            if (s_and.find("false") != string::npos)
            {
                if (s_and.find("not") == string::npos)
                {
                    res = false;
                    break;
                }
            }
            else if (s_and.find("true") != string::npos)
            {
                if (s_and.find("not") != string::npos)
                {
                    res = false;
                    break;
                }
            }
        }
        if (res)
            return "true";
    }
    return "false";
}

void clean(string &str, bool lower = true)
{
    // to lower
    locale loc;

    if (lower)
    {
        string temp = "";
        for (auto &elem : str)
            temp += tolower(elem, loc);
        str = temp;
    }

    // trim
    str = trim(str);

    // remove excess whitespaces
    size_t pos = str.find("  ");
    while (pos != string::npos)
    {
        str.replace(pos, 2, " ");
        pos = str.find("  ");
    }
}

vector<vector<logic_gate_t>> prep_expr(string str){
    vector<vector<logic_gate_t>> res;
    auto or_exp = explode("or", str);
    for (string &s_or : or_exp)
    {
        auto and_exp = explode("and", s_or);
        vector<logic_gate_t> and_node;
        for (string &s_and : and_exp)
        {
            logic_gate_t node;
            int temp = s_and.find("not");
            if (temp != string::npos)
            {
                node.isNot = true;
                s_and.replace(temp, 3, "");
                clean(s_and, false);
            }
            clean(s_and, false);
            node.query = s_and;
            and_node.push_back(node);
        }
        res.push_back(and_node);
    }
    return res;
}

map<string, vector<vector<logic_gate_t>>> prep(string str){
    clean(str, false);

    map<string, vector<vector<logic_gate_t>>> logic;

    if(str == "")
        return logic;

    int c = 0;
    while (str.find("(") != string::npos)
    {
        // find last "("
        size_t pos_start = str.find_last_of("(");
        // find the following ")"
        size_t pos_end = str.find(")", pos_start + 1);
        if (pos_end == string::npos)
            throw std::invalid_argument("invalid expression: missing closing braces");
        vector<vector<logic_gate_t>> node = prep_expr(str.substr(pos_start + 1, pos_end - pos_start - 1));
        string name = "_node_"+to_string(c);
        str.replace(pos_start, pos_end - pos_start + 1, " " + name + " ");
        clean(str, false);
        logic[name] = node;
        c++;
    }
    // no more braces
    logic["_node_start"] = prep_expr(str);
    return logic;
}

bool eval(string query, instruction_t &instruction, q_t &q)
{
    for(vector<logic_gate_t> &and_node : instruction.cpp.formula.logic[query]){
        for(logic_gate_t &gate : and_node){
            if(!q.evals.count(gate.query)){
                bool temp = eval(gate.query, instruction, q);
                if(!temp && !gate.isNot || temp && gate.isNot){
                    goto cont_or;
                }
            }else{
                if(!q.evals[gate.query] && !gate.isNot || q.evals[gate.query] && gate.isNot){
                    goto cont_or;
                }
            }
        }
        // passes
        return true;
        cont_or:;
    }
    return false;
}

string operands(string str)
{
    if (str.find("!=") != string::npos)
    {
        return "!=";
    }
    else if (str.find(">=") != string::npos)
    {
        return ">=";
    }
    else if (str.find("<=") != string::npos)
    {
        return "<=";
    }
    else if (str.find("=") != string::npos)
    {
        return "=";
    }
    else if (str.find(">") != string::npos)
    {
        return ">";
    }
    else if (str.find("<") != string::npos)
    {
        return "<";
    }
    return "";
};

bool eval_with_op(string left, string right, string op)
{
    // check null string
    double l, r;
    string temp = right;
    if (temp == "null")
        right = ""; // no null in c++ :(

    // removed is_numeric because it's slow. assume comparison based on operator
    if (op == "<" || op == ">" || op == ">=" || op == "<=")
    {
        if(is_numeric(left) && is_numeric(right)){
            l = ::atof(left.c_str());
            r = ::atof(right.c_str());
            if (op == ">")
                return l > r;
            else if (op == "<")
                return l < r;
            else if (op == ">=")
                return l >= r;
            else if (op == "<=")
                return l <= r;
        }else{
            if (op == ">")
                return left > right;
            else if (op == "<")
                return left < right;
            else if (op == ">=")
                return left >= right;
            else if (op == "<=")
                return left <= right;
        }
    }
    else
    {
        if (op == "=")
            return left == right;
        else if (op == "!=")
            return left != right;
    }
    return false;
}

bool in_array(string needle, vector<string> &haystack)
{
    return count(haystack.begin(), haystack.end(), needle);
}

bool isEventCondition(string name, map<string, string> &conditions, string primary)
{
    if (name == primary)
        return true;
    vector<string> primary_event_selectors = {"before", "after", "around", "skip"};
    string temp = conditions[name];

    for (string &selector : primary_event_selectors)
    {
        if (temp.find(selector) != std::string::npos)
            return true;
    }
    return false;
}

// declare
bool testConditions(string name, instruction_t &instruction, vector<event_t> &events, int index, int primary_index, q_t &q, vector<string> &it);

bool looper(int which, int s, int times, int j, int end)
{
    if (which == 1)
        return (s <= times && j <= end);
    else
        return (s <= times && j >= end);
}

bool findEvent(string name, instruction_t &instruction, int primary_index, vector<event_t> &events, q_t &q, vector<string> &it)
{
    if (in_array(name, it))
        throw std::invalid_argument("circular reference");
    it.push_back(name);

    event_t &event = events[primary_index];
    condition_t &condition = instruction.cpp.conditions[name];

    string within_type = condition.withinType, selector = condition.iterator;
    int step = primary_index;
    int within = condition.within, times = condition.times;
    int start = 0, direction = condition.direction, end = 0;
    int i = 0;
    int j = 0;
    int s = 0;

    start = step + direction;

    if (within != 0)
    {
        if (within_type == "steps")
        {
            end = step + (within * direction);
        }
        else
        {
            end = start;
            if (!(end > events.size()) || abs(event.time - events[end].time) > within){
                return false;
            }
            else
            {
                end += direction;
                while (!(end > events.size()) && !((end + direction) > events.size()) && abs(event.time - events[end + direction].time) <= within)
                {
                    end += direction;
                }
            }
        }
    }
    else
    {
        if (direction == -1)
            end = 0;
        else
            end = events.size() - 1;
    }
    if (end > events.size() - 1)
        end = events.size() - 1;
    else if (end < 0)
        end = 0;
    if (start > events.size() - 1)
        start = events.size() - 1;
    else if (start < 0)
        start = 0;

    int which;
    if (direction == 1)
        which = 1;
    else
        which = 2;

    if (start == step || direction == 1 && start > end || direction == -1 && start < end)
    {
        return false;
    }
    int around_alternator = 1;

    for (j = start; looper(which, s, times, j, end); j += direction)
    {
        if (selector == "around")
        {
            i = ((j - start) * around_alternator) + start;
            if (around_alternator == 1)
            {
                j--; // skip a beat
            }
            around_alternator = -around_alternator;
            if (i < 0 || i > end)
                continue;
        }
        else
            i = j;

        if (events[i].params["typeId"] == "43")
        {
            end += direction;
            if (end < 0)
                end = 0;
            else if (end > events.size() - 1)
                end = events.size() - 1;
            continue;
        }

        // check if skip event
        int x = 0;
        bool xdo = false;
        string skipstop = "";
        for (string &skip : condition.skips)
        {
            auto &skipset = instruction.skips[skip];
            for (string t_skipstop : {"stop", "skip", "count"})
            {
                skipstop = t_skipstop;
                for (string team : {"team", "teamvs"})
                {
                    map <string, string> context;
                    if(!skipset.count(skipstop) || !skipset[skipstop].count(team))
                        continue;
                    if(skipset[skipstop][team].count("is"))
                        context["is"] = "is";
                    else if(skipset[skipstop][team].count("is_not"))
                        context["is"] = "is_not";
                    else
                        continue;
                    context["team"] = team;
                    if (context["team"] == "team" && q.eq[instruction.primary].event.params["contestantId"] == events[i].params["contestantId"] || context["team"] == "teamvs" && q.eq[instruction.primary].event.params["contestantId"] != events[i].params["contestantId"])
                    {
                        if (context["is"] == "is")
                            xdo = false;
                        else
                            xdo = true;
                        int x = -1;
                        for (auto &ruleset : skipset[skipstop][context["team"]][context["is"]])
                        {
                            x++;
                            for (auto&& [var, items] : ruleset)
                            {
                                for (auto &value : items)
                                {
                                    if (var != "qualifierId")
                                    {
                                        if (events[i].params[var] == value)
                                        {
                                            goto cnt_rules;
                                        }
                                    }
                                    else
                                    {
                                        for (auto qualifier : events[i].qualifier)
                                        {
                                            if (qualifier.params["qualifierId"] == value)
                                            {
                                                goto cnt_rules;
                                            }
                                        }
                                    }
                                }
                                // no match
                                if (x == skipset[skipstop][context["team"]][context["is"]].size()-1)
                                    goto break_skip;
                                else
                                    goto cnt_ruleset;
                                cnt_rules:;
                            }
                            if (context["is"] == "is")
                            {
                                // found match -> skip
                                xdo = true;
                                goto break_skip;
                            }
                            else
                            {
                                // found match -> skip
                                xdo = false;
                                goto cnt_team;
                            }
                            cnt_ruleset:;
                        }
                    }
                    cnt_team:;
                }
            }
        }
        break_skip:;
        if(xdo){
            if(skipstop == "stop"){
                q.evals[name] = false;
                return false;
            }else if(skipstop == "skip") {
                end += direction;
                if (end < 0)
                    end = 0;
                else if (end > events.size() - 1)
                    end = events.size() - 1;
                continue;
            }else if(skipstop == "count"){
                s++;
                continue;
            }
        }

        if(!testConditions(name, instruction, events, i, primary_index, q, it)){
            continue;
        }

        s++;
        if (s >= times && q.evals[name])
        {
            return true;
        }
    }

    return false;
}

bool testEventQualifierConditions(string &name, string &qname, instruction_t &instruction, int primary_index, vector<event_t> &events, q_t &q, vector<string> &it)
{
    vector<qualifier_t> &qualifiers = q.eq[name].event.qualifier;
    condition_t &condition = instruction.cpp.conditions[qname];

    for (qualifier_t &qualifier : qualifiers){
        for(rule_t &rule : condition.rules){
            string left = qualifier.params[rule.left.var];
            string right;
            if(rule.right.constant){
                right = rule.right.var;
            }else{
                if (!q.evals[rule.right.event] || !q.eq[rule.right.event].evals[rule.right.qualifier]){
                    q.eq[rule.right.event].evals[rule.right.qualifier] = false;
                    return false;
                }else if(!testEventQualifierConditions(rule.right.event, rule.right.qualifier, instruction, primary_index, events, q, it)){
                    q.eq[rule.right.event].evals[rule.right.qualifier] = false;
                    return false;
                }
                right = q.eq[rule.right.event].qualifier[rule.right.qualifier].params[rule.right.var];
            }
            if(!eval_with_op(left, right, rule.op)){
                goto cnt;
            }
        }
        q.eq[name].evals[qname] = true;
        q.eq[name].qualifier[qname] = qualifier;
        return true;
        cnt:;
    }
    q.eq[name].evals[qname] = false;
    return false;
}

bool testConditions(string name, instruction_t &instruction, vector<event_t> &events, int index, int primary_index, q_t &q, vector<string> &it)
{

    // put the event in q so we can use it onwards
    q.eq[name].event = events[index];

    condition_t &condition = instruction.cpp.conditions[name];
    for(rule_t &rule : condition.rules){
        string left, right;
        // left
        if (rule.left.qualifier == ""){
            string temp = q.eq[rule.left.event].event.params[rule.left.var];
            left = temp;
        }else{
            if (!q.eq[rule.left.event].evals.count(rule.left.qualifier)){
                bool test = testEventQualifierConditions(rule.left.event, rule.left.qualifier, instruction, primary_index, events, q, it);
                if(test && rule.isNot || !test && !rule.isNot){
                    q.evals[name] = false;
                    q.eq.erase(name);
                    return false;
                }else{
                    continue;
                }
            }else if(!q.eq[rule.left.event].evals[rule.left.qualifier]){
                if (!rule.isNot){
                    q.evals[name] = false;
                    q.eq.erase(name);
                    return false;
                }
                else
                {
                    continue;
                }
            }
            left = q.eq[rule.left.event].qualifier[rule.left.qualifier].params[rule.left.var];
        }

        // right
        if(rule.right.abstract){
            string query = "";
            if(rule.right.event != "" && !q.evals[rule.right.event]){
                q.evals[name] = false;
                q.eq.erase(name);
                return false;
            }
            if(rule.right.qualifier != ""){
                if(!q.eq[rule.right.event].evals[rule.right.qualifier]){
                    bool test = testEventQualifierConditions(rule.right.event, rule.right.qualifier, instruction, primary_index, events, q, it);
                    if (!test)
                    {
                        q.evals[name] = false;
                        q.eq.erase(name);
                        return false;
                    }
                }
                right = q.eq[rule.right.event].qualifier[rule.right.qualifier].params[rule.right.var];
            }else{
                string temp = q.eq[rule.right.event].event.params[rule.right.var];
                right = temp;
            }
        }else{
            right = rule.right.var;
        }
        if (!eval_with_op(left, right, rule.op))
        {
            q.evals[name] = false;
            q.eq.erase(name);
            return false;
        }
    }

    // passed all checks
    q.evals[name] = true;
    return true;
}

string primaryEventSelector(string arg)
{
    vector<string> primary_event_selectors = {"before", "after", "around", "skip", "within"};
    for (string &x : primary_event_selectors)
    {
        if (arg == x)
            return x;
    }
    return "";
}

void replacePredefinedVar(string &var)
{
    if (var == "tid")
        var = "typeId";
    if (var == "qid")
        var = "qualifierId";
}

string interpreter(Value &code_json, Value &events_json)
{
    /*struct event e;
    e.params = {{"k", "l"}, {"k", "l"}};
    e.qualifier = {{{"k", "l"}, {"k", "l"}}, {{"k", "l"}, {"k", "l"}}, {{"k", "l"}, {"k", "l"}}};
    return e.qualifier[2]["k"];
    /*std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    Php::Value lul = xdd;
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();*/

    vector<result_t> collection;

    // normalize code object
    map<string, vector<instruction_t>> code;

    for (Value::ConstMemberIterator iter = code_json.MemberBegin(); iter != code_json.MemberEnd(); ++iter)
    {
        string i_event_name = iter->name.GetString();
        vector<instruction_t> code_block;
        for (SizeType block = 0; block < iter->value.Size(); block++) {
            // get
            struct instruction_t c;
            const auto &instruction = iter->value[block].GetObject();

            if (instruction["primary"].GetString() != "")
            {
                string primary_s = instruction["primary"].GetString();
                c.primary = primary_s;
            }

            map<string, string> conditions_s;
            for (Value::ConstMemberIterator it1 = instruction["conditions"].MemberBegin(); it1 != instruction["conditions"].MemberEnd(); ++it1){
                conditions_s[it1->name.GetString()] = it1->value.GetString();
            }
            c.conditions = conditions_s;

            string formula_s = instruction["formula"].GetString();
            c.formula = formula_s;

            map<string, string> values_s;
            for (Value::ConstMemberIterator it1 = instruction["values"].MemberBegin(); it1 != instruction["values"].MemberEnd(); ++it1){
                values_s[it1->name.GetString()] = it1->value.GetString();
            }
            c.values = values_s;

            map<string, map<string, map<string, map<string, vector<map<string, vector<string>>>>>>> skips_s;
            if(instruction.HasMember("skips") && instruction["skips"].IsObject()){
                for (Value::ConstMemberIterator it1 = instruction["skips"].MemberBegin(); it1 != instruction["skips"].MemberEnd(); ++it1){
                    string key2 = it1->name.GetString();
                    const auto &obj2 = it1->value.GetObject();
                    for (Value::ConstMemberIterator it2 = obj2.MemberBegin(); it2 != obj2.MemberEnd(); ++it2){
                        string key3 = it2->name.GetString();
                        const auto &obj3 = it2->value.GetObject();
                        for (Value::ConstMemberIterator it3 = obj3.MemberBegin(); it3 != obj3.MemberEnd(); ++it3){
                            string key4 = it3->name.GetString();
                            const auto &obj4 = it3->value.GetObject();
                            for (Value::ConstMemberIterator it4 = obj4.MemberBegin(); it4 != obj4.MemberEnd(); ++it4){
                                string key5 = it4->name.GetString();
                                const auto &obj5 = it4->value.GetArray();
                                int key6 = 0;
                                for (Value::ConstValueIterator it5 = obj5.Begin(); it5 != obj5.End(); ++it5){
                                    const auto &obj6 = it5->GetObject();
                                    for (Value::ConstMemberIterator it6 = obj6.MemberBegin(); it6 != obj6.MemberEnd(); ++it6){
                                        string key7 = it6->name.GetString();
                                        const auto &obj7 = it6->value.GetArray();
                                        for (Value::ConstValueIterator it7 = obj7.Begin(); it7 != obj7.End(); ++it7){
                                            skips_s[key2][key3][key4][key5][key6][key7].push_back(it7->GetString());
                                        }
                                    }
                                    key6++;
                                }
                            }
                        }   
                    }
                }
            }

            c.skips = skips_s;
            if(instruction.HasMember("noten_context")){
                string noten_context_s = instruction["noten_context"].GetString();
                c.noten_context = noten_context_s;
            }

            // replace vars in skips
            for (auto &&[skip_k, skip_v] : c.skips)
            {
                for (auto &&[skipstop_k, skipstop_v] : skip_v)
                {
                    for (auto &&[team_k, team_v] : skipstop_v)
                    {
                        for (auto &&[is_k, is_v] : team_v)
                        {
                            for (size_t skipcond_k = 0; skipcond_k < is_v.size(); ++skipcond_k){
                                for (auto &&[val_k, val_v] : is_v[skipcond_k])
                                {
                                    if(val_k == "tid"){
                                        c.skips[skip_k][skipstop_k][team_k][is_k][skipcond_k]["typeId"] = val_v;
                                        c.skips[skip_k][skipstop_k][team_k][is_k][skipcond_k].erase(val_k);
                                    }else if(val_k == "qid"){
                                        c.skips[skip_k][skipstop_k][team_k][is_k][skipcond_k]["qualifierId"] = val_v;
                                        c.skips[skip_k][skipstop_k][team_k][is_k][skipcond_k].erase(val_k);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // create cpp node
            /*map<string, condition_t> cs;
            for (auto&& [name, condition] : c.conditions)
            {
                bool entry_opt_flag = false;
                vector<rule_t> opt_rules;
                vector<rule_t> rules;
                condition_t cond;
                cond.isEvent = isEventCondition(name, c.conditions, c.primary);
                vector<string> args = trim_explode(",", condition);
                for (string &arg : args)
                {
                    entry_opt_flag = false;
                    bool dont_add = false;
                    rule_t r;
                    // is a NOT condition
                    if (arg.find_first_of('!') == 0)
                    {
                        r.isNot = true;
                        arg.replace(0, 1, "");
                    }
                    string op = operands(arg);
                    r.op = op;
                    vector<string> sides = trim_explode(op, arg);
                    for (string &side : sides)
                    {
                        rule_side_t s;
                        int i = &side - &sides[0];
                        string which;
                        if (i == 0)
                            which = "left";
                        else
                            which = "right";
                        bool isNumeric = is_numeric(side);
                        if (!isNumeric && side.find_first_of('.') != string::npos)
                        {
                            // abstract
                            s.abstract = true;
                            s.numeric = false;
                            s.constant = false;
                            vector<string> broken_args = trim_explode(".", side);
                            if (broken_args.size() == 3)
                            {
                                // simplest identifier
                                s.event = broken_args[0];
                                s.qualifier = broken_args[1];
                                s.var = broken_args[2];
                                replacePredefinedVar(s.var);
                            }
                            else if (broken_args.size() == 2)
                            {
                                if (which == "left")
                                {
                                    // has to be qualifier
                                    s.event = name;
                                    s.qualifier = broken_args[0];
                                    s.var = broken_args[1];
                                    replacePredefinedVar(s.var);
                                }
                                else
                                {
                                    // could be both, primary qualifier variable value or event variable value
                                    if (isEventCondition(broken_args[0], c.conditions, c.primary))
                                    {
                                        s.event = broken_args[0];
                                        s.qualifier = "";
                                        s.var = broken_args[1];
                                        replacePredefinedVar(s.var);
                                    }
                                    else
                                    {
                                        s.event = c.primary;
                                        s.qualifier = broken_args[0];
                                        s.var = broken_args[1];
                                        replacePredefinedVar(s.var);
                                    }
                                }
                            }
                        }
                        else
                        {
                            s.abstract = false;
                            if (which == "left")
                            {
                                // has to be a variable on the condition name / event selector

                                // primary selector ?
                                string ps = primaryEventSelector(side);
                                s.var = side;
                                if(ps != ""){
                                    dont_add = true;
                                    cond.withinType = "steps";
                                    if(ps == "after"){
                                        cond.direction = 1;
                                        cond.iterator = "after";
                                        cond.times = stoi(sides[1]);
                                    }else if(ps == "before"){
                                        cond.direction = -1;
                                        cond.iterator = "before";
                                        cond.times = stoi(sides[1]);
                                    }else if(ps == "around"){
                                        cond.direction = 1;
                                        cond.iterator = "around";
                                        cond.times = 1; // around is always first match
                                        string temp = sides[1];
                                        if (temp.find("sec") != std::string::npos)
                                        {
                                            cond.withinType = "time";
                                            temp.replace(temp.find("sec"), 3, "");
                                        }
                                        else
                                        {
                                            if (temp.find("steps") != std::string::npos)
                                                temp.replace(temp.find("steps"), 5, "");
                                        }
                                        cond.within = stoi(temp);
                                    }else if(ps == "within"){
                                        string temp = sides[1];
                                        if (temp.find("sec") != std::string::npos)
                                        {
                                            cond.withinType = "time";
                                            temp.replace(temp.find("sec"), 3, "");
                                        }
                                        else
                                        {
                                            if (temp.find("steps") != std::string::npos)
                                                temp.replace(temp.find("steps"), 5, "");
                                        }
                                        cond.within = stoi(temp);
                                    }else if(ps == "skip"){
                                        cond.skips = trim_explode("|", sides[1]);
                                    }
                                    break;
                                }else{
                                    replacePredefinedVar(s.var);
                                    // check if we can put this condition first because it's the most restricting one
                                    if (s.var == "typeId" || s.var == "qualifierId")
                                    {
                                        entry_opt_flag = true;
                                        if (s.var == "typeId" && name == c.primary){
                                            if(op != "=")
                                                throw std::invalid_argument("invalid expression: primary typeId has to have a single constraint with '='");
                                            c.optTid = sides[1];
                                            dont_add = true;
                                        }
                                    }
                                }

                                s.constant = false;
                                s.event = cond.isEvent ? name : "";
                                s.qualifier = cond.isEvent ? "" : name;
                            }
                            else if (which == "right")
                            {
                                // has to be constant
                                s.numeric = isNumeric;
                                s.constant = true;
                                s.event = "";
                                s.qualifier = "";
                                s.var = side;
                            }
                        }
                        if (which == "left")
                        {
                            r.left = s;
                        }
                        else
                        {
                            r.right = s;
                        }
                    }
                    if(!dont_add){
                        if (entry_opt_flag)
                            opt_rules.push_back(r);
                        else
                            rules.push_back(r);
                    }
                }
                // merge opt rules
                opt_rules.insert(opt_rules.end(), rules.begin(), rules.end());
                cond.rules = opt_rules;
                cs[name] = cond;
            }
            c.cpp.conditions = cs;

            c.cpp.formula.logic = prep(c.formula);

            // eval formula
            // extract all elements
            string formula_temp = " " + c.formula + " ";
            while (formula_temp.find("(") != std::string::npos)
            {
                formula_temp.replace(formula_temp.find("("), 1, " ");
            }
            while (formula_temp.find(")") != std::string::npos)
            {
                formula_temp.replace(formula_temp.find(")"), 1, " ");
            }
            while (formula_temp.find(" and ") != std::string::npos)
            {
                formula_temp.replace(formula_temp.find(" and "), 5, " ");
            }
            while (formula_temp.find(" or ") != std::string::npos)
            {
                formula_temp.replace(formula_temp.find(" or "), 4, " ");
            }
            while (formula_temp.find(" not ") != std::string::npos)
            {
                formula_temp.replace(formula_temp.find(" not "), 5, " ");
            }
            clean(formula_temp, false);

            vector<string> formula_args = trim_explode(" ", formula_temp);
            vector<string> vec = formula_args;
            sort(vec.begin(), vec.end());
            vec.erase(unique(vec.begin(), vec.end()), vec.end());
            formula_args = vec;

            for (string &arg : formula_args){
                string op = operands(arg);
                if(op != ""){
                    // connected
                    formula_connected_t fct;
                    fct.op = op;
                    fct.query = arg;
                    vector<string> sides = trim_explode(op, arg);
                    vector<string> exp;
                    for (string &side : sides)
                    {
                        rule_side_t s;
                        int i = &side - &sides[0];
                        string which;
                        if (i == 0)
                            which = "left";
                        else
                            which = "right";

                        bool isNumeric = is_numeric(side);
                        if (!isNumeric && side.find_first_of('.') != string::npos)
                        {
                            // abstract
                            s.abstract = true;
                            s.numeric = false;
                            s.constant = false;
                            vector<string> broken_args = trim_explode(".", side);
                            if (broken_args.size() == 3)
                            {
                                // simplest identifier
                                s.event = broken_args[0];
                                s.qualifier = broken_args[1];
                                s.var = broken_args[2];
                                replacePredefinedVar(s.var);
                            }
                            else if (broken_args.size() == 2)
                            {
                                // could be both, primary qualifier variable value or event variable value
                                if (isEventCondition(broken_args[0], c.conditions, c.primary))
                                {
                                    s.event = broken_args[0];
                                    s.qualifier = "";
                                    s.var = broken_args[1];
                                    replacePredefinedVar(s.var);
                                }
                                else
                                {
                                    s.event = c.primary;
                                    s.qualifier = broken_args[0];
                                    s.var = broken_args[1];
                                    replacePredefinedVar(s.var);
                                }
                            }
                        }else{
                            s.numeric = isNumeric;
                            s.constant = true;
                            s.var = side;
                        }
                        if (which == "left")
                            fct.left = s;
                        else
                            fct.right = s;
                    }
                    c.cpp.formula.connected.push_back(fct);
                }else{
                    // simple
                    rule_side_t s;
                    if(arg.find(".") != string::npos){
                        vector<string> args = trim_explode(".", arg);
                        s.event = args[0];
                        s.qualifier = args[1];
                    }else{
                        if(isEventCondition(arg, c.conditions, c.primary)){
                            s.event = arg;
                        }else{
                            s.event = c.primary;
                            s.qualifier = arg;
                        }
                    }
                    c.cpp.formula.simple.push_back(s);
                }
            }

            // values
            map<string, string> values = instruction["values"];
            for(auto&& [key, value] : values){
                rule_side_t s;
                if(!is_numeric(value) && value.find(".") != string::npos){
                    auto args = trim_explode(".", value);
                    if (args.size() == 3)
                    {
                        s.event = args[0];
                        s.qualifier = args[1];
                        s.var = args[2];
                    }
                    else
                    {
                        s.event = isEventCondition(args[0], c.conditions, c.primary) ? args[0] : c.primary;
                        s.qualifier = isEventCondition(args[0], c.conditions, c.primary) ? "" : args[0];
                        s.var = args[1];
                    }
                }else{
                    // assume constant
                    s.var = value;
                    s.constant = true;
                }
                c.cpp.values[key] = s;
            }*/

            code_block.push_back(c);
        }
        code[i_event_name] = code_block;
    }
    return code2string(code);

    vector<event_t> events;
    map<string, vector<int>> eventsOpt;
    events.reserve(3000);
    /*for (auto &&[step, event_php] : events_json){
        event_t event;
        for (auto &&[k, v] : event_php){
            if(k != "qualifier"){
                string s = v;
                event.params[k] = s;
            }else{
                qualifier_t qualifier;
                for (auto &&[kq, vq] : event_php["qualifier"]){
                    for (auto &&[kqq, vqq] : vq){
                        string s = vqq;
                        qualifier.params[kqq] = s;
                    }
                    event.qualifier.push_back(qualifier);
                }
            }
        }
        event.time = strtotime(event.params["timeStamp"]);
        events.push_back(event);
        eventsOpt[event.params["typeId"]].push_back(step);
    }*/
    //return events2string(events);

    // type conversions complete / begin query
    // multithread loop
    // sep code
    vector<string> v_i_event_name;
    for (auto&& [i_event_name, code_blocks] : code){
        v_i_event_name.push_back(i_event_name);
    }

    #pragma omp parallel for
    for(int code_step = 0; code_step < code.size(); code_step++){
        string i_event_name = v_i_event_name[code_step];
        vector<instruction_t> &code_blocks = code[i_event_name];
        for (instruction_t &instruction : code_blocks)
        {
            for(int step : eventsOpt[instruction.optTid]){
                vector<string> it;
                q_t q;
                if (!testConditions(instruction.primary, instruction, events, step, step, q, it))
                    continue;
                // find all events to prevent senseless loop
                for(auto&& [name, condition] : instruction.cpp.conditions){
                    if(name == instruction.primary || !condition.isEvent)
                        continue;
                    findEvent(name, instruction, step, events, q, it);
                }
                // simple
                for(rule_side_t &side : instruction.cpp.formula.simple){
                    if (side.qualifier != "")
                    {
                        if(!q.eq[side.event].evals.count(side.qualifier)){
                            if(testEventQualifierConditions(side.event, side.qualifier, instruction, step, events, q, it)){
                                q.evals[side.event + "." + side.qualifier] = true;
                                if (instruction.primary == side.event)
                                    q.evals[side.qualifier] = true;
                            }else{
                                q.evals[side.event + "." + side.qualifier] = false;
                                if (instruction.primary == side.event)
                                    q.evals[side.qualifier] = false;
                            }
                        }else{
                            q.evals[side.event + "." + side.qualifier] = q.eq[side.event].evals.count(side.qualifier);
                            if (instruction.primary == side.event)
                                q.evals[side.qualifier] = q.eq[side.event].evals.count(side.qualifier);
                        }
                    }
                }
                // connected
                for(formula_connected_t &rule : instruction.cpp.formula.connected){
                    string left, right = "";
                    for(int i = 0; i < 2; i++){
                        rule_side_t side;
                        if (i == 0)
                            side = rule.left;
                        else
                            side = rule.right;
                        if(side.constant){
                            if(i == 0)
                                left = side.var;
                            else
                                right = side.var;
                        }else{
                            if (!q.eq.count(side.event))
                            {
                                q.evals[rule.query] = false;
                                break;
                            }
                            if (side.qualifier != "")
                            {
                                if(!q.eq[side.event].evals.count(side.qualifier)){
                                    if (!testEventQualifierConditions(side.event, side.qualifier, instruction, step, events, q, it))
                                    {
                                        q.evals[rule.query] = false;
                                        break;
                                    }
                                }
                            }
                            string temp;
                            if(side.qualifier != ""){
                                temp = q.eq[side.event].qualifier[side.qualifier].params[side.var];
                            }else{
                                temp = q.eq[side.event].event.params[side.var];
                            }
                            if (i == 0)
                                left = temp;
                            else
                                right = temp;
                        }
                    }
                    if(eval_with_op(left, right, rule.op)){
                        q.evals[rule.query] = true;
                    }else{
                        q.evals[rule.query] = false;
                    }
                }

                // moment of truth
                if(!instruction.cpp.formula.logic.count("_node_start"))
                    goto passed;
                for(vector<logic_gate_t> &and_node : instruction.cpp.formula.logic["_node_start"]){
                    for(logic_gate_t &gate : and_node){
                        if(!q.evals.count(gate.query)){
                            bool temp = eval(gate.query, instruction, q);
                            if(!temp && !gate.isNot || temp && gate.isNot){
                                goto cont_or;
                            }
                        }else{
                            if(!q.evals[gate.query] && !gate.isNot || q.evals[gate.query] && gate.isNot){
                                goto cont_or;
                            }
                        }
                    }
                    // passes
                    goto passed;
                    cont_or:;
                    continue;
                }
                continue;
                passed:;
                result_t temp;
                temp.event_type = i_event_name;
                temp.event_id = q.eq[instruction.primary].event.params["eventId"];
                temp.id = q.eq[instruction.primary].event.params["id"];
                temp.time = q.eq[instruction.primary].event.params["timeStamp"];
                temp.noten_context = instruction.noten_context;
                for(auto&& [key, val] : instruction.cpp.values){
                    if(val.constant){
                        temp.values[key] = val.var;
                    }else{
                        if(val.qualifier != ""){
                            temp.values[key] = q.eq[val.event].qualifier[val.qualifier].params[val.var];
                        }else{
                            temp.values[key] = q.eq[val.event].event.params[val.var];
                        }
                    }
                }
                m.lock();
                collection.push_back(temp);
                m.unlock();
            }
        }
    }
    std::sort(collection.begin(),
              collection.end(),
              [](const result_t &lhs, const result_t &rhs)
              {
                  return lhs.time < rhs.time || (lhs.time == rhs.time && lhs.id < rhs.id);
              });

    return result2json(collection);
    //return result2phpvalue(collection);

 }

int main(int argc, char** argv){

    /*string c_path = argv[1];
    string e_path = argv[2];*/

    FILE* fpc = fopen("code.txt", "rb"); // non-Windows use "r"
 
    char readBufferc[256];
    FileReadStream bisc(fpc, readBufferc, sizeof(readBufferc));
 
    AutoUTFInputStream<unsigned, FileReadStream> eisc(bisc);  // wraps bis into eis
 
    Document dc;         // Document is GenericDocument<UTF8<> > 
    dc.ParseStream<0, AutoUTF<unsigned> >(eisc); // This parses any UTF file into UTF-8 in memory
 
    fclose(fpc);

    /*StringBuffer bufferc;
    Writer<StringBuffer> writerc(bufferc);
    dc.Accept(writerc);
    string code = bufferc.GetString();*/

    FILE* fpe = fopen("file.txt", "rb"); // non-Windows use "r"
 
    char readBuffere[256];
    FileReadStream bise(fpe, readBuffere, sizeof(readBuffere));
 
    AutoUTFInputStream<unsigned, FileReadStream> eise(bise);  // wraps bis into eis
 
    Document de;         // Document is GenericDocument<UTF8<> > 
    de.ParseStream<0, AutoUTF<unsigned> >(eise); // This parses any UTF file into UTF-8 in memory
 
    fclose(fpe);

    /*StringBuffer buffere;
    Writer<StringBuffer> writere(buffere);
    de.Accept(writere);
    string events = buffere.GetString();*/

    cout << interpreter(dc, de);
}
//g++ -O3 -std=c++17 -fopenmp -o test main.cpp