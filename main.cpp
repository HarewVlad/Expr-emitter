#include <iostream>
#include <string>
#include <stdexcept>
#include <assert.h>
#include <fstream>

enum TokenKind
{
	TOKEN_NONE,
	TOKEN_INT,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_ADD,
	TOKEN_SUB
};

struct Token
{
	TokenKind kind;
	std::string value;
};

Token token = {};
char *stream = nullptr;

#define OP(k, kind, s) \
{ \
	case k: \
	{ \
		token = Token {kind, s}; \
		stream++; \
	} \
	break; \
} \


void next()
{
begin:
	switch (*stream)
	{
	case ' ': case '\r': case '\n': case '\t':
	{
		stream++;
		goto begin;
	}
	break;
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
	{
		const char *begin = stream;
		while (isdigit(*stream))
		{
			stream++;
		}
		const char *end = stream;

		token = Token{ TOKEN_INT, std::string(begin, end - begin) };
	}
	break;
	OP('+', TOKEN_ADD, std::string("+"))
		OP('-', TOKEN_SUB, std::string("-"))
		OP('*', TOKEN_MUL, std::string("*"))
		OP('/', TOKEN_DIV, std::string("/"))
		OP('(', TOKEN_LPAREN, std::string("("))
		OP(')', TOKEN_RPAREN, std::string(")"))

	case '\0':
		break;
	default:
	{
		std::cout << "unexpected symbol: " << *stream;
		exit(EXIT_FAILURE);
	}
	break;
	}
}

#undef OP

enum
{
	MAX_CODE = 1024
};

uint8_t code[MAX_CODE] = {};
uint8_t *p = code;

void emit8(uint8_t byte)
{
	*p = byte;
	p++;
}

void emit32(uint32_t byte4)
{
	emit8(byte4 & 0xFF);
	emit8((byte4 >> 8) & 0xFF);
	emit8((byte4 >> 16) & 0xFF);
	emit8((byte4 >> 24) & 0xFF);
}

void emit64(uint64_t byte8)
{
	emit32(byte8 & 0xFFFFFFFF);
	emit32((byte8 >> 32) & 0xFFFFFFFF);
}

enum
{
	RAX,
	RCX,
	RDX,
	RBX,
	RSP,
	RBP,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	RIP,
	R_NONE
};

enum
{
	DIRECT = 3
};

void emitModRM(uint8_t mode, uint8_t rx, uint8_t rm)
{
	assert(mode < 4);
	assert(rx < 16);
	assert(rm < 16);
	emit8((mode << 6) | ((rx & 7) << 3) | (rm & 7));
}

void emitRex(uint8_t rx, uint8_t base)
{
	emit8(0x48 | (base >> 3) | ((rx >> 3) << 2));
}

#define EMIT_ADD_RR(dst, src) \
	emitRex(dst, src); \
	emit8(0x03); \
	emitModRM(DIRECT, dst, src); \

#define EMIT_SUB_RR(dst, src) \
	emitRex(dst, src); \
	emit8(0x2B); \
	emitModRM(DIRECT, dst, src); \

#define EMIT_MOV_RI(dst, immediate) \
	emitRex(0, dst); \
	emit8(0xC7); \
	emitModRM(DIRECT, 0, dst); \
	emit32(immediate); \

using Register = uint8_t;
uint32_t freeRegsMask = 0;
void initRegs()
{
	Register freeRegs[] = { RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 };
	for (size_t i = 0; i < sizeof(freeRegs) / sizeof(*freeRegs); i++)
	{
		freeRegsMask |= 1 << freeRegs[i];
	}
}

Register allocReg()
{
	assert(freeRegsMask != 0);
	Register r = 0;
	uint32_t temp = 1;
	while (r != 0xffff)
	{
		if (freeRegsMask & temp)
		{
			freeRegsMask ^= temp;
			return r;
		}
		r++;
		temp <<= 1;
	}
	return R_NONE;
}

void freeReg(Register r)
{
	freeRegsMask |= (1 << r);
}

bool isToken(TokenKind kind)
{
	if (token.kind == kind)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Register parse1()
{
	Register r = allocReg();
	EMIT_MOV_RI(r, std::stoi(token.value));
	next();
	return r;
}

Register parse0()
{
	Register r1 = parse1();
	while (isToken(TOKEN_ADD) || isToken(TOKEN_SUB))
	{
		TokenKind op = token.kind;
		next();
		Register r2 = parse1();
		if (op == TOKEN_ADD)
		{
			EMIT_ADD_RR(r1, r2);
		}
		else
		{
			EMIT_SUB_RR(r1, r2);
		}
	}
	return r1;
}

void parse()
{
	(void)parse0();
}

void emitCode()
{
	std::ofstream output("output.txt", std::ofstream::binary | std::ofstream::trunc);
	if (!output)
	{
		std::cout << "fatal: can't open file for output";
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < p - code; i++)
	{
		output << code[i];
	}

	output.close();
}

void initStream(char *expr)
{
	stream = expr;
	next();
}

int main()
{
	initStream(const_cast<char *>("1 + 2 + 3"));
	initRegs();

	parse();
	emitCode();
}