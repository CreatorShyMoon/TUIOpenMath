#include "workspace_manager.h"
#include "logger.h"
#include "profiles.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int ensure_dir_exists(const char *path)
{
    struct stat st;

    if (!path || path[0] == '\0')
    {
        return 0;
    }

    if (stat(path, &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }

    return mkdir(path, 0755) == 0;
}

static int write_text_file(const char *path, const char *text)
{
    if (!path || !text)
    {
        return 0;
    }

    FILE *f = fopen(path, "wb");
    if (!f)
    {
        return 0;
    }

    if (fputs(text, f) == EOF)
    {
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

static int is_profile(const Profile *profile, const char *name)
{
    return profile && name && strcmp(profile->name, name) == 0;
}

int workspace_create(AppState *app, const Profile *profile)
{
    if (!app || !profile)
    {
        return 0;
    }

    if (!profile_name_is_safe(profile->name))
    {
        log_error(app, "Некорректное имя профиля для workspace: %s", profile->name);
        return 0;
    }

    if (!ensure_dir_exists("workspaces"))
    {
        log_error(app, "Не удалось создать каталог workspaces.");
        return 0;
    }

    char ws_dir[512];
    char notebook_path[512];
    char readme_path[512];

    if (snprintf(ws_dir, sizeof(ws_dir), "workspaces/%s", profile->name) >= (int)sizeof(ws_dir) ||
        snprintf(notebook_path, sizeof(notebook_path), "%s/%s_starter.ipynb", ws_dir, profile->name) >= (int)sizeof(notebook_path) ||
        snprintf(readme_path, sizeof(readme_path), "%s/README.md", ws_dir) >= (int)sizeof(readme_path))
    {
        log_error(app, "Путь для workspace слишком длинный.");
        return 0;
    }

    if (!ensure_dir_exists(ws_dir))
    {
        log_error(app, "Не удалось создать каталог %s: %s", ws_dir, strerror(errno));
        return 0;
    }

    const char *notebook_json = NULL;
    const char *readme_extra = NULL;

    if (is_profile(profile, "math"))
    {
        notebook_json =
            "{\n"
            "  \"cells\": [\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\n"
            "        \"# SysOpenMath: Math Workspace\\n\",\n"
            "        \"\\n\",\n"
            "        \"Стартовый notebook для математического профиля.\\n\",\n"
            "        \"\\n\",\n"
            "        \"В этом шаблоне уже есть примеры для:\\n\",\n"
            "        \"- численных вычислений\\n\",\n"
            "        \"- символьной математики\\n\",\n"
            "        \"- построения графиков\\n\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"import sys\\n\",\n"
            "        \"import numpy as np\\n\",\n"
            "        \"import scipy\\n\",\n"
            "        \"import sympy as sp\\n\",\n"
            "        \"import matplotlib.pyplot as plt\\n\",\n"
            "        \"\\n\",\n"
            "        \"print(\\\"Python executable:\\\", sys.executable)\\n\",\n"
            "        \"print(\\\"NumPy version:\\\", np.__version__)\\n\",\n"
            "        \"print(\\\"SciPy version:\\\", scipy.__version__)\\n\",\n"
            "        \"print(\\\"SymPy version:\\\", sp.__version__)\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Численные вычисления\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"x = np.linspace(0, 2*np.pi, 200)\\n\",\n"
            "        \"y1 = np.sin(x)\\n\",\n"
            "        \"y2 = np.cos(x)\\n\",\n"
            "        \"print(\\\"sin mean =\\\", y1.mean())\\n\",\n"
            "        \"print(\\\"cos mean =\\\", y2.mean())\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Символьная математика\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"x = sp.symbols(\\\"x\\\")\\n\",\n"
            "        \"expr = sp.sin(x)**2 + sp.cos(x)**2\\n\",\n"
            "        \"print(\\\"Expression:\\\", expr)\\n\",\n"
            "        \"print(\\\"Simplified:\\\", sp.simplify(expr))\\n\",\n"
            "        \"print(\\\"Integral of sin(x):\\\", sp.integrate(sp.sin(x), x))\\n\",\n"
            "        \"print(\\\"Derivative of exp(x)*sin(x):\\\", sp.diff(sp.exp(x) * sp.sin(x), x))\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Построение графика\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"plt.figure(figsize=(10, 5))\\n\",\n"
            "        \"plt.plot(x, y1, label=\\\"sin(x)\\\")\\n\",\n"
            "        \"plt.plot(x, y2, label=\\\"cos(x)\\\")\\n\",\n"
            "        \"plt.title(\\\"Basic trigonometric functions\\\")\\n\",\n"
            "        \"plt.xlabel(\\\"x\\\")\\n\",\n"
            "        \"plt.ylabel(\\\"y\\\")\\n\",\n"
            "        \"plt.grid(True)\\n\",\n"
            "        \"plt.legend()\\n\",\n"
            "        \"plt.show()\"\n"
            "      ]\n"
            "    }\n"
            "  ],\n"
            "  \"metadata\": {\n"
            "    \"kernelspec\": {\n"
            "      \"display_name\": \"SysOpenMath Python (math)\",\n"
            "      \"language\": \"python\",\n"
            "      \"name\": \"sysopenmath-math\"\n"
            "    },\n"
            "    \"language_info\": {\n"
            "      \"name\": \"python\"\n"
            "    }\n"
            "  },\n"
            "  \"nbformat\": 4,\n"
            "  \"nbformat_minor\": 5\n"
            "}\n";

        readme_extra =
            "## Профиль Math\n\n"
            "Этот профиль ориентирован на:\n"
            "- `numpy`\n"
            "- `scipy`\n"
            "- `sympy`\n"
            "- `matplotlib`\n"
            "- Julia-пакеты `IJulia`, `Plots`, `Symbolics`\n"
            "- системный пакет `octave`\n\n"
            "### Идеи для работы\n"
            "- символьные преобразования\n"
            "- численные методы\n"
            "- сравнение Python и Julia-подходов\n"
            "- построение графиков и быстрые эксперименты\n";
    }
    else if (is_profile(profile, "ml"))
    {
        notebook_json =
            "{\n"
            "  \"cells\": [\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\n"
            "        \"# SysOpenMath: ML Workspace\\n\",\n"
            "        \"\\n\",\n"
            "        \"Стартовый notebook для профиля машинного обучения.\\n\",\n"
            "        \"\\n\",\n"
            "        \"В этом шаблоне есть:\\n\",\n"
            "        \"- загрузка данных в pandas\\n\",\n"
            "        \"- базовая статистика\\n\",\n"
            "        \"- простая модель scikit-learn\\n\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"import sys\\n\",\n"
            "        \"import numpy as np\\n\",\n"
            "        \"import pandas as pd\\n\",\n"
            "        \"from sklearn.datasets import load_iris\\n\",\n"
            "        \"from sklearn.model_selection import train_test_split\\n\",\n"
            "        \"from sklearn.ensemble import RandomForestClassifier\\n\",\n"
            "        \"from sklearn.metrics import accuracy_score, classification_report\\n\",\n"
            "        \"\\n\",\n"
            "        \"print(\\\"Python executable:\\\", sys.executable)\\n\",\n"
            "        \"print(\\\"NumPy version:\\\", np.__version__)\\n\",\n"
            "        \"print(\\\"Pandas version:\\\", pd.__version__)\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Загрузка датасета\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"iris = load_iris(as_frame=True)\\n\",\n"
            "        \"df = iris.frame.copy()\\n\",\n"
            "        \"df.head()\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Базовая аналитика\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"print(df.describe())\\n\",\n"
            "        \"print()\\n\",\n"
            "        \"print(\\\"Class distribution:\\\")\\n\",\n"
            "        \"print(df[\\\"target\\\"].value_counts())\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Простая модель\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"X = df.drop(columns=[\\\"target\\\"])\\n\",\n"
            "        \"y = df[\\\"target\\\"]\\n\",\n"
            "        \"X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42, stratify=y)\\n\",\n"
            "        \"model = RandomForestClassifier(random_state=42)\\n\",\n"
            "        \"model.fit(X_train, y_train)\\n\",\n"
            "        \"pred = model.predict(X_test)\\n\",\n"
            "        \"print(\\\"Accuracy:\\\", accuracy_score(y_test, pred))\\n\",\n"
            "        \"print(classification_report(y_test, pred))\"\n"
            "      ]\n"
            "    }\n"
            "  ],\n"
            "  \"metadata\": {\n"
            "    \"kernelspec\": {\n"
            "      \"display_name\": \"SysOpenMath Python (ml)\",\n"
            "      \"language\": \"python\",\n"
            "      \"name\": \"sysopenmath-ml\"\n"
            "    },\n"
            "    \"language_info\": {\n"
            "      \"name\": \"python\"\n"
            "    }\n"
            "  },\n"
            "  \"nbformat\": 4,\n"
            "  \"nbformat_minor\": 5\n"
            "}\n";

        readme_extra =
            "## Профиль ML\n\n"
            "Этот профиль ориентирован на:\n"
            "- `numpy`\n"
            "- `pandas`\n"
            "- `scikit-learn`\n"
            "- `jupyterlab`\n"
            "- базовые ML-эксперименты\n\n"
            "### Идеи для работы\n"
            "- табличные данные\n"
            "- быстрые baseline-модели\n"
            "- подготовка данных\n"
            "- сравнение экспериментов в notebook\n";
    }
    else
    {
        notebook_json =
            "{\n"
            "  \"cells\": [\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\n"
            "        \"# SysOpenMath: Base Workspace\\n\",\n"
            "        \"\\n\",\n"
            "        \"Базовый стартовый notebook для научной работы.\\n\",\n"
            "        \"\\n\",\n"
            "        \"Шаблон содержит:\\n\",\n"
            "        \"- проверку Python-окружения\\n\",\n"
            "        \"- базовые вычисления NumPy\\n\",\n"
            "        \"- простую визуализацию\\n\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"import sys\\n\",\n"
            "        \"import numpy as np\\n\",\n"
            "        \"import scipy\\n\",\n"
            "        \"import matplotlib.pyplot as plt\\n\",\n"
            "        \"\\n\",\n"
            "        \"print(\\\"Python executable:\\\", sys.executable)\\n\",\n"
            "        \"print(\\\"NumPy version:\\\", np.__version__)\\n\",\n"
            "        \"print(\\\"SciPy version:\\\", scipy.__version__)\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Базовый массив\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"x = np.arange(1, 11)\\n\",\n"
            "        \"print(\\\"x =\\\", x)\\n\",\n"
            "        \"print(\\\"sum =\\\", x.sum())\\n\",\n"
            "        \"print(\\\"mean =\\\", x.mean())\\n\",\n"
            "        \"print(\\\"std =\\\", x.std())\"\n"
            "      ]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"markdown\",\n"
            "      \"metadata\": {},\n"
            "      \"source\": [\"## Быстрый график\"]\n"
            "    },\n"
            "    {\n"
            "      \"cell_type\": \"code\",\n"
            "      \"execution_count\": null,\n"
            "      \"metadata\": {},\n"
            "      \"outputs\": [],\n"
            "      \"source\": [\n"
            "        \"x = np.linspace(0, 10, 100)\\n\",\n"
            "        \"y = np.sin(x)\\n\",\n"
            "        \"plt.figure(figsize=(10, 4))\\n\",\n"
            "        \"plt.plot(x, y)\\n\",\n"
            "        \"plt.title(\\\"sin(x)\\\")\\n\",\n"
            "        \"plt.grid(True)\\n\",\n"
            "        \"plt.show()\"\n"
            "      ]\n"
            "    }\n"
            "  ],\n"
            "  \"metadata\": {\n"
            "    \"kernelspec\": {\n"
            "      \"display_name\": \"SysOpenMath Python (base)\",\n"
            "      \"language\": \"python\",\n"
            "      \"name\": \"sysopenmath-base\"\n"
            "    },\n"
            "    \"language_info\": {\n"
            "      \"name\": \"python\"\n"
            "    }\n"
            "  },\n"
            "  \"nbformat\": 4,\n"
            "  \"nbformat_minor\": 5\n"
            "}\n";

        readme_extra =
            "## Профиль Base\n\n"
            "Этот профиль ориентирован на:\n"
            "- `numpy`\n"
            "- `scipy`\n"
            "- `matplotlib`\n"
            "- `jupyterlab`\n"
            "- базовые вычисления и визуализацию\n\n"
            "### Идеи для работы\n"
            "- быстрые численные эксперименты\n"
            "- графики и визуализация\n"
            "- подготовка к более узким профилям\n";
    }

    char readme[16384];
    int rw = snprintf(
        readme,
        sizeof(readme),
        "# SysOpenMath Workspace: %s\n\n"
        "Этот workspace создан автоматически.\n\n"
        "## Что внутри\n"
        "- стартовый notebook: `%s_starter.ipynb`\n"
        "- этот README\n\n"
        "## Рекомендуемый порядок\n"
        "1. Убедись, что профиль `%s` установлен.\n"
        "2. Зарегистрируй Python kernel для `%s`.\n"
        "3. При необходимости зарегистрируй Julia kernel.\n"
        "4. Запусти JupyterLab.\n\n"
        "%s\n"
        "## Проверка\n"
        "- Показать kernels Jupyter\n"
        "- Проверить компоненты\n"
        "- Показать состояние SysOpenMath\n",
        profile->name,
        profile->name,
        profile->name,
        profile->name,
        readme_extra
    );

    if (rw < 0 || (size_t)rw >= sizeof(readme))
    {
        log_error(app, "README для workspace слишком большой.");
        return 0;
    }

    if (!write_text_file(readme_path, readme))
    {
        log_error(app, "Не удалось записать %s: %s", readme_path, strerror(errno));
        return 0;
    }

    if (!write_text_file(notebook_path, notebook_json))
    {
        log_error(app, "Не удалось записать %s: %s", notebook_path, strerror(errno));
        return 0;
    }

    log_info(app, "Workspace created: %s", ws_dir);
    log_info(app, "Notebook: %s", notebook_path);
    log_info(app, "README: %s", readme_path);

    return 1;
}