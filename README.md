This is a line-for-line remake of niklasf's 'python-chess' in C++\
All credit for the original code and algorithms go to niklasf and his credits\
The original source code can be found here: https://github.com/niklasf/python-chess

cpp-chess: a chess library for C++
========================================

Introduction
------------

cpp-chess is a chess library for C++, with move generation,
move validation, and support for common formats. This is the Scholar's mate in
cpp-chess:

```cpp

    >>> #include "chess.cpp"
    >>> #include <iostream>

    >>> chess::Board board;

    >>> std::cout << board.legal_moves();  // doctest: +ELLIPSIS
    <LegalMoveGenerator at ... (Nh3, Nf3, Nc3, Na3, h3, g3, f3, e3, d3, c3, ...)>
    >>> chess::LegalMoveGenerator legal_moves = board.legal_moves();
    >>> std::cout << (std::find(std::begin(legal_moves), std::end(legal_moves), chess::Move::from_uci("a8a1")) != std::end(legal_moves));
    0

    >>> std::cout << board.push_san("e4");
    Move::from_uci("e2e4")
    >>> std::cout << board.push_san("e5");
    Move::from_uci("e7e5")
    >>> std::cout << board.push_san("Qh5");
    Move::from_uci("d1h5")
    >>> std::cout << board.push_san("Nc6");
    Move::from_uci("b8c6")
    >>> std::cout << board.push_san("Bc4");
    Move::from_uci("f1c4")
    >>> std::cout << board.push_san("Nf6");
    Move::from_uci("g8f6")
    >>> std::cout << board.push_san("Qxf7");
    Move::from_uci("h5f7")

    >>> std::cout << board.is_checkmate();
    1

    >>> std::cout << board;
    Board("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4")
```

Features
--------

* Supports C++ 20+.

* Make and unmake moves.

  ```cpp

      >>> chess::Move Nf3 = chess::Move::from_uci("g1f3");
      >>> board.push(Nf3);  // Make the move

      >>> std::cout << board.pop();  // Unmake the last move
      Move.from_uci("g1f3")
  ```

* Show a simple ASCII board.

  ```cpp

      >>> chess::Board board = chess::Board("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4");
      >>> std::cout << std::string(board);
      r . b q k b . r
      p p p p . Q p p
      . . n . . n . .
      . . . . p . . .
      . . B . P . . .
      . . . . . . . .
      P P P P . P P P
      R N B . K . N R
  ```

* Detects checkmates, stalemates and draws by insufficient material.

  ```cpp

      >>> std::cout << board.is_stalemate();
      0
      >>> std::cout << board.is_insufficient_material();
      0
      >>> std::cout << *board.outcome();
      Outcome(termination=Termination::CHECKMATE, winner=true)
  ```

* Detects repetitions. Has a half-move clock.

  ```cpp

      >>> std::cout << board.can_claim_threefold_repetition();
      0
      >>> std::cout << board.halfmove_clock;
      0
      >>> std::cout << board.can_claim_fifty_moves();
      0
      >>> std::cout << board.can_claim_draw();
      0
  ```

  With the new rules from July 2014, a game ends as a draw (even without a
  claim) once a fivefold repetition occurs or if there are 75 moves without
  a pawn push or capture. Other ways of ending a game take precedence.

  ```cpp

      >>> std::cout << board.is_fivefold_repetition();
      0
      >>> std::cout << board.is_seventyfive_moves();
      0
  ```

* Detects checks and attacks.

  ```cpp

      >>> std::cout << board.is_check();
      1
      >>> std::cout << board.is_attacked_by(chess::WHITE, chess::E8);
      1

      >>> chess::SquareSet attackers = board.attackers(chess::WHITE, chess::F3);
      >>> std::cout << attackers;
      SquareSet(0x0000'0000'0000'4040)
      >>> std::cout << (std::find(std::begin(attackers), std::end(attackers), chess::G2) != std::end(attackers));
      1
      >>> std::cout << std::string(attackers);
      . . . . . . . .
      . . . . . . . .
      . . . . . . . .
      . . . . . . . .
      . . . . . . . .
      . . . . . . . .
      . . . . . . 1 .
      . . . . . . 1 .
  ```

* Parses and creates SAN representation of moves.

  ```cpp

      >>> chess::Board board;
      >>> std::cout << board.san(chess::Move(chess::E2, chess::E4));
      e4
      >>> std::cout << board.parse_san("Nf3");
      Move::from_uci("g1f3")
      >>> std::cout << board.variation_san({chess::Move::from_uci("e2e4"), chess::Move::from_uci("e7e5"), chess::Move::from_uci("g1f3")});
      1. e4 e5 2. Nf3
  ```

* Parses and creates FENs, extended FENs and Shredder FENs.

  ```cpp

      >>> std::cout << board.fen();
      rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
      >>> std::cout << board.shredder_fen();
      rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w HAha - 0 1
      >>> chess::Board board = chess::Board("8/8/8/2k5/4K3/8/8/8 w - - 4 45");
      >>> std::cout << board.piece_at(chess::C5);
      Piece::from_symbol('k')
  ```

* Parses and creates EPDs.

  ```cpp

      >>> chess::Board board;
      >>> std::cout << board.epd(false, "legal", std::nullopt, {{"bm", board.parse_uci("d2d4")}});
      rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - bm d4;

      >>> auto ops = board.set_epd("1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - bm Qd1+; id \"BK.01\";");
      >>> std::cout << (std::get<std::vector<chess::Move>>(ops.at("bm")) == std::vector({chess::Move::from_uci("d6d1")}) && std::get<std::string>(ops.at("id")) == "BK.01");
      1
  ```
