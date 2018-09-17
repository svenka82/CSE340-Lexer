/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */

#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>


#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
	"IF", "WHILE", "DO", "THEN", "PRINT",
	"PLUS", "MINUS", "DIV", "MULT",
	"EQUAL", "COLON", "COMMA", "SEMICOLON",
	"LBRAC", "RBRAC", "LPAREN", "RPAREN",
	"NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
	"DOT", "NUM", "ID", "ERROR","REALNUM","BASE08NUM","BASE16NUM" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };
Token previous;

void Token::Print()
{
	cout << "{" << this->lexeme << " , "
		<< reserved[(int) this->token_type] << " , "
		<< this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
	this->line_no = 1;
	tmp.lexeme = "";
	tmp.line_no = 1;
	tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
	char c;
	bool space_encountered = false;

	input.GetChar(c);
	line_no += (c == '\n');

	while (!input.EndOfInput() && isspace(c)) {
		space_encountered = true;
		input.GetChar(c);
		line_no += (c == '\n');
	}

	if (!input.EndOfInput()) {
		input.UngetChar(c);
	}
	return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
	for (int i = 0; i < KEYWORDS_COUNT; i++) {
		if (s == keyword[i]) {
			return true;
		}
	}
	return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
	for (int i = 0; i < KEYWORDS_COUNT; i++) {
		if (s == keyword[i]) {
			return (TokenType)(i + 1);
		}
	}
	return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
	char c;

	input.GetChar(c);
	if (isdigit(c)) {
		if (c == '0') {
			tmp.lexeme = "0";
		}
		else {
			tmp.lexeme = "";
			while (!input.EndOfInput() && isdigit(c)) {
				tmp.lexeme += c;
				input.GetChar(c);
			}
			if (!input.EndOfInput()) {
				input.UngetChar(c);
			}
		}
		// TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!

		tmp.token_type = NUM;
		tmp.line_no = line_no;
		return tmp;
	}
	else {
		if (!input.EndOfInput()) {
			input.UngetChar(c);
		}
		tmp.lexeme = "";
		tmp.token_type = ERROR;
		tmp.line_no = line_no;
		return tmp;
	}
}

Token LexicalAnalyzer::ScanRealNumber(string integralPart) {
	char c;
	string strtmp;

	input.GetChar(c);
	if (c == '.') {
		input.GetChar(c);
		while (!input.EndOfInput() && isdigit(c)) {
			strtmp += c;
			input.GetChar(c);
		}
		if (!input.EndOfInput()) {
			input.UngetChar(c);
		}

		if (strtmp.length() != 0) {
			tmp.lexeme = integralPart;
			tmp.lexeme += '.';
			tmp.lexeme += strtmp;
			tmp.token_type = REALNUM;
			tmp.line_no = line_no;
			return tmp;
		}
		else {
			input.UngetString(strtmp);
			input.UngetChar('.');

			tmp.lexeme = integralPart;
			tmp.token_type = NUM;
			tmp.line_no = line_no;
			return tmp;
		}

		/*if (ScanNumber().token_type == NUM) {
			strtmp = tmp.lexeme;
			tmp.lexeme = integralPart;
			tmp.lexeme += '.';
			tmp.lexeme += strtmp;
			tmp.token_type = REALNUM;
			tmp.line_no = line_no;
			return tmp;
		}
		else {
			input.UngetString(strtmp);
			input.UngetChar('.');

			tmp.lexeme = integralPart;
			tmp.token_type = NUM;
			tmp.line_no = line_no;
			return tmp;
		}*/
	}
}

Token LexicalAnalyzer::ScanBaseNumber(string baseNumber) {
	char c;
	string strtmp;

	input.GetChar(c);
	if (c == 'x') {

		strtmp = 'x';
		input.GetChar(c);
		//if (isspace(c)) input.UngetChar(c);
		strtmp += c;
		input.GetChar(c);
		//if (isspace(c)) input.UngetChar(c);
		strtmp += c;

		if (strtmp == "x08") {
			for (int i = 0; i < baseNumber.length(); i++) {
				if (!(baseNumber[i] - '7' <= 0 && baseNumber[i] - '0' >= 0)) {
					input.UngetString(strtmp);

					tmp.lexeme = baseNumber;
					tmp.token_type = NUM;
					tmp.line_no = line_no;
					return tmp;
				}
			}

			tmp.lexeme = baseNumber + "x08";
			tmp.token_type = BASE08NUM;
			tmp.line_no = line_no;
			return tmp;
		}
		else if (strtmp == "x16") {
			tmp.lexeme = baseNumber + "x16";
			tmp.token_type = BASE16NUM;
			tmp.line_no = line_no;
			return tmp;
		}

		if (isspace(c)) input.UngetChar(c);
		input.UngetString(strtmp);

		tmp.lexeme = baseNumber;
		tmp.token_type = NUM;
		tmp.line_no = line_no;
		return tmp;
	}
}

Token LexicalAnalyzer::ScanBase16Number(string baseNumber) {
	char c;
	string prefix;
	string suffix;
	input.GetChar(c);
	tmp.lexeme = baseNumber;

	while (!input.EndOfInput() && ((c - 'A' >= 0 && c - 'F' <= 0) || isdigit(c))) {
		prefix += c;
		input.GetChar(c);
	}
	if (!input.EndOfInput() && !isspace(c)) {
		if (c == 'x') {

			suffix = c;
			input.GetChar(c);
			//if (isspace(c)) input.UngetChar(c);
			suffix += c;
			input.GetChar(c);
			//if (isspace(c)) input.UngetChar(c);
			suffix += c;

			if (suffix == "x16") {
				tmp.lexeme += prefix;
				tmp.lexeme += suffix;
				tmp.token_type = BASE16NUM;
				tmp.line_no = line_no;

				return tmp;
			}
			input.UngetString(suffix);
		}
	}
	else if (isspace(c)) input.UngetChar(c);

	input.UngetString(prefix);
	tmp.line_no = line_no;
	tmp.token_type = NUM;

	return tmp;
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
	char c;
	input.GetChar(c);

	if (isalpha(c)) {
		tmp.lexeme = "";
		while (!input.EndOfInput() && isalnum(c)) {
			tmp.lexeme += c;
			input.GetChar(c);
		}
		if (!input.EndOfInput()) {
			input.UngetChar(c);
		}
		tmp.line_no = line_no;
		if (IsKeyword(tmp.lexeme))
			tmp.token_type = FindKeywordIndex(tmp.lexeme);
		else
			tmp.token_type = ID;
	}
	else {
		if (!input.EndOfInput()) {
			input.UngetChar(c);
		}
		tmp.lexeme = "";
		tmp.token_type = ERROR;
	}
	return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
	tokens.push_back(tok);;
	return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
	char c;
	Token temporary;

	// if there are tokens that were previously
	// stored due to UngetToken(), pop a token and
	// return it without reading from input
	if (!tokens.empty()) {
		tmp = tokens.back();
		tokens.pop_back();
		return tmp;
	}

	SkipSpace();
	tmp.lexeme = "";
	tmp.line_no = line_no;
	input.GetChar(c);
	switch (c) {
	case '.':
		tmp.token_type = DOT;
		return tmp;
	case '+':
		tmp.token_type = PLUS;
		return tmp;
	case '-':
		tmp.token_type = MINUS;
		return tmp;
	case '/':
		tmp.token_type = DIV;
		return tmp;
	case '*':
		tmp.token_type = MULT;
		return tmp;
	case '=':
		tmp.token_type = EQUAL;
		return tmp;
	case ':':
		tmp.token_type = COLON;
		return tmp;
	case ',':
		tmp.token_type = COMMA;
		return tmp;
	case ';':
		tmp.token_type = SEMICOLON;
		return tmp;
	case '[':
		tmp.token_type = LBRAC;
		return tmp;
	case ']':
		tmp.token_type = RBRAC;
		return tmp;
	case '(':
		tmp.token_type = LPAREN;
		return tmp;
	case ')':
		tmp.token_type = RPAREN;
		return tmp;
	case '<':
		input.GetChar(c);
		if (c == '=') {
			tmp.token_type = LTEQ;
		}
		else if (c == '>') {
			tmp.token_type = NOTEQUAL;
		}
		else {
			if (!input.EndOfInput()) {
				input.UngetChar(c);
			}
			tmp.token_type = LESS;
		}
		return tmp;
	case '>':
		input.GetChar(c);
		if (c == '=') {
			tmp.token_type = GTEQ;
		}
		else {
			if (!input.EndOfInput()) {
				input.UngetChar(c);
			}
			tmp.token_type = GREATER;
		}
		return tmp;
	default:
		if (isdigit(c)) {

			input.UngetChar(c);
			temporary = ScanNumber();

			input.GetChar(c);
			if (c == '.') {
				input.UngetChar(c);
				return ScanRealNumber(temporary.lexeme);
			}
			else if (c == 'x') {
				input.UngetChar(c);
				return ScanBaseNumber(temporary.lexeme);
			}
			else if (c - 'A' >= 0 && c - 'F' <= 0) {
				input.UngetChar(c);
				return ScanBase16Number(temporary.lexeme);
			}
			else {
				input.UngetChar(c);
				return temporary;
			}
		}
		else if (isalpha(c)) {
			input.UngetChar(c);
			return ScanIdOrKeyword();
		}
		else if (input.EndOfInput())
			tmp.token_type = END_OF_FILE;
		else
			tmp.token_type = ERROR;

		return tmp;
	}
}

int main()
{
	LexicalAnalyzer lexer;
	Token token;

	token = lexer.GetToken();
	token.Print();
	while (token.token_type != END_OF_FILE)
	{
		token = lexer.GetToken();
		token.Print();
	}
}
