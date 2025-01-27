# Build Instructions
To build and run the server you need the nix package manager with flakes enabled.
```sh
nix run .#server -- $WEBROOT
```
Web root is the directory of files that will be served from.

# TODO
- [ ] Create way to define routes programtically.
- [ ] Add support to query other web servers like a proxy.
- [ ] Implement support for responding to request headers.
- [ ] Add support for decoding paths.
- [ ] Look into a way to define routes nix flakes.
