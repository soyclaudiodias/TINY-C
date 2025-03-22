/*
Para compilar no vscode use:
gcc compilador.c -Wall -Og -g -o compilador

// teste de memoria
https://diveintosystems.org/book/C3-C_debug/valgrind.html

Rode o Valgrind com 
valgrind --leak-check=yes ./miniLexico 

caso não esteja instalado use
sudo apt update
sudo apt install valgrind
sudo apt upgrade
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h> // strncpy
#include <stdlib.h> // atof

// definicoes dos atomos
typedef enum {
    ERRO,
    ENDOFFILE,
    ENDOFSTRING,
    COMENTARIO,
    ID,
    INT,
    CHAR,
    VOID,
    PONTO_VIRGULA,
    ABRE_PAR,
    FECHA_PAR,
    ABRE_CHAVES,
    FECHA_CHAVES,
    ASPAS,
    IF,
    ELSE,
    WHILE,
    MAIN,
    READINT,
    WRITEINT,
    VIRGULA,
    INTCONST,
    CHARCONST,
    IGUAL,
    MAIOR,
    MENOR,
    EXCLAMACAO,
    ECOMERCIAL,
    PIPE // fim de buffer
} TAtomo;

typedef struct {
    TAtomo atomo;
    int linha;
    char identificador[15];
} TInfoAtomo;

// declaracao de variaveis globais

int contaLinha = 1;
char *entrada = NULL,
*strAtomo[] = {
    "erro", "endoffile", "endofstring", "comentario", "id", "int", "char", "void",
    "ponto_virgula", "abre_par", "fecha_par", "abre_chaves", "fecha_chaves", "aspas",
    "if", "else", "while", "main", "readint", "writeint", "virgula", "intconst",
    "charconst", "igual", "maior", "menor", "exclamacao", "ecomercial", "pipe"
},
*palavras_reservadas[] = {
    "int", "char", "void", "if", "else", "while", "main", "readint", "writeint"
};

// declaracao da funcao
TInfoAtomo obter_atomo();
TInfoAtomo reconhece_id();
TInfoAtomo reconhece_int();
TInfoAtomo reconhece_comentario();

int main(int num_argumentos, char **argumentos) {
    // verifica se o nome do arquivo foi passado como argumento
    if (num_argumentos < 2) {
        printf("Uso: %s <arquivo>\n", argumentos[0]);
        return 1;
    }

    FILE *arquivo = fopen(argumentos[1], "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s!\n", argumentos[1]);
        return 1;
    }

    // determina o tamanho do arquivo
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);
    
    // aloca memória para o conteúdo do arquivo
    char *buffer = (char *)malloc(tamanho + 1);
    if (buffer == NULL) {
        printf("Erro de alocação de memória!\n");
        fclose(arquivo);
        return 1;
    }
    
    // lê o conteúdo de um arquivo e armazena os dados em um buffer
    size_t lidos = fread(buffer, 1, tamanho, arquivo);
    buffer[lidos] = '\0';  // Adiciona o terminador nulo
    entrada = buffer;

    fclose(arquivo);

    TInfoAtomo info_atm;
    do {
        info_atm = obter_atomo();

        // FUNCAO DO SINTATICO
        printf("#  %d:%s", info_atm.linha, strAtomo[info_atm.atomo]);
        if (info_atm.atomo == ID) {
            printf(" | %s", info_atm.identificador);
        }
        printf("\n");

    } while (info_atm.atomo != ERRO && info_atm.atomo != ENDOFFILE);

    free(buffer);

    printf("%d linhas analisadas, programa sintaticamente correto", contaLinha);
}

// ---------- LEXICO ----------
// implementacao da funcao
TInfoAtomo obter_atomo() {
    TInfoAtomo info_atomo;
    info_atomo.atomo = ERRO;

    if (*entrada == '\0') {
        info_atomo.atomo = ENDOFFILE;
        return info_atomo;
    }

    // eliminar delimitadores
    while (*entrada == ' ' || *entrada == '\n' || *entrada == '\r' || *entrada == '\t') {
        if (*entrada == '\n')
            contaLinha++;
        entrada++;
    }

    switch (*entrada) {
        case '\0':
            info_atomo.atomo = ENDOFSTRING;
            entrada++;
            break;
        case '{':
            info_atomo.atomo = ABRE_CHAVES;
            entrada++;
            break;
        case '}':
            info_atomo.atomo = FECHA_CHAVES;
            entrada++;
            break;
        case '(':
            info_atomo.atomo = ABRE_PAR;
            entrada++;
            break;
        case ')':
            info_atomo.atomo = FECHA_PAR;
            entrada++;
            break;
        case '"':
        case '\'':
            info_atomo.atomo = ASPAS;
            entrada++;
            break;
        case ';':
            info_atomo.atomo = PONTO_VIRGULA;
            entrada++;
            break;
        case ',':
            info_atomo.atomo = VIRGULA;
            entrada++;
            break;
        case '=':
            info_atomo.atomo = IGUAL;
            entrada++;
            break;
        case '>':
            info_atomo.atomo = MAIOR;
            entrada++;
            break;
        case '<':
            info_atomo.atomo = MENOR;
            entrada++;
            break;
        case '!':
            info_atomo.atomo = EXCLAMACAO;
            entrada++;
            break;
        case '&':
            info_atomo.atomo = ECOMERCIAL;
            entrada++;
            break;
        case '|':
            info_atomo.atomo = PIPE;
            entrada++;           
            break;
        default:
            if (isdigit(*entrada)) {
                info_atomo = reconhece_int();
            } else if (isalpha(*entrada) || *entrada == '_') {
                info_atomo = reconhece_id();
            } else if (*entrada == '/' && (*(entrada + 1) == '/' || *(entrada + 1) == '*')) {
                info_atomo = reconhece_comentario();
            }
            break;
    }

    info_atomo.linha = contaLinha;
    return info_atomo;
}

// IDENTIFICADOR -> (LETRA_MINUSCULA | LETRA_MAIUSCULA | _)+(LETRA_MINUSCULA | LETRA_MAIUSCULA | _ | DIGITO)*
TInfoAtomo reconhece_id() {
    TInfoAtomo info_id;
    char *ini_id = entrada;
    int count = 1;
    info_id.atomo = ERRO;

    TAtomo tipos_reservados[] = {
        INT, CHAR, VOID, IF, ELSE, WHILE, MAIN, READINT, WRITEINT
    };

    if (isalpha(*entrada) || *entrada == '_') {
        entrada++; // consome letra maiuscula, minuscula ou _
        goto q1;
    }
    return info_id;

q1:
    if (count > 15) {
        return info_id; // Excedeu 15 caracteres
    }
    if (isalpha(*entrada) || *entrada == '_' || isdigit(*entrada)) {
        count++;
        entrada++; // consome letra maiuscula, minuscula ou _
        goto q1;
    }

    for (int i = 0; i < sizeof(palavras_reservadas) / sizeof(palavras_reservadas[0]); i++) {
        size_t len_reservada = strlen(palavras_reservadas[i]);

        if (entrada - ini_id == len_reservada && strncmp(ini_id, palavras_reservadas[i], len_reservada) == 0) {
            info_id.atomo = tipos_reservados[i];
            return info_id;
        }
    }

    info_id.atomo = ID;
    strncpy(info_id.identificador, ini_id, entrada - ini_id);
    info_id.identificador[entrada - ini_id] = '\0';

    return info_id;
}

// INT -> DIGITO+
TInfoAtomo reconhece_int() {
    TInfoAtomo info_int;
    info_int.atomo = ERRO;

    if (isdigit(*entrada)) {
        entrada++;
        goto q1;
    }

    return info_int;

q1:
    if (isdigit(*entrada)) {
        entrada++;
        goto q1;
    }

    info_int.atomo = INT;
    return info_int;
}

// CHAR
TInfoAtomo reconhece_char(){
    TInfoAtomo info_char;
    info_char.atomo = ERRO;

    if(isalpha(*entrada) || isdigit(*entrada) || *entrada == '_'){
        entrada++;
        info_char.atomo = CHAR;
    }

    return info_char;
}

// COMENTARIO
TInfoAtomo reconhece_comentario() {
    TInfoAtomo info_coment;
    info_coment.atomo = COMENTARIO;

    if (*entrada == '/') {
        entrada++;
        if (*entrada == '/') {
            entrada++;

            while (*entrada != '\0' && *entrada != '\n') {
                entrada++;
            } 
        } else if (*entrada == '*') {
            entrada++;

            while (*entrada != '\0' && !(*entrada == '*' && *(entrada + 1) == '/')) {
                if (*entrada == '\n')
                    contaLinha++;
                entrada++;
            }
            
            if (*entrada == '*' && *(entrada + 1) == '/') {
                entrada += 2;
            }
        }
    }

    return info_coment;
}
