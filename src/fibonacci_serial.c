#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * fibonacci_serial.c
 * Versión SIN hilos de la generación de Fibonacci, usada en la Sección 2
 * del análisis para comparar tiempos con N grande (> 100e3 valores).
 *
 * Compilar:  gcc -o fibonacci_serial fibonacci_serial.c
 * Ejecutar:  ./fibonacci_serial 105000
 */

double GetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
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

    unsigned long long *fib = malloc(n * sizeof(unsigned long long));
    if (fib == NULL) return 1;

    double start = GetTime();
    if (n >= 1) fib[0] = 0;
    if (n >= 2) fib[1] = 1;
    for (int i = 2; i < n; i++) {
        fib[i] = fib[i - 1] + fib[i - 2];
    }
    double end = GetTime();

    // Nota: con enteros de 64 bits la secuencia desborda a partir de F(93);
    // para N grande el interés es medir el TIEMPO del cálculo secuencial.
    printf("Generados %d valores (último, módulo 10^10: %llu)\n",
           n, fib[n - 1] % 10000000000ULL);
    printf("Tiempo de ejecución serial: %.6f segundos\n", end - start);

    free(fib);
    return 0;
}
