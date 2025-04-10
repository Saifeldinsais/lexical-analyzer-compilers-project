#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <regex>
using namespace std;

// Python keywords
vector<string> python_keywords = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is", "lambda",
    "nonlocal", "not", "or", "pass", "raise", "return", "try", "while", "with", "yield",
    "match", "case", "print"};

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

bool isexpocase(const string &word)
{
    static const regex expo_regex(R"(^-?\d+(\.\d+)?[eE][+-]?\d+$)");
    return regex_match(word, expo_regex);
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
////////////////////////////////////////////////

void detectDataType(const string &identifier, const string &value)
{
    string cleanedVal = value;
    cleanedVal.erase(remove_if(cleanedVal.begin(), cleanedVal.end(), ::isspace), cleanedVal.end());

    string type = "unknown";
    if (cleanedVal.front() == '[')
        type = "list";
    else if (cleanedVal.front() == '{')
        type = (cleanedVal.find(':') != string::npos ? "dict" : "set");
    else if (cleanedVal.front() == '(')
        type = "tuple";
    else if (cleanedVal.front() == '"' || cleanedVal.front() == '\'')
        type = "string";
    else if (cleanedVal == "True" || cleanedVal == "False")
        type = "bool";
    else if (cleanedVal.find('.') != string::npos || cleanedVal.find('e') != string::npos || cleanedVal.find('E') != string::npos)
        type = "float";
    else if (isdigit(cleanedVal.front()) || (cleanedVal.size() > 1 && cleanedVal[0] == '-' && isdigit(cleanedVal[1])))
        type = "int";
    else
    {
        for (const auto &id : identifiers_list)
        {
            if (id.name == cleanedVal)
            {
                type = id.type;
                break;
            }
        }
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

///////////////////////////////////////////////////////////////////
int main()
{
    string file;
    cout << "Enter the Python file name: ";
    cin >> file;

    vector<string> cleanedFileLines = removemultiline(file);

    string line;
    for (string line : cleanedFileLines)
    {

        string cleanedLine = removecomments(line);

        vector<string> literals = getLiterals(cleanedLine);

        for (const string &literal : literals)
        {
            size_t pos = cleanedLine.find(literal);
            if (pos != string::npos)
            {
                cleanedLine.replace(pos, literal.length(), string(literal.length(), ' '));
            }
        }
        ///////////////////////////////////////////////////////////////////
        string currentToken;
        for (size_t i = 0; i < cleanedLine.size(); i++)
        {
            char c = cleanedLine[i];

            string one_syntax(1, c);
            string two_syntax = (i + 1 < cleanedLine.size()) ? cleanedLine.substr(i, 2) : "XXXX";
            string three_syntax = (i + 2 < cleanedLine.size()) ? cleanedLine.substr(i, 3) : "XXXX";
            ///////////////////////////////////////////////////////////////////
            if ((isOperator(three_syntax) && isIsolated(i, 3, cleanedLine)) || isPunctuation(three_syntax))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken) || isexpocase(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";

                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                        }
                        currentToken.clear();
                    }
                }

                if (isOperator(three_syntax))
                {
                    cout << "Operator: <" << three_syntax << ">" << endl;
                    i = i + 2;
                    currentToken.clear();
                }
                else
                {
                    cout << "Punctuation: <" << three_syntax << ">" << endl;
                    i = i + 2;
                    currentToken.clear();
                }

                continue;
            }
            ///////////////////////////////////////////////////////////////////
            if ((isOperator(two_syntax) && isIsolated(i, 2, cleanedLine)) || isPunctuation(two_syntax))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken) || isexpocase(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    ///////////////////////////////////////////////////////////////////
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";

                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                        }
                        currentToken.clear();
                    }
                }
                ///////////////////////////////////////////////////////////////////
                if (isOperator(two_syntax))
                {
                    cout << "Operator: <" << two_syntax << ">" << endl;
                    i = i + 1;
                    currentToken.clear();
                }
                ///////////////////////////////////////////////////////////////////
                else
                {
                    cout << "Punctuation: <" << two_syntax << ">" << endl;
                    i = i + 1;
                    currentToken.clear();
                }

                continue;
            }
            ///////////////////////////////////////////////////////////////////
            if ((isOperator(one_syntax) && isIsolated(i, 1, cleanedLine)) || isPunctuation(one_syntax) && !(c == '.' && i > 0 && i + 1 < cleanedLine.size() && isdigit(cleanedLine[i - 1]) && isdigit(cleanedLine[i + 1])))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken) || isexpocase(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        currentToken.clear();
                    }
                    ///////////////////////////////////////////////////////////////////
                    else
                    {
                        if (availableIdentifiers(currentToken) == -1)
                        {
                            size_t equalPos = cleanedLine.find('=');
                            string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";

                            detectDataType(currentToken, value);
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                        }
                        currentToken.clear();
                    }
                }
                ///////////////////////////////////////////////////////////////////
                if (isOperator(one_syntax))
                {
                    cout << "Operator: <" << one_syntax << ">" << endl;
                    currentToken.clear();
                }
                else
                {
                    cout << "Punctuation: <" << one_syntax << ">" << endl;
                    currentToken.clear();
                }

                continue;
            }
            ///////////////////////////////////////////////////////////////////
            if (isspace(c))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken) || isexpocase(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                    }

                    else if (availableIdentifiers(currentToken) == -1)
                    {
                        size_t equalPos = cleanedLine.find('=');
                        string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";

                        detectDataType(currentToken, value);
                        cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                    }
                    else
                    {
                        cout << "identifier: <id," << availableIdentifiers(currentToken) << ">" << "\t" << currentToken << endl;
                    }
                }
                currentToken.clear();
            }
            ///////////////////////////////////////////////////////////////////
            else if (c == '.' && !currentToken.empty() && isdigit(currentToken.back()) &&
                     i + 1 < cleanedLine.size() && isdigit(cleanedLine[i + 1]))
            {
                currentToken += c;
            }
            else if (isalnum(c) || c == '_')
            {
                currentToken += c;
            }
            else
            {
                currentToken += c;
            }
        }
        ///////////////////////////////////////////////////////////////////
        if (!currentToken.empty())
        {
            if (isKeyword(currentToken))
            {
                cout << "<Keyword," << currentToken << ">" << endl;
            }
            else if (isNumeric(currentToken) || ishexa(currentToken) || isexpocase(currentToken))
            {
                cout << "Numeric: <" << currentToken << ">" << endl;
            }

            else if (availableIdentifiers(currentToken) == -1)
            {
                size_t equalPos = cleanedLine.find('=');
                string value = (equalPos != string::npos) ? cleanedLine.substr(equalPos + 1) : "";

                detectDataType(currentToken, value);
                cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
            }
            else
            {
                cout << "identifier: <id," << availableIdentifiers(currentToken) << ">\t" << currentToken << endl;
            }
            currentToken.clear();
        }
        ///////////////////////////////////////////////////////////////////
        for (const string &literal : literals)
        {
            cout << "<StringLiteral," << literal << ">" << endl;
        }
    }

    print_symbolsTable();
    return 0;
}