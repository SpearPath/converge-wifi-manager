# Security Policy

## Scope

Converge WiFi CLI Device Manager is a **local network administration tool**. It communicates only with a router on your LAN — it makes no internet-facing connections and exposes no server ports.

## Supported Versions

| Version | Supported |
|---------|-----------|
| 0.5.x   | ✅        |
| < 0.5   | ❌        |

## Reporting a Vulnerability

If you discover a security issue, please report it privately:

1. **Email:** Open an issue on the repository marked `[SECURITY]`, or contact the maintainer directly.
2. **Do not** open a public issue with exploit details.
3. Expect an acknowledgment within 72 hours.

## Security Considerations

- **No credentials stored on disk.** The router password is entered interactively each session.
- **`config.json`** contains only non-secret settings (IP, username, refresh interval).
- **HTTP only.** Communication with the router uses plain HTTP because the ZTE F670L's admin interface does not support HTTPS. This is expected for LAN-only operation.
- **Password hashing.** The router's login flow hashes the password client-side with SHA-256 + a random nonce before transmission (matching the router's JavaScript-based login protocol).
