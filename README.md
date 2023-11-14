![Screenshot of a gameplay.](/screenshot.png)

# Build
For MacOS:

```
make
```
For other platforms modify flags in the Makefile.

# FPS
Cap FPS:

```
make DFLAGS="-D FPS=30"
```

Display FPS:

```
make DFLAGS="-D ENABLE_FPS=1"
```

# Embed Fonts

```
xxd -i iosevka-bold.ttf > iosevka_bold.h
make DFLAGS="-D EMBED_FONTS=1"
```

# Run
```
./pingpong
```

# Iosevka Font
[typeof.net/Iosevka](https://typeof.net/Iosevka/)
[LICENSE](https://github.com/be5invis/Iosevka/blob/main/LICENSE.md)
