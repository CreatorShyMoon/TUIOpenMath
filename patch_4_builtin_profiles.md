# Патч: профили перенесены в бинарник

Профили перестали читаться из `profiles/system/*.json` и теперь зашиты в `src/profile_registry.c`.

## Что сломалось и где починили

**`src/profiles.c`** — `load_profiles` переписан: вместо чтения файлов с диска итерируется по `BUILTIN_PROFILES[]`. Все вспомогательные функции для работы с файлами удалены.

**`src/actions.c`, `src/state_manager.c`, `src/setup_manager.c`** — в каждом файле одна строка:
```c
// Было:
profile_config_load(profile->path, &config)  // path = "(builtin)"  не находило

// Стало:
profile_config_load(profile->name, &config)  // name = "symbolic"  находит
```
