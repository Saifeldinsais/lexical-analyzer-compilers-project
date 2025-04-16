#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <regex>
#include <sstream>

using namespace std;

// Python keywords
vector<string> python_keywords = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is", "lambda",
    "nonlocal", "not", "or", "pass", "raise", "return", "try", "while", "with", "yield",
    "match", "case", "print", "f", "lower", "input"};

vector<string> operators_list = {
    "+", "-", "", "/", "%", "*", "//",
    "=", "+=", "-=", "=", "/=", "%=", "//=", "*=", "&=", "|=", "^=", ">>=", "<<=",
    "==", "!=", ">", "<", ">=", "<=",
    "and", "or", "not",
    "&", "|", "^", "~", "<<", ">>", "<>"};

vector<string> punctuation_list = {
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    ",",
    ":",
    ".",
    ";",
    "@",
    "->",
    "=>",
    "\\",
    "\"",
    "'",
    "#",
    "...",
};

struct Identifier
{
    string name;
    string type;
};

vector<Identifier> identifiers_list;
vector<string> functions_list;
//////////////////////////////////

void print_symbolsTable()
{
    cout << "---------------------------------------------" << endl;
    cout << "table of this file" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "|\t" << "index\t|\tidentifier\t\t|" << endl;
    for (int i = 0; i < identifiers_list.size(); i++)
    {
        cout << "|\t" << i << "\t|\t" << identifiers_list[i].name << "\t\t|\t" << identifiers_list[i].type << "\t|" << endl;
    }
};
///////////////////////////////////////////////////////////////////
bool isKeyword(const string &word)
{
    for (const string &keyword : python_keywords)
    {
        if (word == keyword)
        {
            return true;
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////

bool isOperator(const string &word)
{
    for (const string &op : operators_list)
    {
        if (word == op)
        {
            return true;
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////

bool isPunctuation(const string &word)
{
    for (const string &punc : punctuation_list)
    {
        if (word == punc)
        {
            return true;
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////
bool isIsolated(size_t i, size_t length, const string &line)
{
    string op = line.substr(i, length);

    if (op == "and" || op == "not" || op == "or")
    {
        char before = (i > 0) ? line[i - 1] : ' ';
        char after = (i + length < line.size()) ? line[i + length] : ' ';
        return !(isalnum(before) || before == '_') && !(isalnum(after) || after == '_');
    }

    return true;
}
///////////////////////////////////////////////////////////////////

bool isNumeric(const string &word)
{
    static const regex number_regex(R"(^-?\d+(\.\d+)?)");
    return regex_match(word, number_regex);
}

///////////////////////////////////////////////////////////////////

bool isexpocase(const string &line, size_t i)
{
    if (i == 0 || i + 2 >= line.size())
        return false;

    if (line[i] == 'e' || line[i] == 'E')
    {

        if (isdigit(line[i - 1]))
        {

            if ((line[i + 1] == '+' || line[i + 1] == '-') && isdigit(line[i + 2]))
            {
                return true;
            }

            if (isdigit(line[i + 1]))
            {
                return true;
            }
        }
    }
    return false;
}
///////////////////////////////////////////////////////////////////

bool ishexa(const string &word)
{
    static const regex hex_regex(R"(^0[xX][0-9a-fA-F]+$)");
    return regex_match(word, hex_regex);
}

///////////////////////////////////////////////////////////////////
int availableIdentifiers(const string &word)
{
    for (int i = 0; i < identifiers_list.size(); i++)
    {
        if (word == identifiers_list[i].name)
        {
            return i;
        }
    }
    return -1;
};
/////////////////////////////////////////////////

bool isFunction(const string &word)
{
    for (const string &function : functions_list)
    {
        if (word == function)
        {
            return true;
        }
    }
    return false;
};

////////////////////////////////////////////////

void detectDataType(const string &identifier, const string &value)
{
    string cleanedVal = value;
    cleanedVal.erase(remove_if(cleanedVal.begin(), cleanedVal.end(), ::isspace), cleanedVal.end());

    string type = "unknown";
    regex functionCallPattern(R"(^[a-zA-Z_][a-zA-Z0-9_]*\(.*\)$)");
    regex hexPattern(R"(0[xX][0-9a-fA-F]+)");
    regex numericPattern(R"(^-?\d+(\.\d+)?([eE][+-]?\d+)?$)");

    for (const auto &id : identifiers_list)
        if (id.name == identifier)
            return;

    if (isFunction(identifier)) {
        type = "function";
    }
    else if (cleanedVal == "\"__main__\"" || cleanedVal == "'__main__'" || cleanedVal == "__main__") {
        type = "string";
    }
    else if (regex_match(cleanedVal, functionCallPattern)) {
        string fname = cleanedVal.substr(0, cleanedVal.find('('));
        if (isFunction(fname)) {
            type = "function_call";
        }
    }
    else if (regex_match(cleanedVal, hexPattern)) {
        type = "int";
    }
    else if (!cleanedVal.empty() && cleanedVal.front() == '[') {
        type = "list";
    }
    else if (!cleanedVal.empty() && cleanedVal.front() == '{') {
        type = (cleanedVal.find(':') != string::npos ? "dict" : "set");
    }
    else if (!cleanedVal.empty() && cleanedVal.front() == '(' && cleanedVal.back() == ')') {
        type = "tuple";
    }
    else if (!cleanedVal.empty() && (cleanedVal.front() == '"' || cleanedVal.front() == '\'')) {
        type = "string";
    }
    else if (cleanedVal == "True" || cleanedVal == "False") {
        type = "bool";
    }
    else if (regex_match(cleanedVal, numericPattern)) {
        type = (cleanedVal.find('.') != string::npos || cleanedVal.find('e') != string::npos || cleanedVal.find('E') != string::npos) ? "float" : "int";
    }
    else {
        // expression handling
        bool hasFloat = false, hasInt = false;

        string token;
        for (size_t i = 0; i <= cleanedVal.size(); ++i) {
            if (i < cleanedVal.size() && (isalnum(cleanedVal[i]) || cleanedVal[i] == '_' || cleanedVal[i] == '.' || cleanedVal[i] == '-')) {
                token += cleanedVal[i];
            } else {
                if (!token.empty()) {
                    if (regex_match(token, numericPattern)) {
                        if (token.find('.') != string::npos || token.find('e') != string::npos || token.find('E') != string::npos)
                            hasFloat = true;
                        else
                            hasInt = true;
                    } else {
                        for (const auto &id : identifiers_list) {
                            if (id.name == token) {
                                if (id.type == "float") hasFloat = true;
                                else if (id.type == "int") hasInt = true;
                                else if (type == "unknown") type = id.type;
                                break;
                            }
                        }
                    }
                    token.clear();
                }
            }
        }

        if (hasFloat) type = "float";
        else if (hasInt && type == "unknown") type = "int";
    }

    identifiers_list.push_back({identifier, type});
}




///////////////////////////////////////////////////////////////////
string removecomments(string line)
{
    size_t singleCommentPos = line.find('#');
    if (singleCommentPos != string::npos)
    {
        line = line.substr(0, singleCommentPos);
    }
    return line;
}
////////////////////        remove multiline comments function //////////////////////////////////
vector<string> removemultiline(const string &file)
{
    ifstream pyFile(file);

    vector<string> cleanedLines;
    vector<string> allLines;
    string line;
    bool in_multiline_comment = false;
    string comment_char;

    if (!pyFile.is_open())
    {
        cout << "Could not open file.\n";
        return cleanedLines;
    }

    while (getline(pyFile, line))
    {
        allLines.push_back(line);
    }
    pyFile.close();

    for (int i = 0; i < allLines.size(); i++)
    {
        string &currentLine = allLines[i];
        string newLine;
        for (size_t j = 0; j < currentLine.size(); j++)
        {
            if (!in_multiline_comment && j + 2 < currentLine.size())
            {
                string current_three = currentLine.substr(j, 3);

                if (current_three == "\"\"\"" || current_three == "'''")
                {
                    string before_quote = currentLine.substr(0, j);
                    if (before_quote.find('=') != string::npos)
                    {

                        newLine += current_three;
                        j += 2;
                        continue;
                    }
                    else
                    {

                        in_multiline_comment = true;
                        comment_char = current_three;
                        j += 2;
                        continue;
                    }
                }
            }
            else if (in_multiline_comment && j + 2 < currentLine.size())
            {
                string current_three = currentLine.substr(j, 3);
                if (current_three == comment_char)
                {
                    in_multiline_comment = false;
                    comment_char.clear();
                    j += 2;
                    continue;
                }
            }

            if (!in_multiline_comment)
            {
                newLine += currentLine[j];
            }
        }
        cleanedLines.push_back(newLine);
    }
    return cleanedLines;
}
///////////////////////////////////////////////////////////////////
vector<string> getLiterals(string line)
{
    vector<string> literals;
    size_t startPos = 0;

    while ((startPos = line.find('"', startPos)) != string::npos)
    {
        size_t endPos = line.find('"', startPos + 1);
        if (endPos != string::npos)
        {

            literals.push_back(line.substr(startPos, endPos - startPos + 1));
            startPos = endPos + 1;
        }
        else
        {
            break;
        }
    }

    startPos = 0;
    while ((startPos = line.find('\'', startPos)) != string::npos)
    {
        size_t endPos = line.find('\'', startPos + 1);
        if (endPos != string::npos)
        {
            literals.push_back(line.substr(startPos, endPos - startPos + 1));
            startPos = endPos + 1;
        }
        else
        {
            break;
        }
    }

    return literals;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isCommentLine2(const string &line)
{
    size_t firstChar = line.find_first_not_of(" \t");
    return firstChar != string::npos && line[firstChar] == '#';
}
///////////////////////////////////////////////////////////////////
bool isCommentLine(const string &line)
{
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];

        if (c == '"' && !inSingleQuote)
        {
            inDoubleQuote = !inDoubleQuote;
        }
        else if (c == '\'' && !inDoubleQuote)
        {
            inSingleQuote = !inSingleQuote;
        }

        if (c == '#' || inSingleQuote || inDoubleQuote)
        {
            return true; // It's a comment line
        }
    }

    return false; // No standalone '#' found
}
//////////////////////////////////   invalid numbers ///////////////////////////////////////
bool check_invalidnumber(const vector<string> &lines)
{
    bool founderror = false;

    // Regular expression to match invalid number formats (including multiple dots
    regex invalidNumberPattern(R"(^-?\d*\.\d*\.\d*|^-?\d+\.\.$|^-?\.\d+\.|^-?\d*\.\d*e[+-]?$|^-?\d*\.e[+-]?$|^\.\.$|^\d+\.[a-zA-Z]+$)");

    // Regular expression to match numbers with leading zeros (excluding '0')
    regex leadingZeroPattern(R"(^0\d+)");

    // Regular expression to match invalid exponential numbers (e.g., 1e, 2e+, 3e- without digits after 'e')
    regex invalidExponentialPattern(R"((\d+[eE][^0-9+\-])|(\d+[eE][+\-]?$))");

    ////  for invalid hexadecimal
    regex invalid_hex_regex(R"(0[xX](?![0-9a-fA-F]+$)[^\s]*)");

    for (size_t lineNum = 0; lineNum < lines.size(); ++lineNum)
    {
        const string &line = lines[lineNum];
        if (isCommentLine(line))
            continue;
        istringstream iss(line);
        string word;

        while (iss >> word)
        {

            // Check for invalid number formats (like multiple dots)
            if (regex_search(word, invalidNumberPattern))
            {
                cout << "Invalid number format found at line " << (lineNum + 1)
                     << ": " << word << endl;
                founderror = true;
            }

            // Check for numbers with leading zeros (excluding '0')
            if (regex_match(word, leadingZeroPattern))
            {
                cout << "Invalid number with leading zeros found at line " << (lineNum + 1)
                     << ": " << word << endl;
                founderror = true;
            }

            // Check for invalid exponential numbers (e.g., 1e, 2e+, or missing digits after 'e')
            if (regex_match(word, invalidExponentialPattern))
            {
                cout << "Invalid exponential number format found at line " << (lineNum + 1)
                     << ": " << word << endl;
                founderror = true;
            }
            if (regex_match(word, invalid_hex_regex))
            {
                cout << "Invalid hexadecimal number format found at line " << (lineNum + 1)
                     << ": " << word << endl;
                founderror = true;
            }
        }
    }
    return founderror;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////// invalid operators/////////////////////////////////
bool check_invalid_operators(const vector<string> &lines)
{
    bool founderror = false;

    // One combined regex pattern to match various invalid Python operators
    static const regex invalidOperatorPattern(R"((\!\s*|\!\=|\=\+|\=\-\-|\=\+\+|\=\>\=|\<\=\>|\-\-\>|\*\*\*|\+\+|\-\-))");

    for (size_t lineNum = 0; lineNum < lines.size(); ++lineNum)
    {
        const string &line = lines[lineNum];
        if (isCommentLine(line))
            continue;

        istringstream iss(line);
        string word;

        while (iss >> word)
        {
            // Check if the word matches the invalid operator pattern
            if (regex_search(word, invalidOperatorPattern))
            {
                cout << "Invalid operator found at line " << (lineNum + 1)
                     << ": " << word << endl;
                founderror = true;
            }
        }
    }

    return founderror;
}

//////////////////////////////////////////////////////////////////////////

// unterminated string literals
bool check_unterminatedStrings(const vector<string> &lines)
{
    bool foundError = false;

    for (int i = 0; i < lines.size(); i++)
    {
        const string &line = lines[i];
        if (isCommentLine2(line))
            continue;

        int doubleQuotes = count(line.begin(), line.end(), '"');
        int singleQuotes = count(line.begin(), line.end(), '\'');

        if (doubleQuotes % 2 != 0 || singleQuotes % 2 != 0)
        {
            cout << "Unterminated string at line " << i + 1 << ": " << line << endl;
            foundError = true;
        }
    }

    return foundError;
}
/////////////////////////////////////////////////////////////////////////////

// check for invalid charachters (NOT IN STRING LITERALS OR MULTILINE COMMENTS)
bool check_invalidCharacters(const vector<string> &lines)
{
    regex invalidPattern(R"([\b\w]*[@$&][\w\b]*)");
    bool inMultiline = false;
    string quoteType = "";
    bool errorFound = false;

    for (int i = 0; i < lines.size(); ++i)
    {
        string line = lines[i];
        if (isCommentLine(line))
            continue;

        if (!inMultiline && (line.find("\"\"\"") != string::npos || line.find("'''") != string::npos))
        {
            inMultiline = true;
            quoteType = (line.find("\"\"\"") != string::npos) ? "\"\"\"" : "'''";
            continue;
        }
        else if (inMultiline && line.find(quoteType) != string::npos)
        {
            inMultiline = false;
        }

        if (inMultiline || line.find('#') == 0)
            continue;

        line = regex_replace(line, regex(R"((\"[^\"]*\"|'[^']*'))"), "");

        if (regex_search(line, invalidPattern))
        {
            cout << "Invalid identifier character at line " << i + 1 << ": " << line << endl;
            errorFound = true;
        }
    }

    return errorFound;
}

bool check_invalidIdentifiers(const vector<string> &lines)
{
    bool errorFound = false;

    regex startsWithDigit(R"(\b\d\w*[a-zA-Z_]+\w*\b)");          // Starts with digit
    regex containsSpecialChars(R"(\b\w*[~`!@$%^&*|\\/?]\w*\b)"); // Contains special characters
    regex containsSpaceOrTab(R"(\b\w*[\s\t]+\w*\b)");            // Contains space or tab
    regex validHex(R"(^0[xX][0-9a-fA-F]+$)");                    // Valid hexa to avoid errors in hexadecimal cases

    for (size_t i = 0; i < lines.size(); ++i)
    {
        const string &line = lines[i];
        if (isCommentLine(line))
            continue;
        istringstream iss(line);
        string word;
        while (iss >> word)
        {
            if (regex_match(word, validHex))
                continue;

            if (regex_match(word, startsWithDigit))
            {
                cout << "Invalid identifier (starts with digit) at line " << i + 1 << ": " << word << endl;
                errorFound = true;
            }

            if (regex_match(word, containsSpecialChars))
            {
                cout << "Invalid identifier (contains special characters) at line " << i + 1 << ": " << word << endl;
                errorFound = true;
            }

            if (regex_match(word, containsSpaceOrTab))
            {
                cout << "Invalid identifier (contains space/tab) at line " << i + 1 << ": " << word << endl;
                errorFound = true;
            }
        }
    }

    return errorFound;
}

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// handle unclosed brackets /////////////////////////////////
bool check_unclosedBrackets(const vector<string> &lines)
{
    bool errorFound = false;
    int parenCount = 0; // 3ashan  ()
    int braceCount = 0; // 3ashan {}

    for (size_t lineNum = 0; lineNum < lines.size(); ++lineNum)
    {
        const string &line = lines[lineNum];

        if (isCommentLine(line))
        {
            continue;
        }

        for (size_t i = 0; i < line.length(); ++i)
        {
            char c = line[i];

            if (c == '(')
            {
                parenCount++;
            }
            else if (c == ')')
            {
                parenCount--;
                if (parenCount < 0)
                {
                    cout << "Extra closing parenthesis at line " << lineNum + 1 << ": " << line << endl;
                    errorFound = true;
                    parenCount = 0;
                }
            }
            else if (c == '{')
            {
                braceCount++;
            }
            else if (c == '}')
            {
                braceCount--;
                if (braceCount < 0)
                {
                    cout << "Extra closing brace at line " << lineNum + 1 << ": " << line << endl;
                    errorFound = true;
                    braceCount = 0;
                }
            }
        }
    }

    if (parenCount > 0)
    {
        cout << "Unclosed parenthesis detected in the file." << endl;
        errorFound = true;
    }
    if (braceCount > 0)
    {
        cout << "Unclosed brace detected in the file." << endl;
        errorFound = true;
    }

    return errorFound;
}

///////////////////////////////////////////////////////////

bool handleErrors(const vector<string> &lines)
{
    bool errorFound = false;
    errorFound = (check_invalidCharacters(lines) || check_unterminatedStrings(lines) || check_invalidnumber(lines) || check_invalid_operators(lines) || check_invalidIdentifiers(lines) || check_unclosedBrackets(lines));
    return errorFound;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    string file;
    cout << "Enter the Python file name: ";
    cin >> file;

    vector<string> cleanedFileLines = removemultiline(file);

    if (handleErrors(cleanedFileLines))
    {
        cout << "Lexical analysis stopped due to errors." << endl;
        return 0;
    }

    for (string line : cleanedFileLines)
    {
        string cleanedLine = removecomments(line);

        if (line.find(',') != string::npos && line.find('=') != string::npos)
        {
            size_t eq = line.find('=');
            string left = line.substr(0, eq);
            string right = line.substr(eq + 1);

            auto splitAndClean = [](const string &str, char delim)
            {
                vector<string> tokens;
                stringstream stringg(str);
                string token;
                while (getline(stringg, token, delim))
                {
                    token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
                    tokens.push_back(token);
                }
                return tokens;
            };

            vector<string> vars = splitAndClean(left, ',');
            vector<string> values = splitAndClean(right, ',');

            for (size_t i = 0; i < vars.size() && i < values.size(); ++i)
            {
                detectDataType(vars[i], values[i]);
            }
        }

        vector<string> literals = getLiterals(cleanedLine);

        for (const string &literal : literals)
        {
            size_t pos = cleanedLine.find(literal);
            if (pos != string::npos)
            {
                cleanedLine.replace(pos, literal.length(), string(literal.length(), ' '));
            }
        }

        string currentToken;
        for (size_t i = 0; i < cleanedLine.size(); i++)
        {
            char c = cleanedLine[i];

            if (isexpocase(cleanedLine, i) && !currentToken.empty())
            {
                string expoNum = currentToken + cleanedLine.substr(i, 2);
                i += 2;
                while (i < cleanedLine.size() && isdigit(cleanedLine[i]))
                {
                    expoNum += cleanedLine[i++];
                }
                cout << "Numeric: <" << expoNum << ">" << endl;
                currentToken.clear();
                --i;
                continue;
            }

            string one_syntax(1, c);
            string two_syntax = (i + 1 < cleanedLine.size()) ? cleanedLine.substr(i, 2) : "";
            string three_syntax = (i + 2 < cleanedLine.size()) ? cleanedLine.substr(i, 3) : "";

            // Check for multi-character operators or punctuation (e.g., "and", "==", "...")
            if ((three_syntax.size() == 3 && isOperator(three_syntax) && isIsolated(i, 3, cleanedLine)) ||
                (three_syntax.size() == 3 && isPunctuation(three_syntax)))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";
                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(three_syntax))
                {
                    cout << "Operator: <" << three_syntax << ">" << endl;
                }
                else
                {
                    cout << "Punctuation: <" << three_syntax << ">" << endl;
                }
                i += 2;
                continue;
            }

            if ((two_syntax.size() == 2 && isOperator(two_syntax) && isIsolated(i, 2, cleanedLine)) ||
                (two_syntax.size() == 2 && isPunctuation(two_syntax)))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";
                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(two_syntax))
                {
                    cout << "Operator: <" << two_syntax << ">" << endl;
                }
                else
                {
                    cout << "Punctuation: <" << two_syntax << ">" << endl;
                }
                i += 1;
                continue;
            }

            // Handle single-character operators or punctuation
            if ((isOperator(one_syntax) && isIsolated(i, 1, cleanedLine) ||
                 (isPunctuation(one_syntax) &&
                  !(c == '.' && i > 0 && i + 1 < cleanedLine.size() &&
                    isdigit(cleanedLine[i - 1]) && isdigit(cleanedLine[i + 1])))))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken + "(") != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos && equalPos > i) ? cleanedLine.substr(equalPos + 1) : "";
                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(one_syntax))
                {
                    cout << "Operator: <" << one_syntax << ">" << endl;
                }
                else
                {
                    cout << "Punctuation: <" << one_syntax << ">" << endl;
                }
                continue;
            }

            if (isspace(c))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";
                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                    }
                    currentToken.clear();
                }
            }
            else
            {
                currentToken += c; // Collect all characters, including number-related ones
            }
        }

        // Handle any remaining token at the end of the line
        if (!currentToken.empty())
        {
            if (isKeyword(currentToken))
            {
                cout << "<Keyword," << currentToken << ">" << endl;
            }
            else if (isNumeric(currentToken) || ishexa(currentToken))
            {
                cout << "Numeric: <" << currentToken << ">" << endl;
            }
            else
            {
                if (availableIdentifiers(currentToken) == -1)
                {
                    bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                    if (isFunc)
                    {
                        functions_list.push_back(currentToken);
                    }
                    size_t equalPos = cleanedLine.find('=');
                    string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";
                    detectDataType(currentToken, value);
                    cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                }
                else
                {
                    cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                }
            }
            currentToken.clear();
        }

        for (const string &literal : literals)
        {
            cout << "<StringLiteral," << literal << ">" << endl;
        }
    }

    print_symbolsTable();
    return 0;
}