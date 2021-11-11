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

    return (((year*365+month*30+day)*24+hour)*60+min)*60+sec;
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

bool isEventCondition(string &name, Php::Value &conditions){
    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};
    string temp = conditions[name];

    for (string selector : $primary_event_selectors){
        if(temp.find(selector) != std::string::npos)
            return true;
    }
    return false;
}

// declare
bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it);

bool findEvent(string &name, Php::Value &skips, Php::Value &conditions, int primary_index, int index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){

    if(in_array(name, it))
        throw std::invalid_argument("circular reference");
    it.push_back(name);

    int step = primary_index;
    int within, times;
    int start, direction, end;
    int i;
    int j;
    int s;

    string selector, within_type;
    Php::Value event = events[primary_index];

    string condition = conditions[name];

    vector<string> $primary_event_selectors = {"before", "after", "around"};
    for (string s : $primary_event_selectors){
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
            if(rule.find("around") != std::string::npos)
                int times = temp[1];
            else {
                times = 1; // in around there's no xth match, always first
                if (rule.find("sec") != std::string::npos) {
                    within_type = "time";
                    rule.replace(rule.find("sec"), 3, "");
                } else {
                    within_type = "steps";
                    rule.replace(rule.find("steps"), 5, "");
                }
                Php::Value args = trim_explode("=", rule);
                int within = args[1];
                if(within == 0)
                    within = NULL;
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
                    rule.replace(rule.find("steps"), 5, "");
                }
                Php::Value args = trim_explode("=", rule);
                int within = args[1];
            }
            rules[pos] = NULL;
            continue;
        }
        if(rule.find("skip") != std::string::npos){
            skips = trim_explode("|", trim_explode("=", rule)[1]);
            rules[pos] = NULL;
        }
    }

    // CURRENT WORK
    start = step + direction;

    if (within != NULL) {
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
    } /*else {
        if ($direction == -1)
            $end = 0;
        else
            $end = count($events) - 1;
    }
    if ($end > count($events) - 1)
        $end = count($events) - 1;
    elseif ($end < 0)
        $end = 0;
    if ($start > count($events) - 1)
        $start = count($events) - 1;
    elseif ($start < 0)
        $start = 0;

    $s = 1;
    if ($direction == 1)
        $eval = "return (\$s <= \$times && \$j <= \$end);";
    else
        $eval = "return (\$s <= \$times && \$j >= \$end);";

    if ($start == $step || $direction == 1 && $start > $end || $direction == -1 && $start < $end) {
        $q[$set] = false;
        continue;
    }
    $around_alternator = 1;
    for ($j = $start; eval($eval); $j += $direction) {
        if($selector == 'around') {
            $i = (($j-$start) * $around_alternator)+$start;
            if($around_alternator == 1){
                $j--; // skip a beat
            }
            $around_alternator = -$around_alternator;
            if($i < 0 || $i > $end)
                continue;
        }else
            $i = $j;
        $q_temp = [];
        $rules_x = [];

        if($events[$i]->typeId == 43){
            $end += $direction;
            if ($end > count($events) - 1)
                $end = count($events) - 1;
            elseif ($end < 0)
                $end = 0;
            continue;
        }

        // check if skip event
        bool do = false;
        if(isset($skips)){
            foreach($skips as $skip){
                $skipset = $instruction->skips->{$skip};
                if(!isset($skipset))
                    return ['error_interpreter' => 'skipset '.$skip.' is undefined.'];
                foreach(['stop', 'skip', 'count'] as $skipstop){
                    foreach(['team', 'teamvs'] as $team){
                        $context['team'] = isset($skipset->{$skipstop}->{$team}) ? $team : null;
                        $context['is'] = isset($skipset->{$skipstop}->{$context['team']}->is) ? 'is' : (isset($skipset->{$skipstop}->{$context['team']}->is_not) ? 'is_not' : null);
                        if(isset($context['team']) && ($context['team'] == 'team' && $q[$primary]->contestantId == $events[$i]->contestantId || $context['team'] == 'teamvs' && $q[$primary]->contestantId != $events[$i]->contestantId)) {
                            if($context['is'] == 'is')
                                $do = false;
                            else
                                $do = true;
                            foreach($skipset->{$skipstop}->{$context['team']}->{$context['is']} as $x => $ruleset){
                                foreach($ruleset as $var => $items){
                                    if(!is_array($items))
                                        $items = [$items];
                                    foreach($items as $value){
                                        if($var == 'tid')
                                            $var = 'typeId';
                                        if(!in_array($var, ['qid', 'qualifierId'])){
                                            if($events[$i]->{$var} == $value){
                                                continue 2;
                                            }
                                        }else{
                                            foreach($events[$i]->qualifier as $qualifier){
                                                if($qualifier->qualifierId == $value){
                                                    continue 3;
                                                }
                                            }
                                        }
                                    }
                                    // no match
                                    if($x == count($skipset->{$skipstop}->{$context['team']}->{$context['is']})-1)
                                        break 5;
                                    else
                                        continue 2;
                                }
                                if($context['is'] == 'is'){
                                    // found match -> skip
                                    $do = true;
                                    break 4;
                                }else{
                                    // found match -> skip
                                    $do = false;
                                    continue 2;
                                }
                            }
                        }
                    }
                }
            }
            if($do){
                if($skipstop == 'stop'){
                    $q[$set] = false;
                    break;
                }elseif($skipstop == 'skip') {
                    $end += $direction;
                    if ($end > count($events) - 1)
                        $end = count($events) - 1;
                    elseif ($end < 0)
                        $end = 0;
                    continue;
                }elseif($skipstop == 'count'){
                    $s++;
                    continue;
                }
            }
        }
        // END CURRENT WORK
        // ADD THE REST OF CHECKS HERE
    }*/

    if(q[name] == false){
        return false;
    }
    return true;
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

string getAbstractValue(string query, Php::Value &conditions, Php::Value &skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){
    Php::Value sides = trim_explode(".", query);
    string name = sides[0];
    string fquery;
    string var;

    if(sides.size() == 3){
        string qname = sides[1];
        string var = sides[2];
        string fquery = name+"."+qname;
        if(q[name+"."+qname] == NULL){
            if(q[name] == NULL){
                findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
            }
            if(q[name] == false){
                return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
            }else{
                if(q[name+"."+qname] == NULL){
                    if(!testEventQualifierConditions(name, qname, conditions, q)){
                        return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                    }
                }else if(q[name+"."+qname] == false){
                    return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
                }
            }
        }else if(q[name+"."+qname] == false){
            return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
        }
    }else{
        string var = sides[1];
        string fquery = name;
        if(q[name] == NULL){
            findEvent(name, conditions, skips, index, primary_index, events, q, primary, all_i, it);
        }
        if(q[name] == false){
            return "Q NOT SET / VALUE NOT FOUND"; // equivalent for false
        }
    }
    string r = q[fquery][var];
    return r;
}

bool testConditions(string &name, Php::Value &conditions, Php::Value skips, int index, int primary_index, Php::Value &events, Php::Value &q, string &primary, vector<string> &all_i, vector<string> &it){

    string left, right, op, q_op;
    Php::Value args, q_args, sides, q_sides, cond, q_cond, qual, q_qual;

    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};

    cond = trim_explode(",", conditions[name]);

    // put the event in q so we can use it onwards
    q[name] = events[index];
    for (auto&& [i, condition] : cond){
        // check if its not a predefined variable
        string t_condition = condition;
        for (string selector : $primary_event_selectors){
            if(t_condition.find(selector) != std::string::npos)
                continue;
        }
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
                // TODO add more cases for abstract right hand side stuff (probably pointless / but theres a function for it)
                if(eval_with_op(left, right, op)){
                    q[name] = false;
                    return false;
                }
            }
        }else{
            // abstract
            if(!is_numeric(side_s) && side_s.find_first_of('.') != std::string::npos){
                string left = getAbstractValue(side_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                if(left == "Q NOT SET / VALUE NOT FOUND"){
                    q[name] = false;
                    return false;
                }
                string right_s = sides[1];
                if(!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos){
                    string right = getAbstractValue(right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                    if(right == "Q NOT SET / VALUE NOT FOUND"){
                       q[name] = false;
                        return false;
                    }
                }else{
                    string right = right_s;
                }
            }else{
                string left = q[name][side_s];
                // check if right side another abstract value
                string right_s = sides[1];
                if(!is_numeric(right_s) && right_s.find_first_of('.') != std::string::npos){
                    string right = getAbstractValue(right_s, conditions, skips, index, primary_index, events, q, primary, all_i, it);
                    if(right == "Q NOT SET / VALUE NOT FOUND"){
                       q[name] = false;
                        return false;
                    }
                }else{
                    string right = right_s;
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
    auto $events = params[0];
    auto $code = params[1];

    Php::Value $collection, $q, $empty, $temp, $args, $sides;

    vector<string> $all_i, $it;

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
