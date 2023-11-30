# Molybdenum

**Molybdenum is a chess engine written in C++**.

## Usage
Molybdenum supports the [UCI-Protocol](https://wbec-ridderkerk.nl/html/UCIProtocol.html), 
the easiest way to use Molybdenum is to use a UCI compliant GUI such as [cutechess](https://github.com/cutechess/cutechess). 
However, if you wish you can use Molybdenum by directly communicating with it through the command line. Please incorrect inputs may lead to crashes or undefined behaviour,
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
Something to note is that every move given as output will be in the from of
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