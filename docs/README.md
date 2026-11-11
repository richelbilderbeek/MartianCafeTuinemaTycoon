# Martian Café Tuinema Tycoon

Martian Café Tuinema Tycoon is a bartender game
in which you serve beer to a café full of thirsty Martians.
It was written by Joost van den Bogaart in 2005
and is one of
[Richèl Bilderbeek's games](https://richelbilderbeek.github.io/games).

![The bar](screenshot_bar.png)

![The band](screenshot_band.png)

## Building the game

The game was written in C++Builder using the VCL
(`MCTTycoon.bpr`).
This is the only version of the game:
there is no version that builds with other tools.

To build the game, open `MCTTycoon.bpr` in C++Builder
and build the project.
Put the bitmaps (`*.bmp`) in the same folder as the executable,
as the game loads these at startup.
