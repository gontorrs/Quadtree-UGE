# QtreeComp - QuadTree Image Compression

This project was developed as part of a Programmation avancée en C at Université Gustave Eiffel in Paris. The implementation was carried out by Gonzalo Torras and Mario Lepage.

## Project Overview

This project implements a hierarchical image compression scheme based on the QuadTree data structure. The main goal is to compress grayscale images (PGM format) by exploiting spatial redundancy and uniformity in image blocks.

## How the QuadTree Works

L'image est traitée comme une structure récursive :

1. **Division récursive** : L'image carrée est divisée en 4 quadrants égaux. Ce processus se répète pour chaque quadrant jusqu'à ce qu'un seul pixel soit atteint ou que le bloc soit "uniforme".
2. **Représentation des nœuds** : Chaque nœud dans le QuadTree stocke :
   * `m` (Moyenne) : La valeur moyenne de gris du bloc.
   * `e` (Erreur) : Le reste de la somme des enfants divisée par 4 (utilisé pour la reconstruction parfaite/sans perte).
   * `u` (Uniformité) : Un indicateur indiquant si le bloc est une feuille (uniforme) ou divisé en quatre enfants.
3. **Compression sans perte vs compression avec perte** :
   * **Sans perte** : Un bloc est marqué comme uniforme uniquement si tous ses pixels sont strictement identiques. Les valeurs `m` et `e` permettent de calculer les valeurs originales sans erreur.
   * **Avec perte** : Ce mode utilise la variance du bloc. Si la variance est inférieure à un seuil défini par le paramètre `alpha`, le bloc est forcé à être "uniforme", économisant de l'espace mais la qualité de l'image est réduite.
4. **Format QTC** : Les données sont compressées au niveau des bits (Bitstream) pour minimiser la taille finale du fichier, en utilisant uniquement le nombre nécessaire de bits pour chaque champ.

## Features

- Supports compression and decompression of PGM images (binary format P5).
- Images must be square with dimensions \(2^n \times 2^n\).
- Compression can be lossless or lossy, depending on user options.
- The output is a custom `.qtc` format file.

## Usage
This is a standard 512x512 image in .png (Github doesn't display .pgm) from the PGM folder in black and white:

![Original Image](./results/original.png)

### Lossless Encoding
```sh
./codec -c -i ../../PGM/image.pgm -o output.qtc
```
This is the image in .png (Github doesn't display .pgm) after the lossless coding:

![Lossless Image](./results/lossless.png)

### Lossy Encoding
```sh
./codec -c -i ../../PGM/image.pgm -o output.qtc -a 3.0
```
This is the image in .png (Github doesn't display .pgm) after the lossy coding with alpha 3.0:  

![Lossy 3.0 Image](./results/lossy.png)
### Decoding
```sh
./codec -u -i output.qtc -o reconstructed.pgm
```

### Options

- `-c`: Encode (compress)
- `-u`: Decode (decompress)
- `-i <file>`: Input file (.pgm or .qtc)
- `-o <file>`: Output file (.qtc or .pgm)
- `-a <alpha>`: Lossy compression factor (e.g., 5.0). Default is 0.0 (lossless).
- `-g`: Generate segmentation grid
- `-h`: Help
- `-v`: Verbose mode (displays the QuadTree)

## File Formats

- Input: PGM (Portable Gray Map, binary P5)
- Output: QTC (custom QuadTree compressed format)

## Build Instructions

The project consists of a library (`libqtc`) and an application (`app`). To build:

1. **Build the library:**
```sh
cd Quadtree/libqtc
make clean && make
```

2. **Build the application:**
```sh
cd ../app
make clean && make
```

The `codec` executable will be generated in the `Quadtree/app` directory.