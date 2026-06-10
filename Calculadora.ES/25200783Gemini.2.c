#include "calculadora.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 128
#define MAX_EXPR 512
#define PI 3.14159265358979323846f
#define EPSILON 0.000001f

typedef struct {
    char texto[MAX_EXPR];
    int prioridade;
    char operador;
} ItemInfixo;

static int ehOperadorBinario(const char *token) {
    return strlen(token) == 1 && strchr("+-*/%^", token[0]) != NULL;
}

static int ehOperadorUnario(const char *token) {
    return strcmp(token, "raiz") == 0 ||
           strcmp(token, "sen") == 0 ||
           strcmp(token, "cos") == 0 ||
           strcmp(token, "tg") == 0 ||
           strcmp(token, "log") == 0;
}

static int prioridadeOperador(char operador) {
    if (operador == '+' || operador == '-') {
        return 1;
    }
    if (operador == '*' || operador == '/' || operador == '%') {
        return 2;
    }
    if (operador == '^') {
        return 3;
    }
    return 4;
}

static int ehNumero(const char *token, float *valor) {
    char *fim = NULL;
    if (token == NULL || token[0] == '\0') {
        return 0;
    }
    *valor = strtof(token, &fim);
    return fim != token && *fim == '\0';
}

static int precisaParentesesEsquerda(const ItemInfixo *item, char operadorAtual) {
    int prioridadeAtual = prioridadeOperador(operadorAtual);
    
    // Se o item esquerdo tem prioridade menor, precisa de parenteses
    if (item->prioridade < prioridadeAtual) {
        return 1;
    }
    // Caso especial do operador ^ (associatividade a direita)
    if (operadorAtual == '^' && item->prioridade == prioridadeAtual) {
        return 1;
    }
    return 0;
}

static int precisaParentesesDireita(const ItemInfixo *item, char operadorAtual) {
    int prioridadeAtual = prioridadeOperador(operadorAtual);
    
    // Forca parenteses se o operador atual for + ou - e o bloco da direita for uma subexpressao composta (*, /, %, ^)
    // Isso garante exatamente o comportamento dos testes 3 e 5 da tabela do professor
    if ((operadorAtual == '+' || operadorAtual == '-') && item->prioridade > 1 && item->prioridade < 4) {
        return 1;
    }
    
    if (item->prioridade < prioridadeAtual) {
        return 1;
    }
    if ((operadorAtual == '-' || operadorAtual == '/' || operadorAtual == '%') &&
        item->prioridade == prioridadeAtual) {
        return 1;
    }
    if (operadorAtual == '*' &&
        item->prioridade == prioridadeAtual &&
        (item->operador == '/' || item->operador == '%')) {
        return 1;
    }
    return 0;
}

static int montaExpressaoBinaria(ItemInfixo *destino,
                                 const ItemInfixo *esquerda,
                                 const ItemInfixo *direita,
                                 char operador) {
    char ladoEsquerdo[MAX_EXPR];
    char ladoDireito[MAX_EXPR];
    int escritos;

    if (precisaParentesesEsquerda(esquerda, operador)) {
        escritos = snprintf(ladoEsquerdo, sizeof(ladoEsquerdo), "(%s)", esquerda->texto);
    } else {
        escritos = snprintf(ladoEsquerdo, sizeof(ladoEsquerdo), "%s", esquerda->texto);
    }
    if (escritos < 0 || escritos >= (int)sizeof(ladoEsquerdo)) {
        return 0;
    }

    if (precisaParentesesDireita(direita, operador)) {
        escritos = snprintf(ladoDireito, sizeof(ladoDireito), "(%s)", direita->texto);
    } else {
        escritos = snprintf(ladoDireito, sizeof(ladoDireito), "%s", direita->texto);
    }
    if (escritos < 0 || escritos >= (int)sizeof(ladoDireito)) {
        return 0;
    }

    escritos = snprintf(destino->texto,
                        sizeof(destino->texto),
                        "%s %c %s",
                        ladoEsquerdo,
                        operador,
                        ladoDireito);

    if (escritos < 0 || escritos >= (int)sizeof(destino->texto)) {
        return 0;
    }

    destino->prioridade = prioridadeOperador(operador);
    destino->operador = operador;
    return 1;
}

static int montaExpressaoUnaria(ItemInfixo *destino,
                                const ItemInfixo *operando,
                                const char *operador) {
    int escritos = snprintf(destino->texto,
                            sizeof(destino->texto),
                            "%s(%s)",
                            operador,
                            operando->texto);

    if (escritos < 0 || escritos >= (int)sizeof(destino->texto)) {
        return 0;
    }

    destino->prioridade = 4;
    destino->operador = '\0';
    return 1;
}

static int aplicaBinario(float a, float b, char operador, float *resultado) {
    switch (operador) {
        case '+':
            *resultado = a + b;
            return 1;
        case '-':
            *resultado = a - b;
            return 1;
        case '*':
            *resultado = a * b;
            return 1;
        case '/':
            if (fabsf(b) < EPSILON) {
                return 0;
            }
            *resultado = a / b;
            return 1;
        case '%':
            if (fabsf(b) < EPSILON) {
                return 0;
            }
            *resultado = fmodf(a, b);
            return 1;
        case '^':
            *resultado = powf(a, b);
            return 1;
        default:
            return 0;
    }
}

static int aplicaUnario(float a, const char *operador, float *resultado) {
    float radianos = a * PI / 180.0f;

    if (strcmp(operador, "raiz") == 0) {
        if (a < 0.0f) {
            return 0;
        }
        *resultado = sqrtf(a);
        return 1;
    }
    if (strcmp(operador, "sen") == 0) {
        *resultado = sinf(radianos);
        return 1;
    }
    if (strcmp(operador, "cos") == 0) {
        *resultado = cosf(radianos);
        return 1;
    }
    if (strcmp(operador, "tg") == 0) {
        if (fabsf(cosf(radianos)) < EPSILON) {
            return 0;
        }
        *resultado = tanf(radianos);
        return 1;
    }
    if (strcmp(operador, "log") == 0) {
        if (a <= 0.0f) {
            return 0;
        }
        *resultado = log10f(a);
        return 1;
    }
    return 0;
}

char *getInFixa(char *Str) {
    char copia[MAX_EXPR];
    char *token = NULL;
    ItemInfixo pilha[MAX_TOKENS];
    int topo = 0;

    if (Str == NULL || strlen(Str) >= sizeof(copia)) {
        return NULL;
    }

    strcpy(copia, Str);
    token = strtok(copia, " \t\r\n");

    while (token != NULL) {
        float valorNumerico;

        if (ehNumero(token, &valorNumerico)) {
            if (topo >= MAX_TOKENS) {
                return NULL;
            }
            snprintf(pilha[topo].texto, sizeof(pilha[topo].texto), "%s", token);
            pilha[topo].prioridade = 4;
            pilha[topo].operador = '\0';
            topo++;
        } else if (ehOperadorBinario(token)) {
            ItemInfixo novoItem;
            if (topo < 2) {
                return NULL;
            }
            if (!montaExpressaoBinaria(&novoItem, &pilha[topo - 2], &pilha[topo - 1], token[0])) {
                return NULL;
            }
            pilha[topo - 2] = novoItem;
            topo--;
        } else if (ehOperadorUnario(token)) {
            ItemInfixo novoItem;
            if (topo < 1) {
                return NULL;
            }
            if (!montaExpressaoUnaria(&novoItem, &pilha[topo - 1], token)) {
                return NULL;
            }
            pilha[topo - 1] = novoItem;
        } else {
            return NULL;
        }
        token = strtok(NULL, " \t\r\n");
    }

    if (topo != 1) {
        return NULL;
    }

    char *resultado = (char *)malloc(strlen(pilha[0].texto) + 1);
    if (resultado == NULL) {
        return NULL;
    }
    strcpy(resultado, pilha[0].texto);
    return resultado;
}

float getValor(char *Str) {
    char copia[MAX_EXPR];
    char *token = NULL;
    float pilha[MAX_TOKENS];
    int topo = 0;

    if (Str == NULL || strlen(Str) >= sizeof(copia)) {
        return NAN;
    }

    strcpy(copia, Str);
    token = strtok(copia, " \t\r\n");

    while (token != NULL) {
        float valorNumerico;

        if (ehNumero(token, &valorNumerico)) {
            if (topo >= MAX_TOKENS) {
                return NAN;
            }
            pilha[topo] = valorNumerico;
            topo++;
        } else if (ehOperadorBinario(token)) {
            float resultado;
            if (topo < 2) {
                return NAN;
            }
            if (!aplicaBinario(pilha[topo - 2], pilha[topo - 1], token[0], &resultado)) {
                return NAN;
            }
            pilha[topo - 2] = resultado;
            topo--;
        } else if (ehOperadorUnario(token)) {
            float resultado;
            if (topo < 1) {
                return NAN;
            }
            if (!aplicaUnario(pilha[topo - 1], token, &resultado)) {
                return NAN;
            }
            pilha[topo - 1] = resultado;
        } else {
            return NAN;
        }
        token = strtok(NULL, " \t\r\n");
    }

    if (topo != 1) {
        return NAN;
    }
    return pilha[0];
}