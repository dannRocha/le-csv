#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>

struct mapa {
	char* chave;
	int valor;
};

struct pilha {
	struct mapa* mapa;
	int capacidade;
};

struct cabecalho {
	char** colunas;
	int capacidade;
};

enum OpcoesArgumentos {
	NOME_ARQUIVO = 1,
	TITULO_COLUNA = 2
};

enum {
	VAZIO = 0
};


char* removerCaracter(char*, char);
int comparar(const void*, const void*);
void ordernarMapa(struct mapa*, int);
void pularLinha(FILE*);
struct cabecalho  extrairCabecalho(FILE*, char);
struct pilha extrairColuna(FILE*, int, char);
void exibirTabela(struct pilha, char*);
int selecionarColuna(struct cabecalho, char*);
void destruir( void *, char*);

int main(int argc, char**argv) {

	FILE *arquivo = NULL;
	const int TOTAL_DE_PARAMETROS = 3;
	const char delimitador = ';';
	
	if(argc < TOTAL_DE_PARAMETROS) {
		fprintf(stderr, "Passe um arquivo csv e o titulo da posicaoColuna como parametro: OBS: delimitador ponto e virgula(;)");
		exit(1);
	}

	char* tituloColuna = argv[TITULO_COLUNA];
	char* nomeArquivo = argv[NOME_ARQUIVO];

	arquivo = fopen(nomeArquivo , "r");
	
	if(arquivo == NULL) {
		fprintf(stderr, "Não foi possivel abri o arquivo \"%s\"", nomeArquivo);
		exit(1);
	}
	
	struct cabecalho cabecalho = extrairCabecalho(arquivo, delimitador);

	int coluna = selecionarColuna(cabecalho, tituloColuna);

	if(coluna < 0){
		fprintf(stderr, "Coluna \033[31;1m %s \033[0m não existe", tituloColuna);
		destruir(&cabecalho, "struct cabecalho");
		fclose(arquivo);
		exit(1);
	}
	
	struct pilha dadosColuna = extrairColuna(arquivo, coluna, delimitador);
		
	fclose(arquivo);
	ordernarMapa(dadosColuna.mapa, dadosColuna.capacidade);
	exibirTabela(dadosColuna, tituloColuna);
	
	destruir(&dadosColuna, "struct pilha");
	destruir(&cabecalho, "struct cabecalho");

	return 0;
}

void pularLinha(FILE* file) {
	while(fgetc(file) != '\n');
}

char* removerCaracter(char*s, char c) {
	
	char*n = (char*) malloc(strlen(s) * sizeof(char));
	int capacidade = 0;
	
	for(size_t i = 0; i < strlen(s); i++)
		n[i] = '\0';

	for(size_t i = 0; i < strlen(s); i++) {
		if(s[i] == c) continue;
		capacidade++;
		n[capacidade - 1] = s[i];
	}

	return n;
}

int comparar(const void* a, const void* b) {
	if((*(struct mapa *) a).valor < (*(struct mapa *) b).valor) return  1;
	if((*(struct mapa *) a).valor > (*(struct mapa *) b).valor) return -1;
	return 0;
} 

void ordernarMapa(struct mapa *s, int n) {
  qsort(s, n, sizeof(struct mapa), comparar);
}

struct cabecalho  extrairCabecalho(FILE *file, char delimitador) {

	struct cabecalho cabecalho = {.capacidade=0, .colunas=NULL};
	char*  tituloColuna = NULL;
	char letra;
	const int INICIO_ARQUIVO = 1;
	int capacidade = 0;
	int start = 0;
	fpos_t pos;

	fgetpos(file, &pos);
	fseek(file, 0, SEEK_SET);

	while(letra != '\n') {
		
		letra = fgetc(file);
		if((letra == delimitador && !start) || ftell(file) == INICIO_ARQUIVO) {
			start = 1;
			capacidade = 0;
		}
		if(letra == delimitador && start) {
			tituloColuna[capacidade - 1] = '\0';
			char* tituloTratado = removerCaracter(tituloColuna, '"');
			
			start = 0;
			capacidade = 0;
			
			cabecalho.capacidade++;
			cabecalho.colunas = (char**) realloc(cabecalho.colunas, sizeof(char**) * cabecalho.capacidade);
			cabecalho.colunas[cabecalho.capacidade - 1] = (char*)malloc(sizeof(char) * (strlen(tituloTratado) + 1));
			strcpy(cabecalho.colunas[cabecalho.capacidade - 1], tituloTratado);
			
			tituloColuna = (char*)malloc(0);
			free(tituloTratado);
			
			continue;
		}

				
		capacidade++;
		tituloColuna = (char*) realloc(tituloColuna, sizeof(char) * (capacidade + 1));
		tituloColuna[capacidade - 1] = letra;
	}

	fsetpos(file, &pos);

	
	return cabecalho;
}

struct pilha extrairColuna(FILE *file, int posicaoColuna, char delimitador) {
	
	struct pilha dadosColuna = {.capacidade = VAZIO};
	char* valorColuna = (char*)malloc(0);
	int tamanhoDaString = VAZIO;
	int qtdDelimitador = VAZIO;
	fpos_t pos;
	fgetpos(file, &pos);
	// PULUAR CABECAÇHO
	pularLinha(file);
	
	do {
		
		char letra = fgetc(file);
		if(letra == delimitador) qtdDelimitador++;
	
		if(qtdDelimitador == posicaoColuna && letra != delimitador) {
			tamanhoDaString++;
			valorColuna = (char*) realloc(valorColuna, sizeof(char) * (tamanhoDaString + 1));
			valorColuna[tamanhoDaString - 1] = letra;
		}
		if(qtdDelimitador > posicaoColuna && letra == delimitador) {
			valorColuna[tamanhoDaString] = '\0';
			char* valorTratado = removerCaracter(valorColuna, '"');

			if(dadosColuna.capacidade == VAZIO) {
				dadosColuna.capacidade = 1;
				dadosColuna.mapa = (struct mapa*)realloc(dadosColuna.mapa, sizeof(struct mapa) * dadosColuna.capacidade);
				dadosColuna.mapa[dadosColuna.capacidade - 1].chave = (char*)calloc(strlen(valorTratado) + 1, sizeof(char));
				strcpy(dadosColuna.mapa[dadosColuna.capacidade - 1].chave, valorTratado);
				dadosColuna.mapa[dadosColuna.capacidade - 1].valor = 1;

			} else {
			
				bool existeChave = false;
				
				for(int i = 0; i < dadosColuna.capacidade; i++) {
					if(!strcmp(dadosColuna.mapa[i].chave, valorTratado)) {
						dadosColuna.mapa[i].valor += 1;
						existeChave = true;
						break;
					}
				}

				if(!existeChave) {
					dadosColuna.capacidade += 1;
					dadosColuna.mapa = (struct mapa*)realloc(dadosColuna.mapa, sizeof(struct mapa) * dadosColuna.capacidade);
					dadosColuna.mapa[dadosColuna.capacidade - 1].chave = (char*)calloc(strlen(valorTratado) + 1, sizeof(char));
					dadosColuna.mapa[dadosColuna.capacidade - 1].valor = 1;
					strcpy(dadosColuna.mapa[dadosColuna.capacidade - 1].chave, valorTratado);
				}

			}

			tamanhoDaString = VAZIO;
			qtdDelimitador = VAZIO;
			valorColuna = (char*) malloc(0);
			free(valorTratado);
				
			pularLinha(file);
		}

	}
	while(!feof(file));

	fsetpos(file, &pos);
	
	return dadosColuna;
}

int selecionarColuna(struct cabecalho cabecalho, char* tituloColuna) {
	
	for(int i = 0; i < cabecalho.capacidade; i++) {
		if(!strcmp(cabecalho.colunas[i], tituloColuna))
			return i;
	}

	return -1;
}

void exibirTabela(struct pilha dadosColuna, char* posicaoColuna ) {

	printf("|     INDEX \t | Nº CANDIDATOS |    %s \n", posicaoColuna);
	for(int i = 0; i < dadosColuna.capacidade; i++)
		printf("|\t%d \t | \t %d \t | \t%s\n", i + 1, dadosColuna.mapa[i].valor, dadosColuna.mapa[i].chave);
	
	printf("---------------------------------------------------------\n");
	printf("| \t-\t |\tTOTAL\t | \t -\n");

	int totalCandidatos = 0;

	for(int i = 0; i < dadosColuna.capacidade; i++)
		totalCandidatos += dadosColuna.mapa[i].valor;

	printf("|\t   \t |\t%d\t |\n",totalCandidatos);

}

void destruir( void *memoria, char* tipo ) {
	if(!strcmp(tipo, "struct pilha")) {
		struct pilha* pilha = (struct pilha*) memoria;
		for(int i = 0; i < pilha->capacidade; i++)
			free(pilha->mapa[i].chave);

		free(pilha->mapa);

	}

	if(!strcmp(tipo, "struct cabecalho")) {
		struct cabecalho* cabecalho = (struct cabecalho*) memoria;
		for(int i = 0; i < cabecalho->capacidade; i++)
			free(cabecalho->colunas[i]);
		
		free(cabecalho->colunas);
	}
}
