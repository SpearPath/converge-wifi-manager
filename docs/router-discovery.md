# ZTE F670L Router Discovery Notes

The project should not assume undocumented endpoints.

Before implementing login, connected-device listing, block, or unblock:

1. Log in to the router web UI through a browser.
2. Open developer tools and record the relevant network requests.
3. Capture request method, URL path, headers, cookies, form data, and response shape.
4. Remove passwords, session tokens, public IPs, and other private data before committing notes.
5. Implement the observed behavior inside `ZteF670LRouterClient`.

The CLI and service layers should not change when router behavior is added.
