# Changelog

## 0.5.0

- Added `WinHttpClient` implementation (Windows-only, zero dependencies) for real HTTP interactions.
- Implemented `ZteF670LRouterClient` protocol based on community research.
  - Supports login (with CSRF token handling).
  - Supports fetching router information and connected devices via HTML parsing.
  - Supports blocking/unblocking devices via WLAN access control endpoint.

## 0.1.0

- Added CMake-based C++23 project scaffold.
- Added clean architecture layers for CLI, application, services, router client, models, and utilities.
- Added ZTE F670L router client placeholder that avoids assuming undocumented endpoints.
