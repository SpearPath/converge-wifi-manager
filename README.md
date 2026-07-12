# Converge WiFi Manager

A cross-platform C++23 CLI for managing an owned or authorized Converge ZTE F670L router.

This repository is currently at the v0.1 scaffold stage: the project builds, the CLI menu exists, and the clean architecture boundaries are in place. Router HTTP endpoints are intentionally not guessed. Login, device listing, block, and unblock will be implemented after the ZTE F670L request/response behavior is confirmed.

## Current Features

- CMake C++23 project setup
- CLI menu matching the project spec
- Application, service, router-client, model, and utility layers
- Config loading from `config/config.json`
- Basic console logger
- Real WinHTTP-based HTTP client (Windows, no dependencies)
- ZTE F670L router adapter with protocol implementation based on community research
- No hardcoded passwords

## Architecture

```text
CLI
 |
 v
Application
 |
 v
Services
 |
 v
RouterClient
 |
 v
HttpClient (planned)
 |
 v
Router
```

The CLI must not call networking directly. Router behavior belongs behind `IRouterClient`, with one implementation per supported router family.

## Build

Windows:

```powershell
cmake -B build
cmake --build build
```

Linux:

```bash
cmake -B build
cmake --build build
```

Termux:

```bash
pkg update
pkg install clang cmake git make
cmake -B build
cmake --build build
```

## Run

Windows:

```powershell
.\build\Debug\converge-wifi-manager.exe
```

MinGW single-config builds may output here instead:

```powershell
.\build\converge-wifi-manager.exe
```

Linux or Termux:

```bash
./build/converge-wifi-manager
```

## Configuration

Edit `config/config.json`:

```json
{
  "router_ip": "192.168.1.1",
  "username": "admin",
  "refresh_interval": 30,
  "auto_login": false
}
```

Passwords are requested interactively and are not stored by default.

## Roadmap

- v0.1: Project setup and CLI scaffold
- v0.5: Real HTTP transport and ZTE F670L interaction logic (login, list, block)
- v1.0: Polish, tests, and stable CLI

## Security

Use this only on routers you own or are authorized to administer. Do not add block/unblock or login endpoints by guessing undocumented behavior; capture and confirm router requests first.
