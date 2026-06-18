/* ============================================================
 *  produtos.c — Módulo de controle de estoque
 * ============================================================ */
#include "produtos.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static FILE *prod_abrir(const char *modo) {
    char caminho[300];
    caminho_arquivo(caminho, ARQ_PRODUTOS);
    return fopen(caminho, modo);
}

static FILE *hist_abrir(const char *modo) {
    char caminho[300];
    caminho_arquivo(caminho, ARQ_HIST);
    return fopen(caminho, modo);
}

/* ============================================================
 *  CLASSIFICAÇÃO POR VALIDADE
 * ============================================================ */
static int prod_calcular_dias(int dia, int mes, int ano) {
    time_t agora = time(NULL);
    struct tm t  = {0};
    t.tm_mday = dia; t.tm_mon = mes - 1; t.tm_year = ano - 1900;
    time_t validade = mktime(&t);
    return (int)(difftime(validade, agora) / 86400);
}

void prod_classificar(Produto *p) {
    int dias = prod_calcular_dias(p->dia_val, p->mes_val, p->ano_val);
    p->dias_restantes = dias;
    if      (dias <= 0)              p->prioridade = 0;
    else if (dias <= LIMITE_CRITICO) p->prioridade = 1;
    else if (dias <= LIMITE_ATENCAO) p->prioridade = 2;
    else                             p->prioridade = 3;
}

const char *prod_label(int p) {
    switch (p) {
        case 0: return "VENCIDO";
        case 1: return "CRITICO";
        case 2: return "ATENCAO";
        default: return "OK";
    }
}

const char *prod_cor(int p) {
    switch (p) {
        case 0: return RED;
        case 1: return YELLOW;
        case 2: return YELLOW;
        default: return GREEN;
    }
}

/* ============================================================
 *  CONSULTAS BÁSICAS
 * ============================================================ */
int prod_proximo_codigo(void) {
    FILE *f = prod_abrir("rb");
    if (!f) return 1;
    int max = 0; Produto p;
    while (fread(&p, sizeof(Produto), 1, f) == 1)
        if (p.codigo > max) max = p.codigo;
    fclose(f);
    return max + 1;
}

int prod_total_registros(void) {
    FILE *f = prod_abrir("rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fclose(f);
    return (int)(tam / sizeof(Produto));
}

int prod_buscar_por_codigo(int codigo, Produto *dest) {
    FILE *f = prod_abrir("rb");
    if (!f) return 0;
    Produto p;
    while (fread(&p, sizeof(Produto), 1, f) == 1)
        if (p.codigo == codigo) { *dest = p; fclose(f); return 1; }
    fclose(f);
    return 0;
}

int prod_atualizar(int codigo, const Produto *novo) {
    FILE *f = prod_abrir("r+b");
    if (!f) return 0;
    Produto tmp; int ok = 0;
    while (fread(&tmp, sizeof(Produto), 1, f) == 1) {
        if (tmp.codigo == codigo) {
            fseek(f, -(long)sizeof(Produto), SEEK_CUR);
            fwrite(novo, sizeof(Produto), 1, f);
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

void registrar_movimentacao(int cod, const char *tipo, int qtd) {
    Movimentacao m;
    m.codigo_produto = cod;
    strncpy(m.tipo_acao, tipo, sizeof(m.tipo_acao) - 1);
    m.tipo_acao[sizeof(m.tipo_acao) - 1] = '\0';
    m.quantidade = qtd;
    data_hora_agora(m.data_hora);
    FILE *f = hist_abrir("ab");
    if (!f) return;
    fwrite(&m, sizeof(Movimentacao), 1, f);
    fclose(f);
}

void prod_exibir_ficha(const Produto *p) {
    Produto cp = *p;
    prod_classificar(&cp);
    const char *cor = prod_cor(cp.prioridade);
    const char *lbl = prod_label(cp.prioridade);

    printf("\n ╔══════════════════════════════════════════════════════╗\n");
    printf(" ║ FICHA DO PRODUTO — Cód: %-5d Status: %-8s  ║\n",
           cp.codigo, cp.ativo ? "ATIVO" : "INATIVO");
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Nome       : %-38s ║\n", cp.nome);
    printf(" ║ Categoria  : %-38s ║\n", cp.categoria);
    printf(" ║ Unidade    : %-10s  Peso/Vol: %-18.3f ║\n", cp.unidade, cp.peso_volume);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Qtd. estoque: %-37d ║\n", cp.quantidade);
    printf(" ║ Preço custo : R$ %-34.2f ║\n", cp.preco_custo);
    printf(" ║ Preço venda : R$ %-34.2f ║\n", cp.preco_venda);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Validade   : %02d/%02d/%04d  Situação: %s%-7s" RESET "  ║\n",
           cp.dia_val, cp.mes_val, cp.ano_val, cor, lbl);
    printf(" ║ Dias rest. : %-38d ║\n", cp.dias_restantes);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Cadastrado : %-38s ║\n", cp.data_cadastro);
    if (strlen(cp.observacoes) > 0)
        printf(" ║ Obs.       : %-38s ║\n", cp.observacoes);
    printf(" ╚══════════════════════════════════════════════════════╝\n");
}

/* ============================================================
 *  CADASTRO
 * ============================================================ */
void cadastrar_produto(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║       CADASTRO DE NOVO PRODUTO           ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    Produto p;
    memset(&p, 0, sizeof(Produto));
    p.codigo = prod_proximo_codigo();
    p.ativo  = 1;
    data_hoje(p.data_cadastro);
    printf(" Código gerado: %d\n\n", p.codigo);

    printf(" --- IDENTIFICAÇÃO ---\n");
    do {
        ler_string(" Nome do produto   : ", p.nome, MAX_NOME_PROD);
        if (strlen(p.nome) == 0) printf(" Nome e obrigatorio!\n");
    } while (strlen(p.nome) == 0);
    do {
        ler_string(" Categoria         : ", p.categoria, MAX_CATEGORIA);
        if (strlen(p.categoria) == 0) printf(" Categoria e obrigatoria!\n");
    } while (strlen(p.categoria) == 0);
    do {
        ler_string(" Unidade (UN/KG/L) : ", p.unidade, MAX_UNIDADE);
        if (strlen(p.unidade) == 0) printf(" Unidade e obrigatoria!\n");
    } while (strlen(p.unidade) == 0);
    p.peso_volume = ler_float_positivo(" Peso/Volume (por unidade): ");

    printf("\n --- ESTOQUE ---\n");
    p.quantidade = ler_inteiro_positivo(" Quantidade inicial: ");

    printf("\n --- PREÇOS ---\n");
    p.preco_custo = ler_float_positivo(" Preco de custo (R$): ");
    p.preco_venda = ler_float_positivo(" Preco de venda (R$): ");

    printf("\n --- VALIDADE ---\n");
    ler_data_validade(&p.dia_val, &p.mes_val, &p.ano_val);
    prod_classificar(&p);

    printf("\n --- OBSERVAÇÕES ---\n");
    ler_string(" Observações: ", p.observacoes, MAX_OBS_PROD);

    prod_exibir_ficha(&p);
    printf("\n Confirmar cadastro? [S/N]: ");
    char conf; scanf(" %c", &conf); limpar_buffer();

    if (toupper((unsigned char)conf) == 'S') {
        FILE *f = prod_abrir("ab");
        if (!f) { printf(" ERRO!\n"); pausar(); return; }
        fwrite(&p, sizeof(Produto), 1, f);
        fclose(f);
        registrar_movimentacao(p.codigo, "CADASTRO", p.quantidade);
        char det[150];
        sprintf(det, "Produto #%d cadastrado: %s (qtd: %d)", p.codigo, p.nome, p.quantidade);
        log_evento("PRODUTO", "CADASTRO", det);
        printf("\n " GREEN "✔ Produto cadastrado! Código: %d" RESET "\n", p.codigo);
    } else {
        printf("\n Cancelado.\n");
    }
    pausar();
}

/* ============================================================
 *  LISTAGEM / CONSULTA
 * ============================================================ */
void listar_produtos(int apenas_ativos) {
    limpar_tela();
    printf("\n ╔══════╦═══════════════════╦════════════════╦══════╦══════════╦══════════╗\n");
    printf(" ║ CÓD. ║ NOME              ║ CATEGORIA      ║ QTD  ║ VDA(R$)  ║ SITUAÇÃO ║\n");
    printf(" ╠══════╬═══════════════════╬════════════════╬══════╬══════════╬══════════╣\n");

    FILE *f = prod_abrir("rb");
    if (!f) {
        printf(" ║ Nenhum produto cadastrado.                                        ║\n");
        printf(" ╚══════╩═══════════════════╩════════════════╩══════╩══════════╩══════════╝\n");
        pausar(); return;
    }

    Produto p; int count = 0;
    while (fread(&p, sizeof(Produto), 1, f) == 1) {
        if (apenas_ativos && !p.ativo) continue;
        prod_classificar(&p);
        char n[20]; strncpy(n, p.nome,      19); n[19] = '\0';
        char c[15]; strncpy(c, p.categoria, 14); c[14] = '\0';
        printf(" ║ %-4d ║ %-17s ║ %-14s ║ %-4d ║ %-8.2f ║ %s%-8s" RESET " ║\n",
               p.codigo, n, c, p.quantidade, p.preco_venda,
               prod_cor(p.prioridade), prod_label(p.prioridade));
        count++;
    }
    fclose(f);
    printf(" ╚══════╩═══════════════════╩════════════════╩══════╩══════════╩══════════╝\n");
    printf(" Total: %d produto(s).\n", count);
    pausar();
}

void consultar_produto(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║         CONSULTA DE PRODUTO              ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" [1] Código  [2] Nome  [3] Categoria\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    FILE *f = prod_abrir("rb");
    if (!f) { printf("\n Nenhum registro.\n"); pausar(); return; }

    int encontrou = 0; Produto p;
    if (op == 1) {
        printf(" Código: "); int cod; scanf("%d", &cod); limpar_buffer();
        while (fread(&p, sizeof(Produto), 1, f) == 1)
            if (p.codigo == cod) { prod_exibir_ficha(&p); encontrou = 1; }
    } else if (op == 2) {
        char busca[MAX_NOME_PROD]; ler_string(" Nome (parte): ", busca, MAX_NOME_PROD);
        char b_low[MAX_NOME_PROD]; para_minusculas(busca, b_low);
        while (fread(&p, sizeof(Produto), 1, f) == 1) {
            char n_low[MAX_NOME_PROD]; para_minusculas(p.nome, n_low);
            if (strstr(n_low, b_low)) { prod_exibir_ficha(&p); encontrou = 1; }
        }
    } else if (op == 3) {
        char busca[MAX_CATEGORIA]; ler_string(" Categoria: ", busca, MAX_CATEGORIA);
        char b_low[MAX_CATEGORIA]; para_minusculas(busca, b_low);
        while (fread(&p, sizeof(Produto), 1, f) == 1) {
            char c_low[MAX_CATEGORIA]; para_minusculas(p.categoria, c_low);
            if (strstr(c_low, b_low)) { prod_exibir_ficha(&p); encontrou = 1; }
        }
    }
    fclose(f);
    if (!encontrou) printf("\n Produto não encontrado.\n");
    pausar();
}

/* ============================================================
 *  EDIÇÃO / EXCLUSÃO
 * ============================================================ */
void editar_produto(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║            EDITAR PRODUTO                ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Código do produto: "); int cod; scanf("%d", &cod); limpar_buffer();

    Produto p;
    if (!prod_buscar_por_codigo(cod, &p)) { printf(" Produto não encontrado!\n"); pausar(); return; }
    prod_exibir_ficha(&p);

    printf("\n [1] Nome  [2] Categoria  [3] Unidade/Peso  [4] Preços\n");
    printf(" [5] Validade  [6] Observações  [7] Status  [0] Cancelar\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    switch (op) {
        case 1: ler_string(" Novo nome: ", p.nome, MAX_NOME_PROD); break;
        case 2: ler_string(" Categoria: ", p.categoria, MAX_CATEGORIA); break;
        case 3:
            ler_string(" Unidade: ", p.unidade, MAX_UNIDADE);
            printf(" Peso/Volume: "); scanf("%f", &p.peso_volume); limpar_buffer();
            break;
        case 4:
            p.preco_custo = ler_float_positivo(" Custo (R$): ");
            p.preco_venda = ler_float_positivo(" Venda (R$): ");
            break;
        case 5:
            ler_data_validade(&p.dia_val, &p.mes_val, &p.ano_val);
            prod_classificar(&p);
            break;
        case 6: ler_string(" Observações: ", p.observacoes, MAX_OBS_PROD); break;
        case 7:
            p.ativo = !p.ativo;
            printf(" Status: %s\n", p.ativo ? "ATIVO" : "INATIVO");
            break;
        default: printf(" Cancelado.\n"); pausar(); return;
    }

    prod_atualizar(cod, &p);
    registrar_movimentacao(p.codigo, "EDICAO", 0);
    printf("\n " GREEN "✔ Produto atualizado!" RESET "\n");
    pausar();
}

void excluir_produto(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║      EXCLUIR / INATIVAR PRODUTO          ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Código: "); int cod; scanf("%d", &cod); limpar_buffer();

    Produto p;
    if (!prod_buscar_por_codigo(cod, &p)) { printf(" Produto não encontrado!\n"); pausar(); return; }
    prod_exibir_ficha(&p);

    printf("\n [1] Inativar  [2] Excluir definitivamente  [0] Cancelar\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    if (op == 1) {
        p.ativo = 0;
        prod_atualizar(cod, &p);
        registrar_movimentacao(cod, "INATIVADO", p.quantidade);
        printf("\n " YELLOW "✔ Produto inativado." RESET "\n");
    } else if (op == 2) {
        printf(" Confirma? [S/N]: "); char conf; scanf(" %c", &conf); limpar_buffer();
        if (toupper((unsigned char)conf) == 'S') {
            char caminho[300], caminho_tmp[310];
            caminho_arquivo(caminho, ARQ_PRODUTOS);
            caminho_arquivo(caminho_tmp, "produtos_tmp.dat");
            FILE *in  = fopen(caminho, "rb");
            FILE *out = fopen(caminho_tmp, "wb");
            if (!in || !out) {
                if (in)  fclose(in);
                if (out) fclose(out);
                printf(" ERRO!\n"); pausar(); return;
            }
            Produto tmp;
            while (fread(&tmp, sizeof(Produto), 1, in) == 1)
                if (tmp.codigo != cod) fwrite(&tmp, sizeof(Produto), 1, out);
            fclose(in); fclose(out);
            remove(caminho); rename(caminho_tmp, caminho);
            registrar_movimentacao(cod, "EXCLUSAO", p.quantidade);
            printf("\n " RED "✔ Produto excluído." RESET "\n");
        } else printf(" Cancelado.\n");
    }
    pausar();
}

/* ============================================================
 *  MOVIMENTAÇÃO DE ESTOQUE
 * ============================================================ */
void movimentar_estoque(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║     MOVIMENTAR ESTOQUE (ENTRADA/SAÍDA)   ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Código do produto: "); int cod; scanf("%d", &cod); limpar_buffer();

    Produto p;
    if (!prod_buscar_por_codigo(cod, &p)) { printf(" Produto não encontrado!\n"); pausar(); return; }
    printf("\n Produto: %s | Estoque: %d %s\n\n", p.nome, p.quantidade, p.unidade);
    printf(" [1] Entrada (+)  [2] Saída (-)  [0] Cancelar\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();
    if (op == 0) return;

    int qtd = ler_inteiro_positivo(" Quantidade (> 0): ");
    if (qtd <= 0) { printf(" Quantidade deve ser maior que zero!\n"); pausar(); return; }

    if (op == 1) {
        p.quantidade += qtd;
        registrar_movimentacao(cod, "ENTRADA", qtd);
        printf("\n " GREEN "✔ Entrada! Novo saldo: %d %s" RESET "\n", p.quantidade, p.unidade);
    } else if (op == 2) {
        if (qtd > p.quantidade) {
            printf("\n " RED "[ERRO] Estoque insuficiente!" RESET "\n"); pausar(); return;
        }
        p.quantidade -= qtd;
        registrar_movimentacao(cod, "SAIDA", qtd);
        printf("\n " GREEN "✔ Saída! Novo saldo: %d %s" RESET "\n", p.quantidade, p.unidade);
    } else { printf(" Opção inválida.\n"); pausar(); return; }

    prod_atualizar(cod, &p);
    pausar();
}

/* ============================================================
 *  RELATÓRIO / EXPORTAÇÃO / HISTÓRICO
 * ============================================================ */
void relatorio_produtos(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║       RELATÓRIO — PRODUTOS               ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    FILE *f = prod_abrir("rb");
    if (!f) { printf(" Nenhum registro.\n"); pausar(); return; }

    int total=0,ativos=0,inativos=0,vencidos=0,criticos=0,atencao=0,ok=0;
    float v_custo=0.0f, v_venda=0.0f;
    Produto p;
    while (fread(&p, sizeof(Produto), 1, f) == 1) {
        prod_classificar(&p); total++;
        if (p.ativo) ativos++; else inativos++;
        if      (p.prioridade == 0) vencidos++;
        else if (p.prioridade == 1) criticos++;
        else if (p.prioridade == 2) atencao++;
        else                         ok++;
        v_custo += p.preco_custo * p.quantidade;
        v_venda += p.preco_venda * p.quantidade;
    }
    fclose(f);

    printf(" Total de produtos       : %d\n", total);
    printf(" Ativos                  : %d\n", ativos);
    printf(" Inativos                : %d\n\n", inativos);
    printf(" " GREEN  "[OK]      " RESET ": %d\n", ok);
    printf(" " YELLOW "[ATENÇÃO] " RESET ": %d\n", atencao);
    printf(" " YELLOW "[CRÍTICO] " RESET ": %d\n", criticos);
    printf(" " RED    "[VENCIDO] " RESET ": %d\n\n", vencidos);
    printf(" Valor estoque (custo)   : R$ %.2f\n", v_custo);
    printf(" Valor estoque (venda)   : R$ %.2f\n", v_venda);
    if (total > 0)
        printf(" Margem bruta estimada   : R$ %.2f\n", v_venda - v_custo);
    pausar();
}

static void prod_ficha_para_txt(FILE *f, const Produto *p) {
    sep_txt(f, '=', 64);
    fprintf(f, " FICHA DO PRODUTO\n");
    fprintf(f, " Codigo    : %d\n", p->codigo);
    fprintf(f, " Status    : %s\n", p->ativo ? "ATIVO" : "INATIVO");
    sep_txt(f, '-', 64);
    fprintf(f, " IDENTIFICACAO\n");
    fprintf(f, " Nome      : %s\n", p->nome);
    fprintf(f, " Categoria : %s\n", p->categoria);
    fprintf(f, " Unidade   : %s\n", p->unidade);
    fprintf(f, " Peso/Vol  : %.3f\n", p->peso_volume);
    sep_txt(f, '-', 64);
    fprintf(f, " ESTOQUE E PRECOS\n");
    fprintf(f, " Quantidade: %d\n",  p->quantidade);
    fprintf(f, " Custo(R$) : %.2f\n", p->preco_custo);
    fprintf(f, " Venda(R$) : %.2f\n", p->preco_venda);
    sep_txt(f, '-', 64);
    fprintf(f, " VALIDADE\n");
    fprintf(f, " Data      : %02d/%02d/%04d\n", p->dia_val, p->mes_val, p->ano_val);
    fprintf(f, " Situacao  : %s\n", prod_label(p->prioridade));
    fprintf(f, " Dias rest.: %d\n",  p->dias_restantes);
    sep_txt(f, '-', 64);
    fprintf(f, " Cadastro  : %s\n", p->data_cadastro);
    if (strlen(p->observacoes) > 0)
        fprintf(f, " Obs.      : %s\n", p->observacoes);
    sep_txt(f, '=', 64);
    fputc('\n', f);
}

void exportar_produtos_txt(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║   EXPORTAR PRODUTOS PARA ARQUIVO .TXT    ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" O que deseja exportar?\n");
    printf(" [1] Ficha de um produto especifico -> ficha_prod_NNN.txt\n");
    printf(" [2] Lista completa (tabela)        -> %s\n", ARQ_PROD_TXT);
    printf(" [3] Relatorio gerencial completo   -> %s\n", ARQ_PROD_TXT);
    printf(" [0] Cancelar\n Opcao: ");
    int op; scanf("%d", &op); limpar_buffer();
    if (op == 0) return;

    FILE *dat = prod_abrir("rb");
    if (!dat) { printf("\n Nenhum registro encontrado. Cadastre produtos primeiro.\n"); pausar(); return; }

    char nome_saida[80], caminho_saida[320];
    strcpy(nome_saida, ARQ_PROD_TXT);
    char agora[20]; data_hoje(agora);

    if (op == 1) {
        printf(" Codigo do produto: "); int cod; scanf("%d", &cod); limpar_buffer();
        Produto p; int achou = 0;
        while (fread(&p, sizeof(Produto), 1, dat) == 1)
            if (p.codigo == cod) { prod_classificar(&p); achou = 1; break; }
        fclose(dat);
        if (!achou) { printf("\n Produto nao encontrado.\n"); pausar(); return; }

        sprintf(nome_saida, "ficha_prod_%03d.txt", p.codigo);
        caminho_arquivo(caminho_saida, nome_saida);
        FILE *txt = fopen(caminho_saida, "w");
        if (!txt) { printf("\n ERRO ao criar arquivo!\n"); pausar(); return; }
        fprintf(txt, "SISTEMA DE PRODUTOS - MERCADINHO v%s\n", VERSAO);
        fprintf(txt, "Gerado em: %s\n\n", agora);
        prod_ficha_para_txt(txt, &p);
        fclose(txt);
        printf("\n " GREEN "Arquivo gerado: %s" RESET "\n", caminho_saida);
        pausar(); return;
    }

    caminho_arquivo(caminho_saida, nome_saida);
    FILE *txt = fopen(caminho_saida, "w");
    if (!txt) { fclose(dat); printf("\n ERRO ao criar arquivo!\n"); pausar(); return; }

    if (op == 2) {
        fprintf(txt, "SISTEMA DE PRODUTOS - MERCADINHO v%s\n", VERSAO);
        fprintf(txt, "Relatorio : LISTA COMPLETA DE PRODUTOS\n");
        fprintf(txt, "Gerado em : %s\n\n", agora);
        sep_txt(txt, '-', 80);
        fprintf(txt, "%-5s %-20s %-14s %-5s %-9s %-9s %-8s\n",
                "COD.", "NOME", "CATEGORIA", "QTD", "CUSTO", "VENDA", "SITUACAO");
        sep_txt(txt, '-', 80);
        Produto p; int count = 0;
        while (fread(&p, sizeof(Produto), 1, dat) == 1) {
            prod_classificar(&p);
            char n[21]; strncpy(n, p.nome,      20); n[20] = '\0';
            char c[15]; strncpy(c, p.categoria, 14); c[14] = '\0';
            fprintf(txt, "%-5d %-20s %-14s %-5d %-9.2f %-9.2f %-8s\n",
                    p.codigo, n, c, p.quantidade, p.preco_custo, p.preco_venda, prod_label(p.prioridade));
            count++;
        }
        sep_txt(txt, '-', 80);
        fprintf(txt, "Total: %d produto(s).\n", count);
    } else if (op == 3) {
        fprintf(txt, "================================================================\n");
        fprintf(txt, " SISTEMA DE PRODUTOS - MERCADINHO v%s\n", VERSAO);
        fprintf(txt, " RELATORIO GERENCIAL COMPLETO\n");
        fprintf(txt, " Gerado em: %s\n", agora);
        fprintf(txt, "================================================================\n\n");

        int total=0,ativos=0,inativos=0,vencidos=0,criticos=0,atencao_c=0,ok_c=0;
        float v_custo=0.0f, v_venda=0.0f;
        Produto p;
        while (fread(&p, sizeof(Produto), 1, dat) == 1) {
            prod_classificar(&p); total++;
            if (p.ativo) ativos++; else inativos++;
            if      (p.prioridade == 0) vencidos++;
            else if (p.prioridade == 1) criticos++;
            else if (p.prioridade == 2) atencao_c++;
            else                         ok_c++;
            v_custo += p.preco_custo * p.quantidade;
            v_venda += p.preco_venda * p.quantidade;
        }
        fprintf(txt, "RESUMO ESTATISTICO\n");
        sep_txt(txt, '-', 42);
        fprintf(txt, " Total de produtos    : %d\n", total);
        fprintf(txt, " Ativos               : %d\n", ativos);
        fprintf(txt, " Inativos             : %d\n", inativos);
        fprintf(txt, " [OK]                 : %d\n", ok_c);
        fprintf(txt, " [ATENCAO]            : %d\n", atencao_c);
        fprintf(txt, " [CRITICO]            : %d\n", criticos);
        fprintf(txt, " [VENCIDO]            : %d\n", vencidos);
        fprintf(txt, " Valor custo (R$)     : %.2f\n", v_custo);
        fprintf(txt, " Valor venda (R$)     : %.2f\n", v_venda);
        if (total > 0)
            fprintf(txt, " Margem bruta (R$)    : %.2f\n", v_venda - v_custo);
        fprintf(txt, "\n\nFICHAS INDIVIDUAIS\n\n");
        rewind(dat);
        while (fread(&p, sizeof(Produto), 1, dat) == 1) {
            prod_classificar(&p);
            prod_ficha_para_txt(txt, &p);
        }
    }

    fclose(dat); fclose(txt);
    log_evento("PRODUTO", "EXPORT", "Exportacao de produtos para TXT realizada");
    printf("\n " GREEN "Arquivo gerado: %s" RESET "\n", caminho_saida);
    pausar();
}

void consultar_historico(void) {
    limpar_tela();
    printf("\n ╔═══════════════════════════════════════════════════════════════════╗\n");
    printf(" ║       HISTÓRICO DE MOVIMENTAÇÕES (AUDITORIA)                     ║\n");
    printf(" ╠═════════════════════════╦════════════╦══════════════╦═════════════╣\n");
    printf(" ║ Data / Hora             ║ Cód. Prod. ║ Ação         ║ Quantidade  ║\n");
    printf(" ╠═════════════════════════╬════════════╬══════════════╬═════════════╣\n");

    FILE *f = hist_abrir("rb");
    if (!f) {
        printf(" ║ Nenhuma movimentação registrada ainda.                           ║\n");
        printf(" ╚═════════════════════════╩════════════╩══════════════╩═════════════╝\n");
        pausar(); return;
    }
    Movimentacao m; int count = 0;
    while (fread(&m, sizeof(Movimentacao), 1, f) == 1) {
        printf(" ║ %-23s ║ %-10d ║ %-12s ║ %-11d ║\n",
               m.data_hora, m.codigo_produto, m.tipo_acao, m.quantidade);
        count++;
    }
    fclose(f);
    printf(" ╚═════════════════════════╩════════════╩══════════════╩═════════════╝\n");
    printf(" Total de movimentações: %d\n", count);
    pausar();
}

/* ============================================================
 *  SUBMENU
 * ============================================================ */
void submenu_estoque(void) {
    int op;
    do {
        limpar_tela();
        printf("\n ╔══════════════════════════════════════════╗\n");
        printf(" ║        CONTROLE DE ESTOQUE               ║\n");
        printf(" ╠══════════════════════════════════════════╣\n");
        printf(" ║ [1] Cadastrar novo produto               ║\n");
        printf(" ║ [2] Consultar produto                    ║\n");
        printf(" ║ [3] Editar produto                       ║\n");
        printf(" ║ [4] Excluir / Inativar produto           ║\n");
        printf(" ║ [5] Movimentar estoque (entrada/saída)   ║\n");
        printf(" ║ [6] Listar todos os produtos             ║\n");
        printf(" ║ [7] Listar apenas ativos                 ║\n");
        printf(" ║ [8] Relatório de estoque                 ║\n");
        printf(" ║ [9] Histórico de movimentações           ║\n");
        printf(" ║ [10] Exportar para TXT                   ║\n");
        printf(" ║ [0] Voltar ao menu principal             ║\n");
        printf(" ╚══════════════════════════════════════════╝\n");
        printf(" Total de produtos: %d\n\n", prod_total_registros());
        printf(" Opção: "); scanf("%d", &op); limpar_buffer();

        switch (op) {
            case 1:  cadastrar_produto();    break;
            case 2:  consultar_produto();    break;
            case 3:  editar_produto();       break;
            case 4:  excluir_produto();      break;
            case 5:  movimentar_estoque();   break;
            case 6:  listar_produtos(0);     break;
            case 7:  listar_produtos(1);     break;
            case 8:  relatorio_produtos();   break;
            case 9:  consultar_historico();  break;
            case 10: exportar_produtos_txt();break;
            case 0: break;
            default: printf("\n Opção inválida!\n"); pausar();
        }
    } while (op != 0);
}
