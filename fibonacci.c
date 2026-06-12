#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * fibonacci.c
 * Genera los primeros N números de la secuencia de Fibonacci usando
 * UN único hilo trabajador (Pthreads), según la Práctica No. 4.
 *
 * - main reserva memoria dinámica (malloc) para el arreglo compartido.
 * - main crea un hilo trabajador y le pasa (puntero al arreglo, N)
 *   empaquetados en una estructura.
 * - El hilo trabajador llena el arreglo secuencialmente.
 * - main se bloquea con pthread_join y solo después imprime la secuencia,
 *   garantizando que no lee resultados antes de que existan.
 *
 * Compilar:  gcc -o fibonacci fibonacci.c -lpthread
 * Ejecutar:  ./fibonacci 10
 */

// Estructura para transferir los datos del hilo principal al trabajador
typedef struct {
    unsigned long long *fib; // puntero al arreglo compartido
    int n;                   // cantidad de elementos a generar
} FibArgs;

// Rutina del hilo trabajador: calcula los N números de Fibonacci
// y los almacena secuencialmente en el arreglo compartido.
void *FibWorker(void *arg) {
    FibArgs *args = (FibArgs *)arg;
    unsigned long long *fib = args->fib;
    int n = args->n;

    if (n >= 1) fib[0] = 0;          // F(0) = 0
    if (n >= 2) fib[1] = 1;          // F(1) = 1
    for (int i = 2; i < n; i++) {
        fib[i] = fib[i - 1] + fib[i - 2];
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <N elementos>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Error: N debe ser un entero positivo.\n");
        return 1;
    }

    // 1. El hilo main asigna memoria dinámica para el arreglo compartido
    unsigned long long *fib = malloc(n * sizeof(unsigned long long));
    if (fib == NULL) {
        fprintf(stderr, "Error: no se pudo asignar memoria.\n");
        return 1;
    }

    // 2. Empaquetar argumentos y crear el ÚNICO hilo trabajador
    FibArgs args;
    args.fib = fib;
    args.n = n;

    pthread_t worker;
    if (pthread_create(&worker, NULL, FibWorker, (void *)&args) != 0) {
        fprintf(stderr, "Error: fallo en pthread_create.\n");
        free(fib);
        return 1;
    }

    // 3. Sincronización: main se bloquea hasta que el trabajador termine
    pthread_join(worker, NULL);

    // 4. Solo DESPUÉS del join, main imprime la secuencia
    printf("Secuencia de Fibonacci (%d elementos):\n", n);
    for (int i = 0; i < n; i++) {
        printf("%llu ", fib[i]);
    }
    printf("\n");

    free(fib);
    return 0;
}
