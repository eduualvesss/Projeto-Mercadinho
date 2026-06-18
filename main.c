/*
 * ============================================================
 *  MERCADINHO DO PORTO — Sistema Integrado
 *  Módulos: Cliente | Produto/Estoque | Pedido (PDV)
 * ============================================================
 */
#include <locale.h>
#include "comum/comum.h"
#include "clientes/clientes.h"
#include "produtos/produtos.h"
#include "pedidos/pedidos.h"

void menu_principal(void) {
    int escolha;
    do {
        limpar_tela();
        printf("\n ╔══════════════════════════════════════════╗\n");
        printf(" ║                                          ║\n");
        printf(" ║       MERCADINHO DO PORTO                ║\n");
        printf(" ║         Sistema v1.0                    ║\n");
        printf(" ║                                          ║\n");
        printf(" ╠══════════════════════════════════════════╣\n");
        printf(" ║ [1] PONTO DE VENDA                       ║\n");
        printf(" ║ [2] CONTROLE DE ESTOQUE                  ║\n");
        printf(" ║ [3] VENDAS E RELATÓRIOS                  ║\n");
        printf(" ║ [4] CADASTRO DE CLIENTES                 ║\n");
        printf(" ║ [5] GERAR SNAPSHOT DO DIA                ║\n");
        printf(" ╠══════════════════════════════════════════╣\n");
        printf(" ║ [0] SAIR DO SISTEMA                      ║\n");
        printf(" ╚══════════════════════════════════════════╝\n");
        printf(" Dados em: %s\n\n", config_dir_dados());
        printf(" Opção: ");
        scanf("%d", &escolha);
        limpar_buffer();

        switch (escolha) {
            case 1: realizar_venda();         break;
            case 2: submenu_estoque();        break;
            case 3: submenu_vendas();         break;
            case 4: submenu_clientes();       break;
            case 5: gerar_snapshot_do_dia();  pausar(); break;
            case 0:
                limpar_tela();
                printf("\n Encerrando o sistema. Até logo!\n\n");
                break;
            default:
                printf("\n Opção inválida, tente novamente!\n");
                pausar();
        }
    } while (escolha != 0);
}

int main(void) {
    setlocale(LC_ALL, "Portuguese");

    config_inicializar();
    log_evento("SISTEMA", "INICIO", "Sistema iniciado");

    menu_principal();

    log_evento("SISTEMA", "FIM", "Sistema encerrado");
    return 0;
}
