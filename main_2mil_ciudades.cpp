#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <utility>

struct Ciudad {
    double x, y;
};

// Función para calcular la distancia euclidiana entre dos ciudades
double calcularDistancia(const Ciudad& c1, const Ciudad& c2) {
    return std::sqrt(std::pow(c1.x - c2.x, 2) + std::pow(c1.y - c2.y, 2));
}

int main() {
    int numCiudades = 2000;  // Escalado a 2,000 ciudades
    int k = 30;              // Tamaño del vecindario k-NN
    int numHormigas = 30;    // Cantidad de agentes
    int maxIteraciones = 30; // Iteraciones de optimización

    // Parámetros ACO
    double alfa = 1.0;
    double beta = 3.0;
    double rho = 0.5;
    double Q = 100.0;

    // Iniciar cronómetro global
    auto tiempoInicio = std::chrono::high_resolution_clock::now();

    // 1. Generar coordenadas aleatorias para las 2,000 ciudades
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> distCoord(0.0, 5000.0);

    std::vector<Ciudad> ciudades(numCiudades);
    for (int i = 0; i < numCiudades; ++i) {
        ciudades[i] = {distCoord(rng), distCoord(rng)};
    }

    // 2. Precomputación de Estructuras Dispersas (k-NN)
    // Evita usar matrices densas de N x N y reduce el consumo masivo de memoria RAM
    std::vector<std::vector<int>> vecinosKNN(numCiudades, std::vector<int>(k));
    std::vector<std::vector<double>> distKNN(numCiudades, std::vector<double>(k));
    std::vector<std::vector<double>> feromonasKNN(numCiudades, std::vector<double>(k, 1.0));

    for (int i = 0; i < numCiudades; ++i) {
        std::vector<std::pair<double, int>> todasLasDistancias;
        todasLasDistancias.reserve(numCiudades);

        for (int j = 0; j < numCiudades; ++j) {
            if (i == j) continue;
            double d = calcularDistancia(ciudades[i], ciudades[j]);
            todasLasDistancias.push_back({d, j});
        }

        // Ordenar de menor a mayor distancia para aislar los k vecinos más próximos
        std::sort(todasLasDistancias.begin(), todasLasDistancias.end());

        for (int j = 0; j < k; ++j) {
            vecinosKNN[i][j] = todasLasDistancias[j].second;
            distKNN[i][j] = todasLasDistancias[j].first;
        }
    }

    auto tiempoPrecomputacion = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duracionPre = tiempoPrecomputacion - tiempoInicio;
    std::cout << "Precomputacion k-NN completada en: " << duracionPre.count() << " ms\n";

    std::vector<int> mejorRutaGlobal;
    double mejorCostoGlobal = 1e18;

    // 3. Ciclo principal de iteraciones del ACO
    for (int iter = 0; iter < maxIteraciones; ++iter) {
        std::vector<std::vector<int>> rutasHormigas(numHormigas);
        std::vector<double> costosHormigas(numHormigas, 0.0);

        for (int h = 0; h < numHormigas; ++h) {
            std::vector<bool> visitados(numCiudades, false);
            std::vector<int> ruta;
            ruta.reserve(numCiudades);

            std::uniform_int_distribution<int> distribNode(0, numCiudades - 1);
            int actual = distribNode(rng);
            ruta.push_back(actual);
            visitados[actual] = true;

            // Construcción de la ruta paso a paso
            for (int paso = 1; paso < numCiudades; ++paso) {
                std::vector<double> probabilidades;
                std::vector<int> candidatosValidos;
                probabilidades.reserve(k);
                candidatosValidos.reserve(k);

                double sumaProbabilidad = 0.0;

                // Evaluar exclusivamente dentro del vecindario k-NN (O(k))
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

                // Regla de transición probabilística o aplicación de Respaldo (Fallback)
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
                    // REGLA DE RESPALDO (Fallback): Si todos los k vecinos ya fueron visitados,
                    // se busca globalmente la ciudad no visitada más cercana disponible.
                    double minGlobalDist = 1e18;
                    int mejorCandidatoGlobal = -1;
                    for (int j = 0; j < numCiudades; ++j) {
                        if (!visitados[j] && j != actual) {
                            double d = calcularDistancia(ciudades[actual], ciudades[j]);
                            if (d < minGlobalDist) {
                                minGlobalDist = d;
                                mejorCandidatoGlobal = j;
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

            // Calcular costo total del recorrido de la hormiga
            double costoRuta = 0.0;
            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];
                costoRuta += calcularDistancia(ciudades[desde], ciudades[hasta]);
            }
            costosHormigas[h] = costoRuta;

            if (costoRuta < mejorCostoGlobal) {
                mejorCostoGlobal = costoRuta;
                mejorRutaGlobal = ruta;
            }
        }

        // 4. Evaporación global en la estructura k-NN
        for (int i = 0; i < numCiudades; ++i) {
            for (int j = 0; j < k; ++j) {
                feromonasKNN[i][j] *= (1.0 - rho);
            }
        }

        // 5. Depósito de nuevas feromonas en las aristas recorridas por las hormigas
        for (int h = 0; h < numHormigas; ++h) {
            double aporte = Q / costosHormigas[h];
            const auto& ruta = rutasHormigas[h];

            for (int i = 0; i < numCiudades; ++i) {
                int desde = ruta[i];
                int hasta = ruta[(i + 1) % numCiudades];

                // Actualizar feromona si 'hasta' pertenece al k-NN de 'desde'
                for (int j = 0; j < k; ++j) {
                    if (vecinosKNN[desde][j] == hasta) {
                        feromonasKNN[desde][j] += aporte;
                        break;
                    }
                }
                // Simetría en grafo no dirigido
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

    // Detener cronómetro global
    auto tiempoFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duracionTotal = tiempoFin - tiempoInicio;

    // Resultados y Métricas de Desempeño
    std::cout << "\n========================================\n";
    std::cout << "METRICAS DE DESEMPENO (2,000 Ciudades)\n";
    std::cout << "========================================\n";
    std::cout << "Mejor costo global encontrado: " << mejorCostoGlobal << "\n";
    std::cout << "Tiempo total de ejecucion: " << duracionTotal.count() << " ms ("
              << duracionTotal.count() / 1000.0 << " segundos)\n";
    std::cout << "========================================\n";

    return 0;
}
