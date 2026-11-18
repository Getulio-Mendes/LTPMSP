# LTPMSP — Meta-heurísticas ILS e VND

Implementação em C++ (OpenMP) de variantes de *Iterated Local Search* (ILS) e *Variable Neighborhood Descent* (VND) para o *Long-Term Preventive Maintenance Scheduling Problem* (LTPMSP), baseada no código de referência de Coelho et al. (2025).

## Compilação

```bash
cd Experimentos/Meu
make
```

## Uso

```bash
./manut --input <arquivo_instancia> [opções]
```

### Opções

| Parâmetro | Valores aceitos | Padrão |
| :--- | :--- | :--- |
| `--input <file>` | Caminho da instância (obrigatório) | — |
| `--algorithm <name>` | `LateAcceptanceILS`, `SmartILS`, `SmartLateAcceptanceILS` | `LateAcceptanceILS` |
| `--descida <type>` | `VND_randomico` (`vnd_par`), `vnd_swap_2opt`, `vnd_2opt_swap`, `vnd_aleatorio`, `swap_randomico`, `2opt_classico`, `2opt_randomico`, `2opt_segmento` | `VND_randomico` |
| `--itermax <int>` | Limite de iterações sem melhora na descida (% de $N$) | `5` |
| `--nivel <int>` | Falhas consecutivas antes de incrementar a perturbação | `7` |
| `--ilsmax <int>` | Iterações do ILS sem melhora da melhor solução | `70` |
| `--runs <int>` | Número de execuções independentes | `1` |
| `--seed <int>` | Semente do gerador pseudoaleatório | `1` |

### Exemplo (Configuração calibrada via `irace`)

```bash
./manut --input ./data/LTPMSP_01_00150_00148_00120.txt \
        --algorithm SmartLateAcceptanceILS \
        --descida VND_randomico \
        --itermax 10 --nivel 10 --ilsmax 100 --runs 10
```

