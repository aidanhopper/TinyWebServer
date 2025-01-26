# Build Instructions
To build and run the server you need the nix package manager with flakes enabled.
```sh
nix run .#server -- $WEBROOT
```
Web root is the directory of files that will be served from.
