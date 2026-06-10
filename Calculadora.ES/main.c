#include "calculadora.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void testaExpressao(const char *posFixa, float esperado) {
    char *inFixa = getInFixa((char *)posFixa);
    float valor = getValor((char *)posFixa);

    printf("Pos-fixa: %s\n", posFixa);

    if (inFixa == NULL || isnan(valor)) {
        printf("Resultado: expressao invalida\n\n");
        free(inFixa);
        return;
    }

    printf("Infixa:   %s\n", inFixa);
    printf("Valor:    %.6f", valor);

    if (!isnan(esperado)) {
        printf(" | Esperado: %.6f | Diferenca: %.6f", esperado, fabsf(valor - esperado));
    }

    printf("\n\n");
    free(inFixa);
}

int main(void) {
    printf("Avaliador de expressoes numericas\n\n");

    testaExpressao("3 4 + 5 *", 35.0f);
    testaExpressao("7 2 * 4 +", 18.0f);
    testaExpressao("8 5 2 4 + * +", 38.0f);
    testaExpressao("6 2 / 3 + 4 *", 24.0f);
    testaExpressao("9 5 2 8 * 4 + * +", 109.0f);
    testaExpressao("2 3 + log 5 /", 0.139794f);
    testaExpressao("10 log 3 ^ 2 +", 3.0f);
    testaExpressao("45 60 + 30 cos *", 90.932663f);
    testaExpressao("0.5 45 sen 2 ^ +", 1.0f);

    printf("Testes extras\n\n");

    testaExpressao("9 raiz 2 ^", 9.0f);
    testaExpressao("90 sen 45 cos +", 1.707107f);
    testaExpressao("10 3 %", 1.0f);
    testaExpressao("4 0 /", NAN);
    testaExpressao("-9 raiz", NAN);
    testaExpressao("0 log", NAN);

    return 0;
}