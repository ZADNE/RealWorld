# RealWorld Naming Convention

RealWorld adheres to [Real Naming Convention](https://github.com/ZADNE/Real/blob/main/doc/NamingConvetion.md) which codifies naming of types, variables, files and many more.

Naming conventions specific to RealWorld follow.

## Units of distance

Variables, both in C++ and GLSL, representing measures of length use a suffix denoting the unit of the measurement: 

* `Px` stands for measurement in pixels of the main framebuffer in the default zoom
* `Ti` stands for measurement in tiles of the world (1 Ti = 4 Px)
* `Ch` stands for measurement in chunks of the world (1 Ch = 128 Ti)
* `At` stands for a position in tiles that is active, i.e. modulo-clamped to the size of the world texture
* `Ac` stands for a position in chunks that is active

An example snippet which converts a position from chunks to tiles is below:

```
glm::ivec2 posTi = chToTi(posCh);
```
