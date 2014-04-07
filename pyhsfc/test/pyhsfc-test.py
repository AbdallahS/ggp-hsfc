#!/usr/bin/env python

import unittest
from pyhsfc import *

class TictactoeTest(unittest.TestCase):
    
    #-----------------------------
    # Run a playout from any (non-terminal) tictactoe
    # game state, and ensure that someone is either a 
    # winner or it is a 50-50 draw.
    #-----------------------------
    def playout_check(self, state):
        if state.IsTerminal(): return
        tmpstate = State(state)
        results = tmpstate.Playout()
        self.assertEqual(len(results),2)
        if results[0].goal == 100:
            self.assertEqual(results[1].goal,0)
        elif results[1].goal == 100:
            self.assertEqual(results[0].goal,0)
        else:
            self.assertEqual(results[0].goal,50)
            self.assertEqual(results[1].goal,50)

    #-----------------------------
    # Pick the first available move for the player
    #-----------------------------
    def pick_first_legal_move(self, moves, player):
        for m in moves:
            if str(m.player) == player:
                return m


    #-----------------------------
    # Count the legal moves for a given player
    #-----------------------------
    def count_legal_moves(self, moves, player):
        count = 0
        for m in moves:
            if str(m.player) == player:
                count = count + 1
        return count


    #-----------------------------
    # Load a GDL description
    #-----------------------------

    def test_load_gdldescription(self):
        gdldescription="""
;;;; RULES  

(role xplayer)
(role oplayer)
(init (cell 1 1 b))
(init (cell 1 2 b))
(init (cell 1 3 b))
(init (cell 2 1 b))
(init (cell 2 2 b))
(init (cell 2 3 b))
(init (cell 3 1 b))
(init (cell 3 2 b))
(init (cell 3 3 b))
(init (control xplayer))
(<= (next (cell ?m ?n x)) (does xplayer (mark ?m ?n)) (true (cell ?m ?n b)))
(<= (next (cell ?m ?n o)) (does oplayer (mark ?m ?n)) (true (cell ?m ?n b)))
(<= (next (cell ?m ?n ?w)) (true (cell ?m ?n ?w)) (distinct ?w b))
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?m ?j))
(<= (next (cell ?m ?n b)) (does ?w (mark ?j ?k)) (true (cell ?m ?n b)) (distinct ?n ?k))
(<= (next (control xplayer)) (true (control oplayer)))
(<= (next (control oplayer)) (true (control xplayer)))
(<= (row ?m ?x) (true (cell ?m 1 ?x)) (true (cell ?m 2 ?x)) (true (cell ?m 3 ?x)))
(<= (column ?n ?x) (true (cell 1 ?n ?x)) (true (cell 2 ?n ?x)) (true (cell 3 ?n ?x)))
(<= (diagonal ?x) (true (cell 1 1 ?x)) (true (cell 2 2 ?x)) (true (cell 3 3 ?x)))
(<= (diagonal ?x) (true (cell 1 3 ?x)) (true (cell 2 2 ?x)) (true (cell 3 1 ?x)))
(<= (line ?x) (row ?m ?x))
(<= (line ?x) (column ?m ?x))
(<= (line ?x) (diagonal ?x))
(<= open (true (cell ?m ?n b)))
(<= (legal ?w (mark ?x ?y)) (true (cell ?x ?y b)) (true (control ?w)))
(<= (legal xplayer noop) (true (control oplayer)))
(<= (legal oplayer noop) (true (control xplayer)))
(<= (goal xplayer 100) (line x))
(<= (goal xplayer 50) (not (line x)) (not (line o)) (not open))
(<= (goal xplayer 0) (line o))
(<= (goal oplayer 100) (line o))
(<= (goal oplayer 50) (not (line x)) (not (line o)) (not open))
(<= (goal oplayer 0) (line x))
(<= terminal (line x))
(<= terminal (line o))
(<= terminal (not open))

;;;; STRATS 

(strat does 0)
(strat goal 1)
(strat init 0)
(strat legal 0)
(strat next 0)
(strat role 0)
(strat terminal 1)
(strat true 0)
(strat cell 0)
(strat column 0)
(strat control 0)
(strat diagonal 0)
(strat line 0)
(strat open 0)
(strat row 0)

;;;; PATHS  

(arg does 0 oplayer)
(arg does 0 xplayer)
(arg does 1 mark 0 1)
(arg does 1 mark 0 2)
(arg does 1 mark 0 3)
(arg does 1 mark 1 1)
(arg does 1 mark 1 2)
(arg does 1 mark 1 3)
(arg does 1 noop)
(arg goal 0 oplayer)
(arg goal 0 xplayer)
(arg goal 1 0)
(arg goal 1 100)
(arg goal 1 50)
(arg init 0 cell 0 1)
(arg init 0 cell 0 2)
(arg init 0 cell 0 3)
(arg init 0 cell 1 1)
(arg init 0 cell 1 2)
(arg init 0 cell 1 3)
(arg init 0 cell 2 b)
(arg init 0 control 0 xplayer)
(arg legal 0 oplayer)
(arg legal 0 xplayer)
(arg legal 1 mark 0 1)
(arg legal 1 mark 0 2)
(arg legal 1 mark 0 3)
(arg legal 1 mark 1 1)
(arg legal 1 mark 1 2)
(arg legal 1 mark 1 3)
(arg legal 1 noop)
(arg next 0 cell 0 1)
(arg next 0 cell 0 2)
(arg next 0 cell 0 3)
(arg next 0 cell 1 1)
(arg next 0 cell 1 2)
(arg next 0 cell 1 3)
(arg next 0 cell 2 b)
(arg next 0 cell 2 o)
(arg next 0 cell 2 x)
(arg next 0 control 0 oplayer)
(arg next 0 control 0 xplayer)
(arg role 0 oplayer)
(arg role 0 xplayer)
(arg terminal )
(arg true 0 cell 0 1)
(arg true 0 cell 0 2)
(arg true 0 cell 0 3)
(arg true 0 cell 1 1)
(arg true 0 cell 1 2)
(arg true 0 cell 1 3)
(arg true 0 cell 2 b)
(arg true 0 cell 2 o)
(arg true 0 cell 2 x)
(arg true 0 control 0 oplayer)
(arg true 0 control 0 xplayer)
(arg column 0 1)
(arg column 0 2)
(arg column 0 3)
(arg column 1 b)
(arg column 1 o)
(arg column 1 x)
(arg diagonal 0 b)
(arg diagonal 0 o)
(arg diagonal 0 x)
(arg line 0 b)
(arg line 0 o)
(arg line 0 x)
(arg open )
(arg row 0 1)
(arg row 0 2)
(arg row 0 3)
(arg row 1 b)
(arg row 1 o)
(arg row 1 x)

;;;; DOMAINS

(domain_p does (set oplayer xplayer) (set mark noop))
(domain_p goal (set oplayer xplayer) (set 0 100 50))
(domain_p init (set cell control))
(domain_p legal (set oplayer xplayer) (set mark noop))
(domain_p next (set cell control))
(domain_p role (set oplayer xplayer))
(domain_p terminal )
(domain_p true (set cell control))
(domain_p column (set 1 2 3) (set b o x))
(domain_p diagonal (set b o x))
(domain_p line (set b o x))
(domain_p open )
(domain_p row (set 1 2 3) (set b o x))

(domain_s 0 )
(domain_s 1 )
(domain_s 100 )
(domain_s 2 )
(domain_s 3 )
(domain_s 50 )
(domain_s b )
(domain_s cell (set 1 2 3) (set 1 2 3) (set b o x))
(domain_s control (set oplayer xplayer))
(domain_s mark (set 1 2 3) (set 1 2 3))
(domain_s noop )
(domain_s o )
(domain_s oplayer )
(domain_s x )
(domain_s xplayer )
"""
        game = Game(gdl=gdldescription)
        self.assertEqual(game.NumPlayers(), 2)


    #-----------------------------
    # The main test - load up tictactoe and pick
    # moves until the game terminates while running
    # random playouts from each game state.
    #-----------------------------

    def test_tictactoe(self):
        game = Game(file="./tictactoe.gdl")
        self.assertEqual(game.NumPlayers(), 2)

        # Test that the different ways of create states work
        state = State(game)
        self.assertFalse(state.IsTerminal())
        state = State(game)
        self.assertFalse(state.IsTerminal())
        state2 = State(state)
        self.assertFalse(state2.IsTerminal())

        step=9
        turn=0
        while not state.IsTerminal():
            self.playout_check(state)
            legals = state.Legals()
            self.assertNotEqual(len(legals), 0)
            if turn == 0:
                self.assertEqual(self.count_legal_moves(legals, "xplayer"), step)
                self.assertEqual(self.count_legal_moves(legals, "oplayer"), 1)
            else:
                self.assertEqual(self.count_legal_moves(legals, "xplayer"), 1)
                self.assertEqual(self.count_legal_moves(legals, "oplayer"), step)

            movep0 = self.pick_first_legal_move(legals, "xplayer")
            movep1 = self.pick_first_legal_move(legals, "oplayer")
            state.Play((movep0,movep1))
            turn = (turn + 1) % 2
            step = step - 1
        
        # game has terminated so check for a valid result
	# Tictactoe will terminate early only if there is a winner
	# so test for this and also that a draw is 50/50.
        self.assertTrue(step < 5)
        results = state.Goals()
        self.assertEqual(len(results), 2)
        if step > 1:
            self.assertTrue(((results[0].goal == 100) and (results[1].goal == 0)) or 
                            ((results[1].goal == 100) and (results[0].goal == 0)))
        else:
            self.assertTrue(((results[0].goal == 100) and (results[1].goal == 0)) or 
                            ((results[1].goal == 100) and (results[0].goal == 0)) or
                            ((results[0].goal == 50) and (results[0].goal == 50)))

#-----------------------------
# main
#-----------------------------

def main():
    unittest.main()

if __name__ == '__main__':
    main()
