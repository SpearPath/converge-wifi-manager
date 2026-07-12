# Roadmap

## v0.5 — CLI Foundation

- ✅ CLI interface for login, device listing, block/unblock
- ✅ ZTE ZXHN F670L (Converge PH) router support
- ✅ Cross-platform: Windows (WinHTTP) and Linux/Termux (libcurl)
- ✅ Unit tests with GoogleTest/GMock
- ✅ CI pipeline (GitHub Actions) — build, test, clang-tidy, sanitizers
- ✅ Doxygen documentation

## v1.0 — Stable CLI Release (Current)

- ✅ Code coverage reporting in CI (lcov + artifact upload)
- ✅ Tagged-release workflow (GitHub Releases with cross-platform binaries)
- ✅ Expanded test suite (27 tests — session management, error recovery, block/unblock)
- ⏭️ Router plugin architecture — deferred (factory + interface pattern already supports extension; no second router to implement yet)

## v2.0 — Desktop GUI

- [ ] Qt-based desktop frontend
- [ ] Real-time device monitoring dashboard
- [ ] Notification system for new devices

## v3.0 — Mobile

- [ ] Android native application
- [ ] Push notifications for device connect/disconnect events
