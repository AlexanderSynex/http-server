# C++ Linux Html-сервер

Простой http-сервер для Linux на базе linux socket'ов.

## Сборка

### As it is

Требования:
- CMake 3.16+
- g++/clang++ (поддерживающие C++20)

```bash
(mkdir -p build && cd build && cmake -S .. -B . && cmake --build .)
```

### Docker

```bash
docker build . -t synex/http-server
```

## Запуск

### As it is

```bash
  ./server [host] [port]
```

- `host` - адрес сервера (по умолчанию `127.0.0.1`)
- `port` - порт сервера (по умолчанию `8080`)

### Docker

Вместо `1111` указывается любой другой порт.

```bash
docker run --rm -it -p 1111:8080 synex/http-server
```


