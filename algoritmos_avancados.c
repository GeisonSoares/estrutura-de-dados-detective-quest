#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- Constantes Globais ---
#define MAX_NOME 50
#define MAX_PISTA 100
#define MAX_SUSPEITO 30
#define TAMANHO_HASH 10 // Tamanho da Tabela Hash (simplificado)

// ----------------------------------------------------------------------------
// 1. ESTRUTURAS DE DADOS
// ----------------------------------------------------------------------------

// ÁRVORE BINÁRIA (Mapa da Mansão)
typedef struct Sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA]; // Pista opcional
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// ÁRVORE BINÁRIA DE BUSCA (BST - Pistas Coletadas)
typedef struct PistaNode {
    char pista[MAX_PISTA];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// TABELA HASH (Associação Pista -> Suspeito)
typedef struct HashEntry {
    char pista[MAX_PISTA];
    char suspeito[MAX_SUSPEITO]; // Suspeito associado
    struct HashEntry *proximo;
} HashEntry;

HashEntry *tabelaHash[TAMANHO_HASH]; // A Tabela Hash propriamente dita

// ----------------------------------------------------------------------------
// --- Protótipos das Funções (Modularização) ---
// ----------------------------------------------------------------------------

// Funções de SETUP
void inicializarHash();
unsigned int hash(const char *chave);
void inserirNaHash(const char *pista, const char *suspeito); 

// Funções de AB (Mapa)
Sala* criarSala(const char* nome, const char* pista); 

// Funções de BST (Pistas)
PistaNode* inserirPista(PistaNode* raiz, const char* pista); 
char* encontrarSuspeito(const char *pista); 
void exibirPistas(PistaNode* raiz);

// Funções de Jogo
void explorarSalas(Sala* raiz, PistaNode** bstPistas); 
void verificarSuspeitoFinal(PistaNode* bstPistas); 
int contarPistasPorSuspeito(PistaNode* raiz, const char* suspeitoAlvo); 

// Funções de Limpeza
void liberarMapa(Sala* raiz);
void liberarBST(PistaNode* raiz);
void liberarHash();

// FUNÇÃO UTILITÁRIA FALTANTE NO PROTÓTIPO
void limparBufferEntrada(); // <-- ESTA LINHA RESOLVE O PROBLEMA
// ----------------------------------------------------------------------------
// --- IMPLEMENTAÇÃO DA TABELA HASH ---
// ----------------------------------------------------------------------------

// Função Hash Simples: Soma dos caracteres (método divisório)
unsigned int hash(const char *chave) {
    unsigned int valor = 0;
    for (int i = 0; chave[i] != '\0'; i++) {
        valor += chave[i];
    }
    return valor % TAMANHO_HASH;
}

// inicializarHash(): Zera todos os ponteiros da tabela
void inicializarHash() {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        tabelaHash[i] = NULL;
    }
}

// inserirNaHash(): Insere associação pista/suspeito na tabela hash.
void inserirNaHash(const char *pista, const char *suspeito) {
    unsigned int indice = hash(pista);
    
    // Aloca a nova entrada
    HashEntry *novaEntrada = (HashEntry*)malloc(sizeof(HashEntry));
    if (novaEntrada == NULL) {
        perror("Falha ao alocar entrada Hash.");
        return;
    }
    
    // Copia dados
    strncpy(novaEntrada->pista, pista, MAX_PISTA);
    strncpy(novaEntrada->suspeito, suspeito, MAX_SUSPEITO);
    
    // Insere no início da lista encadeada (colisão)
    novaEntrada->proximo = tabelaHash[indice];
    tabelaHash[indice] = novaEntrada;
}

// encontrarSuspeito(): Consulta o suspeito correspondente a uma pista.
char* encontrarSuspeito(const char *pista) {
    unsigned int indice = hash(pista);
    HashEntry *atual = tabelaHash[indice];

    while (atual != NULL) {
        if (strcmp(atual->pista, pista) == 0) {
            return atual->suspeito; // Suspeito encontrado
        }
        atual = atual->proximo;
    }
    return NULL; // Não encontrado
}

// ----------------------------------------------------------------------------
// --- IMPLEMENTAÇÃO DAS ÁRVORES E JOGO ---
// ----------------------------------------------------------------------------

// criarSala(): Cria dinamicamente um cômodo.
Sala* criarSala(const char* nome, const char* pista) {
    Sala* novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Falha na alocação de memória para Sala.");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, MAX_NOME);
    strncpy(novaSala->pista, pista, MAX_PISTA);
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

// inserirPista(): Insere a pista coletada na BST (recursivo).
PistaNode* inserirPista(PistaNode* raiz, const char* pista) {
    if (raiz == NULL) {
        PistaNode* novo = (PistaNode*)malloc(sizeof(PistaNode));
        if (novo == NULL) return NULL;
        strncpy(novo->pista, pista, MAX_PISTA);
        novo->esquerda = novo->direita = NULL;
        return novo;
    }

    int cmp = strcmp(pista, raiz->pista);

    if (cmp < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, pista);
    } else if (cmp > 0) {
        raiz->direita = inserirPista(raiz->direita, pista);
    }
    return raiz;
}

// explorarSalas(): Navega pela árvore e ativa o sistema de pistas.
void explorarSalas(Sala* raiz, PistaNode** bstPistas) {
    Sala* atual = raiz;
    char escolha;
    
    printf("\n--- Início da Exploração ---\n");

    while (atual != NULL) {
        printf("\n[LOCALIZAÇÃO] Você está em: %s\n", atual->nome);

        // Identificar e exibir pista
        if (strlen(atual->pista) > 0) {
            printf("[PISTA ENCONTRADA] Conteúdo: \"%s\"\n", atual->pista);
            
            // Armazenar a pista na BST
            *bstPistas = inserirPista(*bstPistas, atual->pista);
            
            // Exibir Suspeito Associado (Usando Hash)
            char* suspeito = encontrarSuspeito(atual->pista);
            if (suspeito) {
                printf("[ASSOCIAÇÃO HASH] Pista aponta para: %s\n", suspeito);
            } else {
                printf("[ASSOCIAÇÃO HASH] Suspeito ainda nao identificado.\n");
            }
            
            // Marca a pista como coletada para não pegar novamente
            atual->pista[0] = '\0'; 
        } else {
            printf("[PISTA] Nenhum indício novo encontrado aqui.\n");
        }

        // Menu e Navegação
        printf("Caminhos: (e) esquerda, (d) direita, (s) sair > ");
        if (scanf(" %c", &escolha) != 1) { limparBufferEntrada(); continue; }

        Sala* proximaSala = NULL;
        if (escolha == 'e' || escolha == 'E') {
            proximaSala = atual->esquerda;
        } else if (escolha == 'd' || escolha == 'D') {
            proximaSala = atual->direita;
        } else if (escolha == 's' || escolha == 'S') {
            printf("\n[SAÍDA] Fim da jornada de exploração.\n");
            break; 
        } else {
            printf("[ERRO] Comando inválido.\n");
            continue;
        }

        if (proximaSala != NULL) {
            atual = proximaSala;
        } else {
            printf("[ALERTA] Caminho inexistente ou bloqueado.\n");
            // Não encerra automaticamente, permite nova escolha
        }
    }
}

// contarPistasPorSuspeito(): Contagem recursiva
int contarPistasPorSuspeito(PistaNode* raiz, const char* suspeitoAlvo) {
    if (raiz == NULL) return 0;

    int contagem = 0;
    
    // Verifica a pista atual
    char* suspeito = encontrarSuspeito(raiz->pista);
    if (suspeito && strcmp(suspeito, suspeitoAlvo) == 0) {
        contagem = 1;
    }
    
    // Soma com a contagem dos filhos (Recursividade)
    contagem += contarPistasPorSuspeito(raiz->esquerda, suspeitoAlvo);
    contagem += contarPistasPorSuspeito(raiz->direita, suspeitoAlvo);
    
    return contagem;
}


// verificarSuspeitoFinal(): Conduz à fase de julgamento final.
void verificarSuspeitoFinal(PistaNode* bstPistas) {
    char acusacao[MAX_SUSPEITO];
    printf("\n\n--- FASE DE JULGAMENTO ---\n");
    printf("Acuse o culpado (Ex: 'Mordomo', 'Cozinheira', 'Bibliotecario'): ");
    if (scanf("%29s", acusacao) != 1) {
        printf("[ERRO] Acusacao invalida.\n");
        return;
    }

    // Verifica se, pelo menos, duas pistas apontam para o suspeito
    int pistasDeSuporte = contarPistasPorSuspeito(bstPistas, acusacao);

    printf("\n--- VERIFICAÇÃO DE EVIDÊNCIAS ---\n");
    printf("Suspeito Acusado: %s\n", acusacao);
    printf("Pistas de Suporte Encontradas: %d\n", pistasDeSuporte);

    if (pistasDeSuporte >= 2) {
        printf("\n[VEREDITO] Acusação BEM SUCEDIDA! HÁ EVIDÊNCIAS SUFICIENTES! (%s é o culpado)\n", acusacao);
    } else {
        printf("\n[VEREDITO] Acusação FALHA. Evidências insuficientes (%d pistas). A verdade escapou!\n", pistasDeSuporte);
    }
}

// exibirPistas(): Imprime a BST em ordem alfabética.
void exibirPistas(PistaNode* raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esquerda);
    printf("  - %s\n", raiz->pista);
    exibirPistas(raiz->direita);
}

// ----------------------------------------------------------------------------
// --- Funções de Limpeza de Memória ---
// ----------------------------------------------------------------------------

void liberarMapa(Sala* raiz) {
    if (raiz == NULL) return;
    liberarMapa(raiz->esquerda);
    liberarMapa(raiz->direita);
    free(raiz);
}

void liberarBST(PistaNode* raiz) {
    if (raiz == NULL) return;
    liberarBST(raiz->esquerda);
    liberarBST(raiz->direita);
    free(raiz);
}

void liberarHash() {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        HashEntry *atual = tabelaHash[i];
        HashEntry *proximo;
        while (atual != NULL) {
            proximo = atual->proximo;
            free(atual);
            atual = proximo;
        }
    }
}

void limparBufferEntrada() { int c; while ((c = getchar()) != '\n' && c != EOF) {} }


// ----------------------------------------------------------------------------
// --- Função Principal (main) ---
// ----------------------------------------------------------------------------

int main() {
    // Inicialização
    PistaNode* pistasColetadas = NULL;
    inicializarHash();

    // 1. Definição da Tabela Hash (Associação Pista -> Suspeito)
    // As pistas devem ser as mesmas definidas na AB da Mansão.
    inserirNaHash("O mordomo e canhoto.", "Mordomo");
    inserirNaHash("A arma e de prata.", "Bibliotecario");
    inserirNaHash("Ha rastros de cafe.", "Cozinheira");
    inserirNaHash("A porta do jardim esta aberta.", "Jardineiro");
    inserirNaHash("O assassino deixou um bilhete.", "Mordomo"); // Pista de suporte

    // 2. Criação do Mapa da Mansão (Árvore Binária)
    Sala* hall = criarSala("Hall de Entrada", "A porta do jardim esta aberta.");

    Sala* sala_estar = criarSala("Sala de Estar", "O mordomo e canhoto.");
    Sala* biblioteca = criarSala("Biblioteca", ""); 

    Sala* cozinha = criarSala("Cozinha", "Ha rastros de cafe.");
    Sala* escritorio = criarSala("Escritorio", "O assassino deixou um bilhete.");
    
    hall->esquerda = sala_estar;
    hall->direita = biblioteca;

    sala_estar->esquerda = cozinha;
    sala_estar->direita = criarSala("Jantar", "A vitima usava um lenco."); 
    
    biblioteca->esquerda = criarSala("Quarto Mestre", "A arma e de prata."); 
    biblioteca->direita = escritorio;

    cozinha->esquerda = criarSala("Jardim", "");
    cozinha->direita = criarSala("Quarto de Hospedes", "");

    // 3. Execução do Jogo
    explorarSalas(hall, &pistasColetadas);
    
    // 4. Julgamento Final
    printf("\n\n#################################################\n");
    printf("     RELATÓRIO FINAL: PISTAS COLETADAS \n");
    printf("#################################################\n");
    if (pistasColetadas == NULL) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        exibirPistas(pistasColetadas);
    }
    printf("#################################################\n");

    verificarSuspeitoFinal(pistasColetadas);

    // 5. Limpeza de Memória
    liberarMapa(hall);
    liberarBST(pistasColetadas);
    liberarHash();
    
    return 0;
}
