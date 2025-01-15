# QtreeComp - Compression d'images par QuadTree

Ce projet a été développé dans le cadre d'un cours de programmation à l'Université Gustave Eiffel à Paris. L'implémentation a été réalisée par Gonzalo Torras et Mario Lepage.

## Aperçu du projet

Ce projet met en œuvre un schéma de compression d'images hiérarchique basé sur la structure de données QuadTree. L'objectif principal est de compresser des images en niveaux de gris (format PGM) en exploitant la redondance spatiale et l'uniformité des blocs d'images.

## Fonctionnalités

- Prise en charge de la compression et de la décompression d'images PGM (format binaire P5).
- Les images doivent être carrées, de dimensions \(2^n \times 2^n\).
- La compression peut être sans perte ou avec perte, selon les options choisies.
- La sortie est un fichier au format personnalisé `.qtc`.

## Utilisation

### Encodage (compression)
```sh
codec -c -i PGM/input.pgm -o QTC/output.qtc
```

### Décodage (décompression)
```sh
codec -u -i QTC/input.qtc -o PGM/output.pgm
```

### Options

- `-c` : Encoder (compresser)
- `-u` : Décoder (décompresser)
- `-i <fichier>` : Fichier d'entrée (.pgm ou .qtc)
- `-o <fichier>` : Fichier de sortie (.qtc ou .pgm)
- `-a <alpha>` : Paramètre de compression avec perte (pour l'encodage)
- `-g` : Générer la grille de segmentation (sortie PGM)
- `-h` : Aide
- `-v` : Mode verbeux

## Formats de fichiers

- Entrée : PGM (Portable Gray Map, binaire P5)
- Sortie : QTC (format compressé QuadTree personnalisé)

## Instructions de compilation

- Le projet est écrit en C standard.
- Utilisez le Makefile fourni pour construire la bibliothèque partagée (`libqtc.so`) et l'exécutable (`codec`).
- Tous les modules doivent pouvoir être compilés indépendamment.