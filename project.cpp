#include <unordered_map>
#include <stack>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <regex>
#include <sstream>
#include <iomanip>

using namespace std;

///////////////////////////////////////

unordered_map<string, int> functionIndentLevels;

// Python keywords
vector<string> python_keywords = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is", "lambda",
    "nonlocal", "not", "or", "pass", "raise", "return", "try", "while", "with", "yield",
    "match", "case", "f"};

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

struct Token
{
    string type;
    string value;
};

struct Identifier
{
    int index;
    string name;
    string type;
    string scope;
    string value;
};

vector<Token> tokensList;
vector<Identifier> identifiers_List;
int current = 0;

////////////////////// LEXER VARIABLES///////////////
vector<Token> tokensListForParser;
vector<string> functions_list;

////////////////// FILE READER ///////////////
vector<string> fileReader(const string &filename)
{
    ifstream file(filename);
    vector<string> lines;
    string line;

    if (!file.is_open())
    {
        cout << "Error: Could not open file " << filename << endl;
        return lines;
    }

    while (getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();
    return lines;
}
///////////////////////////////////////////////

////////////////// PARSING FUNCTION ///////////////
bool match(const string &expected)
{
    //cout << "Matching: " << tokensList[current].value << " WITH " << expected << endl;
    if (current < tokensList.size() && tokensList[current].type == expected)
    {
        current++;
        return true;
    }
    return false;
}

Token peek()
{
    return tokensList[current];
}

Token previous()
{
    return tokensList[current - 1];
}

void error(const string &msg)
{
    cerr << "Syntax Error near '" << peek().value << "': " << msg << endl;
    exit(1);
}

//////////////////////////////////////////////////

// Function Declarations
void factor();
void term();
void expr();
void assignment_stmt();
void if_else_stmt();
void func_def();
void func_call();
void while_stmt();
void return_stmt();
void class_stmt();
void for_stmt();
void print_stmt();
void statement();
void program();
void import_stmt();

/////////////// GRAMMAR CHECKER ///////////////
void factor()
{
    if (match("("))
    {
        expr();
        if (!match(")"))
            error("Expected ')'");
    }
    else if (match("NUMBER"))
    {
        cout << "Number: " << previous().value << endl;
    }
    // FIX: Handle string literals so assignments like a = init work
    else if (match("StringLiteral"))
    {
        cout << "String literal: " << previous().value << endl;
    }
    else if (match("id"))
    {
        int idx = stoi(previous().value);
        cout << "Identifier: " << identifiers_List[idx].name << endl;
    }
    else
    {
        error("Expected NUMBER, IDENTIFIER, STRING, or '('");
    }
}

// term ::= factor (('*' | '/') factor)*
void term()
{
    factor();
    while (match("*") || match("/") || match("%"))
    {
        cout << "Operator: " << previous().type << endl;
        factor();
    }
}


// expr ::= term (('+' | '-') term)*
void expr()
{
    term();
    while (match("+") || match("-"))
    {
        cout << "Operator: " << previous().type << endl;
        term();
    }
}

string parse_expr_as_string()
{
    stringstream exprStream;
    size_t start = current;

    expr(); // Just parse normally

    for (size_t i = start; i < current; ++i)
    {
        exprStream << tokensList[i].value;
    }

    return exprStream.str();
}

// assignment ::= id '=' expr
void assignment_stmt()
{
    vector<int> lhs_ids;

    // Expect at least one identifier on LHS
    if (!match("id"))
        error("Expected identifier on left-hand side of assignment");

    lhs_ids.push_back(stoi(previous().value));

    // Handle multiple identifiers (a, b, c)
    while (match(","))
    {
        if (!match("id"))
            error("Expected identifier after ',' in assignment");
        lhs_ids.push_back(stoi(previous().value));
    }

    // Expect '='
    if (!match("="))
        error("Expected '=' in assignment");

    // Parse right-hand expressions
    vector<string> rhs_values;

    // First expression
    stringstream value;
    value << parse_expr_as_string();
    rhs_values.push_back(value.str());

    // Handle multiple expressions
    while (match(","))
    {
        stringstream val;
        val << parse_expr_as_string();
        rhs_values.push_back(val.str());
    }

    // Optional: check LHS and RHS count match
    if (lhs_ids.size() != rhs_values.size())
    {
        error("Unbalanced unpacking: " + to_string(lhs_ids.size()) + " identifiers but " +
              to_string(rhs_values.size()) + " values.");
    }

    // Print debug
    for (size_t i = 0; i < lhs_ids.size(); ++i)
    {
        cout << "Assignment: " << identifiers_List[lhs_ids[i]].name
             << " = " << rhs_values[i] << endl;
    }
}


// bool_expr ::= expr [ '==' expr ]
void bool_expr()
{
    expr(); // left-hand side

    // Handle optional comparison
    if (match("==") || match("!=") || match(">") || match("<") || match(">=") || match("<="))
    {
        string op = previous().type;
        cout << "Comparison operator: " << op << endl;
        expr(); // right-hand side
    }
}


// if_stmt → 'if' expr '==' expr ':' statement{ 'elif' expr '==' expr ':' statement }['else' ':' statement]
void if_else_stmt()
{
    if (!match("keyword") || previous().value != "if")
        error("Expected 'if'");
    cout << "If statement" << endl;

    // Optional parentheses around condition
    bool hasParens = match("(");
    bool_expr();
    if (hasParens && !match(")"))
        error("Missing closing ')' after if condition");

    if (!match(":"))
        error("Expected ':' after if condition");

    if (!match("INDENT"))
        error("Expected INDENT after ':' in if block");

    while (peek().type != "DEDENT" && peek().type != "EOF")
        statement();

    if (!match("DEDENT"))
        error("Expected DEDENT at end of if block");

    while (peek().type == "keyword" && peek().value == "elif")
    {
        match("keyword");
        cout << "Elif clause" << endl;

        hasParens = match("(");
        bool_expr();
        if (hasParens && !match(")"))
            error("Missing closing ')' after elif condition");

        if (!match(":"))
            error("Expected ':' after elif condition");

        if (!match("INDENT"))
            error("Expected INDENT after ':' in elif block");

        while (peek().type != "DEDENT" && peek().type != "EOF")
            statement();

        if (!match("DEDENT"))
            error("Expected DEDENT at end of elif block");
    }

    if (peek().type == "keyword" && peek().value == "else")
    {
        match("keyword");
        cout << "Else clause" << endl;

        if (!match(":"))
            error("Expected ':' after else");

        if (!match("INDENT"))
            error("Expected INDENT after ':' in else block");

        while (peek().type != "DEDENT" && peek().type != "EOF")
            statement();

        if (!match("DEDENT"))
            error("Expected DEDENT at end of else block");
    }
}


void return_stmt()
{
    if (!match("keyword") || previous().value != "return")
        error("Expected 'return'");
    expr();
}

void class_stmt()
{
    if (!match("keyword") || previous().value != "class")
        error("Expected 'class'");

    if (!match("id"))
        error("Expected class name after 'class'");

    int idx = stoi(previous().value);
    cout << "Class name: " << identifiers_List[idx].name << endl;

    if (!match(":")){
        error("Expected ':' after class name");
    }

    if (!match("INDENT"))
    {
        error("Expected INDENT after ':' in if body");
    }
    while (peek().type != "DEDENT" && peek().type != "EOF")
    {
        statement();
    }
    if (!match("DEDENT"))
    {
        error("Expected DEDENT at end of if block");
    }
}

// while_stmt ::= while expr '<' expr ':' assignment
void while_stmt()
{
    if (!match("keyword") || previous().value != "while")
        error("Expected 'while'");

    bool hasParens = match("(");
    bool_expr();  // Handles full comparison expression like x + 1 == 5

    if (hasParens && !match(")"))
        error("Missing closing ')' after while condition");

    if (!match(":"))
        error("Expected ':' after while condition");

    if (!match("INDENT"))
        error("Expected INDENT after ':' in while block");

    while (peek().type != "DEDENT" && peek().type != "EOF")
        statement();

    if (!match("DEDENT"))
        error("Expected DEDENT at end of while block");
}


// for_stmt ::= 'for' id 'in' expr ':' assignment
void for_stmt()
{
    if (!match("keyword") || previous().value != "for")
        error("Expected 'for'");

    if (!match("id"))
        error("Expected loop variable after 'for'");
    int idx = stoi(previous().value);
    cout << "For loop variable: " << identifiers_List[idx].name << endl;

    if (!match("keyword") || previous().value != "in")
        error("Expected 'in' after loop variable");

    bool hasParens = match("(");
    expr();  // e.g. range(10)
    if (hasParens && !match(")"))
        error("Missing closing ')' after iterable in for loop");

    if (!match(":"))
        error("Expected ':' after 'for' loop header");

    if (!match("INDENT"))
        error("Expected INDENT after ':' in for block");

    while (peek().type != "DEDENT" && peek().type != "EOF")
        statement();

    if (!match("DEDENT"))
        error("Expected DEDENT at end of for block");
}


// func_def ::= 'def' id '(' [id (',' id)*] ')' ':' assignment
void func_def()
{
    if (!match("keyword") || previous().value != "def")
        error("Expected 'def'");
    if (!match("id"))
        error("Expected function name");
    cout << "Function name: " << previous().value << endl;

    if (!match("("))
        error("Expected '(' after function name");

    // Parameter list
    if (peek().type == "id")
    {
        match("id");
        cout << "Parameter: "
             << identifiers_List[stoi(previous().value)].name
             << endl;
        while (peek().type == ",")
        {
            match(",");
            if (!match("id"))
                error("Expected parameter after comma");
            cout << "Parameter: "
                 << identifiers_List[stoi(previous().value)].name
                 << endl;
        }
    }

    if (!match(")"))
        error("Expected ')' after parameters");
    if (!match(":"))
    {
        error("Expected ':' after function definition");
    }
    // Only an assignment in the function body (so we don't consume '$' here)
    if (!match("INDENT"))
    {
        error("Expected INDENT after ':'");
    }
    while (peek().type != "DEDENT" && peek().type != "EOF")
    {
        statement();
    }
    if (!match("DEDENT"))
    {
        error("Expected DEDENT at end of if block");
    }
}

// func_call ::= id '(' [expr (',' expr)*] ')'
void func_call()
{
    if (!match("id"))
        error("Expected function name");
    int idx = stoi(previous().value);
    cout << "Function call: " << identifiers_List[idx].name << endl;

    if (!match("("))
        error("Expected '(' after function name");

    // Parameter list (optional)
    if (peek().type != ")")
    {
        expr(); // Parse first expression

        while (peek().type == ",")
        {
            match(","); // Consume comma
            expr();     // Parse next expression
        }
    }

    if (!match(")"))
        error("Expected ')' after arguments");
}

void import_stmt()
{

    if (!match("keyword") || previous().value != "import")
        error("Expected 'import'");

    if (!match("id"))
        error("Expected module name after 'import'");
    int idx = stoi(previous().value);
    cout << "Importing module: " << identifiers_List[idx].name << endl;

    // Optional 'as' clause
    if (peek().type == "keyword" && peek().value == "as")
    {
        match("keyword"); // Consume 'as'
        if (!match("id"))
            error("Expected alias name after 'as'");
        int alias_idx = stoi(previous().value);
        cout << "Alias: " << identifiers_List[alias_idx].name << endl;
    }
}

// print_stmt ::= print expr
void print_stmt()
{
    if (!match("keyword") || previous().value != "print")
        error("Expected 'print'");

    if (!match("("))
        error("Expected '(' after 'print'");

    expr();  // First expression (e.g., a, "hello", etc.)

    while (match(","))  // Keep parsing additional expressions after each comma
    {
        expr();
    }

    if (!match(")"))
        error("Expected ')' at the end of print statement");

    cout << "Print statement parsed successfully.\n";
}


void statement()
{
    if (peek().type == "id")
    {
        if (tokensList[current + 1].type == "(")
        {
            func_call();
        }
        else
        {
            assignment_stmt();
        }
        if (!match("$"))
            error("Expected '$' after function call or assignment");
    }
    else if (peek().type == "keyword")
    {
        string kw = peek().value;

        if (kw == "if")
        {
            if_else_stmt(); // NO $ expected here
        }
        else if (kw == "while")
        {
            while_stmt();   // NO $ expected here
        }
        else if (kw == "for")
        {
            for_stmt();     // NO $ expected here
        }
        else if (kw == "def")
        {
            func_def();     // NO $ expected here
        }
        else if (kw == "class")
        {
            class_stmt();   // NO $ expected here
        }
        else if (kw == "return")
        {
            return_stmt();
            if (!match("$"))
                error("Expected '$' after return");
        }
        else if (kw == "print")
        {
            print_stmt();
            if (!match("$"))
                error("Expected '$' after print");
        }
        else if (kw == "import")
        {
            import_stmt();
            if (!match("$"))
                error("Expected '$' after import");
        }
        else
        {
            error("Unsupported keyword: " + kw);
        }
    }
    else
    {
        error("Unrecognized statement start");
    }
}

void program()
{
    while (peek().type != "EOF")
    {
        statement();
    }
    cout << "End of program" << endl;
}

void parseTokens(const vector<Token> &tokenStream)
{
    tokensList.clear();
    current = 0;
    tokensList = tokenStream;

    if (tokensList.empty() || tokensList.back().type != "EOF")
        tokensList.push_back({"EOF", ""});

    // Debug print
    cout << "\nParsed Tokens:\n";
    for (const auto &t : tokensList)
    {
        cout << "<" << t.type << "," << t.value << ">\n";
    }

    cout << "\n--- Starting Grammar Parsing ---\n";
    program();
    cout << "--- Parsing completed successfully. ---\n";
}

//////////////////////////////////

void print_symbolsTable()
{
    cout << "\nSymbol Table:\n";
    cout << "--------------------------------------------------------------------------------------------------------" << endl;
    cout << setw(6) << "Index"
         << setw(15) << "Identifier"
         << setw(10) << "Type"
         << setw(20) << "Scope"
         << setw(25) << "Value" << endl;
    cout << "--------------------------------------------------------------------------------------------------------" << endl;

    for (size_t i = 0; i < identifiers_List.size(); ++i)
    {
        const auto &id = identifiers_List[i];
        cout << setw(6) << i
             << setw(15) << id.name
             << setw(10) << id.type
             << setw(20) << id.scope
             << setw(25) << (id.value.empty() ? "-" : id.value) << endl;
    }
    cout << "--------------------------------------------------------------------------------------------------------" << endl;
};

void printTokensForParser()
{
    cout << "___________________________________________________________" << endl;
    cout << "___________________________________________________________" << endl;
    cout << "___________________________________________________________" << endl;
    cout << "\nTokens Stored for Parser:\n";
    for (const auto &token : tokensListForParser)
    {
        cout << "<" << token.type << "," << token.value << ">" << endl;
    }
    cout << "___________________________________________________________" << endl;
    cout << "___________________________________________________________" << endl;
    cout << "___________________________________________________________" << endl;
}

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
int availableIdentifiers(const string &word, const string &scope)
{
    for (int i = 0; i < identifiers_List.size(); i++)
    {
        if (word == identifiers_List[i].name && identifiers_List[i].scope == scope)
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

void detectDataType(const string &identifier, const string &value, const string &scope)
{

    string cleanedVal = value;

    cleanedVal.erase(remove_if(cleanedVal.begin(), cleanedVal.end(), ::isspace), cleanedVal.end());
    string type = "unknown";
    regex functionCallPattern(R"(^[a-zA-Z_][a-zA-Z0-9_]*\(.*\)$)");
    regex hexPattern(R"(0[xX][0-9a-fA-F]+)");
    regex numericPattern(R"(^-?\d+(\.\d+)?([eE][+-]?\d+)?$)");

    for (const auto &id : identifiers_List)
        if (id.name == identifier && id.scope == scope)
            return;

    if (isFunction(identifier))
    {
        type = "function";
    }
    else if (regex_match(cleanedVal, functionCallPattern))
    {
        string fname = cleanedVal.substr(0, cleanedVal.find('('));
        if (isFunction(fname))
        {
            type = "function_call";
        }
    }
    else if (regex_match(cleanedVal, hexPattern))
    {
        type = "int";
    }
    else if (!cleanedVal.empty() && cleanedVal.front() == '[')
    {
        type = "list";
    }
    else if (!cleanedVal.empty() && cleanedVal.front() == '{')
    {
        bool isDict = false;
        bool inString = false;
        char quoteChar = 0;

        for (size_t i = 1; i < cleanedVal.size(); i++)
        {
            char ch = cleanedVal[i];

            if ((ch == '"' || ch == '\'') && (i == 1 || cleanedVal[i - 1] != '\\'))
            {
                if (inString && ch == quoteChar)
                    inString = false;
                else if (!inString)
                    inString = true, quoteChar = ch;
            }

            if (!inString && ch == ':')
            {
                isDict = true;
                break;
            }
        }
        type = isDict ? "dict" : "set";
    }
    else if (cleanedVal.find('(') != string::npos && cleanedVal.find(')') != string::npos)
    {
        string inside = cleanedVal.substr(cleanedVal.find('('), cleanedVal.rfind(')') - cleanedVal.find('(') + 1);

        // Check for tuple by comma inside the parentheses
        if (inside.find(',') != string::npos)
        {
            type = "tuple";
        }
        else
        {
            // Refined detection: check for dot/expo/div inside or immediately outside the expression
            bool hasFloat = inside.find('.') != string::npos;
            bool hasExpo = inside.find('e') != string::npos || inside.find('E') != string::npos;

            // Check if the expression is followed by division (e.g., (2 + 8) / 4)
            size_t closingParen = cleanedVal.rfind(')');
            bool followedByDiv = (closingParen != string::npos &&
                                  closingParen + 1 < cleanedVal.size() &&
                                  cleanedVal[closingParen + 1] == '/');

            type = (hasFloat || hasExpo || followedByDiv) ? "float" : "int";
        }
    }
    else if (!cleanedVal.empty() && (cleanedVal.front() == '"' || cleanedVal.front() == '\''))
    {
        type = "string";
    }
    else if (cleanedVal == "True" || cleanedVal == "False")
    {
        type = "bool";
    }
    else if (regex_match(cleanedVal, numericPattern))
    {
        type = (cleanedVal.find('.') != string::npos || cleanedVal.find('e') != string::npos || cleanedVal.find('E') != string::npos) ? "float" : "int";
    }
    else
    {
        // expression handling
        bool hasFloat = false, hasInt = false;

        string token;
        for (size_t i = 0; i <= cleanedVal.size(); ++i)
        {
            if (i < cleanedVal.size() && (isalnum(cleanedVal[i]) || cleanedVal[i] == '_' || cleanedVal[i] == '.' || cleanedVal[i] == '-'))
            {
                token += cleanedVal[i];
            }
            else
            {
                if (!token.empty())
                {
                    if (regex_match(token, numericPattern))
                    {
                        if (token.find('.') != string::npos || token.find('e') != string::npos || token.find('E') != string::npos)
                            hasFloat = true;
                        else
                            hasInt = true;
                    }
                    else
                    {
                        for (const auto &id : identifiers_List)
                        {
                            if (id.name == token)
                            {
                                if (id.type == "float")
                                    hasFloat = true;
                                else if (id.type == "int")
                                    hasInt = true;
                                else if (type == "unknown")
                                    type = id.type;
                                break;
                            }
                        }
                    }
                    token.clear();
                }
            }
        }

        if (hasFloat)
            type = "float";
        else if (hasInt && type == "unknown")
            type = "int";
    }

    identifiers_List.push_back({static_cast<int>(identifiers_List.size()), identifier, type, scope, cleanedVal});
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

void handleIndentation(const string &line, stack<int> &indentStack, stack<string> &scopeStack, string &currentScope)
{
    int spaces = 0;
    for (char c : line)
    {
        if (c == ' ')
            spaces++;
        else
            break;
    }

    // Ignore blank lines
    if (line.find_first_not_of(" \t\r\n") == string::npos)
        return;

    int currentIndent = indentStack.top();

    if (spaces > currentIndent)
    {
        indentStack.push(spaces);
        cout << "<INDENT>" << endl;
        tokensListForParser.push_back({"INDENT", "INDENT"});
    }
    else if (spaces < currentIndent)
    {
        while (!indentStack.empty() && spaces < indentStack.top())
        {
            indentStack.pop();
            cout << "<DEDENT>" << endl;
            tokensListForParser.push_back({"DEDENT", "DEDENT"});

            // Do not pop function scope unless dedent below function indent level
            if (scopeStack.size() > 1)
            {
                string topScope = scopeStack.top();

                if (functionIndentLevels.count(topScope))
                {
                    int functionLevel = functionIndentLevels[topScope];
                    if (spaces <= functionLevel)
                    {
                        scopeStack.pop();
                        currentScope = scopeStack.top();
                        functionIndentLevels.erase(topScope);
                    }
                }
                else
                {
                    scopeStack.pop();
                    currentScope = scopeStack.top();
                }
            }
        }
    }
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

////////////////////////// misiing colon (:)////////////////
bool check_missingColons(const vector<string> &lines)
{
    bool foundError = false;
    regex blockStartPattern(R"(^\s*(def|if|elif|else|for|while|class)\b[^\n:]*\s*$)");

    for (int i = 0; i < lines.size(); i++)
    {
        string line = lines[i];

        if (isCommentLine(line))
            continue;

        if (regex_match(line, blockStartPattern))
        {
            cout << "Missing colon at line " << i + 1 << ": " << lines[i] << endl;
            foundError = true;
        }
    }

    return foundError;
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

bool check_indentationMismatch(const vector<string> &lines)
{
    bool errorfound = false;
    stack<int> indentStack;
    indentStack.push(0);

    for (int i = 0; i < lines.size(); ++i)
    {
        const string &line = lines[i];
        int spaces = 0;
        if (isCommentLine(line))
        {
            continue;
        }

        // Count leading spaces
        for (char c : line)
        {
            if (c == ' ')
                spaces++;
            else
                break;
        }

        int currentIndent = indentStack.top();

        if (spaces > currentIndent)
        {
            indentStack.push(spaces);
        }
        else if (spaces < currentIndent)
        {
            while (!indentStack.empty() && spaces < indentStack.top())
            {
                indentStack.pop();
            }

            // If after popping it still doesn't match, it's a mismatch
            if (!indentStack.empty() && spaces != indentStack.top())
            {
                cout << "Indentation mismatch at line " << (i + 1) << ": " << line << endl;
                errorfound = true;
            }
        }
    }

    return errorfound;
}

///////////////////////////////////////////////////////////

bool handleErrors(const vector<string> &lines)
{
    bool errorFound = false;
    // to run all errors b3den ywa2f
    errorFound |= check_invalidCharacters(lines);
    errorFound |= check_unterminatedStrings(lines);
    errorFound |= check_invalidnumber(lines);
    errorFound |= check_invalid_operators(lines);
    errorFound |= check_invalidIdentifiers(lines);
    errorFound |= check_unclosedBrackets(lines);
    errorFound |= check_missingColons(lines);
    errorFound |= check_indentationMismatch(lines);

    return errorFound;
}

void placeDelimiter(const string &line)
{
    string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));

    // Lowercase copy for keyword checks
    string lowered = trimmed;
    transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

    // Skip control structure headers
    if (lowered.rfind("if", 0) == 0 ||
        lowered.rfind("for", 0) == 0 ||
        lowered.rfind("while", 0) == 0)
    {
        return; // Do NOT add $
    }

    // Return, print, import, from
    if (lowered.rfind("return", 0) == 0 ||
        lowered.rfind("print", 0) == 0 ||
        lowered.rfind("import", 0) == 0 ||
        lowered.rfind("from", 0) == 0)
    {
        tokensListForParser.push_back({ "$", "$" });
        return;
    }

    // Assignment (not ==, !=, etc.)
    size_t equalPos = trimmed.find('=');
    if (equalPos != string::npos)
    {
        // Avoid '==', '>=', etc.
        if (trimmed[equalPos + 1] != '=' && equalPos > 0)
        {
            tokensListForParser.push_back({ "$", "$" });
            return;
        }
    }

    // Function call (simple pattern check)
    size_t parenPos = trimmed.find('(');
    if (parenPos != string::npos && parenPos > 0)
    {
        bool isValidFuncCall = true;
        for (size_t i = 0; i < parenPos; ++i)
        {
            char c = trimmed[i];
            if (!isalnum(c) && c != '_')
            {
                isValidFuncCall = false;
                break;
            }
        }
        if (isValidFuncCall)
        {
            tokensListForParser.push_back({ "$", "$" });
            return;
        }
    }
}


void lexer(string file)
{

    vector<string> cleanedFileLines = removemultiline(file);

    if (handleErrors(cleanedFileLines))
    {
        cout << "Lexical analysis stopped due to errors." << endl;
        return;
    }

    stack<int> indentStack;
    indentStack.push(0);

    string currentScope = "global";
    stack<string> scopeStack;
    scopeStack.push("global");

    for (string line : cleanedFileLines)
    {
        string cleanedLine = removecomments(line);
        handleIndentation(line, indentStack, scopeStack, currentScope);

        bool isFunctionDef = cleanedLine.find("def ") != string::npos;
        if (isFunctionDef)
        {
            size_t defPos = cleanedLine.find("def ") + 4;
            size_t startParams = cleanedLine.find('(', defPos);
            size_t endParams = cleanedLine.find(')', startParams);

            if (startParams != string::npos && endParams != string::npos && endParams > startParams)
            {
                string funcName = cleanedLine.substr(defPos, startParams - defPos);
                string paramSection = cleanedLine.substr(startParams + 1, endParams - startParams - 1);

                cout << "<Keyword,def>" << endl;
                tokensListForParser.push_back({"keyword", "def"});
                cout << "identifier: <id," << (availableIdentifiers(funcName, "global") == -1 ? identifiers_List.size() : availableIdentifiers(funcName, "global")) << ">\t" << funcName << endl;
                tokensListForParser.push_back({"id", to_string(availableIdentifiers(funcName, "global") == -1 ? identifiers_List.size() : availableIdentifiers(funcName, "global"))});
                cout << "Punctuation: <(>" << endl;
                tokensListForParser.push_back({"(", "("});

                // Add function name as identifier to global scope
                if (availableIdentifiers(funcName, "global") == -1)
                {
                    identifiers_List.push_back({static_cast<int>(identifiers_List.size()), funcName, "function", "global", ""});
                    functions_list.push_back(funcName);
                }

                currentScope = funcName;
                scopeStack.push(funcName);

                int spaces = 0;
                for (char c : line)
                    if (c == ' ')
                        spaces++;
                    else
                        break;

                functionIndentLevels[funcName] = spaces;

                auto splitAndClean = [](const string &str, char delim)
                {
                    vector<string> tokens;
                    stringstream ss(str);
                    string token;
                    while (getline(ss, token, delim))
                    {
                        token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
                        tokens.push_back(token);
                    }
                    return tokens;
                };

                vector<string> params = splitAndClean(paramSection, ',');

                for (size_t i = 0; i < params.size(); ++i)
                {
                    const string &param = params[i];
                    size_t eq = param.find('=');
                    if (eq != string::npos)
                    {
                        string var = param.substr(0, eq);
                        string val = param.substr(eq + 1);

                        // Fix for trailing colon in default values like `10):`
                        if (!val.empty() && val.back() == ':')
                            val.pop_back();

                        if (availableIdentifiers(var, currentScope) == -1)
                            detectDataType(var, val, currentScope);

                        cout << "identifier: <id," << availableIdentifiers(var, currentScope) << ">\t" << var << endl;
                        tokensListForParser.push_back({"id", to_string(availableIdentifiers(var, currentScope))});
                        cout << "Operator: <=>" << endl;
                        tokensListForParser.push_back({"=", "="});
                        cout << "Numeric: <" << val << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", val});
                    }
                    else
                    {
                        if (availableIdentifiers(param, currentScope) == -1)
                        {
                            identifiers_List.push_back({static_cast<int>(identifiers_List.size()), param, "unknown", currentScope});
                        }
                        cout << "identifier: <id," << availableIdentifiers(param, currentScope) << ">\t" << param << endl;
                        tokensListForParser.push_back({"id", to_string(availableIdentifiers(param, currentScope))});
                    }

                    if (i < params.size() - 1)
                    {
                        cout << "Punctuation: <,>" << endl;
                        tokensListForParser.push_back({",", ","});
                    }
                }

                cout << "Punctuation: <)>" << endl;
                tokensListForParser.push_back({")", ")"});

                // Look for colon after ')'
                size_t colonPos = cleanedLine.find(':', endParams);
                if (colonPos != string::npos)
                {
                    cout << "Punctuation: <:>" << endl;
                    tokensListForParser.push_back({":", ":"});
                }

                continue;
            }
        }

        

        if (isCommentLine2(line))
            continue;
        // Handle assignments first
        if (cleanedLine.find('=') != string::npos && cleanedLine.find("def ") == string::npos)
        {
            size_t eq = cleanedLine.find('=');
            string left = cleanedLine.substr(0, eq);
            string right = cleanedLine.substr(eq + 1);

            bool leftHasComma = left.find(',') != string::npos;
            bool rightHasComma = right.find(',') != string::npos;

            if (leftHasComma && rightHasComma)
            {
                auto splitAndClean = [](const string &str, char delim)
                {
                    vector<string> tokens;
                    stringstream ss(str);
                    string token;
                    while (getline(ss, token, delim))
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
                    detectDataType(vars[i], values[i], currentScope);
                }
            }
            else
            {
                string var = left;
                var.erase(remove_if(var.begin(), var.end(), ::isspace), var.end());
                string val = right;
                val.erase(remove_if(val.begin(), val.end(), ::isspace), val.end());
                detectDataType(var, val, currentScope);
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
                tokensListForParser.push_back({"NUMBER", expoNum});
                currentToken.clear();
                --i;
                continue;
            }

            string one_syntax(1, c);
            string two_syntax = (i + 1 < cleanedLine.size()) ? cleanedLine.substr(i, 2) : "";
            string three_syntax = (i + 2 < cleanedLine.size()) ? cleanedLine.substr(i, 3) : "";

            if ((three_syntax.size() == 3 && isOperator(three_syntax) && isIsolated(i, 3, cleanedLine)) ||
                (three_syntax.size() == 3 && isPunctuation(three_syntax)))
            {
                if (!currentToken.empty())
                {
                    if (isKeyword(currentToken))
                    {
                        cout << "<Keyword," << currentToken << ">" << endl;
                        tokensListForParser.push_back({"keyword", currentToken});
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken, currentScope) == -1 &&
                            !isKeyword(currentToken) &&
                            isalpha(currentToken[0]) &&
                            !isPunctuation(currentToken) &&
                            !currentToken.empty() &&
                            currentToken.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = line.find('=');
                            string value = (equalPos != string::npos && equalPos < line.size() - 1)
                                               ? line.substr(equalPos + 1)
                                               : "";
                            size_t commaPos = value.find(',');
                            if (commaPos != string::npos)
                                value = value.substr(0, commaPos);
                            detectDataType(currentToken, value, currentScope);

                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(three_syntax))
                {
                    cout << "Operator: <" << three_syntax << ">" << endl;
                    tokensListForParser.push_back({three_syntax, three_syntax});
                }
                else
                {
                    cout << "Punctuation: <" << three_syntax << ">" << endl;
                    tokensListForParser.push_back({three_syntax, three_syntax});
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
                        tokensListForParser.push_back({"keyword", currentToken});
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken, currentScope) == -1 &&
                            !isKeyword(currentToken) &&
                            isalpha(currentToken[0]) &&
                            !isPunctuation(currentToken) &&
                            !currentToken.empty() &&
                            currentToken.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = line.find('=');
                            string value = (equalPos != string::npos && equalPos < line.size() - 1)
                                               ? line.substr(equalPos + 1)
                                               : "";
                            size_t commaPos = value.find(',');
                            if (commaPos != string::npos)
                                value = value.substr(0, commaPos);
                            detectDataType(currentToken, value, currentScope);

                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(two_syntax))
                {
                    cout << "Operator: <" << two_syntax << ">" << endl;
                    tokensListForParser.push_back({two_syntax, two_syntax});
                }
                else
                {
                    cout << "Punctuation: <" << two_syntax << ">" << endl;
                    tokensListForParser.push_back({two_syntax, two_syntax});
                }
                i += 1;
                continue;
            }

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
                        tokensListForParser.push_back({"keyword", currentToken});
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken, currentScope) == -1 &&
                            !isKeyword(currentToken) &&
                            isalpha(currentToken[0]) &&
                            !isPunctuation(currentToken) &&
                            !currentToken.empty() &&
                            currentToken.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = line.find('=');
                            string value = (equalPos != string::npos && equalPos < line.size() - 1)
                                               ? line.substr(equalPos + 1)
                                               : "";
                            size_t commaPos = value.find(',');
                            if (commaPos != string::npos)
                                value = value.substr(0, commaPos);
                            detectDataType(currentToken, value, currentScope);

                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                    }
                    currentToken.clear();
                }

                if (isOperator(one_syntax))
                {
                    cout << "Operator: <" << one_syntax << ">" << endl;
                    tokensListForParser.push_back({one_syntax, one_syntax});
                }
                else
                {
                    cout << "Punctuation: <" << one_syntax << ">" << endl;
                    tokensListForParser.push_back({one_syntax, one_syntax});
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
                        tokensListForParser.push_back({"keyword", currentToken});
                    }
                    else if (isexpocase(cleanedLine, i))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else if (isNumeric(currentToken) || ishexa(currentToken))
                    {
                        cout << "Numeric: <" << currentToken << ">" << endl;
                        tokensListForParser.push_back({"NUMBER", currentToken});
                    }
                    else
                    {
                        if (availableIdentifiers(currentToken, currentScope) == -1 &&
                            !isKeyword(currentToken) &&
                            isalpha(currentToken[0]) &&
                            !isPunctuation(currentToken) &&
                            !currentToken.empty() &&
                            currentToken.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
                        {
                            bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                            if (isFunc)
                            {
                                functions_list.push_back(currentToken);
                            }
                            size_t equalPos = line.find('=');
                            string value = (equalPos != string::npos && equalPos < line.size() - 1)
                                               ? line.substr(equalPos + 1)
                                               : "";
                            detectDataType(currentToken, value, currentScope);

                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                        else
                        {
                            cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                            tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                        }
                    }
                    currentToken.clear();
                }
            }
            else
            {
                currentToken += c;
            }
        }

        if (!currentToken.empty())
        {
            if (isKeyword(currentToken))
            {
                cout << "<Keyword," << currentToken << ">" << endl;
                tokensListForParser.push_back({"keyword", currentToken});
            }
            else if (isNumeric(currentToken) || ishexa(currentToken))
            {
                cout << "Numeric: <" << currentToken << ">" << endl;
                tokensListForParser.push_back({"NUMBER", currentToken});
            }
            else
            {
                if (availableIdentifiers(currentToken, currentScope) == -1 &&
                    !isKeyword(currentToken) &&
                    isalpha(currentToken[0]) &&
                    !isPunctuation(currentToken) &&
                    !currentToken.empty() &&
                    currentToken.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == string::npos)
                {
                    bool isFunc = cleanedLine.find("def " + currentToken) != string::npos;
                    if (isFunc)
                    {
                        functions_list.push_back(currentToken);
                    }
                    size_t equalPos = line.find('=');
                    string value = (equalPos != string::npos && equalPos < line.size() - 1)
                                       ? line.substr(equalPos + 1)
                                       : "";
                    size_t commaPos = value.find(',');
                    if (commaPos != string::npos)
                        value = value.substr(0, commaPos);
                    detectDataType(currentToken, value, currentScope);

                    cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                    tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                }
                else
                {
                    cout << "identifier: <id," << availableIdentifiers(currentToken, currentScope) << ">\t" << currentToken << endl;
                    tokensListForParser.push_back({"id", to_string(availableIdentifiers(currentToken, currentScope))});
                }
            }
            currentToken.clear();
        }

        for (const string &literal : literals)
        {
            cout << "<StringLiteral," << literal << ">" << endl;
        }

        placeDelimiter(line);
    }

    printTokensForParser();
    print_symbolsTable();

    cout << "Lexical analysis completed successfully." << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    string file;
    cout << "Enter the Python file name: ";
    cin >> file;

    lexer(file);

    // Write tokens to tokens.txt for tracing
    ofstream outFile("tokens.txt");
    if (!outFile.is_open())
    {
        cerr << "Error: Could not open tokens.txt for writing." << endl;
        return 0;
    }

    for (const auto &token : tokensListForParser)
    {
        outFile << "<" << token.type << "," << token.value << ">" << endl;
    }

    outFile.close();
    cout << "Tokens written to tokens.txt successfully." << endl;

    cout << "start parsing." << endl;
    parseTokens(tokensListForParser);
    return 0;
}