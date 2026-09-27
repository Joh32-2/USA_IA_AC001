#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <utility>
#include <fstream>
#include <omp.h>

struct Ciudad {
    double x, y;
};

// Distancia euclidiana normal
double calcularDistancia(const Ciudad& c1, const Ciudad& c2) {
    return std::sqrt(std::pow(c1.x - c2.x, 2) + std::pow(c1.y - c2.y, 2));
}

int main() {
    int numCiudades = 200000; // 200,000 ciudades
    int k = 40;               // Vecinos cercanos locales
    int numHormigas = 50;
    int maxIteraciones = 10;

    double alfa = 1.0;
    double beta = 3.0;
    double rho = 0.5;
    double Q = 100.0;

    auto tiempoInicio = std::chrono::high_resolution_clock::now();

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> distCoord(0.0, 10000.0);

    std::vector<Ciudad> ciudades(numCiudades);
    for (int i = 0; i < numCiudades; ++i) {
        ciudades[i] = {distCoord(rng), distCoord(rng)};
    }

    // Estructuras dispersas k-NN
    std::vector<std::vector<int>> vecinosKNN(numCiudades, std::vector<int>(k));
    std::vector<std::vector<double>> distKNN(numCiudades, std::vector<double>(k));
    std::vector<std::vector<double>> feromonasKNN(numCiudades, std::vector<double>(k, 1.0));

    std::string nombreArchivoCache = "knn_cache_200k.bin";
    std::ifstream archivoLectura(nombreArchivoCache, std::ios::binary);

    // =========================================================================
    // SISTEMA DE CACHÉ Y PRECOMPUTACIÓN OPTIMIZADA (SIN SQRT REPETIDO)
    // =========================================================================
    if (archivoLectura.is_open()) {
        std::cout << "[CACHÉ] Archivo encontrado. Cargando k-NN instantáneamente desde el disco...\n";
        for (int i = 0; i < numCiudades; ++i) {
            archivoLectura.read(reinterpret_cast<char*>(vecinosKNN[i].data()), k * sizeof(int));
            archivoLectura.read(reinterpret_cast<char*>(distKNN[i].data()), k * sizeof(double));
        }
        archivoLectura.close();
        std::cout << "[CACHÉ] ¡Carga completada al instante!\n";
    } else {
        std::cout << "[CACHÉ] No se encontró caché. Iniciando precomputación ultra-optimizada con OpenMP...\n";

#pragma omp parallel for schedule(dynamic, 100)
        for (int i = 0; i < numCiudades; ++i) {
            std::vector<std::pair<double, int>> todasLasDistancias;
            todasLasDistancias.reserve(numCiudades - 1);

            for (int j = 0; j < numCiudades; ++j) {
                if (i == j) continue;
                // OPTIMIZACIÓN CLAVE: Distancia al cuadrado (evita sqrt())
                double dx = ciudades[i].x - ciudades[j].x;
                double dy = ciudades[i].y - ciudades[j].y;
                double distCuadrada = (dx * dx) + (dy * dy);
                todasLasDistancias.push_back({distCuadrada, j});
            }

            // Ordenamos basándonos en la distancia al cuadrado (mismo orden relativo)
            std::sort(todasLasDistancias.begin(), todasLasDistancias.end());

            // Solo aplicamos sqrt() a los k vecinos finalistas
            for (int j = 0; j < k; ++j) {
                vecinosKNN[i][j] = todasLasDistancias[j].second;
                distKNN[i][j] = std::sqrt(todasLasDistancias[j].first);
            }

            if (i % 20000 == 0 && i > 0) {
#pragma omp critical
                std::cout << "Progreso precomputacion: ~" << (i * 100 / numCiudades) << "% completado...\n";
            }
        }

        // Guardar el caché en binario para ejecuciones posteriores
        std::cout << "Guardando caché en disco para futuras ejecuciones...\n";
        std::ofstream archivoEscritura(nombreArchivoCache, std::ios::binary);
        if (archivoEscritura.is_open()) {
            for (int i = 0; i < numCiudades; ++i) {
                archivoEscritura.write(reinterpret_cast<const char*>(vecinosKNN[i].data()), k * sizeof(int));
                archivoEscritura.write(reinterpret_cast<const char*>(distKNN[i].data()), k * sizeof(double));
            }
            archivoEscritura.close();
            std::cout << "¡Caché guardada exitosamente en " << nombreArchivoCache << "!\n";
        }
    }

    auto tiempoPrecomputacion = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duracionPre = tiempoPrecomputacion - tiempoInicio;
    std::cout << "Fase de preparacion lista en: " << duracionPre.count() / 1000.0 << " segundos\n";

    std::vector<int> mejorRutaGlobal;
    double mejorCostoGlobal = 1e18;

    // =========================================================================
    // BUCLE PRINCIPAL DE OPTIMIZACIÓN ACO
    // =========================================================================
    std::cout << "Iniciando bucle de hormigas...\n";
    for (int iter = 0; iter < maxIteraciones; ++iter) {
        std::vector<std::vector<int>> rutasHormigas(numHormigas);
        std::vector<double> costosHormigas(numHormigas, 0.0);

        for (int h = 0; h < numHormigas; ++h) {
            std::vector<bool> visitados(numCiudades, false);
            std::vector<int> ruta;
            ruta.reserve(numCiudades);

            int actual = rng() % numCiudades;
            ruta.push_back(actual);
            visitados[actual] = true;

            for (int paso = 1; paso < numCiudades; ++paso) {
                std::vector<double> probabilidades;
                std::vector<int> candidatosValidos;
                probabilidades.reserve(k);
                candidatosValidos.reserve(k);

                double sumaProbabilidad = 0.0;

                for (int j = 0; j < k; ++j) {
                    int vecino = vecinosKNN[actual][j];
                    if (!visitados[vecino]) {
                        double distancia = distKNN[actual][j];
                        double heuristica = (distancia > 0) ? (1.0 / distancia) : 0.0;
                        double feromona = feromonasKNN[actual][j];

                        double valor = std::pow(feromona, alfa) * std::pow(heuristica, beta);
                        probabilidades.push_back(valor);
                        candidatosValidos.push_back(vecino);
                        sumaProbabilidad += valor;
                    }
                }

                int siguienteSeleccionado = -1;

                if (sumaProbabilidad > 0.0) {
                    std::uniform_real_distribution<double> distProb(0.0, sumaProbabilidad);
                    double r = distProb(rng);
                    double acumulado = 0.0;

                    for (size_t idx = 0; idx < candidatosValidos.size(); ++idx) {
                        acumulado += probabilidades[idx];
                        if (acumulado >= r) {
                            siguienteSeleccionado = candidatosValidos[idx];
                            break;
                        }
                    }
                    if (siguienteSeleccionado == -1) {
                        siguienteSeleccionado = candidatosValidos.back();
                    }
                } else {
                    int mejorCandidatoGlobal = -1;
                    for (int j = 0; j < k; ++j) {
                        int vecino = vecinosKNN[actual][j];
                        if (!visitados[vecino]) {
                            mejorCandidatoGlobal = vecino;
                            break;
                        }
                    }
                    if (mejorCandidatoGlobal == -1) {
                        for (int j = 0; j < numCiudades; ++j) {
                            if (!visitados[j]) {
                                mejorCandidatoGlobal = j;
                                break;
                            }
                        }
                    }
                    siguienteSeleccionado = mejorCandidatoGlobal;
                }

                actual = siguienteSeleccionado;
                ruta.push_back(actual);
                visitados[actual] = true;
            }

            rutasHormigas[h] = ruta;

            double costoRuta = 0.0;
            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];
                double distDirecta = 0;
                bool encontrada = false;
                for(int j = 0; j < k; ++j) {
                    if(vecinosKNN[desde][j] == hasta) {
                        distDirecta = distKNN[desde][j];
                        encontrada = true;
                        break;
                    }
                }
                if(!encontrada) distDirecta = calcularDistancia(ciudades[desde], ciudades[hasta]);
                costoRuta += distDirecta;
            }
            costosHormigas[h] = costoRuta;

            if (costoRuta < mejorCostoGlobal) {
                mejorCostoGlobal = costoRuta;
                mejorRutaGlobal = ruta;
            }
        }

        // Evaporación de feromonas
        for (int i = 0; i < numCiudades; ++i) {
            for (int j = 0; j < k; ++j) {
                feromonasKNN[i][j] *= (1.0 - rho);
            }
        }

        // Depósito de feromonas
        for (int h = 0; h < numHormigas; ++h) {
            double aporte = Q / costosHormigas[h];
            const auto& ruta = rutasHormigas[h];

            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];

                for (int j = 0; j < k; ++j) {
                    if (vecinosKNN[desde][j] == hasta) {
                        feromonasKNN[desde][j] += aporte;
                        break;
                    }
                }
                for (int j = 0; j < k; ++j) {
                    if (vecinosKNN[hasta][j] == desde) {
                        feromonasKNN[hasta][j] += aporte;
                        break;
                    }
                }
            }
        }

        std::cout << "Iteracion " << iter + 1 << " completada. Mejor costo actual: " << mejorCostoGlobal << "\n";
    }

    auto tiempoFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duracionTotal = tiempoFin - tiempoInicio;

    std::cout << "\n==================================================\n";
    std::cout << "METRICAS DE DESEMPENO (200,000 Ciudades - k-NN Optimizado)\n";
    std::cout << "==================================================\n";
    std::cout << "Mejor costo global encontrado: " << mejorCostoGlobal << "\n";
    std::cout << "Tiempo total de ejecucion: " << duracionTotal.count() << " ms ("
              << duracionTotal.count() / 1000.0 << " segundos)\n";
    std::cout << "==================================================\n";

    return 0;
}
