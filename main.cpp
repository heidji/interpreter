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

#include <phpcpp.h>

using namespace std;
using namespace std::chrono;

string bool2str(bool x)
{
    return x ? "true" : "false";
}

string k(int c)
{
    string s;
    for (int i = 0; i < c; i++)
    {
        s += "\t";
    }
    return s;
}

struct qualifier_t
{
    map<string, string> params;
};

struct event_t
{
    map<string, string> params;
    qualifier_t qualifier;
};

struct rule_side_t
{
    string event = "";
    string qualifier = "";
    string var = "";
    bool abstract = false;
    bool constant = false;
    bool numeric = false;
    bool eventselector = false;

    string toString(int c = 0)
    {
        return k(c) + "event: " + event + "\n" + k(c) + "qualifier: " + qualifier + "\n" + k(c) + "var: " + var + "\n" + k(c) + "abstract: " +
               bool2str(abstract) + "\n" + k(c) + "constant: " + bool2str(constant) + "\n" + k(c) + "numeric: " + bool2str(numeric) + "\n" + k(c) + "eventselector: " + bool2str(eventselector);
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
    vector<rule_t> rules;

    string toString(int c = 0)
    {
        string s = k(c) + "isEvent: " + bool2str(isEvent) + "\n" + k(c) + "rules: [\n";
        for (rule_t rule : rules)
        {
            s += rule.toString(c + 1) + ",\n";
        }
        s += "\n" + k(c) + "]";
        return s;
    }
};

struct conditions_t
{
    map<string, condition_t> conditions;

    string toString(int c = 0)
    {
        string s = "";
        for (auto &&[key, v] : conditions)
        {
            s += k(c + 1) + key + ":{ \n" + v.toString(c + 2) + "\n" + k(c + 1) + "},\n";
        }
        return s;
    }
};

struct formula_connected_t
{
    string op = "";
    rule_side_t left;
    rule_side_t right;
};

struct logic_gate_t
{
    bool isNot = false;
    string query;
};

struct formula_t
{
    vector<vector<logic_gate_t>> logic;
    vector<rule_side_t> simple;
    vector<formula_connected_t> connected;
};

struct cpp_t
{
    formula_t formula;
    conditions_t conditions;

    string toString(int c = 0)
    {
        return /*k(c) + "formula: " + formula + "\n" +*/ k(c) + "conditions: {\n" + conditions.toString(c + 1) + "\n" + k(c + 1) + "}" + k(c) + "\n";
    }
};

struct instruction_t
{
    string primary = "";
    map<string, string> conditions;
    string formula = "";
    map<string, string> values;
    map<string, map<string, map<string, map<string, vector<map<string, vector<string>>>>>>> skips;
    cpp_t cpp;

    string toString(int c = 0)
    {
        string s;
        s = k(c) + "primary: " + primary + "\n" + k(c) + "formula: " + formula + "\n" + k(c) + "conditions: {\n";
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
                                for (string v7 : v6)
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

string code2string(map<string, vector<instruction_t>> m, int c = 0)
{
    string s;
    s = k(c) + "{\n";
    for (auto &&[key, v] : m)
    {
        s += k(c + 1) + key + ": " + "[\n";
        for (instruction_t i : v)
        {
            s += i.toString(c + 2) + ",\n";
        }
        s += "\n" + k(c + 1) + "]\n";
    }
    s += "\n" + k(c) + "}\n";
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
    stringstream secValue(date.substr(14, 2));
    int sec;
    secValue >> sec;

    // approximation for comparison purposes only
    return ((((year * 12 + month) * 30 + day) * 24 + hour) * 60 + min) * 60 + sec;
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
    for (string s_or : or_exp)
    {
        auto and_exp = explode("and", s_or);
        res = true;
        for (string s_and : and_exp)
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
        for (auto elem : str)
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

bool eval(string str)
{

    if (str == "")
        return true;

    clean(str);

    while (str.find("(") != string::npos)
    {
        // find last "("
        size_t pos_start = str.find_last_of("(");
        // find the following ")"
        size_t pos_end = str.find(")", pos_start + 1);
        if (pos_end == string::npos)
            throw std::invalid_argument("invalid expression: missing closing braces");
        str.replace(pos_start, pos_end - pos_start + 1, " " + eval_exp(str.substr(pos_start + 1, pos_end - pos_start - 1) + " "));
        clean(str);
    }

    return eval_exp(str) == "true";
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
    if (is_numeric(left) && is_numeric(right))
    {
        l = ::atof(left.c_str());
        r = ::atof(right.c_str());
        if (op == "=")
            return l == r;
        else if (op == "!=")
            return l != r;
        else if (op == ">")
            return l > r;
        else if (op == "<")
            return l < r;
        else if (op == ">=")
            return l >= r;
        else if (op == "<=")
            return l <= r;
    }
    else
    {
        if (op == "=")
            return left == right;
        else if (op == "!=")
            return left != right;
        else if (op == ">")
            return left > right;
        else if (op == "<")
            return left < right;
        else if (op == ">=")
            return left >= right;
        else if (op == "<=")
            return left <= right;
    }
    return false;
}

bool in_array(string needle, vector<string> haystack)
{
    return count(haystack.begin(), haystack.end(), needle);
}

bool isEventCondition(string name, map<string, string> conditions, string primary)
{
    if (name == primary)
        return true;
    vector<string> primary_event_selectors = {"before", "after", "around", "skip"};
    string temp = conditions[name];

    for (string selector : primary_event_selectors)
    {
        if (temp.find(selector) != std::string::npos)
            return true;
    }
    return false;
}

// declare
//bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it);

bool looper(int which, int s, int times, int j, int end)
{
    if (which == 1)
        return (s <= times && j <= end);
    else
        return (s <= times && j >= end);
}

/*bool findEvent(string &name, Php::Value &conditions, Php::Value &skipsets, int primary_index, int index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it)
{

    if (in_array(name, it))
        throw std::invalid_argument("circular reference");
    it.push_back(name);

    int step = primary_index;
    int within = 0, times = 0;
    int start = 0, direction = 0, end = 0;
    int i = 0;
    int j = 0;
    int s = 0;
    Php::Value skips;

    string selector, within_type, skipstop;
    Php::Value event = events[primary_index];

    string condition = conditions[name];

    vector<string> primary_event_selectors = {"before", "after", "around"};
    for (string s : primary_event_selectors)
    {
        if (condition.find(s) != std::string::npos)
        {
            selector = s;
            break;
        }
    }

    Php::Value rules = trim_explode(",", condition);

    for (auto &&[pos, rule_p] : rules)
    {
        string rule = rule_p;
        if (rule.find(selector) != std::string::npos)
        {
            if (rule.find("before") != std::string::npos)
                direction = -1;
            else
                direction = 1;
            Php::Value temp = trim_explode("=", rule);
            // TODO: throw error if not present
            if (selector != "around")
            {
                int times_t = temp[1];
                times = times_t;
            }
            else
            {
                times = 1; // in around there's no xth match, always first
                if (rule.find("sec") != std::string::npos)
                {
                    within_type = "time";
                    rule.replace(rule.find("sec"), 3, "");
                }
                else
                {
                    within_type = "steps";
                    if (rule.find("steps") != std::string::npos)
                        rule.replace(rule.find("steps"), 5, "");
                }
                Php::Value args = trim_explode("=", rule);
                within = args[1];
            }
            rules[pos] = NULL;
            continue;
        }

        if (rule.find("within") != std::string::npos)
        {
            if (selector != "around")
            {
                if (rule.find("sec") != std::string::npos)
                {
                    within_type = "time";
                    rule.replace(rule.find("sec"), 3, "");
                }
                else
                {
                    within_type = "steps";
                    if (rule.find("steps") != std::string::npos)
                        rule.replace(rule.find("steps"), 5, "");
                }
                Php::Value args = trim_explode("=", rule);
                within = args[1];
            }
            rules[pos] = NULL;
            continue;
        }
        if (rule.find("skip") != std::string::npos)
        {
            skips = trim_explode("|", trim_explode("=", rule)[1]);
            rules[pos] = NULL;
        }
    }

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
            int temp_a = strtotime(event["timeStamp"]);
            int temp_b = strtotime(events[end + direction]["timeStamp"]);
            int temp_c = strtotime(events[end]["timeStamp"]);
            while (events[end] != NULL && abs(temp_a - temp_b) <= within)
            {
                end += direction;
            }
            // make sure the first step wasn't already too far
            if (events[end] != NULL && abs(temp_a - temp_c) > within)
                start = step;
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

    /*Php::Value lol;
    lol[0] = which;
    lol[1] = conditions;
    lol[3] = skips;
    lol[4] = index;
    lol[5] = primary_index;
    lol[6] = q;
    lol[7] = primary;
    lol[8] = all_i;
    lol[9] = it;
    lol[10] = events;
    q = lol;
    return false;

    if (start == step || direction == 1 && start > end || direction == -1 && start < end)
    {
        q[name] = -1;
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

        string tid_t = events[i]["typeId"];
        if (tid_t == "43")
        {
            end += direction;
            if (end > events.size() - 1)
                end = events.size() - 1;
            else if (end < 0)
                end = 0;
            continue;
        }

        // check if skip event
        // TODO SKIPS
        // TODO SKIPS

        if (testConditions(name, conditions, skipsets, i, primary_index, events, q, primary, all_i, it))
            s++;
        if (s >= times && q[name] != false)
        {
            return true;
        }
    }

    q[name] = -1;
    return false;
}*/

/*bool testEventQualifierConditions(string &name, string &qname, string primary, Php::Value &conditions, Php::Value &q)
{
    Php::Value qualifier = q[name]["qualifier"];
    Php::Value args = trim_explode(",", conditions[qname]);

    for (auto &&[i, qual] : qualifier)
    {
        for (auto &&[ii, arg] : args)
        {
            string arg_s = arg;
            string op = operands(arg_s);
            Php::Value sides = trim_explode(op, arg_s);
            string left_t = sides[0];
            if (left_t == "qid")
                left_t = "qualifierId";
            string left = qual[left_t];
            string right = sides[1];
            if (!eval_with_op(left, right, op))
            {
                goto cnt;
            }
        }
        // passed all wow
        q[name + "." + qname] = qual;
        if (name == primary)
            q[qname] = qual;
        return true;
    cnt:;
    }
    q[name + "." + qname] = -1;
    if (name == primary)
        q[qname] = -1;
    return false;
}

string getAbstractValue(string name, string query, Php::Value &conditions, Php::Value &skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it, bool isLeft = true)
{
    Php::Value sides = trim_explode(".", query);

    string fquery;
    string qname, var;

    if (sides.size() == 3)
    {
        string name_t = sides[0];
        name = name_t;
        string qname_t = sides[1];
        qname = qname_t;
        string var_t = sides[2];
        var = var_t;
        fquery = name + "." + qname;
        if (q[name + "." + qname] == NULL)
        {
            if (q[name] == NULL)
            {
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if (q[name] == -1)
            {
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }
            else
            {
                if (q[name + "." + qname] == NULL)
                {
                    if (!testEventQualifierConditions(name, qname, primary, conditions, q))
                    {
                        return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                    }
                    // additional setter for future evals
                    // additional setter for future evals
                    if (name == primary)
                    {
                        Php::Value xd;
                        xd = q[name + "." + qname];
                        q[qname] = xd;
                    }
                }
                else if (q[name + "." + qname] == -1)
                {
                    return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                }
            }
        }
        else if (q[name + "." + qname] == -1)
        {
            return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
        }
    }
    else
    {
        string qname_t = sides[0];
        qname = qname_t;
        string var_t = sides[1];
        var = var_t;

        if (isEventCondition(qname, conditions, primary))
        {
            name = qname;
            fquery = name;

            if (q[name] == NULL)
            {
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if (q[name] == -1)
            {
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }
        }
        else
        {
            // is qualifier
            if (!isLeft)
                name = primary;

            fquery = name + "." + qname;

            if (q[name] == NULL)
            {
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if (q[name] == -1)
            {
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }
            else
            {
                if (q[name + "." + qname] == NULL)
                {
                    if (!testEventQualifierConditions(name, qname, primary, conditions, q))
                    {
                        return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                    }
                    // additional setter for future evals
                    if (name == primary)
                    {
                        Php::Value xd;
                        xd = q[name + "." + qname];
                        q[qname] = xd;
                    }
                }
                else if (q[name + "." + qname] == -1)
                {
                    return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                }
            }
        }
    }
    if (var.find("tid") != std::string::npos)
        var.replace(var.find("tid"), 3, "typeId");
    if (var.find("qid") != std::string::npos)
        var.replace(var.find("qid"), 3, "qualifierId");
    string r = q[fquery][var];
    return r;
}

bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it)
{

    string left, right, op, q_op;
    Php::Value args, q_args, sides, q_sides, cond, q_cond, qual, q_qual;

    /*Php::Value lol;
    lol[0] = name;
    lol[1] = conditions;
    lol[3] = skips;
    lol[4] = index;
    lol[5] = primary_index;
    lol[6] = q;
    lol[7] = primary;
    lol[8] = all_i;
    lol[9] = it;
    lol[10] = events;
    q = lol;
    return false;

    vector<string> primary_event_selectors = {"before", "after", "around", "skip", "within"};

    cond = trim_explode(",", conditions[name]);

    // put the event in q so we can use it onwards
    q[name] = events[index];

    for (auto &&[i, t_condition] : cond)
    {
        // check if its not a predefined variable
        string condition = t_condition;
        bool found = false;
        for (string selector : primary_event_selectors)
        {
            if (condition.find(selector) != std::string::npos)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        op = operands(condition);
        sides = trim_explode(op, condition);
        // NOT condition
        string side_s = sides[0];
        if (side_s.find_first_of('!') == 0)
        {
            // has to be a qualifier identifier
            Php::Value temp = trim_explode(".", side_s);
            string neg_qual = temp[0];
            neg_qual.erase(0, 1);
            if (testEventQualifierConditions(name, neg_qual, primary, conditions, q))
            {
                string left_t = temp[1];
                string right = sides[0];
                string left = q[name + "." + neg_qual][left_t];
                // TODO add more cases for abstract right hand side stuff (probably pointless / but theres a function for it)
                if (!eval_with_op(left, right, op))
                {
                    q[name] = -1;
                    return false;
                }
            }
        }
        else
        {
            // abstract
            string left;
            string right;
            if (!is_numeric(side_s) && side_s.find_first_of('.') != std::string::npos)
            {
                string left_t = getAbstractValue(name, side_s, conditions, skips, index, primary_index, events, q, primary, all_i, it, false);
                left = left_t;
                if (left_t == "Q NOT SET / VALUE NOT FOUND")
                {
                    q[name] = -1;
                    return false;
                }
                string right_s = sides[1];
                if (!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos)
                {
                    string right_st = getAbstractValue(name, right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                    right = right_st;
                    if (right == "Q NOT SET / VALUE NOT FOUND")
                    {
                        q[name] = -1;
                        return false;
                    }
                }
                else
                {
                    right = right_s;
                }
            }
            else
            {
                if (side_s.find("tid") != std::string::npos)
                    side_s.replace(side_s.find("tid"), 3, "typeId");
                string left_t = q[name][side_s];
                left = left_t;
                // check if right side another abstract value
                string right_s = sides[1];
                if (!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos)
                {
                    string right_st = getAbstractValue(name, right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it, false);
                    right = right_st;
                    if (right == "Q NOT SET / VALUE NOT FOUND")
                    {
                        q[name] = -1;
                        return false;
                    }
                }
                else
                {
                    right = right_s;
                }
            }

            if (!eval_with_op(left, right, op))
            {
                q[name] = -1;
                return false;
            }
        }
    }
    // passed all checks
    return true;
}*/

bool isPrimaryEventSelector(string arg)
{
    vector<string> primary_event_selectors = {"before", "after", "around", "skip", "within"};
    for (string x : primary_event_selectors)
    {
        if (arg == x)
            return true;
    }
    return false;
}

void replacePredefinedVar(string &var)
{
    if (var == "tid")
        var = "typeId";
    if (var == "qid")
        var = "qualifierId";
}

Php::Value interpreter(Php::Parameters &params)
{
    /*struct event e;
    e.params = {{"k", "l"}, {"k", "l"}};
    e.qualifier = {{{"k", "l"}, {"k", "l"}}, {{"k", "l"}, {"k", "l"}}, {{"k", "l"}, {"k", "l"}}};
    return e.qualifier[2]["k"];*/
    /*std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    Php::Value lul = xdd;
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();*/

    auto code_php = params[0];
    auto events = params[1];

    // normalize code object
    map<string, vector<instruction_t>> code;

    for (auto &&[i_event_name, code_blocks] : code_php)
    {
        vector<instruction_t> code_block;
        for (auto &&[block, instruction] : code_blocks)
        {
            // get
            struct instruction_t c;
            if (instruction["primary"] != "")
            {
                string primary_s = instruction["primary"];
                c.primary = primary_s;
            }
            map<string, string> conditions_s = instruction["conditions"];
            c.conditions = conditions_s;
            string formula_s = instruction["formula"];
            c.formula = formula_s;
            map<string, string> values_s = instruction["values"];
            c.values = values_s;
            map<string, map<string, map<string, map<string, vector<map<string, vector<string>>>>>>> skips_s = instruction["skips"];
            c.skips = skips_s;
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
            conditions_t cs;
            for (auto const &[name, condition] : c.conditions)
            {
                condition_t cond;
                cond.isEvent = isEventCondition(name, c.conditions, c.primary);
                vector<string> args = trim_explode(",", condition);
                for (string arg : args)
                {
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
                                s.eventselector = isPrimaryEventSelector(side);
                                s.constant = false;
                                s.event = cond.isEvent ? name : "";
                                s.qualifier = cond.isEvent ? "" : name;
                                s.var = side;
                                replacePredefinedVar(s.var);
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
                    cond.rules.push_back(r);
                }
                cs.conditions[name] = cond;
            }
            c.cpp.conditions = cs;
            code_block.push_back(c);
        }
        code[i_event_name] = code_block;
    }

    return code2string(code);

    /*Php::Value collection;

    vector<string> all_i, it;

    for (auto &&[step_t, event] : events)
    {
        int step = step_t;
        for (auto &&[i_event_name, code_blocks] : code)
        {
            for (auto &&[block, instruction] : code_blocks)
            {
                Php::Value q;
                all_i = {};
                string primary;
                Php::Value xd = instruction["conditions"];
                for (auto &&[k, v] : xd)
                {
                    all_i.push_back(k);
                }
                string t_p = instruction["primary"];
                if (!t_p.empty())
                    primary = t_p;
                else
                    primary = all_i[0];
                vector<string> it;
                Php::Value conditions = instruction["conditions"];
                Php::Value skips;
                if (instruction["skips"] != NULL)
                    skips = instruction["skips"];
                else
                    skips = NULL;
                if (!testConditions(primary, conditions, skips, step, step, events, q, primary, all_i, it))
                    continue;
                // eval formula
                string formula = instruction["formula"];
                // extract all elements
                string formula_t = " " + formula + " ";
                while (formula_t.find("(") != std::string::npos)
                {
                    formula_t.replace(formula_t.find("("), 1, " ");
                }
                while (formula_t.find(")") != std::string::npos)
                {
                    formula_t.replace(formula_t.find(")"), 1, " ");
                }
                while (formula_t.find(" and ") != std::string::npos)
                {
                    formula_t.replace(formula_t.find(" and "), 5, " ");
                }
                while (formula_t.find(" or ") != std::string::npos)
                {
                    formula_t.replace(formula_t.find(" or "), 4, " ");
                }
                while (formula_t.find(" not ") != std::string::npos)
                {
                    formula_t.replace(formula_t.find(" not "), 5, " ");
                }
                clean(formula_t, false);

                Php::Value formula_args = trim_explode(" ", formula_t);

                for (auto &&[i, arg_t] : formula_args)
                {
                    string arg = arg_t;
                    string op = operands(arg);
                    if (op.empty())
                    {
                        // normal argument
                        if (q[arg] == NULL)
                        {
                            if (arg.find(".") != std::string::npos)
                            {
                                Php::Value parts = trim_explode(".", arg);
                                string part_1 = parts[0];
                                string part_2 = parts[1];
                                if (q[part_1] == NULL)
                                {
                                    findEvent(part_1, conditions, skips, step, step, events, q, primary, all_i, it);
                                }
                                if (q[part_1] == -1)
                                {
                                    q[arg] = -1;
                                }
                                else
                                {
                                    testEventQualifierConditions(part_1, part_2, primary, conditions, q);
                                }
                            }
                            else
                            {
                                if (isEventCondition(arg, conditions, primary))
                                {
                                    findEvent(arg, conditions, skips, step, step, events, q, primary, all_i, it);
                                    string xD = q[primary]["id"];
                                    if (xD == "2227838703")
                                    {
                                        return q;
                                    }
                                }
                                else
                                {
                                    // only a qualifier
                                    testEventQualifierConditions(primary, arg, primary, conditions, q);
                                }
                            }
                        }
                    }
                    else
                    {
                        // complex argument with operand
                        string ev1, var1, ev2, var2;
                        Php::Value sides = trim_explode(op, arg);
                        string left = sides[0];
                        string right = sides[1];
                        // left has to be conjugated, right doesn't

                        Php::Value parts = trim_explode(".", left);
                        if (parts.size() == 2)
                        {
                            string left_t = parts[0];
                            string t1 = parts[1];
                            ev1 = left_t;
                            var1 = t1;
                            if (q[ev1] == NULL)
                            {
                                if (isEventCondition(ev1, conditions, primary))
                                {
                                    findEvent(ev1, conditions, skips, step, step, events, q, primary, all_i, it);
                                }
                                else
                                {
                                    // only a qualifier
                                    testEventQualifierConditions(primary, ev1, primary, conditions, q);
                                }
                            }
                        }
                        else
                        { // 3
                            string left_t = parts[0];
                            string left_tt = parts[1];
                            string t1 = parts[2];
                            ev1 = left_t + "." + left_tt;
                            var1 = t1;
                            if (q[left_t] == NULL)
                            {
                                // only a qualifier
                                testEventQualifierConditions(left_t, left_tt, primary, conditions, q);
                            }
                        }

                        if (!is_numeric(right) && right.find(".") != std::string::npos)
                        {
                            Php::Value parts = trim_explode(".", right);
                            if (parts.size() == 2)
                            {
                                string right_t = parts[0];
                                string t2 = parts[1];
                                ev2 = right_t;
                                var2 = t2;
                                if (q[ev2] == NULL)
                                {
                                    if (isEventCondition(ev2, conditions, primary))
                                    {
                                        findEvent(ev2, conditions, skips, step, step, events, q, primary, all_i, it);
                                    }
                                    else
                                    {
                                        // only a qualifier
                                        testEventQualifierConditions(primary, ev2, primary, conditions, q);
                                    }
                                }
                            }
                            else
                            { // 3
                                string right_t = parts[0];
                                string right_tt = parts[1];
                                string t2 = parts[2];
                                ev2 = right_t + "." + right_tt;
                                var2 = t2;
                                if (q[right_t] == NULL)
                                {
                                    // only a qualifier
                                    testEventQualifierConditions(right_t, right_tt, primary, conditions, q);
                                }
                            }
                        }
                        else
                        {
                            var2 = right;
                        }
                        if (var1 == "tid")
                            var1 = "typeId";
                        else if (var1 == "qid")
                            var1 = "qualifierId";
                        string l = q[ev1][var1];
                        string r;
                        if (!ev2.empty())
                        {
                            if (var2 == "tid")
                                var2 = "typeId";
                            else if (var2 == "qid")
                                var2 = "qualifierId";
                            string t = q[ev2][var2];
                            r = t;
                        }
                        else
                        {
                            r = var2;
                        }
                        q[arg] = eval_with_op(l, r, op);
                    }
                }

                // sort by length so we dont accidentally replace wrong values
                Php::Value sorted_args;
                int i = 0;

                // unique
                vector<string> vec = formula_args;
                sort(vec.begin(), vec.end());
                vec.erase(unique(vec.begin(), vec.end()), vec.end());
                formula_args = vec;

                while (sorted_args.size() < formula_args.size())
                {
                    int s = 0;
                    string max = "";
                    for (auto &&[x, arg_t] : formula_args)
                    {
                        string arg = arg_t;
                        if (in_array(arg, sorted_args))
                            continue;
                        if (arg.length() > s)
                        {
                            max = arg;
                            s = arg.length();
                        }
                    }
                    sorted_args[i] = max;
                    i++;
                }

                for (auto &&[x, arg_t] : sorted_args)
                {
                    string arg = arg_t;
                    if (arg == "")
                        continue;
                    if (q[arg] == -1 || q[arg] == NULL)
                    {
                        while (formula.find(arg) != std::string::npos)
                        {
                            formula.replace(formula.find(arg), arg.length(), "false");
                        }
                    }
                    else
                    {
                        while (formula.find(arg) != std::string::npos)
                        {
                            formula.replace(formula.find(arg), arg.length(), "true");
                        }
                    }
                }

                // moment of truth
                if (eval(formula))
                {
                    Php::Value temp;
                    temp["event_type"] = i_event_name;
                    string t1 = q[primary]["eventId"];
                    temp["event_id"] = t1;
                    string t2 = q[primary]["id"];
                    temp["id"] = t2;
                    string t3 = q[primary]["timeStamp"];
                    temp["time"] = t3;
                    if (instruction["noten_context"] == NULL)
                    {
                        temp["noten_context"] = "";
                    }
                    else
                    {
                        temp["noten_context"] = instruction["noten_context"];
                    }
                    for (auto &&[key_t, items] : instruction["values"])
                    {
                        string key = key_t;
                        Php::Value splits = trim_explode("|", items);
                        for (auto &&[x, value_t] : splits)
                        {
                            string value = value_t;
                            if (!is_numeric(value) && value.find(".") != std::string::npos)
                            {
                                int find = value.find_last_of('.');
                                string value_part = value.substr(find + 1, value.length() - find);
                                if (value_part == "tid")
                                {
                                    value_part = "typeId";
                                }
                                else if (value_part == "qid")
                                {
                                    value_part = "qualifierId";
                                }
                                value = value.substr(0, find);
                                if (q[value] != NULL)
                                {
                                    string t = q[value][value_part];
                                    temp["values"][key] = t;
                                    // convert timeStamp to PHP readable
                                    if (value_part == "timeStamp" && t != "")
                                    {
                                        temp["values"][key] = t.substr(0, 10) + " " + t.substr(11, 8);
                                    }
                                    // break when 1st value is found
                                    if (t != "")
                                        break;
                                }
                                if (temp["values"][key] == NULL)
                                    temp["values"][key] = "";
                            }
                            else
                            {
                                // assume constant
                                temp["values"][key] = value;
                            }
                        }
                    }
                    collection[collection.size()] = temp;
                    // break the code block iteration
                    break;
                }
                //return collection;
            }
        }
    }
    return collection;*/
}

// Symbols are exported according to the "C" language
extern "C"
{
    // export the "get_module" function that will be called by the Zend engine
    PHPCPP_EXPORT void *get_module()
    {
        // create extension
        static Php::Extension extension("interpreter", "1.0");

        // add function, with defined numeric parameters, to extension
        extension.add<interpreter>("interpreter");

        // return the extension module
        return extension.module();
    }
}
