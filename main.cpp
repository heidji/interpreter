#include <iostream>
#include <string>
#include <locale>
#include <vector>
#include <algorithm>
#include <chrono>

#include <phpcpp.h>

using namespace std;
using namespace std::chrono;

bool is_numeric(string s){
    return !s.empty() && s.find_first_not_of("-.0123456789") == std::string::npos;
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
    str = trim(str);

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
    return NULL;
};

bool eval_with_op(string left, string right, string op){
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
    return false;
}

bool in_array(string needle, vector<string> haystack){
    return count(haystack.begin(), haystack.end(), needle);
}

bool findEvent(string &name, Php::Value &conditions, int index, Php::Value events, Php::Value &q, string &primary, vector<string> all_i, int it = 0){

    it++;
    if(it > all_i.size() + 10)
        throw std::invalid_argument("circular reference");
    // also checks for testConditions
    return false;
}

bool isEventCondition(string &name, Php::Value &conditions){
    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};
    string temp = conditions[name];

    for (string selector : $primary_event_selectors){
        if(temp.find_first_of(selector) != std::string::npos)
            return true;
    }
    return false;
}

bool testEventQualifierConditions(string &name, string &qname, Php::Value &conditions, Php::Value &q){
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
        return true;
        cnt:;
    }
    q[name+"."+qname] = false;
    return false;
}

bool testConditions(string &name, Php::Value &conditions, int index, Php::Value events, Php::Value &q, string &primary, vector<string> all_i, int it = 0){

    string left, right, op, q_op;
    Php::Value args, q_args, sides, q_sides, cond, q_cond, qual, q_qual;

    cond = trim_explode(",", conditions[name]);

    // put the event in q so we can use it onwards
    q[name] = events[index];
    for (auto&& [i, condition] : cond){
        op = operands(condition);
        sides = trim_explode(op, condition);
        // NOT condition
        string side_s = sides[0];
        if(side_s.find_first_of('!') == 0){
            // has to be a qualifier identifier
            Php::Value temp = trim_explode(".", side_s);
            string neg_qual = temp[0];
            neg_qual.erase(0, 1);
            if(testEventQualifierConditions(name, neg_qual, conditions, q)){
                string left_t = temp[1];
                string right = sides[0];
                string left = q[name+"."+neg_qual][left_t];
                // TODO add more cases for abstract right hand side stuff (probably pointless)
                if(eval_with_op(left, right, op)){
                    q[name] = false;
                    return false;
                }
            }
        }else{
            // abstract
            if(side_s.find_first_of('.') != std::string::npos){
                args = trim_explode(".", side_s);
                for (auto&& [j, arg] : args){
                    string arg_s = arg;
                    if(arg_s == name)
                        continue;
                    if(q[name+"."+arg_s] == NULL){
                        q_cond = trim_explode(",", conditions[arg_s]);
                        qual = events[index]["qualifier"];
                        for (auto&& [jj, qualifier] : qual){
                            for (auto&& [jjj, q_condition] : q_cond){
                                q_op = operands(q_condition);
                                q_sides = trim_explode(q_op, q_condition);
                                string q_left_temp_s = q_sides[0];
                                string q_right_s = q_sides[1];
                                // check if right side is defined
                                if(!is_numeric(q_right_s)){
                                    // is instruction
                                    if (in_array(q_right_s, all_i)){
                                        if(q[q_right_s] == NULL){
                                            // check if maybe the right hand side is own qualifier or foreign
                                            if(q_right_s.find_first_of('.') != std::string::npos){
                                                q_args = trim_explode(".", q_right_s);
                                                for (auto&& [jjjj, q_arg] : q_args){
                                                    string q_arg_s = q_arg;
                                                    if(q_arg_s == name)
                                                        continue;
                                                    if(isEventCondition(q_arg_s, conditions)){
                                                        if(q[q_arg_s] == NULL)
                                                            findEvent(q_arg_s, conditions, index, events, q, primary, all_i, it);
                                                        if(q[q_arg_s] == false){
                                                            q[name] = false;
                                                            return false;
                                                        }
                                                    }else{
                                                        // if the event is there, then it can be tested for qualifiers
                                                        if(q[q_right_s] == NULL){
                                                            string temp = q_args[0];
                                                            testEventQualifierConditions(temp, q_right_s, conditions, q);
                                                        }else if(q[q_right_s] == false){
                                                            q[name] = false;
                                                            return false;
                                                        }
                                                    }
                                                }
                                            }else{
                                                // is own qualifier
                                            }
                                        }else if(q[q_right_s] == false){
                                            q[name] = false;
                                        }
                                    }
                                }
                                string q_left_s = qual[q_left_temp_s];
                                if(q_left_s == "qid")
                                    q_left_s = "qualifierId";
                                if(!eval_with_op(q_left_s, q_right_s, q_op)){
                                    goto break_qual;
                                }
                            }
                            // passed all
                            if(name == primary){
                                q[arg_s] = qualifier;
                            }
                            q[name+"."+arg_s] = qualifier;
                            break_qual:;
                        }
                    }else{
                        if(q[name+"."+arg_s] == false){
                            return false;
                        }
                    }
                }
            }else{
                string left = q[name][side_s];
                // check if right side another abstract value
                string right_s = sides[1];
                // abstract
                if(!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos){
                    args = trim_explode(".", side_s);
                    for (auto&& [j, arg] : args){
                        // TODO later because the exact same routine is up there. better create a function for that
                    }
                }else{
                    if(!eval_with_op(left, right_s, op)){
                        q[name] = false;
                        return false;
                    }
                }
            }
        }
    }
    // passed all checks
    return true;
}

Php::Value interpreter(Php::Parameters &params)
{
    auto $events = params[0];
    auto $code = params[1];

    Php::Value $collection;
    Php::Value $q;
    Php::Value $empty;
    Php::Value $temp;

    vector<string> $all_i;

    Php::Value $args;
    Php::Value $sides;

    for (auto&& [$step, $event] : $events) {
        for (auto&& [$i_event_name, $code_blocks] : $code){
            $q = $empty;
            for (auto&& [$block, $instruction] : $code_blocks){
                $all_i = {};
                string $primary;
                for (auto&& [k, v] : $instruction["conditions"]){
                    $all_i.push_back(k);
                }
                if ($instruction["primary"] != NULL)
                    string $primary = $instruction["primary"];
                else
                    string $primary = $all_i[0];
                $q[$primary] = $event;
                return $q;
            }
        }
    }

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
