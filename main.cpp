#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <fstream>

using namespace std;

// Token types
enum TokenType
{
    ASSIGN_OP,
    PLUS_OP,
    MINUS_OP,
    SEMICOLON,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    EQ_OP,
    UNKNOWN,
    INT_KEYWORD,
    IF_KEYWORD,
    IDENTIFIER,
    NUMBER
};

// Token structure
struct Token
{
    TokenType type;
    string value;

    Token(TokenType t, const string &v) : type(t), value(v) {}
};

// AST Node structure
struct ASTNode
{
    string value;
    vector<ASTNode> children;

    ASTNode(const string &val, const vector<ASTNode> &childNodes = {})
        : value(val), children(childNodes) {}
};

// Tokenize the source code
vector<Token> tokenize(const string &source)
{
    vector<Token> tokens;
    for (size_t pos = 0; pos < source.size(); ++pos)
    {
        char current = source[pos];

        if (isspace(current))
            continue;
        if (current == '=')
        {
            if (source[pos + 1] == '=')
            {
                tokens.push_back(Token(EQ_OP, "=="));
                ++pos;
            }
            else
                tokens.push_back(Token(ASSIGN_OP, "="));
        }
        else if (current == '+')
            tokens.push_back(Token(PLUS_OP, "+"));
        else if (current == '-')
            tokens.push_back(Token(MINUS_OP, "-"));
        else if (current == ';')
            tokens.push_back(Token(SEMICOLON, ";"));
        else if (current == '{')
            tokens.push_back(Token(LBRACE, "{"));
        else if (current == '}')
            tokens.push_back(Token(RBRACE, "}"));
        else if (current == '(')
            tokens.push_back(Token(LPAREN, "("));
        else if (current == ')')
            tokens.push_back(Token(RPAREN, ")"));
        else if (isalpha(current))
        {
            string result;
            while (isalpha(source[pos]))
                result += source[pos++];
            --pos;
            if (result == "int")
                tokens.push_back(Token(INT_KEYWORD, result));
            else if (result == "if")
                tokens.push_back(Token(IF_KEYWORD, result));
            else
                tokens.push_back(Token(IDENTIFIER, result));
        }
        else if (isdigit(current))
        {
            string number;
            while (isdigit(source[pos]))
                number += source[pos++];
            --pos;
            tokens.push_back(Token(NUMBER, number));
        }
        else
            tokens.push_back(Token(UNKNOWN, string(1, current)));
    }
    return tokens;
}

// Parser functions
size_t pos = 0;
vector<Token> tokens;

ASTNode parseExpression(); // Forward declaration of parseExpression()
ASTNode parseStatement();  // Forward declaration of parseStatement()

// Consume a token
Token consume(TokenType type)
{
    if (tokens[pos].type == type)
        return tokens[pos++];
    throw runtime_error("Unexpected token");
}

// Parse an identifier
ASTNode parseIdentifier()
{
    auto identifier = consume(IDENTIFIER);
    return ASTNode("Identifier", {ASTNode(identifier.value)});
}

// Parse a declaration
ASTNode parseDeclaration()
{
    consume(INT_KEYWORD);
    auto identifier = consume(IDENTIFIER);
    consume(SEMICOLON);
    return ASTNode("Declaration", {ASTNode("Identifier", {ASTNode(identifier.value)})});
}

// Parse an assignment
ASTNode parseAssignment()
{
    auto identifier = consume(IDENTIFIER);
    consume(ASSIGN_OP);
    auto expr = parseExpression();
    consume(SEMICOLON);
    return ASTNode("Assignment", {ASTNode("Identifier", {ASTNode(identifier.value)}), expr});
}

// Parse a term
ASTNode parseTerm()
{
    if (tokens[pos].type == NUMBER)
        return ASTNode("Number", {ASTNode(consume(NUMBER).value)});
    if (tokens[pos].type == IDENTIFIER)
        return parseIdentifier();
    throw runtime_error("Expected term");
}

// Parse an expression
ASTNode parseExpression()
{
    auto left = parseTerm();
    if (tokens[pos].type == PLUS_OP || tokens[pos].type == MINUS_OP)
    {
        auto op = consume(tokens[pos].type);
        auto right = parseTerm();
        return ASTNode(op.value, {left, right});
    }
    return left;
}

// Parse an equality condition
ASTNode parseCondition()
{
    auto left = parseTerm();
    consume(EQ_OP);
    auto right = parseTerm();
    return ASTNode("==", {left, right});
}

// Parse an if statement
ASTNode parseIfStatement()
{
    consume(IF_KEYWORD);
    consume(LPAREN);
    auto condition = parseCondition();
    consume(RPAREN);
    consume(LBRACE);
    vector<ASTNode> body;
    while (tokens[pos].type != RBRACE)
        body.push_back(parseStatement());
    consume(RBRACE);
    return ASTNode("If", {condition, ASTNode("Body", body)});
}

// Parse a statement
ASTNode parseStatement()
{
    if (tokens[pos].type == INT_KEYWORD)
        return parseDeclaration();
    if (tokens[pos].type == IF_KEYWORD)
        return parseIfStatement();
    return parseAssignment();
}

// Parse the entire program
ASTNode parseProgram(const vector<Token> &tokens)
{
    ASTNode root("Program");
    while (pos < tokens.size())
        root.children.push_back(parseStatement());
    return root;
}

// Print AST
void printAST(const ASTNode &node, int level = 0)
{
    for (int i = 0; i < level; ++i)
        cout << "  ";
    cout << node.value << endl;
    for (const auto &child : node.children)
        printAST(child, level + 1);
}

// Generate assembly code
void generateAssembly(const ASTNode &node, unordered_map<string, int> &symbolTable)
{
    if (node.value == "Program")
    {
        for (const auto &child : node.children)
            generateAssembly(child, symbolTable);
    }
    else if (node.value == "Declaration")
    {
        const string &varName = node.children[0].children[0].value;
        symbolTable[varName] = 0;
    }
    else if (node.value == "Assignment")
    {
        const string &varName = node.children[0].children[0].value;
        const ASTNode &expr = node.children[1];
        if (expr.value == "Number")
        {
            cout << "  MVI A, " << expr.children[0].value << endl;
        }
        else if (expr.value == "+")
        {
            const ASTNode &left = expr.children[0];
            const ASTNode &right = expr.children[1];
            if (left.value == "Identifier")
                cout << "  MOV A, " << left.children[0].value << endl;
            else
                cout << "  MVI A, " << left.children[0].value << endl;
            if (right.value == "Identifier")
                cout << "  ADD " << right.children[0].value << endl;
            else
                cout << "  ADI " << right.children[0].value << endl;
        }
        cout << "  STA " << varName << endl;
    }
    else if (node.value == "If")
    {
        static int labelCounter = 0;
        int label = labelCounter++;
        generateAssembly(node.children[0], symbolTable); // Condition
        cout << "  JNZ LABEL" << label << endl;
        for (const auto &stmt : node.children[1].children)
            generateAssembly(stmt, symbolTable);
        cout << "LABEL" << label << ":" << endl;
    }
    else if (node.value == "==")
    {
        const string &leftVar = node.children[0].children[0].value;
        const string &rightVal = node.children[1].children[0].value;
        cout << "  MOV A, " << leftVar << endl;
        cout << "  CPI " << rightVal << endl;
    }
}

// Main function to run the compiler
int main()
{
    //Read the file
    ifstream file("test.txt");
    string line;
    string source = "";
    if (file.is_open())
    {
        while (getline(file, line))
        {
            source += line;
        }
    }
    else
    {
        cout << "Can't open the input file.";
    }
    tokens = tokenize(source);

    unordered_map<string, int> symbolTable;

    try
    {
        ASTNode root = parseProgram(tokens);
        printAST(root);
        cout << "\nAssembly Code:\n";
        generateAssembly(root, symbolTable);
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}