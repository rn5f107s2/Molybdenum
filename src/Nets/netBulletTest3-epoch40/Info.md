# netBulletTest3-epoch40.bin

## Architecture
This net currently has an architecture of 768->32x2->1, 
corresponding to an input layer size of 768 which is an input for every piece on every square (12 * 64 = 768), 
a hidden layer size of 32 with perspective (different weights for the side to move and the opposing side) and an output layer size of 1. 

## Data
This net was trained on 29.005.529 fens generated through self play with the builtin Datagen, utilizing the PSQT only eval that was used before the introduction of NNUE.
Some parts of the data used an older version of the Datagen Module, so Adjudication settings may not always be consistent.
From these generated fens the ones that have a capture as a reported best move were filtered out resulting in this collection of fens. 
For shuffling the data the [CoreTrainer](https://github.com/SzilBalazs/CoreTrainer) by [SzilBalazs](https://github.com/SzilBalazs) was used. 
All fens used for Training can be found in [Marlinformat](https://github.com/jnlt3/marlinflow#legacy-text-format) here: [Data](https://drive.google.com/file/d/16OkUqBoVu6dW4956hmqDzwr8DI6Gm0dP/view?usp=sharing).

## Trainer 
This net was trained using the [bullet trainer](https://github.com/jw1912/bullet) by [jw1912](https://github.com/jw1912).  
The setting used for the training process are mostly the default ones from the readme, except changing the wdl from 0.5 to 1.

