# Práctica No. 4 — API de Hilos (Pthreads)

**Laboratorio de Sistemas Operativos — Universidad de Antioquia**

## (a) Integrantes

| Nombre completo | Correo | Documento |
|---|---|---|
| Jorge [APELLIDOS] | [correo@udea.edu.co] | CC [número] |
| Esteban [APELLIDOS] | [correo@udea.edu.co] | CC [número] |

## Estructura del repositorio

```
.
├── README.md
├── analisis.ipynb          # Notebook con el análisis de rendimiento (Sección 3 de la guía)
└── src/
    ├── pi.c                # Cálculo de π — versión serial
    ├── pi_p.c              # Cálculo de π — versión paralela con Pthreads
    ├── fibonacci.c         # Fibonacci con un hilo trabajador
    └── fibonacci_serial.c  # Fibonacci sin hilos (para el análisis de tiempo)
```

### Compilación y ejecución

```bash
gcc -o pi_s pi.c -lm
gcc -o pi_p pi_p.c -lpthread -lm
gcc -o fibonacci fibonacci.c -lpthread
gcc -o fibonacci_serial fibonacci_serial.c

./pi_s              # serial, n = 2 000 000 000
./pi_p 8            # paralelo con 8 hilos
./fibonacci 15      # primeros 15 números de Fibonacci
./fibonacci_serial 105000
```

## (b) Documentación de las funciones desarrolladas

### `pi.c` (versión serial)

- **`double GetTime()`** — Retorna el tiempo actual en segundos con precisión de nanosegundos usando `clock_gettime(CLOCK_MONOTONIC)`. Se usa un reloj monótono para que la medición no se vea afectada por ajustes del reloj del sistema.
- **`double CalcPi(int n)`** — Aproxima π integrando numéricamente `4/(1+x²)` en [0,1] con la regla del punto medio: divide el intervalo en `n` rectángulos de ancho `fH = 1/n`, evalúa la función en el punto medio de cada uno (`fX = fH*(i+0.5)`) y acumula. Retorna `fH * fSum`.
- **`int main()`** — Fija `n = 2 000 000 000`, mide exclusivamente el tiempo de `CalcPi` con `GetTime()` antes y después, e imprime el resultado y el tiempo.

### `pi_p.c` (versión paralela)

- **`typedef struct ThreadArgs`** — Estructura para pasar a cada hilo su `id`, el total de iteraciones `n`, el número de hilos `num_threads` y el ancho `fH`.
- **`double GetTime()`** — Igual que en la versión serial.
- **`void* ThreadCalcPi(void* arg)`** — Rutina de cada hilo. Recorre las iteraciones con **distribución cíclica** (`for (i = id; i < n; i += T)`), lo que garantiza balance de carga sin cálculo de rangos. Acumula en una **variable local** (`local_sum`) para evitar contención (no se usa mutex). Reserva un `double` en el heap con el resultado parcial y lo retorna mediante `pthread_exit`.
- **`int main(int argc, char* argv[])`** — Recibe el número de hilos `T` por línea de comandos. Crea los `T` hilos con `pthread_create`, los sincroniza con `pthread_join` recogiendo cada suma parcial (y liberando la memoria con `free`), agrega las sumas y multiplica por `fH` para obtener π. Mide el tiempo total de la fase de cálculo.

### `fibonacci.c` (un hilo trabajador)

- **`typedef struct FibArgs`** — Empaqueta el puntero al arreglo compartido y `N` para pasarlos por el único `void*` que admite `pthread_create`.
- **`void* FibWorker(void* arg)`** — Rutina del hilo trabajador: inicializa `fib[0]=0`, `fib[1]=1` y llena el arreglo secuencialmente con `fib[i] = fib[i-1] + fib[i-2]`. Termina con `pthread_exit(NULL)`.
- **`int main(int argc, char* argv[])`** — Recibe `N` por línea de comandos, **reserva el arreglo compartido con `malloc`**, crea el hilo trabajador pasándole la estructura de argumentos, se **bloquea con `pthread_join`** y solo después imprime la secuencia; finalmente libera la memoria.

### `fibonacci_serial.c` (sin hilos)

- **`double GetTime()`** — Igual que en los anteriores.
- **`int main(int argc, char* argv[])`** — Versión secuencial pura usada en el análisis para medir el tiempo con N grande (> 100e3 valores).

## (c) Problemas presentados y soluciones

1. **Retorno de la suma parcial de cada hilo.** Una variable local del hilo desaparece al terminar la rutina, por lo que retornar su dirección sería un error (dangling pointer). **Solución:** cada hilo reserva el resultado en el heap (`malloc`) y lo retorna vía `pthread_exit`; `main` lo recoge con `pthread_join` y hace `free`.
2. **Contención si todos los hilos suman sobre una variable global.** Proteger la suma global con un mutex dentro del bucle serializa el trabajo y destruye el speedup. **Solución (exigida por la guía):** acumulación en variable local y agregación final en `main`.
3. **Paso de múltiples argumentos al hilo.** `pthread_create` solo admite un `void*`. **Solución:** empaquetar los argumentos en una estructura (`ThreadArgs` / `FibArgs`) y pasar su dirección.
4. **Desbordamiento de enteros en Fibonacci.** Con `unsigned long long` (64 bits) la secuencia desborda a partir de F(93). Para N grande el objetivo de la guía es medir el **tiempo** del cálculo secuencial, no los valores exactos; en el notebook se incluye además una variante en Python con precisión arbitraria. **Solución:** documentar el desbordamiento y reportar el último valor módulo 10^10.
5. **Balance de carga entre hilos.** Si n no es divisible exactamente entre T, una partición por bloques requiere manejar el residuo. **Solución:** distribución cíclica (`i = id; i += T`), que reparte las iteraciones de forma naturalmente balanceada.
6. **Tp(1) mayor que Ts.** Al medir, la versión paralela con 1 hilo resultó algo más lenta que la serial. No es un bug: es el overhead de `pthread_create`/`pthread_join`, el `malloc` del resultado y la indirección por la estructura de argumentos (análisis completo en el notebook).

## (d) Pruebas realizadas

| Prueba | Comando | Resultado esperado | Estado |
|---|---|---|---|
| π serial correcto | `./pi_s` | π ≈ 3.14159265358... con n = 2×10⁹ | ✅ |
| π paralelo correcto (varios T) | `./pi_p 1`, `./pi_p 2`, `./pi_p 4`, `./pi_p 8` | Mismo valor de π (±10⁻¹² por orden de suma en punto flotante) | ✅ |
| Validación de argumentos | `./pi_p` (sin argumento), `./fibonacci` | Mensaje de uso y salida con código 1 | ✅ |
| Fibonacci básico | `./fibonacci 10` | `0 1 1 2 3 5 8 13 21 34` | ✅ |
| Fibonacci de la guía | `./fibonacci 15` | Primeros 15 términos (hasta 377) | ✅ |
| Fibonacci N grande serial | `./fibonacci_serial 105000` | Termina en milisegundos y reporta el tiempo | ✅ |
| Sin fugas de memoria | `valgrind ./pi_p 4`, `valgrind ./fibonacci 15` | 0 bytes definitivamente perdidos | ✅ |
| Speedup | Notebook `analisis.ipynb` | Speedup ≈ lineal hasta el número de núcleos físicos | ✅ |

*(Ejecutar las pruebas en su máquina y ajustar la columna Estado si aplica.)*

## (e) Video de sustentación

📹 [Enlace al video (10 minutos)]([PEGAR ENLACE AQUÍ])

Contenido: estrategia de paralelización en `pi_p.c`, paso del arreglo compartido y sincronización en `fibonacci.c`, demostración de `pi_s`, `pi_p` (1 y N hilos) y `fibonacci`, y análisis del gráfico de Speedup.

## (f) Manifiesto de transparencia (uso de IA generativa)

Se utilizó IA generativa (Claude, Anthropic) como apoyo en los siguientes puntos:

- Revisión y verificación del código de `pi.c` y `pi_p.c` desarrollado por el equipo.
- Apoyo en la implementación de `fibonacci.c` y `fibonacci_serial.c` a partir de la especificación de la guía.
- Estructuración del notebook `analisis.ipynb` (automatización de mediciones, tabla de métricas y gráfico de Speedup).
- Redacción base de la documentación de este README, posteriormente revisada y ajustada por los integrantes.

Las mediciones de rendimiento, la ejecución de las pruebas, el análisis final de resultados y el video de sustentación fueron realizados por los integrantes del equipo.

## (g) Conclusiones

1. **La paralelización con Pthreads produce mejoras de rendimiento casi lineales** en problemas *embarrassingly parallel* como la integración numérica de π, donde las iteraciones son independientes entre sí, siempre que el número de hilos no supere el número de núcleos físicos disponibles.
2. **Evitar la contención es tan importante como paralelizar.** Acumular en variables locales y agregar al final (en lugar de proteger una variable compartida con mutex dentro del bucle) elimina la sincronización del camino crítico; con un mutex por iteración el programa paralelo sería incluso más lento que el serial.
3. **El paralelismo tiene un costo fijo (overhead).** Tp(1) > Ts demuestra que crear, gestionar y sincronizar hilos no es gratis; paralelizar solo vale la pena cuando el trabajo por hilo amortiza ese costo, lo cual sí ocurre con n = 2×10⁹ iteraciones.
4. **La eficiencia decrece al aumentar el número de hilos.** Al acercarse y superar el número de núcleos físicos, los hilos compiten por el hardware (time-sharing, SMT) y el speedup se estanca mientras N crece, comportamiento consistente con la Ley de Amdahl y los límites físicos del procesador.
5. **No todo problema es paralelizable.** Fibonacci es inherentemente secuencial (cada término depende de los dos anteriores); el hilo trabajador no busca speedup sino ilustrar el modelo de memoria compartida: ambos hilos ven el mismo heap y basta compartir un puntero para transferir datos.
6. **`pthread_join` es a la vez barrera de sincronización y mecanismo de retorno de resultados.** Garantiza que `main` no lea el arreglo antes de que esté completo (evita la condición de carrera) y permite recibir las sumas parciales de los hilos de π sin variables globales.
7. **La medición de tiempo debe ser quirúrgica:** usar `CLOCK_MONOTONIC` y cronometrar únicamente la región de interés (`CalcPi`) evita que el reloj del sistema o la E/S contaminen las métricas de speedup y eficiencia.

## (h) Notebook de análisis

📓 [`analisis.ipynb`](analisis.ipynb) — Contiene: medición automática de Ts y Tp(N), tabla de Speedup/Eficiencia, gráfico de Speedup, análisis de resultados de π, salida de `./fibonacci 15` y análisis de diseño de Fibonacci.

## Referencias

- Arpaci-Dusseau, R. & Arpaci-Dusseau, A. *Operating Systems: Three Easy Pieces.* Capítulos 26 (Threads Intro) y 27 (Thread API).
