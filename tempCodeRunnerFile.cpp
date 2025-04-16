void detectDataType(const string &identifier, const string &value)
{
    string cleanedVal = value;
    cleanedVal.erase(remove_if(cleanedVal.begin(), cleanedVal.end(), ::isspace), cleanedVal.end());

    string type = "unknown";
    regex functionCallPattern(R"(^[a-zA-Z_][a-zA-Z0-9_]*\(.*\)$)");

    // Known functions with known return types
    map<string, string> functionReturnTypes = {
        {"find_word_in_string", "bool"}
    };

    for (const auto &id : identifiers_list)
    {
        if (id.name == identifier)
            return;
    }

    if (isFunction(identifier)) {
        type = "function";
    }
    else if (cleanedVal == "\"__main__\"" || cleanedVal == "'__main__'" || cleanedVal == "__main__") {
        type = "string";
    }
    else if (regex_match(cleanedVal, functionCallPattern)) {
        size_t openParen = cleanedVal.find('(');
        string funcName = cleanedVal.substr(0, openParen);
        type = functionReturnTypes.count(funcName) ? functionReturnTypes[funcName] : "function_call";
    }
    else if (cleanedVal.front() == '[') {
        type = "list";
    }
    else if (cleanedVal.front() == '{') {
        type = (cleanedVal.find(':') != string::npos ? "dict" : "set");
    }
    else if (cleanedVal.front() == '(' && cleanedVal.back() == ')') {
        type = "tuple";
    }
    else if (cleanedVal.front() == '"' || cleanedVal.front() == '\'') {
        type = "string";
    }
    else if (cleanedVal == "True" || cleanedVal == "False") {
        type = "bool";
    }
    else if (all_of(cleanedVal.begin(), cleanedVal.end(), [](char ch){ return isdigit(ch) || ch == '.'; }) && cleanedVal.find('.') != string::npos) {
        type = "float";
    }
    else if (isdigit(cleanedVal.front()) || (cleanedVal.size() > 1 && cleanedVal[0] == '-' && isdigit(cleanedVal[1]))) {
        type = "int";
    }

    identifiers_list.push_back({identifier, type});
}