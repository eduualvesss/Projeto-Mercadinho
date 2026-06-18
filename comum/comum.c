/* ============================================================
 *  comum.c — Implementação dos utilitários compartilhados
 * ============================================================ */
#include "comum.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ---------- Estado interno do diretório de dados ---------- */
static char g_dir_dados[260] = "";

/* ============================================================
 *  CONFIGURAÇÃO DE DIRETÓRIO
 * ============================================================ */

/* Garante que o caminho termine com separador '/' */
static void normalizar_dir(char *dir) {
    int len = (int)strlen(dir);
    if (len > 0 && dir[len - 1] != '/' && dir[len - 1] != '\\') {
        dir[len] = '/';
        dir[len + 1] = '\0';
    }
}

/* Tenta criar o diretório (ignora erro se já existir) */
static void criar_dir_se_preciso(const char *dir) {
    char cmd[300];
    snprintf(cmd, sizeof(cmd),
#ifdef _WIN32
             "mkdir \"%s\" 2>nul"
#else
             "mkdir -p \"%s\" 2>/dev/null"
#endif
             , dir);
    system(cmd);
}

void config_inicializar(void) {
    FILE *cfg = fopen(ARQ_CONFIG, "r");
    if (cfg) {
        if (fgets(g_dir_dados, sizeof(g_dir_dados), cfg)) {
            int len = (int)strlen(g_dir_dados);
            while (len > 0 && (g_dir_dados[len - 1] == '\n' || g_dir_dados[len - 1] == '\r'))
                g_dir_dados[--len] = '\0';
        }
        fclose(cfg);
        if (strlen(g_dir_dados) > 0) {
            normalizar_dir(g_dir_dados);
            criar_dir_se_preciso(g_dir_dados);
            return;
        }
    }

    /* Primeira execução: pergunta o diretório e salva a escolha */
    printf("\n ============================================================\n");
    printf("  CONFIGURAÇÃO INICIAL — MERCADINHO v%s\n", VERSAO);
    printf(" ============================================================\n");
    printf("  Onde deseja salvar os arquivos de dados (clientes, produtos\n");
    printf("  e pedidos)? Pressione ENTER para usar a pasta padrão 'data'.\n\n");
    printf("  Caminho: ");
    fgets(g_dir_dados, sizeof(g_dir_dados), stdin);
    int len = (int)strlen(g_dir_dados);
    while (len > 0 && (g_dir_dados[len - 1] == '\n' || g_dir_dados[len - 1] == '\r'))
        g_dir_dados[--len] = '\0';

    if (strlen(g_dir_dados) == 0)
        strcpy(g_dir_dados, "data");

    normalizar_dir(g_dir_dados);
    criar_dir_se_preciso(g_dir_dados);

    FILE *out = fopen(ARQ_CONFIG, "w");
    if (out) {
        fprintf(out, "%s\n", g_dir_dados);
        fclose(out);
    }
    printf("\n " GREEN "Diretório configurado: %s" RESET "\n", g_dir_dados);
    pausar();
}

const char *config_dir_dados(void) {
    return g_dir_dados;
}

void caminho_arquivo(char *dest, const char *nome_arquivo) {
    sprintf(dest, "%s%s", g_dir_dados, nome_arquivo);
}

/* ============================================================
 *  UTILITÁRIOS DE TELA / ENTRADA
 * ============================================================ */
void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    printf("\n Pressione ENTER para continuar...");
    getchar();
}

void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ler_string(const char *prompt, char *dest, int max) {
    printf("%s", prompt);
    fgets(dest, max, stdin);
    int len = (int)strlen(dest);
    if (len > 0 && dest[len - 1] == '\n')
        dest[len - 1] = '\0';
}

void data_hoje(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, 20, "%d/%m/%Y %H:%M", tm_info);
}

void data_hora_agora(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, 25, "%d/%m/%Y %H:%M:%S", tm_info);
}

void data_hoje_arquivo(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, 11, "%Y-%m-%d", tm_info);
}

void sep_txt(FILE *f, char c, int n) {
    for (int i = 0; i < n; i++) fputc(c, f);
    fputc('\n', f);
}

void ler_numeros_exatos(const char *prompt, char *dest, int max, int digitos_exatos) {
    char buf[256];
    while (1) {
        printf("%s", prompt);
        fgets(buf, sizeof(buf), stdin);
        int len = (int)strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';

        if (len == 0) { printf(" Campo obrigatorio! Digite apenas numeros.\n"); continue; }

        int valido = 1;
        for (int i = 0; i < len; i++)
            if (!isdigit((unsigned char)buf[i])) { valido = 0; break; }
        if (!valido) { printf(" Apenas numeros sao permitidos neste campo!\n"); continue; }

        if (digitos_exatos > 0 && len != digitos_exatos) {
            printf(" Quantidade incorreta! Este campo exige exatamente %d digito(s). "
                   "Voce digitou %d.\n", digitos_exatos, len);
            continue;
        }
        strncpy(dest, buf, max - 1);
        dest[max - 1] = '\0';
        return;
    }
}

int ler_inteiro_positivo(const char *prompt) {
    int val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &val) == 1) {
            limpar_buffer();
            if (val >= 0) return val;
            printf(" Valor nao pode ser negativo!\n");
        } else {
            limpar_buffer();
            printf(" Entrada invalida! Digite um numero inteiro.\n");
        }
    }
}

float ler_float_positivo(const char *prompt) {
    float val;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &val) == 1) {
            limpar_buffer();
            if (val >= 0.0f) return val;
            printf(" Valor nao pode ser negativo!\n");
        } else {
            limpar_buffer();
            printf(" Entrada invalida! Digite um numero.\n");
        }
    }
}

void ler_data_validade(int *dia, int *mes, int *ano) {
    while (1) {
        printf(" Data de validade (DD MM AAAA): ");
        if (scanf("%d %d %d", dia, mes, ano) != 3) {
            limpar_buffer();
            printf(" Formato invalido! Use DD MM AAAA (ex: 25 12 2026).\n");
            continue;
        }
        limpar_buffer();

        if (*dia < 1 || *dia > 31 || *mes < 1 || *mes > 12 || *ano < 2000) {
            printf(" Data invalida! Verifique dia (1-31), mes (1-12) e ano (>= 2000).\n");
            continue;
        }

        struct tm t = {0};
        t.tm_mday = *dia; t.tm_mon = *mes - 1; t.tm_year = *ano - 1900;
        t.tm_hour = 23; t.tm_min = 59; t.tm_sec = 59;
        time_t val = mktime(&t);

        if (t.tm_mday != *dia || t.tm_mon != (*mes - 1)) {
            printf(" Data invalida! Esse dia nao existe nesse mes.\n");
            continue;
        }

        if (difftime(val, time(NULL)) < 0) {
            printf(" " YELLOW "Atencao: data já passou — produto sera marcado como VENCIDO." RESET "\n");
            printf(" Deseja usar essa data mesmo assim? [S/N]: ");
            char c; scanf(" %c", &c); limpar_buffer();
            if (toupper((unsigned char)c) != 'S') continue;
        }
        return;
    }
}

void formatar_cpf(const char *entrada, char *saida) {
    char digits[12] = {0};
    int j = 0;
    for (int i = 0; entrada[i] && j < 11; i++)
        if (isdigit((unsigned char)entrada[i]))
            digits[j++] = entrada[i];
    if (j == 11)
        sprintf(saida, "%c%c%c.%c%c%c.%c%c%c-%c%c",
                digits[0],digits[1],digits[2], digits[3],digits[4],digits[5],
                digits[6],digits[7],digits[8], digits[9],digits[10]);
    else
        strcpy(saida, entrada);
}

void para_minusculas(const char *origem, char *destino) {
    int i;
    for (i = 0; origem[i]; i++) destino[i] = (char)tolower((unsigned char)origem[i]);
    destino[i] = '\0';
}

/* ============================================================
 *  LOG DIÁRIO  (acumula todo evento relevante do dia)
 * ============================================================ */
void log_evento(const char *modulo, const char *acao, const char *detalhe) {
    char caminho[300];
    caminho_arquivo(caminho, ARQ_LOG_DIARIO);
    FILE *f = fopen(caminho, "a");
    if (!f) return;

    char ts[25];
    data_hora_agora(ts);
    fprintf(f, "[%s] %-10s %-10s %s\n", ts, modulo, acao, detalhe);
    fclose(f);
}

/* ============================================================
 *  SNAPSHOT CONSOLIDADO DO DIA
 * ============================================================
 *  Lê clientes.dat, produtos.dat e pedidos.dat (do diretório de
 *  dados configurado) e grava um resumo do estado atual em:
 *    snapshot_AAAA-MM-DD.txt  (legível)
 *    snapshot_AAAA-MM-DD.dat  (cópia binária dos 3 arquivos, em sequência,
 *                              cada bloco precedido por um cabeçalho simples)
 * ============================================================ */
void gerar_snapshot_do_dia(void) {
    char data_arq[11];
    data_hoje_arquivo(data_arq);

    char nome_txt[64], nome_dat[64];
    sprintf(nome_txt, "snapshot_%s.txt", data_arq);
    sprintf(nome_dat, "snapshot_%s.dat", data_arq);

    char caminho_txt[300], caminho_dat[300];
    caminho_arquivo(caminho_txt, nome_txt);
    caminho_arquivo(caminho_dat, nome_dat);

    FILE *txt = fopen(caminho_txt, "w");
    FILE *dat = fopen(caminho_dat, "wb");
    if (!txt || !dat) {
        printf("\n " RED "[ERRO] Não foi possível criar os arquivos de snapshot." RESET "\n");
        if (txt) fclose(txt);
        if (dat) fclose(dat);
        return;
    }

    char agora[25];
    data_hora_agora(agora);
    fprintf(txt, "================================================================\n");
    fprintf(txt, " SNAPSHOT CONSOLIDADO DO DIA — MERCADINHO v%s\n", VERSAO);
    fprintf(txt, " Gerado em: %s\n", agora);
    fprintf(txt, "================================================================\n\n");

    /* ---- Bloco CLIENTES ---- */
    char p_cli[300]; caminho_arquivo(p_cli, ARQ_CLIENTES);
    FILE *f = fopen(p_cli, "rb");
    int n_cli = 0;
    fprintf(txt, "--- CLIENTES ---\n");
    if (f) {
        Cliente c;
        while (fread(&c, sizeof(Cliente), 1, f) == 1) {
            fprintf(txt, " [%d] %-30s %-15s %s\n", c.codigo, c.nome, c.cpf,
                    c.ativo ? "ATIVO" : "INATIVO");
            n_cli++;
        }
        rewind(f);
        fwrite("CLIENTES", 1, 8, dat);
        fwrite(&n_cli, sizeof(int), 1, dat);
        Cliente cc;
        while (fread(&cc, sizeof(Cliente), 1, f) == 1)
            fwrite(&cc, sizeof(Cliente), 1, dat);
        fclose(f);
    } else {
        fprintf(txt, " (nenhum registro)\n");
        fwrite("CLIENTES", 1, 8, dat);
        fwrite(&n_cli, sizeof(int), 1, dat);
    }
    fprintf(txt, " Total: %d cliente(s)\n\n", n_cli);

    /* ---- Bloco PRODUTOS ---- */
    char p_prod[300]; caminho_arquivo(p_prod, ARQ_PRODUTOS);
    f = fopen(p_prod, "rb");
    int n_prod = 0;
    fprintf(txt, "--- PRODUTOS ---\n");
    if (f) {
        Produto p;
        while (fread(&p, sizeof(Produto), 1, f) == 1) {
            fprintf(txt, " [%d] %-25s qtd:%-5d venda:R$%-8.2f\n",
                    p.codigo, p.nome, p.quantidade, p.preco_venda);
            n_prod++;
        }
        rewind(f);
        fwrite("PRODUTOS", 1, 8, dat);
        fwrite(&n_prod, sizeof(int), 1, dat);
        Produto pp;
        while (fread(&pp, sizeof(Produto), 1, f) == 1)
            fwrite(&pp, sizeof(Produto), 1, dat);
        fclose(f);
    } else {
        fprintf(txt, " (nenhum registro)\n");
        fwrite("PRODUTOS", 1, 8, dat);
        fwrite(&n_prod, sizeof(int), 1, dat);
    }
    fprintf(txt, " Total: %d produto(s)\n\n", n_prod);

    /* ---- Bloco PEDIDOS (somente os do dia de hoje) ---- */
    char p_ped[300]; caminho_arquivo(p_ped, ARQ_PEDIDOS);
    f = fopen(p_ped, "rb");
    int n_ped = 0;
    float total_dia = 0.0f;
    fprintf(txt, "--- PEDIDOS DE HOJE ---\n");
    long pos_contagem = dat ? ftell(dat) : 0;
    fwrite("PEDIDOS!", 1, 8, dat);
    fwrite(&n_ped, sizeof(int), 1, dat); /* placeholder, corrigido depois */
    if (f) {
        Pedido p;
        while (fread(&p, sizeof(Pedido), 1, f) == 1) {
            if (strncmp(p.data_hora, agora, 10) == 0 ||
                strstr(p.data_hora, data_arq) != NULL) {
                /* fallback: compara apenas dd/mm/aaaa quando formatos coincidirem */
            }
            /* Critério simples e robusto: compara dia/mes/ano extraídos de data_hora (DD/MM/AAAA) */
            char dia_ped[11];
            snprintf(dia_ped, sizeof(dia_ped), "%.10s", p.data_hora);
            char hoje_dmy[11];
            {
                time_t t = time(NULL);
                struct tm *tmh = localtime(&t);
                strftime(hoje_dmy, sizeof(hoje_dmy), "%d/%m/%Y", tmh);
            }
            if (strcmp(dia_ped, hoje_dmy) == 0) {
                fprintf(txt, " Pedido #%-4d  %s  Total: R$ %-8.2f Cliente: %s\n",
                        p.numero, p.data_hora, p.total_final, p.nome_cliente);
                total_dia += p.total_final;
                fwrite(&p, sizeof(Pedido), 1, dat);
                n_ped++;
            }
        }
        fclose(f);
    }
    if (n_ped == 0) fprintf(txt, " (nenhum pedido registrado hoje)\n");
    fprintf(txt, " Total: %d pedido(s) | Receita do dia: R$ %.2f\n\n", n_ped, total_dia);

    /* Corrige a contagem de pedidos gravada no .dat */
    fseek(dat, pos_contagem + 8, SEEK_SET);
    fwrite(&n_ped, sizeof(int), 1, dat);

    fprintf(txt, "================================================================\n");
    fprintf(txt, " FIM DO SNAPSHOT\n");
    fprintf(txt, "================================================================\n");

    fclose(txt);
    fclose(dat);

    printf("\n " GREEN "✔ Snapshot do dia gerado:" RESET "\n");
    printf("   %s\n", caminho_txt);
    printf("   %s\n", caminho_dat);
    log_evento("SISTEMA", "SNAPSHOT", "Snapshot consolidado do dia gerado");
}
