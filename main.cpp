#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>

#include <phpcpp.h>

using namespace std;
using namespace std::chrono;

struct event {
    map<string, string> params;
    vector<map<string, string>> qualifier;
};

bool is_numeric(string s){
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
    return ((((year*12+month)*30+day)*24+hour)*60+min)*60+sec;
}

string trim(string str){
    while(str.compare(0,1," ")==0)
        str.erase(str.begin()); // remove leading whitespaces
    while(str.size()>0 && str.compare(str.size()-1,1," ")==0)
        str.erase(str.end()-1); // remove trailing whitespaces

    return str;
}

vector<string> explode(string delim, string s)
{
    vector<string> result;

    if(s.empty())
        return result;

    int temp;
    while (s.find(delim) != string::npos && !delim.empty())
    {
        temp = s.find(delim);
        result.push_back(s.substr(0, temp));
        s.replace(0, temp+delim.length(), "");
    }
    result.push_back(s);

    return result;
}

Php::Value trim_explode(string delim, string s)
{
    Php::Value res;
    vector<string> result;

    if(s.empty())
        return res;

    int temp;
    while (s.find(delim) != string::npos && !delim.empty())
    {
        temp = s.find(delim);
        result.push_back(trim(s.substr(0, temp)));
        s.replace(0, temp+delim.length(), "");
    }
    result.push_back(trim(s));
    res = result;

    return res;
}

string eval_exp(string str){
    bool res;
    auto or_exp = explode("or", str);
    for(string s_or : or_exp){
        auto and_exp = explode("and", s_or);
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
    str = trim(str);

    // remove excess whitespaces
    size_t pos = str.find("  ");
    while(pos != string::npos) {
        str.replace(pos, 2, " " );
        pos = str.find("  ");
    }
}

bool eval(string str){

    if(str == "")
        return true;

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

    return eval_exp(str) == "true";
}

string operands(string str){
    if(str.find("!=") != string::npos){
        return "!=";
    }else if(str.find(">=") != string::npos){
        return ">=";
    }else if(str.find("<=") != string::npos){
        return "<=";
    }else if(str.find("=") != string::npos){
        return "=";
    }else if(str.find(">") != string::npos){
        return ">";
    }else if(str.find("<") != string::npos){
        return "<";
    }
    return "";
};

bool eval_with_op(string left, string right, string op){
    // check null string
    double l, r;
    string temp = right;
    clean(temp);
    if(temp == "null")
        right = ""; // no null in c++ :(
    if(is_numeric(left) && is_numeric(right)){
        l = ::atof(left.c_str());
        r = ::atof(right.c_str());
        if(op == "=")
            return l == r;
        else if(op == "!=")
            return l != r;
        else if(op == ">")
            return l > r;
        else if(op == "<")
            return l < r;
        else if(op == ">=")
            return l >= r;
        else if(op == "<=")
            return l <= r;
    }else{
        if(op == "=")
            return left == right;
        else if(op == "!=")
            return left != right;
        else if(op == ">")
            return left > right;
        else if(op == "<")
            return left < right;
        else if(op == ">=")
            return left >= right;
        else if(op == "<=")
            return left <= right;
    }
    return false;
}

bool in_array(string needle, vector<string> haystack){
    return count(haystack.begin(), haystack.end(), needle);
}

bool isEventCondition(string &name, Php::Value &conditions, string primary){
    if(name == primary)
        return true;
    vector<string> primary_event_selectors = {"before", "after", "around", "skip"};
    string temp = conditions[name];

    for (string selector : primary_event_selectors){
        if(temp.find(selector) != std::string::npos)
            return true;
    }
    return false;
}

// declare
bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it);

bool looper(int which, int s, int times, int j, int end){
    if(which == 1)
        return (s <= times && j <= end);
    else
        return (s <= times && j >= end);
}

bool findEvent(string &name, Php::Value &conditions, Php::Value &skipsets, int primary_index, int index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){

    if(in_array(name, it))
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
    for (string s : primary_event_selectors){
        if(condition.find(s) != std::string::npos){
            selector = s;
            break;
        }
    }

    Php::Value rules = trim_explode(",", condition);

    for (auto&& [pos, rule_p] : rules){
        string rule = rule_p;
        if (rule.find(selector) != std::string::npos) {
            if (rule.find("before") != std::string::npos)
                direction = -1;
            else
                direction = 1;
            Php::Value temp = trim_explode("=", rule);
            // TODO: throw error if not present
            if(selector != "around"){
                int times_t = temp[1];
                times = times_t;
            }else {
                times = 1; // in around there's no xth match, always first
                if (rule.find("sec") != std::string::npos) {
                    within_type = "time";
                    rule.replace(rule.find("sec"), 3, "");
                } else {
                    within_type = "steps";
                    rule.replace(rule.find("steps"), 5, "");
                }
                Php::Value args = trim_explode("=", rule);
                within = args[1];
            }
            rules[pos] = NULL;
            continue;
        }

        if (rule.find("within") != std::string::npos) {
            if(selector != "around"){
                if (rule.find("sec") != std::string::npos) {
                    within_type = "time";
                    rule.replace(rule.find("sec"), 3, "");
                } else {
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
        if(rule.find("skip") != std::string::npos){
            skips = trim_explode("|", trim_explode("=", rule)[1]);
            rules[pos] = NULL;
        }
    }

    start = step + direction;

    if (within != 0) {
        if (within_type == "steps") {
            end = step + (within * direction);
        } else {
            end = start;
            int temp_a = strtotime(event["timeStamp"]);
            int temp_b = strtotime(events[end+direction]["timeStamp"]);
            int temp_c = strtotime(events[end]["timeStamp"]);
            while (events[end] != NULL && abs(temp_a - temp_b) <= within){
                end += direction;
            }
            // make sure the first step wasn't already too far
            if(events[end] != NULL && abs(temp_a - temp_c) > within)
                start = step;
        }
    } else {
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

    if (start == step || direction == 1 && start > end || direction == -1 && start < end) {
        q[name] = false;
        return false;
    }
    int around_alternator = 1;

    for (j = start; looper(which, s, times, j, end); j += direction) {
        if(selector == "around") {
            i = ((j-start) * around_alternator)+start;
            if(around_alternator == 1){
                j--; // skip a beat
            }
            around_alternator = -around_alternator;
            if(i < 0 || i > end)
                continue;
        }else
            i = j;

        string tid_t = events[i]["typeId"];
        if(tid_t == "43"){
            end += direction;
            if (end > events.size() - 1)
                end = events.size() - 1;
            else if (end < 0)
                end = 0;
            continue;
        }
        // check if skip event
        if(skips != NULL){
            bool xdo = false;
            for (auto&& [n, skip_t] : skips){
                string skip = skip_t;
                Php::Value skipset = skipsets[skip];
                for (string skipstop_t : {"stop", "skip", "count"}){
                    skipstop = skipstop_t;
                    for (string team : {"team", "teamvs"}){
                        Php::Value context;
                        context["team"] = skipset[skipstop][team] != NULL ? team : NULL;
                        string context_team = context["team"];
                        context["is"] = skipset[skipstop][context_team]["is"] != NULL ? "is" : (skipset[skipstop][context_team]["is_not"] != NULL ? "is_not" : NULL);
                        string context_is = context["is"];
                        string p_contestantId = q[primary]["contestantId"];
                        string e_contestantId = events[i]["contestantId"];
                        if(!context_team.empty() && (context_team == "team" && p_contestantId == e_contestantId || context_team == "teamvs" && p_contestantId != e_contestantId)) {
                            bool hack3;
                            if(context_is == "is")
                                xdo = false;
                            else
                                xdo = true;
                            Php::Value skipset_st = skipset[skipstop][context_team][context_is];
                            for (auto&& [x, ruleset] : skipset_st){
                                bool hack2 = false;;
                                for (auto&& [var_t, items] : ruleset){
                                    bool hack = false;
                                    string var = var_t;
                                    if(!items.isArray()){
                                        Php::Value temp_items;
                                        temp_items[0] = items;
                                        Php::Value items = temp_items;
                                    }
                                    for (auto&& [xx, value_t] : items){
                                        string value = value_t;
                                        if(var == "tid")
                                            var = "typeId";
                                        if(!in_array(var, {"qid", "qualifierId"})){
                                            string evar_t = events[i][var];
                                            if(evar_t == value){
                                                hack = true;
                                                break;
                                            }
                                        }else{
                                            Php::Value eq_t = events[i]["qualifier"];
                                            for (auto&& [xx, qualifier] : eq_t){
                                                string qualid_t = qualifier["qualifierId"];
                                                if(qualid_t == value){
                                                    hack = true;
                                                    break;
                                                }
                                            }
                                            if(hack)
                                                break;
                                        }
                                    }
                                    if(hack)
                                        continue;
                                    // no match
                                    Php::Value skipset_t = skipset[skipstop][context_team][context_is];
                                    if(x == skipset_t.size()-1){
                                        goto finish;
                                    }else{
                                        hack2 = true;
                                    }
                                    if(hack2){
                                        break;
                                    }
                                }
                                if(hack2)
                                    continue;
                                if(context_is == "is"){
                                    // found match -> skip
                                    xdo = true;
                                    goto finish;
                                }else{
                                    // found match -> skip
                                    xdo = false;
                                    hack3 = true;
                                }
                                if(hack3)
                                    break;
                            }
                        }
                    }
                }
            }
            finish:;
            if(xdo){
                if(skipstop == "stop"){
                    q[name] = false;
                    return false;
                }else if(skipstop == "skip") {
                    end += direction;
                    if (end > events.size() - 1)
                        end = events.size() - 1;
                    else if (end < 0)
                        end = 0;
                    continue;
                }else if(skipstop == "count"){
                    s++;
                    continue;
                }
            }
        }

        if(testConditions(name, conditions, skipsets, i, primary_index, events, q, primary, all_i, it))
            s++;
        if (s >= times && q[name] != false) {
            return true;
        }
    }

    q[name] = false;
    return false;
}

bool testEventQualifierConditions(string &name, string &qname, string primary, Php::Value &conditions, Php::Value &q){
    Php::Value qualifier = q[name]["qualifier"];
    Php::Value args = trim_explode(",", conditions[qname]);

    for (auto&& [i, qual] : qualifier){
        for (auto&& [ii, arg] : args){
            string arg_s = arg;
            string op = operands(arg_s);
            Php::Value sides = trim_explode(op, arg_s);
            string left_t = sides[0];
            if(left_t == "qid")
                left_t = "qualifierId";
            string left = qual[left_t];
            string right = sides[1];
            if(!eval_with_op(left, right, op)){
                goto cnt;
            }
        }
        // passed all wow
        q[name+"."+qname] = qual;
        if(name == primary)
            q[qname] = qual;
        return true;
        cnt:;
    }
    q[name+"."+qname] = false;
    if(name == primary)
        q[qname] = false;
    return false;
}

string getAbstractValue(string name, string query, Php::Value &conditions, Php::Value &skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){
    Php::Value sides = trim_explode(".", query);

    string fquery;
    string qname, var;

    if(sides.size() == 3){
        string name_t = sides[0];
        name = name_t;
        string qname_t = sides[1];
        qname = qname_t;
        string var_t = sides[2];
        var = var_t;
        fquery = name+"."+qname;
        if(q[name+"."+qname] == NULL){
            if(q[name] == NULL){
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if(q[name] == false){
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }else{
                if(q[name+"."+qname] == NULL){
                    if(!testEventQualifierConditions(name, qname, primary, conditions, q)){
                        return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                    }
                    // additional setter for future evals
                    // additional setter for future evals
                    if(name == primary){
                        Php::Value xd;
                        xd = q[name+"."+qname];
                        q[qname] = xd;
                    }
                }else if(q[name+"."+qname] == false){
                    return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                }
            }
        }else if(q[name+"."+qname] == false){
            return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
        }
    }else{
        string qname_t = sides[0];
        qname = qname_t;
        string var_t = sides[1];
        var = var_t;

        if(isEventCondition(qname, conditions, primary)){
            name = qname;
            fquery = name;

            if(q[name] == NULL){
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if(q[name] == false){
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }
        }else{
            // is qualifier
            fquery = name+"."+qname;

            if(q[name] == NULL){
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if(q[name] == false){
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }else{
                if(q[name+"."+qname] == NULL){
                    if(!testEventQualifierConditions(name, qname, primary, conditions, q)){
                        return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                    }
                    // additional setter for future evals
                    if(name == primary){
                        Php::Value xd;
                        xd = q[name+"."+qname];
                        q[qname] = xd;
                    }
                }else if(q[name+"."+qname] == false){
                    return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                }
            }
        }
    }
    if(var.find("tid") != std::string::npos)
        var.replace(var.find("tid"), 3, "typeId");
    if(var.find("qid") != std::string::npos)
        var.replace(var.find("qid"), 3, "qualifierId");
    string r = q[fquery][var];
    return r;
}

bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){

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
    return false;*/

    vector<string> primary_event_selectors = {"before", "after", "around", "skip", "within"};

    cond = trim_explode(",", conditions[name]);

    // put the event in q so we can use it onwards
    q[name] = events[index];

    for (auto&& [i, t_condition] : cond){
        // check if its not a predefined variable
        string condition = t_condition;
        bool found = false;
        for (string selector : primary_event_selectors){
            if(condition.find(selector) != std::string::npos){
                found = true;
                break;
            }
        }
        if(found)
            continue;

        op = operands(condition);
        sides = trim_explode(op, condition);
        // NOT condition
        string side_s = sides[0];
        if(side_s.find_first_of('!') == 0){
            // has to be a qualifier identifier
            Php::Value temp = trim_explode(".", side_s);
            string neg_qual = temp[0];
            neg_qual.erase(0, 1);
            if(testEventQualifierConditions(name, neg_qual, primary, conditions, q)){
                string left_t = temp[1];
                string right = sides[0];
                string left = q[name+"."+neg_qual][left_t];
                // TODO add more cases for abstract right hand side stuff (probably pointless / but theres a function for it)
                if(!eval_with_op(left, right, op)){
                    q[name] = false;
                    return false;
                }
            }
        }else{
            // abstract
            string left;
            string right;
            if(!is_numeric(side_s) && side_s.find_first_of('.') != std::string::npos){
                string left_t = getAbstractValue(name, side_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                left = left_t;
                if(left_t == "Q NOT SET / VALUE NOT FOUND"){
                    q[name] = false;
                    return false;
                }
                string right_s = sides[1];
                if(!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos){
                    string right_st = getAbstractValue(name, right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                    right = right_st;
                    if(right == "Q NOT SET / VALUE NOT FOUND"){
                       q[name] = false;
                        return false;
                    }
                }else{
                    right = right_s;
                }
            }else{
                if(side_s.find("tid") != std::string::npos)
                    side_s.replace(side_s.find("tid"), 3, "typeId");
                string left_t = q[name][side_s];
                left = left_t;
                // check if right side another abstract value
                string right_s = sides[1];
                if(!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos){
                    string right_st = getAbstractValue(name, right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                    right = right_st;
                    if(right == "Q NOT SET / VALUE NOT FOUND"){
                       q[name] = false;
                       return false;
                    }
                }else{
                    right = right_s;
                }
            }

            if(!eval_with_op(left, right, op)){
                q[name] = false;
                return false;
            }
        }
    }
    // passed all checks
    return true;
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

    auto events = params[0];
    auto code = params[1];

    Php::Value collection, temp, args, sides;

    vector<string> all_i, it;

    for (auto&& [step_t, event] : events) {
        int step = step_t;
        for (auto&& [i_event_name, code_blocks] : code){
            for (auto&& [block, instruction] : code_blocks){
                Php::Value q;
                all_i = {};
                string primary;
                Php::Value xd = instruction["conditions"];
                for (auto&& [k, v] : xd){
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
                if(instruction["skips"] != NULL)
                    skips = instruction["skips"];
                if(!testConditions(primary, conditions, skips, step, step, events, q, primary, all_i, it))
                    continue;
                // eval formula
                string formula = instruction["formula"];
                // extract all elements
                string formula_t = " "+formula+" ";
                while(formula_t.find("(") != std::string::npos){
                    formula_t.replace(formula_t.find("("), 1, " ");
                }
                while(formula_t.find(")") != std::string::npos){
                    formula_t.replace(formula_t.find(")"), 1, " ");
                }
                while(formula_t.find(" and ") != std::string::npos){
                    formula_t.replace(formula_t.find(" and "), 5, " ");
                }
                while(formula_t.find(" or ") != std::string::npos){
                    formula_t.replace(formula_t.find(" or "), 4, " ");
                }
                while(formula_t.find(" not ") != std::string::npos){
                    formula_t.replace(formula_t.find(" not "), 5, " ");
                }
                clean(formula_t);

                Php::Value formula_args = trim_explode(" ", formula_t);

                for (auto&& [i, arg_t] : formula_args){
                    string arg = arg_t;
                    string op = operands(arg);
                    if(op.empty()){
                        // normal argument
                        if(q[arg] == NULL){
                            if(arg.find(".") != std::string::npos){
                                Php::Value parts = trim_explode(".", arg);
                                string part_1 = parts[0];
                                string part_2 = parts[1];
                                if(q[part_1] == NULL){
                                    findEvent(part_1, conditions, skips, step, step, events, q, primary, all_i, it);
                                }
                                if(q[part_1] == false){
                                    q[arg] = false;
                                }else{
                                    testEventQualifierConditions(part_1, part_2, primary, conditions, q);
                                }
                            }else{
                                if(isEventCondition(arg, conditions, primary)){
                                    findEvent(arg, conditions, skips, step, step, events, q, primary, all_i, it);
                                }else{
                                    // only a qualifier
                                    testEventQualifierConditions(primary, arg, primary, conditions, q);
                                }
                            }
                        }
                    }else{
                        // complex argument with operand
                        string ev1, var1, ev2, var2;
                        Php::Value sides = trim_explode(op, arg);
                        string left = sides[0];
                        string right = sides[1];
                        // left has to be conjugated, right doesn't

                        Php::Value parts = trim_explode(".", left);
                        if(parts.size() == 2){
                            string left_t = parts[0];
                            string t1 = parts[1];
                            ev1 = left_t;
                            var1 = t1;
                            if(q[left_t] == NULL){
                                // only a qualifier
                                testEventQualifierConditions(primary, left_t, primary, conditions, q);
                            }
                        }else{ // 3
                            string left_t = parts[0];
                            string left_tt = parts[1];
                            string t1 = parts[2];
                            ev1 = left_t+"."+left_tt;
                            var1 = t1;
                            if(q[left_t] == NULL){
                                // only a qualifier
                                testEventQualifierConditions(left_t, left_tt, primary, conditions, q);
                            }
                        }

                        if(!is_numeric(right) && right.find(".") != std::string::npos){
                            Php::Value parts = trim_explode(".", right);
                            if(parts.size() == 2){
                                string right_t = parts[0];
                                string t2 = parts[1];
                                ev2 = right_t;
                                var2 = t2;
                                if(q[right_t] == NULL){
                                    // only a qualifier
                                    testEventQualifierConditions(primary, right_t, primary, conditions, q);
                                }
                            }else{ // 3
                                string right_t = parts[0];
                                string right_tt = parts[1];
                                string t2 = parts[2];
                                ev2 = right_t+"."+right_tt;
                                var2 = t2;
                                if(q[right_t] == NULL){
                                    // only a qualifier
                                    testEventQualifierConditions(right_t, right_tt, primary, conditions, q);
                                }
                            }
                        }else{
                            var2 = right;
                        }
                        string l = q[ev1][var1];
                        string r;
                        if(ev2.empty()){
                            string t = q[ev2][var2];
                            r = t;
                        }else{
                            r = var2;
                        }
                        q[arg] = eval_with_op(op, l, r);
                    }
                }

                // sort by length so we dont accidentally replace wrong values
                Php::Value sorted_args;
                int i = 0;

                // unique
                vector<string> vec = formula_args;
                sort( vec.begin(), vec.end() );
                vec.erase( unique( vec.begin(), vec.end() ), vec.end() );
                formula_args = vec;

                while (sorted_args.size() < formula_args.size()){
                    int s = 0;
                    string max = "";
                    for (auto&& [x, arg_t] : formula_args){
                        string arg = arg_t;
                        if(in_array(arg, sorted_args))
                            continue;
                        if(arg.length() > s){
                            max = arg;
                            s = arg.length();
                        }
                    }
                    sorted_args[i] = max;
                    i++;
                }

                for (auto&& [x, arg_t] : sorted_args){
                    string arg = arg_t;
                    if(arg == "")
                        continue;
                    if(q[arg] == false || q[arg] == NULL){
                        while(formula.find(arg) != std::string::npos){
                            formula.replace(formula.find(arg), arg.length(), "false");
                        }
                    }else{
                        while(formula.find(arg) != std::string::npos){
                            formula.replace(formula.find(arg), arg.length(), "true");
                        }
                    }
                }

                // moment of truth
                if(eval(formula)){
                    Php::Value temp;
                    temp["event_type"] = i_event_name;
                    string t1 = q[primary]["eventId"];
                    temp["event_id"] = t1;
                    string t2 = q[primary]["id"];
                    temp["id"] = t2;
                    string t3 = q[primary]["timeStamp"];
                    temp["time"] = t3;
                    if(instruction["noten_context"] == NULL){
                        temp["noten_context"] = "";
                    }else{
                        temp["noten_context"] = instruction["noten_context"];
                    }
                    for (auto&& [key_t, items] : instruction["values"]){
                        string key = key_t;
                        Php::Value splits = trim_explode("|", items);
                        for (auto&& [x, value_t] : splits){
                            string value = value_t;
                            if (!is_numeric(value) && value.find(".") != std::string::npos) {
                                int find = value.find_last_of('.');
                                string value_part = value.substr(find+1, value.length()-find);
                                if (value_part == "tid") {
                                    value_part = "typeId";
                                }else if (value_part == "qid") {
                                    value_part = "qualifierId";
                                }
                                value = value.substr(0, find);
                                if (q[value] != NULL) {
                                    string t = q[value][value_part];
                                    temp["values"][key] = t;
                                    // convert timeStamp to PHP readable
                                    if (value_part == "timeStamp" && t != ""){
                                        temp["values"][key] = t.substr(0, 10)+" "+t.substr(11, 8);
                                    }
                                    // break when 1st value is found
                                    if(t != "")
                                        break;
                                }
                                if(temp["values"][key] == NULL)
                                    temp["values"][key] = "";
                            } else {
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
    return collection;
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
