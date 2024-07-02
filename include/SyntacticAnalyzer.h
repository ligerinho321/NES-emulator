#pragma once

#include <Emulator.h>
#include <WG/UT/UTarray.h>

void SyntacticAnalyzer_Init(NES *nes);

void SyntacticAnalyzer_Exit(void);

void SyntacticAnalyzer_InitPostfix(UT_array *postfix);

void SyntacticAnalyzer_PrintPostfix(UT_array *postfix);

int64_t SyntacticAnalyzer_GetPostfixResult(UT_array *postfix);

void SyntacticAnalyzer_PrintOperators(void);

void SyntacticAnalyzer_PrintIdentifiers(void);

bool SyntacticAnalyzer_InfixToPostfix(GapBuffer *infix,UT_array *postfix);
