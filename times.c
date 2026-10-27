/*
 * Sorteador de times de futebol.
 *
 * Uso:
 *   Compilar:  gcc times.c -o times
 *   Rodar:     ./times              (cole a lista e finalize com Ctrl+D)
 *              ./times lista.txt    (ou leia de um arquivo)
 *
 * Aceita linhas como "1. Isaque", "7.Pedro", "13) Eduardo" ou apenas "Moka".
 * Remove numeração e caracteres invisiveis (comuns ao copiar do WhatsApp).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_JOGADORES 100
#define MAX_NOME 100
#define NUM_TIMES 3

/* Retorna o tamanho (em bytes) do caractere "em branco" no inicio de s, ou 0. */
static int branco_inicio(const char *s) {
    unsigned char a = (unsigned char)s[0];
    unsigned char b = a ? (unsigned char)s[1] : 0;
    unsigned char c = b ? (unsigned char)s[2] : 0;

    if (a == ' ' || a == '\t' || a == '\r' || a == '\n') return 1;
    if (a == 0xC2 && (b == 0xA0 || b == 0xAD)) return 2;       /* NBSP, soft hyphen */
    if (a == 0xE2 && b == 0x80 && c >= 0x80 && c <= 0x8F) return 3; /* U+2000..200F espacos/zero-width/LRM/RLM */
    if (a == 0xE2 && b == 0x80 && c >= 0xA8 && c <= 0xAF) return 3; /* U+2028..202F marcas de direcao */
    if (a == 0xE2 && b == 0x81 && c >= 0xA0 && c <= 0xAF) return 3; /* U+2060..206F word joiner, isolates */
    if (a == 0xE3 && b == 0x80 && c == 0x80) return 3;          /* espaco ideografico */
    if (a == 0xEF && b == 0xBB && c == 0xBF) return 3;          /* BOM */
    return 0;
}

/* Remove brancos/invisiveis do final da string. */
static void trim_fim(char *s) {
    size_t n = strlen(s);
    int achou = 1;
    while (n > 0 && achou) {
        achou = 0;
        for (size_t len = 1; len <= 3 && len <= n; len++) {
            if ((size_t)branco_inicio(s + n - len) == len) {
                n -= len;
                s[n] = 0;
                achou = 1;
                break;
            }
        }
    }
}

/* Extrai o nome de uma linha. Retorna 1 se encontrou um nome. */
static int extrair_nome(const char *linha, char *saida) {
    const char *p = linha;
    int k;

    while ((k = branco_inicio(p)) > 0) p += k;

    /* numeracao: digitos seguidos de . ) - : */
    const char *q = p;
    while (*q >= '0' && *q <= '9') q++;
    if (q > p) {
        p = q;
        if (*p == '.' || *p == ')' || *p == '-' || *p == ':') p++;
    }

    while ((k = branco_inicio(p)) > 0) p += k;

    strncpy(saida, p, MAX_NOME - 1);
    saida[MAX_NOME - 1] = 0;
    trim_fim(saida);
    return saida[0] != 0;
}

static void embaralhar(char nomes[][MAX_NOME], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char tmp[MAX_NOME];
        strcpy(tmp, nomes[i]);
        strcpy(nomes[i], nomes[j]);
        strcpy(nomes[j], tmp);
    }
}

int main(int argc, char *argv[]) {
    static char nomes[MAX_JOGADORES][MAX_NOME];
    char linha[512];
    int n = 0;
    FILE *in = stdin;

    if (argc > 1) {
        in = fopen(argv[1], "r");
        if (!in) {
            perror("Erro ao abrir o arquivo");
            return 1;
        }
    } else {
        printf("Cole a lista de jogadores e pressione Ctrl+D (Ctrl+Z + Enter no Windows):\n");
    }

    while (n < MAX_JOGADORES && fgets(linha, sizeof linha, in)) {
        if (extrair_nome(linha, nomes[n])) n++;
    }
    if (in != stdin) fclose(in);

    if (n < NUM_TIMES) {
        fprintf(stderr, "Jogadores insuficientes (%d).\n", n);
        return 1;
    }

    srand((unsigned)time(NULL));
    embaralhar(nomes, n);

    printf("\n%d jogadores, %d times\n", n, NUM_TIMES);

    /* distribui em sequencia: com 18 jogadores, 6 por time */
    int por_time = (n + NUM_TIMES - 1) / NUM_TIMES;
    for (int t = 0; t < NUM_TIMES; t++) {
        printf("\n=== TIME %d ===\n", t + 1);
        for (int i = 0; i < por_time; i++) {
            int idx = t * por_time + i;
            if (idx >= n) break;
            printf("  %d. %s\n", i + 1, nomes[idx]);
        }
    }
    printf("\n");
    return 0;
}