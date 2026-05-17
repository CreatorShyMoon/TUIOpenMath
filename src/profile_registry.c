#include <string.h>

#include "../include/profile_registry.h"

/*
|--------------------------------------------------------------------------
| Встроенные JSON-профили
|--------------------------------------------------------------------------
*/

/* Символьная математика: sympy, Symbolics — символьные вычисления,
   упрощение выражений, аналитическое интегрирование и дифференцирование */
#define PROFILE_SYMBOLIC_JSON \
"{" \
"\"name\":\"symbolic\"," \
"\"description\":\"Среда символьной математики и компьютерной алгебры\"," \
"\"locked\":true," \
"\"python\":[" \
"\"sympy\"," \
"\"matplotlib\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"," \
"\"Symbolics\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"" \
"]" \
"}"

/* Численные методы: numpy, scipy — решение СЛАУ, численное
   интегрирование, оптимизация, обработка сигналов */
#define PROFILE_NUMERICAL_JSON \
"{" \
"\"name\":\"numerical\"," \
"\"description\":\"Среда численных методов и вычислительной математики\"," \
"\"locked\":true," \
"\"python\":[" \
"\"numpy\"," \
"\"scipy\"," \
"\"matplotlib\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"" \
"]" \
"}"

/* Научные вычисления: полный стек для исследовательской работы —
   символьные, численные, статистические инструменты + визуализация */
#define PROFILE_SCIENTIFIC_JSON \
"{" \
"\"name\":\"scientific\"," \
"\"description\":\"Комплексная платформа научных вычислений\"," \
"\"locked\":true," \
"\"python\":[" \
"\"numpy\"," \
"\"scipy\"," \
"\"sympy\"," \
"\"matplotlib\"," \
"\"pandas\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"," \
"\"Symbolics\"," \
"\"Plots\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"," \
"\"octave\"" \
"]" \
"}"

/* Машинное обучение: scikit-learn, pandas, seaborn — классические
   алгоритмы ML, предобработка данных, визуализация результатов */
#define PROFILE_ML_JSON \
"{" \
"\"name\":\"ml\"," \
"\"description\":\"Среда машинного обучения и анализа данных\"," \
"\"locked\":true," \
"\"python\":[" \
"\"numpy\"," \
"\"pandas\"," \
"\"scikit-learn\"," \
"\"matplotlib\"," \
"\"seaborn\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"" \
"]" \
"}"

/* Дифференциальные уравнения: scipy + Julia DifferentialEquations —
   ОДУ, ДЧП, жёсткие системы, метод Рунге-Кутты */
#define PROFILE_DIFFEQ_JSON \
"{" \
"\"name\":\"diffeq\"," \
"\"description\":\"Среда для решения дифференциальных уравнений\"," \
"\"locked\":true," \
"\"python\":[" \
"\"numpy\"," \
"\"scipy\"," \
"\"sympy\"," \
"\"matplotlib\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"," \
"\"DifferentialEquations\"," \
"\"Plots\"," \
"\"Symbolics\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"" \
"]" \
"}"

/* Статистика: statsmodels, seaborn, scipy.stats — описательная
   статистика, проверка гипотез, регрессия, временные ряды */
#define PROFILE_STATS_JSON \
"{" \
"\"name\":\"stats\"," \
"\"description\":\"Среда статистического анализа и обработки данных\"," \
"\"locked\":true," \
"\"python\":[" \
"\"numpy\"," \
"\"pandas\"," \
"\"scipy\"," \
"\"statsmodels\"," \
"\"seaborn\"," \
"\"matplotlib\"," \
"\"jupyterlab\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"bash\"" \
"]" \
"}"

/* Базовый профиль: минимальный набор для знакомства с платформой —
   только Jupyter и Python без тяжёлых зависимостей */
#define PROFILE_BASE_JSON \
"{" \
"\"name\":\"base\"," \
"\"description\":\"Базовый профиль для начала работы с платформой\"," \
"\"locked\":true," \
"\"python\":[" \
"\"jupyterlab\"," \
"\"matplotlib\"," \
"\"numpy\"" \
"]," \
"\"julia\":[" \
"\"IJulia\"" \
"]," \
"\"system\":[]," \
"\"kernels\":[" \
"\"python\"," \
"\"julia\"," \
"\"bash\"" \
"]" \
"}"

/*
|--------------------------------------------------------------------------
| Реестр встроенных профилей
|--------------------------------------------------------------------------
*/

const BuiltinProfile BUILTIN_PROFILES[] =
{
    {
        .name        = "base",
        .description = "Базовый профиль для начала работы с платформой",
        .json        = PROFILE_BASE_JSON,
        .locked      = 1
    },

    {
        .name        = "symbolic",
        .description = "Среда символьной математики и компьютерной алгебры",
        .json        = PROFILE_SYMBOLIC_JSON,
        .locked      = 1
    },

    {
        .name        = "numerical",
        .description = "Среда численных методов и вычислительной математики",
        .json        = PROFILE_NUMERICAL_JSON,
        .locked      = 1
    },

    {
        .name        = "scientific",
        .description = "Комплексная платформа научных вычислений",
        .json        = PROFILE_SCIENTIFIC_JSON,
        .locked      = 1
    },

    {
        .name        = "diffeq",
        .description = "Среда для решения дифференциальных уравнений",
        .json        = PROFILE_DIFFEQ_JSON,
        .locked      = 1
    },

    {
        .name        = "ml",
        .description = "Среда машинного обучения и анализа данных",
        .json        = PROFILE_ML_JSON,
        .locked      = 1
    },

    {
        .name        = "stats",
        .description = "Среда статистического анализа и обработки данных",
        .json        = PROFILE_STATS_JSON,
        .locked      = 1
    }
};

const size_t BUILTIN_PROFILES_COUNT =
    sizeof(BUILTIN_PROFILES) / sizeof(BUILTIN_PROFILES[0]);

/*
|--------------------------------------------------------------------------
| Поиск профиля по имени
|--------------------------------------------------------------------------
*/

const BuiltinProfile *find_builtin_profile(const char *name)
{
    if (!name)
    {
        return NULL;
    }

    for (size_t i = 0; i < BUILTIN_PROFILES_COUNT; i++)
    {
        if (strcmp(BUILTIN_PROFILES[i].name, name) == 0)
        {
            return &BUILTIN_PROFILES[i];
        }
    }

    return NULL;
}