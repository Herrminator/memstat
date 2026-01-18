> [!IMPORTANT]
> Note: The MSVC version is about to [become obsolete](#future)!

# MemStat

A very tiny memory statistics tool with an included clock.

# History

It was initially written in the late 1990 as a native Windows program, using the IBM Visual C++
Compiler. Unfortunately, I never used a SCM so the real development history is lost.
Some older versions of some files are preserved in the [history](./history) folder.

It's really surprising, how a tiny program like this still needs 500+ lines of code to be implemented
using the native Windows API.

The last Mickey$oft Visual Studio version used was 14.16
([_MSC_VER](https://gist.github.com/seyhajin/77f5143b8be7d2120c3646b61765476d) 1916, Visual Studio 2017).

# Future

I'm trying to port this to the [llvm / clang toolchain](https://github.com/mstorsjo/llvm-mingw) using
[CMake](https://cmake.org/) with VSCod(e|ium) as IDE, using the
[KylinIde Extension Pack for C/C++](https://open-vsx.org/extension/KylinIdeTeam/kylin-cpp-pack) and
[CodeLLDB](https://open-vsx.org/extension/vadimcn/vscode-lldb) as debugger in VSCodium.

The first attempt actually went quite well, after including both `llvm` and `CMake` in the user `PATH`.

The next step will be to publish this to GitHub.

Then a renovation of the UI (High-DPI) would be necessary.

# Shortcomings

I yet have to find a working resource editor that can handle the string tables (and maybe in
the future, manifests) properly.
Also CMake doesn't [handle manifests](https://github.com/ziglang/zig/issues/17406#issuecomment-1749510298)
properly, should we ever need it.
```c++
// memstat.rc
#define MANIFEST_ID 1
// 24 is the value of RT_MANIFEST, `.rc` files don't have a keyword for RT_MANIFEST but you can specify the type directly like this
MANIFEST_ID 24 "memstat.manifest"
```
