# ar3d64s2.nnue

## Architecture
This net currently has an architecture of 768->32x2->1,
corresponding to an input layer size of 768 which is an input for every piece on every square (12 * 64 = 768),
a hidden layer size of 32 with perspective (different weights for the side to move and the opposing side) and an output layer size of 1.

## Data
This net was trained on 46.862.320 fens generated through self play with the builtin Datagen, utilizing the previous [net](../netBulletTest3-epoch40). 
From these generated fens the ones that have a capture as a reported best move were filtered out resulting in this collection of fens.
For shuffling the data the [CoreTrainer](https://github.com/SzilBalazs/CoreTrainer) by [SzilBalazs](https://github.com/SzilBalazs) was used.
All fens used for Training can be found in [Marlinformat](https://github.com/jnlt3/marlinflow#legacy-text-format) here: [Data](https://drive.google.com/file/d/1sPM0Y-idyWVhjPmtnzJhjK-zrOoxQFZk/view?usp=sharing).

## Trainer
This net was trained using the [bullet trainer](https://github.com/jw1912/bullet) by [jw1912](https://github.com/jw1912).  
The setting used for the training process are mostly the default ones from the readme, except changing the wdl from 0.5 to 1 and increasing the max Epoch to 50.
Additionally, as Molybdenum uses a h1 = 0 board representation, the squares were flipped vertically when training, so if you want to use this net in another engine keep in mind that you may have to flip the squares of the features.

