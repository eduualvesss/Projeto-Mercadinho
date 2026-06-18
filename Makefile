# ============================================================
#  Makefile — Sistema Mercadinho
#  Compila main.c junto com todos os módulos (comum, clientes,
#  produtos, pedidos) e gera o executável "mercadinho".
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = mercadinho

SRCS = main.c \
       comum/comum.c \
       clientes/clientes.c \
       produtos/produtos.c \
       pedidos/pedidos.c

# No Windows o executável precisa da extensão .exe
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) mercadinho mercadinho.exe
