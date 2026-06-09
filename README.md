# SysOpenMath / TUIOpenMath

> Терминальный менеджер открытой математической среды для Linux: профили, JupyterLab, Julia, Python-окружения и вычислительные kernels в одном `ncurses`-интерфейсе.

![C11](https://img.shields.io/badge/C-C11-blue)
![POSIX](https://img.shields.io/badge/POSIX-compatible-green)
![ncurses](https://img.shields.io/badge/UI-ncurses-green)
![Linux](https://img.shields.io/badge/platform-Linux-lightgrey)
![Jupyter](https://img.shields.io/badge/JupyterLab-supported-orange)
![Status](https://img.shields.io/badge/status-educational%20project-yellow)
---

## О проекте

**SysOpenMath / TUIOpenMath** — это терминальное приложение на языке C для подготовки и управления математической вычислительной средой в Linux.

Проект развивает идею системы открытой математики: вместо ручной установки Python-пакетов, Julia, JupyterLab и вычислительных ядер пользователь выбирает готовый профиль, запускает нужное действие из TUI-интерфейса и видит ход выполнения в log-панели.

Приложение ориентировано на учебные, лабораторные и серверные Linux-среды, где важно работать без графического интерфейса, в том числе через SSH.

---

## Возможности

- интерактивный TUI-интерфейс на базе `ncurses`;
- встроенные профили вычислительной среды;
- создание изолированных Python-окружений в `envs/<profile>/`;
- установка Julia LTS / Stable;
- установка JupyterLab в отдельный runtime;
- регистрация Jupyter kernels для Python, Julia, Bash, Octave и Scilab;
- запуск, остановка и перезапуск JupyterLab;
- вывод URL JupyterLab в log-панель;
- профильно-зависимая диагностика;
- асинхронное выполнение длительных команд через `fork` / `pipe` / `waitpid`;
- управление workspace для выбранного профиля;
- очистка state-файлов, runtime и kernelspec-директорий.

---

## Как это выглядит логически

```text
Пользователь
   │
   ▼
Выбирает профиль
   │
   ▼
SysOpenMath создаёт окружение
   │
   ├── Python env: envs/<profile>/
   ├── Julia packages
   ├── Jupyter runtime: tools/jupyter/runtime/
   └── Jupyter kernels
   │
   ▼
Пользователь запускает JupyterLab и работает в браузере
```

---

## Встроенные профили

| Профиль | Назначение | Python-пакеты | Julia-пакеты | Kernels |
|---|---|---|---|---|
| `base` | базовое знакомство с платформой | `numpy`, `matplotlib` | `IJulia` | Python, Julia, Bash |
| `symbolic` | символьная математика | `sympy`, `matplotlib` | `IJulia`, `Symbolics` | Python, Julia, Bash |
| `numerical` | численные методы | `numpy`, `scipy`, `matplotlib` | `IJulia` | Python, Julia, Bash |
| `scientific` | научные вычисления | `numpy`, `scipy`, `sympy`, `pandas`, `matplotlib` | `IJulia`, `Symbolics`, `Plots` | Python, Julia, Bash, Octave |
| `diffeq` | дифференциальные уравнения | `numpy`, `scipy`, `sympy`, `matplotlib` | `IJulia`, `DifferentialEquations`, `Plots`, `Symbolics` | Python, Julia, Bash |
| `ml` | машинное обучение | `numpy`, `pandas`, `scikit-learn`, `matplotlib`, `seaborn` | `IJulia` | Python, Julia, Bash |
| `stats` | статистический анализ | `numpy`, `pandas`, `scipy`, `statsmodels`, `seaborn`, `matplotlib` | `IJulia` | Python, Bash |

> JupyterLab рассматривается как инфраструктурный компонент: он устанавливается в служебное окружение `tools/jupyter/runtime/` и используется всеми профилями для запуска вычислительных ядер.

---

## Архитектура

```text
.
├── manager.c                  # точка входа
├── Makefile                   # сборка проекта
├── include/                   # заголовочные файлы
├── src/
│   ├── ui.c                   # ncurses-интерфейс
│   ├── actions.c              # диспетчер действий
│   ├── process.c              # асинхронный запуск команд
│   ├── logger.c               # log-панель и файл логов
│   ├── profiles.c             # загрузка профилей
│   ├── profile_registry.c     # встроенные JSON-профили
│   ├── profile_config.c       # разбор конфигурации профиля
│   ├── setup_manager.c        # полная подготовка профиля
│   ├── jupyter_manager.c      # установка и запуск JupyterLab
│   ├── julia_manager.c        # установка Julia
│   ├── kernel_manager.c       # регистрация kernelspec
│   ├── checks.c               # диагностика среды
│   ├── cleanup_manager.c      # очистка компонентов
│   ├── state_manager.c        # state-файлы и PID
│   ├── workspace_manager.c    # workspace профилей
│   ├── octave_manager.c       # Octave
│   └── scilab_manager.c       # Scilab
├── envs/                      # Python-окружения профилей
├── tools/                     # Julia, Jupyter runtime и bin-ссылки
├── state/                     # версии, PID, служебное состояние
├── logs/                      # логи приложения
└── workspaces/                # рабочие каталоги профилей
```

---

## Зависимости

Минимально нужны:

- Linux;
- GCC или Clang;
- `make`;
- `ncursesw`;
- `cJSON`;
- `python3`, `venv`, `pip`;
- `curl` или `wget` для загрузки внешних компонентов.

Для Ubuntu / Debian:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  make \
  gcc \
  libncursesw5-dev \
  libcjson-dev \
  python3 \
  python3-venv \
  python3-pip \
  curl \
  wget
```

Дополнительные компоненты для отдельных kernels:

```bash
sudo apt install -y octave scilab
```

---

## Сборка

Для корректной работы
```bash
sudo -v
```
Сборка через GCC:

```bash
make clean
make
```

Сборка через Clang:

```bash
make clean
make CC=clang
```

После успешной сборки появится исполняемый файл:

```text
./manager
```

---

## Запуск

```bash
./manager
```

Рекомендуемый размер терминала: **140×40** или больше.

---

## Управление

| Клавиша | Действие |
|---|---|
| `Tab` | переключение между панелями |
| `↑` / `↓` | перемещение по спискам |
| `Enter` | выполнить выбранное действие |
| `←` / `Backspace` | вернуться на уровень выше |
| `PgUp` / `PgDn` | прокрутка log-панели |
| `q` | выход |

---

## Типовой сценарий работы

1. Запустить приложение:

   ```bash
   ./manager
   ```

2. Выбрать профиль, например `symbolic`.
3. Перейти в категорию **Быстрый старт**.
4. Выполнить действие **Подготовить выбранный профиль полностью**.
5. Дождаться завершения установки в log-панели.
6. Выполнить диагностику.
7. Запустить JupyterLab.
8. Открыть выведенный URL в браузере.

---

## JupyterLab

JupyterLab устанавливается в отдельное служебное окружение:

```text
tools/jupyter/runtime/
```

Исполняемые файлы подключаются через символические ссылки:

```text
tools/bin/jupyter
tools/bin/jupyter-lab
```

Запуск выполняется в фоне. PID сохраняется в:

```text
state/jupyter.pid
```

Лог JupyterLab пишется в:

```text
/tmp/sysopenmath_jupyterlab.log
```

Полезные команды для проверки:

```bash
tools/bin/jupyter --version
tools/bin/jupyter kernelspec list
ps -p "$(cat state/jupyter.pid)"
```

---

## Julia

В проекте предусмотрены два канала установки Julia:

| Канал | Версия |
|---|---|
| Julia LTS | `1.10.10` |
| Julia Stable | `1.11.7` |

Проверка установленной Julia:

```bash
tools/bin/julia --version
```

---

## Диагностика

SysOpenMath проверяет не всю систему сразу, а компоненты активного профиля. Это позволяет избежать ложных ошибок по компонентам, которые профилю не нужны.

Пример диагностического вывода:

```text
[CHECK] profile: symbolic
[OK] python executable found: envs/symbolic/bin/python
[OK] python import: sympy
[OK] python import: matplotlib
[OK] julia executable found: tools/bin/julia
[OK] julia package: IJulia
[OK] julia package: Symbolics
[OK] kernelspec: sysopenmath-symbolic
[OK] kernelspec: sysopenmath-symbolic-julia
[OK] kernelspec: sysopenmath-symbolic-bash
[SKIP] octave kernel is not required for profile symbolic
[SKIP] scilab kernel is not required for profile symbolic
```

---

## Профили

Встроенные профили находятся в файле:

```text
src/profile_registry.c
```

Они описаны как JSON-строки и попадают в исполняемый файл на этапе компиляции. Чтобы изменить встроенный профиль, нужно отредактировать соответствующую JSON-константу и пересобрать проект.

Пример описания профиля:

```json
{
  "name": "symbolic",
  "description": "Среда символьной математики и компьютерной алгебры",
  "locked": true,
  "python": ["sympy", "matplotlib", "jupyterlab"],
  "julia": ["IJulia", "Symbolics"],
  "system": [],
  "kernels": ["python", "julia", "bash"]
}
```

---

## Полезные команды

Проверить kernels:

```bash
tools/bin/jupyter kernelspec list
```

Проверить Python-окружение профиля:

```bash
envs/symbolic/bin/python --version
envs/symbolic/bin/pip list
```

Проверить Julia:

```bash
tools/bin/julia --version
```

Посмотреть состояние JupyterLab:

```bash
cat state/jupyter.pid
ps -p "$(cat state/jupyter.pid)"
```

---

## Очистка

Удалить собранный бинарник:

```bash
make clean
```

Очистка runtime, kernels и state выполняется из интерфейса SysOpenMath через соответствующие действия.

---

## Возможные проблемы

### JupyterLab не запускается

Проверьте, установлен ли Jupyter runtime:

```bash
tools/bin/jupyter --version
tools/bin/jupyter-lab --version
```

### Kernel не отображается в JupyterLab

Проверьте список kernelspec:

```bash
tools/bin/jupyter kernelspec list
```

### Профиль не проходит диагностику

Проверьте, существует ли окружение профиля:

```bash
ls envs/<profile>/bin/python
```

После этого повторите установку профиля или выполните диагностику через интерфейс.

### Octave или Scilab kernel пропущен

Эти компоненты требуют системной установки:

```bash
command -v octave
command -v scilab
```

Если команды отсутствуют, установите соответствующий пакет через менеджер пакетов Linux.

---

## Roadmap

Возможные направления развития:

- пользовательские профили во внешних JSON-файлах;
- проверка версий пакетов профиля;
- восстановление повреждённых окружений;
- более подробная диагностика ошибок установки;
- шаблоны учебных workspace;
- интеграция с JupyterHub;
- экспорт отчёта о состоянии среды.

---

## Назначение

Проект может быть полезен для:

- студентов математических и инженерных направлений;
- преподавателей, готовящих вычислительную среду для занятий;
- учебных лабораторий Linux;
- серверов с доступом по SSH;
- экспериментов с Jupyter, Julia, Python, Octave и Scilab.

---

## Статус проекта

Проект находится в стадии активной разработки и используется как учебно-инженерный прототип терминального менеджера открытой математической среды.

