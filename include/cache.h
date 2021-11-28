#ifndef CACHE_H
#define CACHE_H

struct Bloco {
    int valido;
    int sujo;
    int tag;
    char dados[128];
};


struct Cache {
    Bloco blocos[64];
    unsigned int hits;
    unsigned int misses;
    unsigned int reads;
    unsigned int writes;

    void iniciarCache();
    void AtualizarBloco(int tag, int indice, int bloffset, char *dados, char *Mem, int operacao);
};



#endif
