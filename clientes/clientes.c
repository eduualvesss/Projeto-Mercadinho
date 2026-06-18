/* ============================================================
 *  clientes.c — Módulo de cadastro de clientes
 * ============================================================ */
#include "clientes.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static FILE *cli_abrir(const char *modo) {
    char caminho[300];
    caminho_arquivo(caminho, ARQ_CLIENTES);
    return fopen(caminho, modo);
}

/* ============================================================
 *  CONSULTAS BÁSICAS
 * ============================================================ */
int cli_proximo_codigo(void) {
    FILE *f = cli_abrir("rb");
    if (!f) return 1;
    int max = 0; Cliente c;
    while (fread(&c, sizeof(Cliente), 1, f) == 1)
        if (c.codigo > max) max = c.codigo;
    fclose(f);
    return max + 1;
}

int cli_total_registros(void) {
    FILE *f = cli_abrir("rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fclose(f);
    return (int)(tam / sizeof(Cliente));
}

int cli_buscar_por_codigo(int codigo, Cliente *dest) {
    FILE *f = cli_abrir("rb");
    if (!f) return 0;
    Cliente c;
    while (fread(&c, sizeof(Cliente), 1, f) == 1) {
        if (c.codigo == codigo) { *dest = c; fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}

void cli_exibir_ficha(const Cliente *c) {
    printf("\n ╔══════════════════════════════════════════════════════╗\n");
    printf(" ║ FICHA DO CLIENTE — Cód: %-5d Status: %-8s  ║\n",
           c->codigo, c->ativo ? "ATIVO" : "INATIVO");
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Nome       : %-38s ║\n", c->nome);
    printf(" ║ Tipo       : %-38s ║\n", c->tipo == 1 ? "Pessoa Física" : "Pessoa Jurídica");
    printf(" ║ CPF/CNPJ   : %-38s ║\n", c->cpf);
    printf(" ║ RG/IE      : %-38s ║\n", c->rg);
    if (c->tipo == 1)
        printf(" ║ Nascimento : %-38s ║\n", c->data_nascimento);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ E-mail     : %-38s ║\n", c->email);
    printf(" ║ Telefone   : %-38s ║\n", c->telefone);
    printf(" ║ Celular    : %-38s ║\n", c->celular);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Endereço   : %-38s ║\n", c->endereco);
    printf(" ║ Nº/Compl.  : %-10s / %-25s ║\n", c->numero, c->complemento);
    printf(" ║ Bairro     : %-38s ║\n", c->bairro);
    printf(" ║ Cidade/UF  : %-30s / %-5s ║\n", c->cidade, c->estado);
    printf(" ║ CEP        : %-38s ║\n", c->cep);
    printf(" ╠══════════════════════════════════════════════════════╣\n");
    printf(" ║ Lim. Cred. : R$ %-34.2f ║\n", c->limite_credito);
    printf(" ║ Cadastrado : %-38s ║\n", c->data_cadastro);
    if (strlen(c->observacoes) > 0)
        printf(" ║ Obs.       : %-38s ║\n", c->observacoes);
    printf(" ╚══════════════════════════════════════════════════════╝\n");
}

/* ============================================================
 *  CADASTRO
 * ============================================================ */
void cadastrar_cliente(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║       CADASTRO DE NOVO CLIENTE           ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    Cliente c;
    memset(&c, 0, sizeof(Cliente));
    c.codigo = cli_proximo_codigo();
    c.ativo  = 1;
    data_hoje(c.data_cadastro);
    printf(" Código gerado automaticamente: %d\n\n", c.codigo);

    printf(" Tipo de cliente:\n [1] Pessoa Física\n [2] Pessoa Jurídica\n Opção: ");
    scanf("%d", &c.tipo); limpar_buffer();
    if (c.tipo != 1 && c.tipo != 2) c.tipo = 1;

    printf("\n --- DADOS PESSOAIS ---\n");
    do {
        ler_string(" Nome completo : ", c.nome, MAX_NOME_CLI);
        if (strlen(c.nome) == 0) printf(" Nome e obrigatorio!\n");
    } while (strlen(c.nome) == 0);

    if (c.tipo == 1) {
        char cpf_raw[MAX_CPF];
        ler_numeros_exatos(" CPF (11 digitos, somente numeros)          : ", cpf_raw, MAX_CPF, 11);
        formatar_cpf(cpf_raw, c.cpf);
        ler_numeros_exatos(" RG  (7 a 9 digitos — ex: 1234567)          : ", c.rg, MAX_CPF, 0);
        ler_numeros_exatos(" Data nasc. (8 digitos DDMMAAAA, ex:01011990): ", c.data_nascimento, 12, 8);
    } else {
        ler_numeros_exatos(" CNPJ (14 digitos, somente numeros)          : ", c.cpf, MAX_CPF, 14);
        ler_numeros_exatos(" Inscricao Estadual (somente numeros)        : ", c.rg, MAX_CPF, 0);
    }

    printf("\n --- CONTATO ---\n");
    ler_string(" E-mail   : ", c.email, MAX_EMAIL);
    ler_numeros_exatos(" Telefone fixo (10 digitos, ex: 8133334444): ", c.telefone, MAX_TELEFONE, 10);
    ler_numeros_exatos(" Celular   (11 digitos, ex: 81988887777)   : ", c.celular,  MAX_TELEFONE, 11);

    printf("\n --- ENDEREÇO ---\n");
    do {
        ler_string(" Logradouro  : ", c.endereco, MAX_ENDERECO);
        if (strlen(c.endereco) == 0) printf(" Logradouro e obrigatorio!\n");
    } while (strlen(c.endereco) == 0);
    ler_string(" Número      : ", c.numero,      10);
    ler_string(" Complemento : ", c.complemento, 40);
    do {
        ler_string(" Bairro      : ", c.bairro, 50);
        if (strlen(c.bairro) == 0) printf(" Bairro e obrigatorio!\n");
    } while (strlen(c.bairro) == 0);
    do {
        ler_string(" Cidade      : ", c.cidade, MAX_CIDADE);
        if (strlen(c.cidade) == 0) printf(" Cidade e obrigatoria!\n");
    } while (strlen(c.cidade) == 0);
    do {
        ler_string(" Estado (UF) : ", c.estado, MAX_ESTADO);
        if (strlen(c.estado) == 0) printf(" Estado e obrigatorio!\n");
    } while (strlen(c.estado) == 0);
    ler_numeros_exatos(" CEP (8 digitos, somente numeros, ex: 50000000): ", c.cep, MAX_CEP, 8);

    printf("\n --- FINANCEIRO ---\n");
    c.limite_credito = ler_float_positivo(" Limite de credito (R$): ");

    printf("\n --- OBSERVAÇÕES ---\n");
    ler_string(" Observações: ", c.observacoes, MAX_OBS_CLI);

    printf("\n Confirmar cadastro? [S/N]: ");
    char conf; scanf(" %c", &conf); limpar_buffer();

    if (toupper((unsigned char)conf) == 'S') {
        FILE *f = cli_abrir("ab");
        if (!f) { printf(" ERRO ao abrir arquivo!\n"); pausar(); return; }
        fwrite(&c, sizeof(Cliente), 1, f);
        fclose(f);
        char det[150];
        sprintf(det, "Cliente #%d cadastrado: %s", c.codigo, c.nome);
        log_evento("CLIENTE", "CADASTRO", det);
        printf("\n " GREEN "✔ Cliente cadastrado! Código: %d" RESET "\n", c.codigo);
    } else {
        printf("\n Cadastro cancelado.\n");
    }
    pausar();
}

/* ============================================================
 *  LISTAGEM / CONSULTA
 * ============================================================ */
void listar_clientes(int apenas_ativos) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════════════════════════╗\n");
    printf(" ║ LISTA DE CLIENTES%-42s║\n", apenas_ativos ? " (ATIVOS) " : " ");
    printf(" ╠═══════╦══════════════════════════════╦══════════════╦═════════╣\n");
    printf(" ║ CÓD.  ║ NOME                         ║ CPF/CNPJ     ║ STATUS  ║\n");
    printf(" ╠═══════╬══════════════════════════════╬══════════════╬═════════╣\n");

    FILE *f = cli_abrir("rb");
    if (!f) {
        printf(" ║ Nenhum registro encontrado.                                  ║\n");
        printf(" ╚══════════════════════════════════════════════════════════════╝\n");
        pausar(); return;
    }

    Cliente c; int count = 0;
    while (fread(&c, sizeof(Cliente), 1, f) == 1) {
        if (apenas_ativos && !c.ativo) continue;
        char nome_t[31]; strncpy(nome_t, c.nome, 30); nome_t[30] = '\0';
        printf(" ║ %-5d ║ %-28s ║ %-12s ║ %-7s ║\n",
               c.codigo, nome_t, c.cpf, c.ativo ? "ATIVO" : "INATIVO");
        count++;
    }
    fclose(f);
    printf(" ╚═══════╩══════════════════════════════╩══════════════╩═════════╝\n");
    printf(" Total: %d cliente(s).\n", count);
    pausar();
}

void consultar_cliente(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║         CONSULTA DE CLIENTE              ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Buscar por:\n [1] Código\n [2] Nome\n [3] CPF/CNPJ\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    FILE *f = cli_abrir("rb");
    if (!f) { printf("\n Nenhum registro encontrado.\n"); pausar(); return; }

    int encontrou = 0; Cliente c;
    if (op == 1) {
        printf(" Código: "); int cod; scanf("%d", &cod); limpar_buffer();
        while (fread(&c, sizeof(Cliente), 1, f) == 1)
            if (c.codigo == cod) { cli_exibir_ficha(&c); encontrou = 1; }
    } else if (op == 2) {
        char busca[MAX_NOME_CLI]; ler_string(" Nome (parte): ", busca, MAX_NOME_CLI);
        char b_low[MAX_NOME_CLI]; para_minusculas(busca, b_low);
        while (fread(&c, sizeof(Cliente), 1, f) == 1) {
            char n_low[MAX_NOME_CLI]; para_minusculas(c.nome, n_low);
            if (strstr(n_low, b_low)) { cli_exibir_ficha(&c); encontrou = 1; }
        }
    } else if (op == 3) {
        char busca[MAX_CPF]; ler_string(" CPF/CNPJ: ", busca, MAX_CPF);
        while (fread(&c, sizeof(Cliente), 1, f) == 1)
            if (strstr(c.cpf, busca)) { cli_exibir_ficha(&c); encontrou = 1; }
    }
    fclose(f);
    if (!encontrou) printf("\n Nenhum cliente encontrado.\n");
    pausar();
}

/* ============================================================
 *  EDIÇÃO / EXCLUSÃO
 * ============================================================ */
void editar_cliente(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║            EDITAR CLIENTE                ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Código do cliente: "); int cod; scanf("%d", &cod); limpar_buffer();

    Cliente c;
    if (!cli_buscar_por_codigo(cod, &c)) { printf(" Cliente não encontrado!\n"); pausar(); return; }
    cli_exibir_ficha(&c);

    printf("\n [1] Nome  [2] CPF/CNPJ  [3] Contato  [4] Endereço\n");
    printf(" [5] Limite de crédito  [6] Observações  [7] Status\n");
    printf(" [0] Cancelar\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    switch (op) {
        case 1: ler_string(" Novo nome: ", c.nome, MAX_NOME_CLI); break;
        case 2:
            if (c.tipo == 1) {
                char cpf_raw[MAX_CPF];
                ler_numeros_exatos(" Novo CPF (11 digitos): ", cpf_raw, MAX_CPF, 11);
                formatar_cpf(cpf_raw, c.cpf);
            } else {
                ler_numeros_exatos(" Novo CNPJ (14 digitos): ", c.cpf, MAX_CPF, 14);
            }
            break;
        case 3:
            ler_string(" Novo e-mail  : ", c.email, MAX_EMAIL);
            ler_numeros_exatos(" Novo telefone fixo (10 digitos): ", c.telefone, MAX_TELEFONE, 10);
            ler_numeros_exatos(" Novo celular       (11 digitos): ", c.celular,  MAX_TELEFONE, 11);
            break;
        case 4:
            ler_string(" Logradouro  : ", c.endereco,    MAX_ENDERECO);
            ler_string(" Número      : ", c.numero,      10);
            ler_string(" Complemento : ", c.complemento, 40);
            ler_string(" Bairro      : ", c.bairro,      50);
            ler_string(" Cidade      : ", c.cidade,      MAX_CIDADE);
            ler_string(" Estado (UF) : ", c.estado,      MAX_ESTADO);
            ler_string(" CEP         : ", c.cep,         MAX_CEP);
            break;
        case 5:
            printf(" Novo limite (R$): ");
            scanf("%f", &c.limite_credito); limpar_buffer();
            break;
        case 6: ler_string(" Observações: ", c.observacoes, MAX_OBS_CLI); break;
        case 7:
            c.ativo = !c.ativo;
            printf(" Status: %s\n", c.ativo ? "ATIVO" : "INATIVO");
            break;
        default: printf(" Cancelado.\n"); pausar(); return;
    }

    FILE *f = cli_abrir("r+b");
    if (!f) { printf(" ERRO ao abrir arquivo!\n"); pausar(); return; }
    Cliente tmp;
    while (fread(&tmp, sizeof(Cliente), 1, f) == 1) {
        if (tmp.codigo == cod) {
            fseek(f, -(long)sizeof(Cliente), SEEK_CUR);
            fwrite(&c, sizeof(Cliente), 1, f);
            break;
        }
    }
    fclose(f);
    log_evento("CLIENTE", "EDICAO", "Dados de cliente atualizados");
    printf("\n " GREEN "✔ Dados atualizados!" RESET "\n");
    pausar();
}

void excluir_cliente(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║      EXCLUIR / INATIVAR CLIENTE          ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" Código do cliente: "); int cod; scanf("%d", &cod); limpar_buffer();

    Cliente c;
    if (!cli_buscar_por_codigo(cod, &c)) { printf(" Cliente não encontrado!\n"); pausar(); return; }
    cli_exibir_ficha(&c);

    printf("\n [1] Inativar (recomendado)  [2] Excluir definitivamente  [0] Cancelar\n Opção: ");
    int op; scanf("%d", &op); limpar_buffer();

    if (op == 1) {
        c.ativo = 0;
        FILE *f = cli_abrir("r+b");
        Cliente tmp;
        while (fread(&tmp, sizeof(Cliente), 1, f) == 1) {
            if (tmp.codigo == cod) {
                fseek(f, -(long)sizeof(Cliente), SEEK_CUR);
                fwrite(&c, sizeof(Cliente), 1, f);
                break;
            }
        }
        fclose(f);
        log_evento("CLIENTE", "INATIVADO", "Cliente inativado");
        printf("\n " YELLOW "✔ Cliente inativado." RESET "\n");
    } else if (op == 2) {
        printf(" Confirma exclusão definitiva? [S/N]: ");
        char conf; scanf(" %c", &conf); limpar_buffer();
        if (toupper((unsigned char)conf) == 'S') {
            char caminho[300], caminho_tmp[310];
            caminho_arquivo(caminho, ARQ_CLIENTES);
            caminho_arquivo(caminho_tmp, "clientes_tmp.dat");
            FILE *in  = fopen(caminho, "rb");
            FILE *out = fopen(caminho_tmp, "wb");
            if (!in || !out) {
                printf(" ERRO!\n");
                if (in)  fclose(in);
                if (out) fclose(out);
                pausar(); return;
            }
            Cliente tmp;
            while (fread(&tmp, sizeof(Cliente), 1, in) == 1)
                if (tmp.codigo != cod) fwrite(&tmp, sizeof(Cliente), 1, out);
            fclose(in); fclose(out);
            remove(caminho);
            rename(caminho_tmp, caminho);
            log_evento("CLIENTE", "EXCLUSAO", "Cliente excluido definitivamente");
            printf("\n " RED "✔ Cliente excluído." RESET "\n");
        } else printf(" Cancelado.\n");
    }
    pausar();
}

/* ============================================================
 *  RELATÓRIO / EXPORTAÇÃO TXT
 * ============================================================ */
void relatorio_clientes(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║      RELATÓRIO — CLIENTES                ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");

    FILE *f = cli_abrir("rb");
    if (!f) { printf(" Nenhum registro.\n"); pausar(); return; }

    int total=0,ativos=0,inativos=0,pf=0,pj=0;
    float lim_total=0.0f;
    Cliente c;
    while (fread(&c, sizeof(Cliente), 1, f) == 1) {
        total++;
        if (c.ativo) ativos++; else inativos++;
        if (c.tipo == 1) pf++; else pj++;
        lim_total += c.limite_credito;
    }
    fclose(f);

    printf(" Total de clientes      : %d\n", total);
    printf(" Ativos                 : %d\n", ativos);
    printf(" Inativos               : %d\n", inativos);
    printf(" Pessoa Física (PF)     : %d\n", pf);
    printf(" Pessoa Jurídica (PJ)   : %d\n", pj);
    printf(" Limite total (R$)      : %.2f\n", lim_total);
    if (total > 0)
        printf(" Limite médio (R$)      : %.2f\n", lim_total / total);
    pausar();
}

static void cli_ficha_para_txt(FILE *f, const Cliente *c) {
    sep_txt(f, '=', 62);
    fprintf(f, " FICHA DO CLIENTE\n");
    fprintf(f, " Codigo : %d\n", c->codigo);
    fprintf(f, " Status : %s\n", c->ativo ? "ATIVO" : "INATIVO");
    sep_txt(f, '-', 62);
    fprintf(f, " DADOS PESSOAIS\n");
    fprintf(f, " Nome       : %s\n", c->nome);
    fprintf(f, " Tipo       : %s\n", c->tipo == 1 ? "Pessoa Fisica" : "Pessoa Juridica");
    fprintf(f, " CPF/CNPJ   : %s\n", c->cpf);
    fprintf(f, " RG/IE      : %s\n", c->rg);
    if (c->tipo == 1)
        fprintf(f, " Nascimento : %s\n", c->data_nascimento);
    sep_txt(f, '-', 62);
    fprintf(f, " CONTATO\n");
    fprintf(f, " E-mail     : %s\n", c->email);
    fprintf(f, " Telefone   : %s\n", c->telefone);
    fprintf(f, " Celular    : %s\n", c->celular);
    sep_txt(f, '-', 62);
    fprintf(f, " ENDERECO\n");
    fprintf(f, " Logradouro : %s, %s\n", c->endereco, c->numero);
    if (strlen(c->complemento) > 0)
        fprintf(f, " Complemento: %s\n", c->complemento);
    fprintf(f, " Bairro     : %s\n", c->bairro);
    fprintf(f, " Cidade/UF  : %s / %s\n", c->cidade, c->estado);
    fprintf(f, " CEP        : %s\n", c->cep);
    sep_txt(f, '-', 62);
    fprintf(f, " FINANCEIRO\n");
    fprintf(f, " Lim.Credito: R$ %.2f\n", c->limite_credito);
    sep_txt(f, '-', 62);
    fprintf(f, " Cadastro   : %s\n", c->data_cadastro);
    if (strlen(c->observacoes) > 0)
        fprintf(f, " Observacoes: %s\n", c->observacoes);
    sep_txt(f, '=', 62);
    fputc('\n', f);
}

void exportar_clientes_txt(void) {
    limpar_tela();
    printf("\n ╔══════════════════════════════════════════╗\n");
    printf(" ║   EXPORTAR CLIENTES PARA ARQUIVO .TXT    ║\n");
    printf(" ╚══════════════════════════════════════════╝\n\n");
    printf(" O que deseja exportar?\n");
    printf(" [1] Ficha de um cliente especifico -> ficha_cli_NNN.txt\n");
    printf(" [2] Lista completa (tabela)        -> %s\n", ARQ_CLI_TXT);
    printf(" [3] Relatorio gerencial completo   -> %s\n", ARQ_CLI_TXT);
    printf(" [0] Cancelar\n Opcao: ");
    int op; scanf("%d", &op); limpar_buffer();
    if (op == 0) return;

    FILE *dat = cli_abrir("rb");
    if (!dat) { printf("\n Nenhum registro encontrado. Cadastre clientes primeiro.\n"); pausar(); return; }

    char nome_saida[80], caminho_saida[320];
    strcpy(nome_saida, ARQ_CLI_TXT);
    char agora[20]; data_hoje(agora);

    if (op == 1) {
        printf(" Codigo do cliente: "); int cod; scanf("%d", &cod); limpar_buffer();
        Cliente c; int achou = 0;
        while (fread(&c, sizeof(Cliente), 1, dat) == 1)
            if (c.codigo == cod) { achou = 1; break; }
        fclose(dat);
        if (!achou) { printf("\n Cliente nao encontrado.\n"); pausar(); return; }

        sprintf(nome_saida, "ficha_cli_%03d.txt", c.codigo);
        caminho_arquivo(caminho_saida, nome_saida);
        FILE *txt = fopen(caminho_saida, "w");
        if (!txt) { printf("\n ERRO ao criar arquivo!\n"); pausar(); return; }
        fprintf(txt, "SISTEMA DE CADASTRO DE CLIENTES v%s\n", VERSAO);
        fprintf(txt, "Gerado em: %s\n\n", agora);
        cli_ficha_para_txt(txt, &c);
        fclose(txt);
        printf("\n " GREEN "Arquivo gerado: %s" RESET "\n", caminho_saida);
        pausar(); return;
    }

    caminho_arquivo(caminho_saida, nome_saida);
    FILE *txt = fopen(caminho_saida, "w");
    if (!txt) { fclose(dat); printf("\n ERRO ao criar arquivo!\n"); pausar(); return; }

    if (op == 2) {
        fprintf(txt, "SISTEMA DE CADASTRO DE CLIENTES v%s\n", VERSAO);
        fprintf(txt, "Relatorio : LISTA COMPLETA DE CLIENTES\n");
        fprintf(txt, "Gerado em : %s\n\n", agora);
        sep_txt(txt, '-', 78);
        fprintf(txt, "%-5s %-30s %-15s %-12s %-7s\n", "COD.", "NOME", "CPF/CNPJ", "CIDADE", "STATUS");
        sep_txt(txt, '-', 78);
        Cliente c; int count = 0;
        while (fread(&c, sizeof(Cliente), 1, dat) == 1) {
            char nome_t[31]; strncpy(nome_t, c.nome,   30); nome_t[30] = '\0';
            char cpf_t[16];  strncpy(cpf_t,  c.cpf,   15); cpf_t[15]  = '\0';
            char cid_t[13];  strncpy(cid_t,  c.cidade, 12); cid_t[12]  = '\0';
            fprintf(txt, "%-5d %-30s %-15s %-12s %-7s\n",
                    c.codigo, nome_t, cpf_t, cid_t, c.ativo ? "ATIVO" : "INATIVO");
            count++;
        }
        sep_txt(txt, '-', 78);
        fprintf(txt, "Total: %d cliente(s).\n", count);
    } else if (op == 3) {
        fprintf(txt, "================================================================\n");
        fprintf(txt, " SISTEMA DE CADASTRO DE CLIENTES v%s\n", VERSAO);
        fprintf(txt, " RELATORIO GERENCIAL COMPLETO\n");
        fprintf(txt, " Gerado em: %s\n", agora);
        fprintf(txt, "================================================================\n\n");

        int total=0,ativos=0,inativos=0,pf=0,pj=0;
        float lim_total=0.0f, lim_max=0.0f;
        char nome_maior[MAX_NOME_CLI] = "";
        Cliente c;
        while (fread(&c, sizeof(Cliente), 1, dat) == 1) {
            total++;
            if (c.ativo) ativos++; else inativos++;
            if (c.tipo == 1) pf++; else pj++;
            lim_total += c.limite_credito;
            if (c.limite_credito > lim_max) {
                lim_max = c.limite_credito;
                strncpy(nome_maior, c.nome, MAX_NOME_CLI - 1);
            }
        }
        fprintf(txt, "RESUMO ESTATISTICO\n");
        sep_txt(txt, '-', 40);
        fprintf(txt, " Total de clientes    : %d\n", total);
        fprintf(txt, " Clientes ativos      : %d\n", ativos);
        fprintf(txt, " Clientes inativos    : %d\n", inativos);
        fprintf(txt, " Pessoa Fisica (PF)   : %d\n", pf);
        fprintf(txt, " Pessoa Juridica (PJ) : %d\n", pj);
        fprintf(txt, " Limite total (R$)    : %.2f\n", lim_total);
        if (total > 0) {
            fprintf(txt, " Limite medio (R$)    : %.2f\n", lim_total / total);
            fprintf(txt, " Maior limite (R$)    : %.2f (%s)\n", lim_max, nome_maior);
        }
        fprintf(txt, "\n\nFICHAS INDIVIDUAIS\n\n");
        rewind(dat);
        while (fread(&c, sizeof(Cliente), 1, dat) == 1)
            cli_ficha_para_txt(txt, &c);
    }

    fclose(dat); fclose(txt);
    log_evento("CLIENTE", "EXPORT", "Exportacao de clientes para TXT realizada");
    printf("\n " GREEN "Arquivo gerado: %s" RESET "\n", caminho_saida);
    pausar();
}

/* ============================================================
 *  SUBMENU
 * ============================================================ */
void submenu_clientes(void) {
    int op;
    do {
        limpar_tela();
        printf("\n ╔══════════════════════════════════════════╗\n");
        printf(" ║        CADASTRO DE CLIENTES              ║\n");
        printf(" ╠══════════════════════════════════════════╣\n");
        printf(" ║ [1] Cadastrar novo cliente               ║\n");
        printf(" ║ [2] Consultar cliente                    ║\n");
        printf(" ║ [3] Editar cadastro                      ║\n");
        printf(" ║ [4] Excluir / Inativar cliente           ║\n");
        printf(" ║ [5] Listar todos os clientes             ║\n");
        printf(" ║ [6] Listar apenas ativos                 ║\n");
        printf(" ║ [7] Relatório resumido                   ║\n");
        printf(" ║ [8] Exportar para TXT                    ║\n");
        printf(" ║ [0] Voltar ao menu principal             ║\n");
        printf(" ╚══════════════════════════════════════════╝\n");
        printf(" Total de clientes: %d\n\n", cli_total_registros());
        printf(" Opção: "); scanf("%d", &op); limpar_buffer();

        switch (op) {
            case 1: cadastrar_cliente();     break;
            case 2: consultar_cliente();     break;
            case 3: editar_cliente();        break;
            case 4: excluir_cliente();       break;
            case 5: listar_clientes(0);      break;
            case 6: listar_clientes(1);      break;
            case 7: relatorio_clientes();    break;
            case 8: exportar_clientes_txt(); break;
            case 0: break;
            default: printf("\n Opção inválida!\n"); pausar();
        }
    } while (op != 0);
}
