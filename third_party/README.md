# Third-party dependencies

This directory is reserved for vendored or submodule dependencies.

Pinned dependency:

- `winlamb/`: rodrigocfd/winlamb pinned to commit `049da50724317b5c78eb6398477ae8649bf183bc`.

If the submodule is missing in a fresh checkout, initialize it with:

```powershell
git submodule update --init --recursive
```

The pinned commit currently uses root-level headers such as `wnd.h`, not a single `winlamb.hpp` umbrella header.
