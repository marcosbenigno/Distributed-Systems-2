#include <stdio.h>

#include <stdlib.h>

#include <semaphore.h>

#include <pthread.h>

#include <math.h>

//COMPILACAO COM g++ -pthread ex2.c -o ex2

sem_t mutex;
sem_t empty;
sem_t full;
clock_t start, stop;
int N;
int numsConsumidos = 0;

//struct buffer com array limitado no estilo fifo

struct buffer {
    int * array;
    int N;
    int posicaoARemover;
    int posicaoAAdicionar;
    int elementosEmUso = 0;
    bool estaCheio = false;
    bool estaVazio = true;
};

struct buffer * sharedBuffer;

void inserirBuffer(int numero, struct buffer * buff) {
    //insere no buffer
    if (!buff -> estaCheio) {
        printf("\nInserindo %d na posicao %d\n", numero, buff -> posicaoAAdicionar);
        buff -> array[buff -> posicaoAAdicionar] = numero;
        buff -> elementosEmUso = buff -> elementosEmUso + 1;
        buff -> estaVazio = false;
        if (buff -> posicaoAAdicionar == buff -> N - 1) {
            buff -> posicaoAAdicionar = 0;
        } else {
            buff -> posicaoAAdicionar = buff -> posicaoAAdicionar + 1;
        }
        if (buff -> elementosEmUso == buff -> N) {
            buff -> estaCheio = true;
        }
    }
}

bool ePrimo(int n) {
    if (n <= 1)
        return false;
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0)
            return false;
    return true;
}

int removerBuffer(struct buffer * buff) {
    //remove de buffer
    if (!buff -> estaVazio) {
        printf("\nRemovendo %d de posicao: %d\n", buff -> array[buff -> posicaoARemover], buff -> posicaoARemover);
        int removido = buff -> array[buff -> posicaoARemover];
        buff -> array[buff -> posicaoARemover] = 0;
        buff -> elementosEmUso = buff -> elementosEmUso - 1;
        buff -> estaCheio = false;
        if (buff -> posicaoARemover == buff -> N - 1) {
            buff -> posicaoARemover = 0;
        } else {
            buff -> posicaoARemover = buff -> posicaoARemover + 1;
        }
        if (buff -> elementosEmUso == 0) {
            buff -> estaVazio = true;
        }
        numsConsumidos += 1;
        return removido;
    }
    return -1;
}

struct buffer * createBuffer(int N) {
    //funcao cria buffer
    struct buffer * buff;
    buff -> posicaoARemover = 0;
    buff -> posicaoAAdicionar = 0;
    buff -> N = N;
    buff -> array = (int * ) malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        buff -> array[i] = 0;
    }
    return buff;
}

int randomNumber() {
    int num = rand() % 10000000 + 1;
    return num;
}

void * funcaoProdutora(void * parameter) {

    while (1) {
        sem_wait( & empty);

        sem_wait( & mutex);

        inserirBuffer(randomNumber(), sharedBuffer);
        sem_post( & mutex);

        sem_post( & full);

    }

}

void * funcaoConsumidora(void * parameter) {
    while (1) {

        sem_wait( & full);

        sem_wait( & mutex);

        int numero = removerBuffer(sharedBuffer);
        if (numsConsumidos == 500000) {
            //limitado ao consumo de 500000 numeros
            stop = clock();
            double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
            printf("Tempo de execucao (ms): %f", elapsed);
            exit(0);
        }
        
        bool primo = ePrimo(numero);
        if (primo) {
            printf("\n%d e primo\n", numero);
        } else {
            printf("\n%d nao e primo\n", numero);
        }
        sem_post( & mutex);

        sem_post( & empty);

    }

}

int main() {
    srand(time(0));
    int produtores, consumidores;

    printf("Entre com o tamanho do buffer\n");
    scanf("%d", & N);
    sharedBuffer = createBuffer(N);
    printf("Entre com o numero de produutores\n");
    scanf("%d", & produtores);

    printf("Entre com o numero de consumidores\n");
    scanf("%d", & consumidores);

    //inicializ semaforos
    sem_init( & mutex, 0, 1);
    sem_init( & empty, 0, N);
    sem_init( & full, 0, 0);

    pthread_t threadsProdutoras[produtores];
    pthread_t threadsConsumidoras[consumidores];
    start = clock();
    //cria threads
    for (int i = 0; i < consumidores; i++) {
        pthread_create( & (threadsConsumidoras[i]), NULL, & funcaoConsumidora, NULL);
    }

    for (int i = 0; i < produtores; i++) {
        pthread_create( & (threadsProdutoras[i]), NULL, & funcaoProdutora, NULL);
    }

    for (int i = 0; i < consumidores; i++) {
        pthread_join(threadsConsumidoras[i], NULL);
    }
    
    for (int i = 0; i < produtores; i++) {
        pthread_join(threadsProdutoras[i], NULL);
    }

    return 0;
}
