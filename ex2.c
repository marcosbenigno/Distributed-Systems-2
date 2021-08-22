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
    int * data;
    int N;
    int currentRemovePosition;
    int currentAddPosition;
    int elementsInUse = 0;
    bool isFull = false;
    bool isEmpty = true;
};

struct buffer * sharedBuffer;

void inserirBuffer(int numero, struct buffer * buff) {
    //insere no buffer
    if (!buff -> isFull) {
        printf("\nInserindo %d na posicao %d\n", numero, buff -> currentAddPosition);
        buff -> data[buff -> currentAddPosition] = numero;
        buff -> elementsInUse = buff -> elementsInUse + 1;
        buff -> isEmpty = false;
        if (buff -> currentAddPosition == buff -> N - 1) {
            buff -> currentAddPosition = 0;
        } else {
            buff -> currentAddPosition = buff -> currentAddPosition + 1;
        }
        if (buff -> elementsInUse == buff -> N) {
            buff -> isFull = true;
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
    if (!buff -> isEmpty) {
        printf("\nRemovendo %d de posicao: %d\n", buff -> data[buff -> currentRemovePosition], buff -> currentRemovePosition);
        int removido = buff -> data[buff -> currentRemovePosition];
        buff -> data[buff -> currentRemovePosition] = 0;
        buff -> elementsInUse = buff -> elementsInUse - 1;
        buff -> isFull = false;
        if (buff -> currentRemovePosition == buff -> N - 1) {
            buff -> currentRemovePosition = 0;
        } else {
            buff -> currentRemovePosition = buff -> currentRemovePosition + 1;
        }
        if (buff -> elementsInUse == 0) {
            buff -> isEmpty = true;
        }
        numsConsumidos += 1;
        return removido;
    }
    return -1;
}

struct buffer * createBuffer(int N) {
    //funcao cria buffer
    struct buffer * buff;
    buff -> currentRemovePosition = 0;
    buff -> currentAddPosition = 0;
    buff -> N = N;
    buff -> data = (int * ) malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        buff -> data[i] = 0;
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
