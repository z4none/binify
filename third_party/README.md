# Third-party dependencies

This directory is reserved for vendored or submodule dependencies.

Planned dependency:

- `winlamb/`: rodrigocfd/winlamb pinned to commit `049da50724317b5c78eb6398477ae8649bf183bc`.

The initial fetch failed in this environment because GitHub connections were reset. Retry with:

```powershell
git submodule add https://github.com/rodrigocfd/winlamb.git third_party/winlamb
git -C third_party/winlamb checkout 049da50724317b5c78eb6398477ae8649bf183bc
git add .gitmodules third_party/winlamb
```

