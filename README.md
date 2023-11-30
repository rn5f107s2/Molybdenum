# Molybdenum

**Molybdenum is a chess engine written in C++**.

## Usage
Molybdenum supports the [UCI-Protocol](https://wbec-ridderkerk.nl/html/UCIProtocol.html), 
the easiest way to use Molybdenum is to use a UCI compliant GUI such as [cutechess](https://github.com/cutechess/cutechess). 
However, if you wish you can use Molybdenum by directly communicating with it through the command line. Please note that incorrect inputs may lead to crashes or undefined behaviour,
therefore it's **not** recommended to do communicate via the commandline without prior knowledge of the UCI Protocol, but some important commands to know are:
```
    Position fen [FEN]
```
to set the current position to a give fen and
```
    go movetime [time for this move]
    go depth [depth]
    go nodes [nodes]
    go infinite
```
to search a position with the respective limits.
The output will be in the form of
```
    info depth [current depth] 
    currmove [the current move deemed the best] 
    score cp [the current evaluation in centipawns from the side to move perspective]
    nodes [the nodes searched]
    time [the time searched in milliseconds]
    nps [the nodes searched per second]
    pv [the line the engine deems to be the outcome with best play from both sides]
```
and after running out of the given search limit:
```
    bestmove [the move deemed to be the best one]
```
Something to note is that every move given as output will be in the form of
```
    [from square of the move][to square of the move]
```
Some additional commands that may be of use and are supported by Molybdenum but are **not** specified in the UCI Protocol
are:
```
    bench
```
this performs a fixed depth searched on a few positions,
```
    d
```
this prints the current board state as well as a few additional informations and
```
    eval
```
this prints the current evaluation from the perspective of the side to move.

## Evaluation
The currently used evaluation function is Piece-Square-Tables (PSQTs) with Tempo.
The eval function was tuned with the [lichess-big3-resolved Dataset](https://cdn.discordapp.com/attachments/936829036104142848/1014793028042510346/lichess-big3-resolved.7z)
using a slightly modified version of this [Texel-Tuner implementation](https://github.com/GediminasMasaitis/texel-tuner).

## Credits

During my journey creating this there were a lot of people / ressources that helped me along the way, notably
* ### [The Stockfish Project](https://github.com/rn5f107s2/Stockfish)
    I learned a lot through the Stockfish project, 
    mainly through the fishtest framework where I tested some ideas, greatly helping my understanding of chess engines and
    the people on the [Discord Server](https://discord.gg/KSmxy3eKED), helping with explaining why and how some things work.
* ### [The Enigne Programming Discord Server](https://discord.gg/PMeBDB8N)
    This discord server is great server to lurk in, where I learned a lot of things, 
    through either reading people explain things, or the resources listed in this Server.
* ### Other Open-source Engines 
  Notable ones being
  * [Mess](https://github.com/raklaptudirm/mess), even though mess is not too strong of an engine the well sorted release notes helped me orient myself on what features I could try implementing.
   Some Implementations are also inspired by mess, for example the Late Move-/ Move Count Pruning implementation.
  
  * [Stormphrax](https://github.com/Ciekce/Stormphrax) was never an engine I really read the source code of, however I would like to mention it as the author [Ciekce](https://github.com/Ciekce)
    is great at explaining things to people which is something I learned a lot from.