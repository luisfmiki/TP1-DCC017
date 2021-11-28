#include <stdio.h>
#include <stdlib.h>
#include "../include/cache.h"


void DecodeReqLine(char *reqLine, int *endereco, int *operacao, char dados[32]) {
    int aux = 0;
    char ender[4];
    while(reqLine[aux] != ' ') {
        ender[aux] = reqLine[aux];
        aux++;
    }
    aux++;
    *endereco = atoi(ender);
    *operacao = reqLine[aux] - '0';
    aux++;
    if(reqLine[aux] == ' ') {
        for(int i=0;i<32;i++) dados[i] = reqLine[aux + 1 + i];
    }
}



void MemoryWrite(Cache& cache, int index, char Mem[32768]) {
    int endereco_anterior = (cache.blocos[index].tag << 10) + (index << 4);
    for(int i=0; i<128; i++) { 
        // byte addressing   
        Mem[endereco_anterior + i] = cache.blocos[index].dados[i];
    }
    printf("Dados do endereço espacial %d foram escritos na memória\n", endereco_anterior);
}


/* algoritmo principal - abordagem de write allocation */
char CPUreq(int endereco, int operacao, char dados[32], Cache& cache, char Mem[32768]) {

    int index = (endereco & 1008) >> 4;
    int tag = (endereco & 3072) >> 10;
    int byte_offset = endereco & 3;
    int block_offset = (endereco & 12) >> 2;
    
    // Bloco de código responsável pela escrita
    if(operacao) {
        cache.writes++;
        /* escreve na memória o bloco que já estava na cache antes se o bloco estiver sujo
           e se o tag for diferente */ 
        if(cache.blocos[index].sujo && tag != cache.blocos[index].tag) {
            MemoryWrite(cache, index, Mem);
        }  

        cache.AtualizarBloco(tag, index, block_offset, dados, Mem, operacao);
        cache.blocos[index].sujo = 1;

        return 'W';
    }

    // Bloco da leitura  
    else {
        cache.reads++;

        // bloco de código respósavel pelo miss
        if(!(cache.blocos[index].valido) ||  tag != cache.blocos[index].tag) {
            cache.misses++;
            // escreve na memória a palavra que já estava na cache antes se o bloco estiver sujo
            if(cache.blocos[index].sujo) {
                MemoryWrite(cache, index, Mem);
            }
            printf("MISS! Dados do endereco %d não estam na cache\n", endereco);
            
            // atualiza a cache com os dados do endereço requisitado
            cache.AtualizarBloco(tag, index, block_offset, dados, Mem, operacao);
            cache.blocos[index].sujo = 0;
            return 'M';
        }
        
        cache.hits++;
        printf("HIT! Dados do endereço %d: ", endereco);
        for(int i=0; i<32; i++) {
            if(i >= byte_offset * 8 && i < byte_offset * 8 + 8) {
                printf("\033[0;31m");
                printf("%c", cache.blocos[index].dados[block_offset*32 + i]);
                printf("\033[0m");
            }
            else printf("%c", cache.blocos[index].dados[block_offset*32 + i]);
        }
        printf("\n");
        return 'H';
    }
}



int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: ./MemSimulator.out [CPU request file]\n");
        return 1;
    }

    struct Cache cache;
    cache.iniciarCache();

    char Memoria[32768];    // 1024 words
    for(int i=0;i<32768;i++) Memoria[i] = '0'; // inicia a memória com todos os bits nulos

    FILE* input_file=fopen(argv[1], "r+t");
    FILE* output_file=fopen("result.txt", "w+t");
   
    if(input_file != NULL && output_file != NULL) {
        char reqLine[41];

        printf("Rodando a simulação em C...\n");

        /* arquivo com até 150 requisições da CPU
        armazena as informações para imprimir depois no arquivo de saída */
        int enderecos[150];
        int operacoes[150];
        char writeReqs_data[150 * 32];
        char respostas[150];

        int line = 0;
        while(!feof(input_file)) {
            fgets(reqLine, 41, input_file);  // armazena o conteúdo da linha atual

            /* filtro para ver se o programa leu uma linha que n existe */
            if(reqLine[0] - '0' < 0 || (reqLine[1] - '0' < 0 && reqLine[1] != ' ')) continue;
            
            DecodeReqLine(reqLine, &enderecos[line], &operacoes[line], &writeReqs_data[line*32]);

            /* chama o algoritmo principal para retornar o char 'W', 'H' ou 'M' */
            respostas[line] = CPUreq(enderecos[line],operacoes[line],&writeReqs_data[line*32],cache,Memoria);
            line++;
        }

        /* imprime os resultados no arquivo de saída */

        fprintf(output_file, "READS: %d\nWRITES: %d\nHITS: %d\nMISSES: %d\nHIT RATE: %.3f\nMISS RATE: %.3f\n\n", 
                    cache.reads, cache.writes, cache.hits, cache.misses, (float)cache.hits/cache.reads, (float)cache.misses/cache.reads);

        for(int i=0;i<line;i++) {
            fprintf(output_file, "%d %d ", enderecos[i], operacoes[i]);
            if(operacoes[i] == 1) {
                for(int j=0;j<32;j++) fprintf(output_file, "%c", writeReqs_data[i*32 + j]);
                fprintf(output_file, " ");
            }
            fprintf(output_file, "%c\n", respostas[i]);
        }


        fclose(input_file);
        fclose(output_file);
        printf("-------------------------------------------------------------\n\
            Resultados escritos em result.txt\n");
        return 0;
    }
    
    printf("Erro ao carregar os arquivos de entrada/saída.");
    return 1;  
}

