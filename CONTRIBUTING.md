# Contributing

Keep changes small and architecture-friendly:

- CLI code should call application/services, not networking directly.
- Router-specific behavior belongs in a router client implementation.
- Do not hardcode credentials.
- Do not implement block/unblock until router behavior is confirmed.
