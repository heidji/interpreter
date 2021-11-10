#include <iostream>
#include <string>
#include <locale>
#include <vector>
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

// Php::Value keeps em sorted
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

bool testConditions(string &name, Php::Value &conditions, Php::Value &e, Php::Value &q, string &primary){

    string left, right, op;
    Php::Value args, sides, q_sides, cond, q_cond, qual;

    cond = trim_explode(",", conditions[name]);

    for (auto&& [i, condition] : cond){
        op = operands(condition);
        sides = trim_explode(op, condition);
        // NOT condition
        string side_s = sides[0];
        if(side_s.find_first_of('!') == 0){
        }else{
            // abstract
            if(side_s.find_first_of('.') == 0){
                args = trim_explode(".", side_s);
                for (auto&& [j, arg] : args){
                    string arg_s = arg;
                    if(arg_s == name)
                        continue;
                    if(q[name+"."+arg_s] == NULL){
                        q_cond = trim_explode(",", conditions[arg_s]);
                        qual = e["qualifier"];
                        for (auto&& [jj, qualifier] : qual){
                            for (auto&& [jjj, condition] : q_cond){

                            }
                        }
                    }
                }
            }else{
                string left = e[side_s];
            }
        }
    }
    return false;
}

Php::Value interpreter(Php::Parameters &params)
{
    auto $events = params[0];
    auto $code = params[1];

    vector<string> $primary_event_selectors = {"before", "after", "around", "skip"};

    map<string, string> $predefined_vars = {{"tid", "typeId"}, {"qid", "qualifierId"}};

    Php::Value $collection;
    Php::Value $q;
    Php::Value $empty;
    Php::Value $temp;

    vector<string> $all_i;

    Php::Value $args;
    Php::Value $sides;

    for (auto&& [$step, $event] : $events) {
        for (auto&& [$i_event_name, $code_blocks] : $code){
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
                /*for (auto&& [$c, $condition] : $instruction["conditions"]){
                    $args = trim_explode(",", $condition);
                    for (auto&& [$x, $argz] : $args){
                        string $arg = $argz;
                        $temp = operands($arg);
                        string $operand = $temp["operand"];
                        string $replace = $temp["replace"];
                        $sides = trim_explode($operand, $arg);
                        for (auto&& [$i, $sidez] : $sides){
                            string $side = $sidez;
                            if (!is_numeric($side) && $side.find(".") != string::npos) {
                                return $arg;
                                $temp = explode(".", $side);
                                return $temp;
                                /*$var = $temp.pop_back();
                                return $var;
                            }
                        }*/

                        /*foreach ($sides as $i => $side) {
                                                    if (!is_numeric($side) && strpos($side, '.') !== false) {
                                                        $temp = explode('.', $side);
                                                        $var = array_splice($temp, -1)[0];
                                                        if (isset($predefined_vars[$var]))
                                                            $var = $predefined_vars[$var];
                                                        $sides[$i] = implode('.', $temp) . '.' . $var;
                                                    } elseif($i == 1 && !is_numeric($side) && strpos($side, ' sec') === false && strpos($side, 'null') === false && $sides[0] != 'skip') {
                                                        // assume constant
                                                        $sides[$i] = '"'.$side.'"';
                                                    } else {
                                                        if (isset($predefined_vars[$side]))
                                                            $sides[$i] = $predefined_vars[$side];
                                                    }
                                                }
                                                $args[$x] = implode($operand, $sides);*/
                    //}
                //}
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
