#!/usr/bin/env python

import re
import unittest
from pyhsfc import *

#-------------------------------------------------------------
#
#-------------------------------------------------------------
g_ttt1="""
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Tictactoe
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Components
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (role white)
    (role black)

    (<= (base (cell ?m ?n x)) (index ?m) (index ?n))
    (<= (base (cell ?m ?n o)) (index ?m) (index ?n))
    (<= (base (cell ?m ?n b)) (index ?m) (index ?n))
    (base (control white))
    (base (control black))

    (<= (input ?r (mark ?m ?n)) (role ?r) (index ?m) (index ?n))
    (<= (input ?r noop) (role ?r))

    (index 1)
    (index 2)
    (index 3)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; init
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (init (cell 1 1 b))
    (init (cell 1 2 b))
    (init (cell 1 3 b))
    (init (cell 2 1 b))
    (init (cell 2 2 b))
    (init (cell 2 3 b))
    (init (cell 3 1 b))
    (init (cell 3 2 b))
    (init (cell 3 3 b))
    (init (control white))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; legal
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (<= (legal ?w (mark ?x ?y))
        (true (cell ?x ?y b))
        (true (control ?w)))

    (<= (legal white noop)
        (true (control black)))

    (<= (legal black noop)
        (true (control white)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; next
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (<= (next (cell ?m ?n x))
        (does white (mark ?m ?n))
        (true (cell ?m ?n b)))

    (<= (next (cell ?m ?n o))
        (does black (mark ?m ?n))
        (true (cell ?m ?n b)))

    (<= (next (cell ?m ?n ?w))
        (true (cell ?m ?n ?w))
        (distinct ?w b))

    (<= (next (cell ?m ?n b))
        (does ?w (mark ?j ?k))
        (true (cell ?m ?n b))
        (distinct ?m ?j))

    (<= (next (cell ?m ?n b))
        (does ?w (mark ?j ?k))
        (true (cell ?m ?n b))
        (distinct ?n ?k))

    (<= (next (control white))
        (true (control black)))

    (<= (next (control black))
        (true (control white)))


    (<= (row ?m ?x)
        (true (cell ?m 1 ?x))
        (true (cell ?m 2 ?x))
        (true (cell ?m 3 ?x)))

    (<= (column ?n ?x)
        (true (cell 1 ?n ?x))
        (true (cell 2 ?n ?x))
        (true (cell 3 ?n ?x)))

    (<= (diagonal ?x)
        (true (cell 1 1 ?x))
        (true (cell 2 2 ?x))
        (true (cell 3 3 ?x)))

    (<= (diagonal ?x)
        (true (cell 1 3 ?x))
        (true (cell 2 2 ?x))
        (true (cell 3 1 ?x)))


    (<= (line ?x) (row ?m ?x))
    (<= (line ?x) (column ?m ?x))
    (<= (line ?x) (diagonal ?x))


    (<= open (true (cell ?m ?n b)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; goal
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    (<= (goal white 100)
        (line x)
        (not (line o)))

    (<= (goal white 50)
        (not (line x))
        (not (line o)))

    (<= (goal white 0)
        (not (line x))
        (line o))

    (<= (goal black 100)
        (not (line x))
        (line o))

    (<= (goal black 50)
        (not (line x))
        (not (line o)))

    (<= (goal black 0)
        (line x)
        (not (line o)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; terminal
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    (<= terminal
        (line x))

    (<= terminal
        (line o))

    (<= terminal
        (not open))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
"""


#----------------------------------------------------------
# Tictactoe that is generated by the Dresden game controller
#----------------------------------------------------------

g_ttt2="""(ROLE WHITE) (ROLE BLACK) (<= (BASE (CELL ?M ?N X)) (INDEX ?M) (INDEX ?N)) (<= (BASE (CELL ?M ?N O)) (INDEX ?M) (INDEX ?N)) (<= (BASE (CELL ?M ?N B)) (INDEX ?M) (INDEX ?N)) (BASE (CONTROL WHITE)) (BASE (CONTROL BLACK)) (<= (INPUT ?R (MARK ?M ?N)) (ROLE ?R) (INDEX ?M) (INDEX ?N)) (<= (INPUT ?R NOOP) (ROLE ?R)) (INDEX 1) (INDEX 2) (INDEX 3) (INIT (CELL 1 1 B)) (INIT (CELL 1 2 B)) (INIT (CELL 1 3 B)) (INIT (CELL 2 1 B)) (INIT (CELL 2 2 B)) (INIT (CELL 2 3 B)) (INIT (CELL 3 1 B)) (INIT (CELL 3 2 B)) (INIT (CELL 3 3 B)) (INIT (CONTROL WHITE)) (<= (LEGAL ?W (MARK ?X ?Y)) (TRUE (CELL ?X ?Y B)) (TRUE (CONTROL ?W))) (<= (LEGAL WHITE NOOP) (TRUE (CONTROL BLACK))) (<= (LEGAL BLACK NOOP) (TRUE (CONTROL WHITE))) (<= (NEXT (CELL ?M ?N X)) (DOES WHITE (MARK ?M ?N)) (TRUE (CELL ?M ?N B))) (<= (NEXT (CELL ?M ?N O)) (DOES BLACK (MARK ?M ?N)) (TRUE (CELL ?M ?N B))) (<= (NEXT (CELL ?M ?N ?W)) (TRUE (CELL ?M ?N ?W)) (DISTINCT ?W B)) (<= (NEXT (CELL ?M ?N B)) (DOES ?W (MARK ?J ?K)) (TRUE (CELL ?M ?N B)) (DISTINCT ?M ?J)) (<= (NEXT (CELL ?M ?N B)) (DOES ?W (MARK ?J ?K)) (TRUE (CELL ?M ?N B)) (DISTINCT ?N ?K)) (<= (NEXT (CONTROL WHITE)) (TRUE (CONTROL BLACK))) (<= (NEXT (CONTROL BLACK)) (TRUE (CONTROL WHITE))) (<= (ROW ?M ?X) (TRUE (CELL ?M 1 ?X)) (TRUE (CELL ?M 2 ?X)) (TRUE (CELL ?M 3 ?X))) (<= (COLUMN ?N ?X) (TRUE (CELL 1 ?N ?X)) (TRUE (CELL 2 ?N ?X)) (TRUE (CELL 3 ?N ?X))) (<= (DIAGONAL ?X) (TRUE (CELL 1 1 ?X)) (TRUE (CELL 2 2 ?X)) (TRUE (CELL 3 3 ?X))) (<= (DIAGONAL ?X) (TRUE (CELL 1 3 ?X)) (TRUE (CELL 2 2 ?X)) (TRUE (CELL 3 1 ?X))) (<= (LINE ?X) (ROW ?M ?X)) (<= (LINE ?X) (COLUMN ?M ?X)) (<= (LINE ?X) (DIAGONAL ?X)) (<= OPEN (TRUE (CELL ?M ?N B))) (<= (GOAL WHITE 100) (LINE X) (NOT (LINE O))) (<= (GOAL WHITE 50) (NOT (LINE X)) (NOT (LINE O))) (<= (GOAL WHITE 0) (NOT (LINE X)) (LINE O)) (<= (GOAL BLACK 100) (NOT (LINE X)) (LINE O)) (<= (GOAL BLACK 50) (NOT (LINE X)) (NOT (LINE O))) (<= (GOAL BLACK 0) (LINE X) (NOT (LINE O))) (<= TERMINAL (LINE X)) (<= TERMINAL (LINE O)) (<= TERMINAL (NOT OPEN))"""


#----------------------------------------------------------
# Tictactoe generated by the Dresden game controller but with line breaks added
#----------------------------------------------------------

g_ttt3="""
(ROLE WHITE)
(ROLE BLACK)

(<= (BASE (CELL ?M ?N X)) (INDEX ?M) (INDEX ?N))
(<= (BASE (CELL ?M ?N O)) (INDEX ?M) (INDEX ?N))
(<= (BASE (CELL ?M ?N B)) (INDEX ?M) (INDEX ?N))

(BASE (CONTROL WHITE))
(BASE (CONTROL BLACK))

(<= (INPUT ?R (MARK ?M ?N)) (ROLE ?R) (INDEX ?M) (INDEX ?N))
(<= (INPUT ?R NOOP) (ROLE ?R))

(INDEX 1)
(INDEX 2)
(INDEX 3)

(INIT (CELL 1 1 B))
(INIT (CELL 1 2 B))
(INIT (CELL 1 3 B))
(INIT (CELL 2 1 B))
(INIT (CELL 2 2 B))
(INIT (CELL 2 3 B))
(INIT (CELL 3 1 B))
(INIT (CELL 3 2 B))
(INIT (CELL 3 3 B))
(INIT (CONTROL WHITE))

(<= (LEGAL ?W (MARK ?X ?Y)) (TRUE (CELL ?X ?Y B)) (TRUE (CONTROL ?W)))
(<= (LEGAL WHITE NOOP) (TRUE (CONTROL BLACK)))
(<= (LEGAL BLACK NOOP) (TRUE (CONTROL WHITE)))

(<= (NEXT (CELL ?M ?N X)) (DOES WHITE (MARK ?M ?N)) (TRUE (CELL ?M ?N B)))
(<= (NEXT (CELL ?M ?N O)) (DOES BLACK (MARK ?M ?N)) (TRUE (CELL ?M ?N B)))
(<= (NEXT (CELL ?M ?N ?W)) (TRUE (CELL ?M ?N ?W)) (DISTINCT ?W B))
(<= (NEXT (CELL ?M ?N B)) (DOES ?W (MARK ?J ?K)) (TRUE (CELL ?M ?N B)) (DISTINCT ?M ?J))
(<= (NEXT (CELL ?M ?N B)) (DOES ?W (MARK ?J ?K)) (TRUE (CELL ?M ?N B)) (DISTINCT ?N ?K))
(<= (NEXT (CONTROL WHITE)) (TRUE (CONTROL BLACK)))
(<= (NEXT (CONTROL BLACK)) (TRUE (CONTROL WHITE)))

(<= (ROW ?M ?X) (TRUE (CELL ?M 1 ?X)) (TRUE (CELL ?M 2 ?X)) (TRUE (CELL ?M 3 ?X)))
(<= (COLUMN ?N ?X) (TRUE (CELL 1 ?N ?X)) (TRUE (CELL 2 ?N ?X)) (TRUE (CELL 3 ?N ?X)))
(<= (DIAGONAL ?X) (TRUE (CELL 1 1 ?X)) (TRUE (CELL 2 2 ?X)) (TRUE (CELL 3 3 ?X)))
(<= (DIAGONAL ?X) (TRUE (CELL 1 3 ?X)) (TRUE (CELL 2 2 ?X)) (TRUE (CELL 3 1 ?X)))

(<= (LINE ?X) (ROW ?M ?X))
(<= (LINE ?X) (COLUMN ?M ?X))
(<= (LINE ?X) (DIAGONAL ?X))

(<= OPEN (TRUE (CELL ?M ?N B)))

(<= (GOAL WHITE 100) (LINE X) (NOT (LINE O)))
(<= (GOAL WHITE 50) (NOT (LINE X)) (NOT (LINE O)))
(<= (GOAL WHITE 0) (NOT (LINE X)) (LINE O))
(<= (GOAL BLACK 100) (NOT (LINE X)) (LINE O))
(<= (GOAL BLACK 50) (NOT (LINE X)) (NOT (LINE O)))
(<= (GOAL BLACK 0) (LINE X) (NOT (LINE O)))

(<= TERMINAL (LINE X))
(<= TERMINAL (LINE O))
(<= TERMINAL (NOT OPEN))"""

#----------------------------------------------------------
#
#----------------------------------------------------------


def get_joint_move(state, player2move_dict):
    for jm in state.joints():
        jms = {str(p) : str(m) for (p,m) in jm.iteritems()}
        if all(item in jms.items() for item in player2move_dict.items()):
            return jm
    return None

#----------------------------------------------------------
#
#----------------------------------------------------------

class TictactoeTest(unittest.TestCase):

    #-----------------------------
    # Test tictactoe from stanford
    #-----------------------------
    def test_tictactoe1(self):
        global g_ttt1

        # Create game
        game = Game(gdl=g_ttt1)
        self.assertEqual(len(game.players()), 2)
        self.assertEqual(len(game.players()), game.num_players())
        white = next((r for r in game.players() if str(r) == "white"), None)
        black = next((r for r in game.players() if str(r) == "black"), None)
        self.assertTrue(white is not None)
        self.assertTrue(black is not None)

        # Track moves from the initial state to termination
        state = State(game)

        # Move 1
        jm = get_joint_move(state, {'white': '(mark 1 1)', 'black' : 'noop'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 2
        jm = get_joint_move(state, {'white': 'noop', 'black' : '(mark 2 1)'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 3
        jm = get_joint_move(state, {'white': '(mark 1 2)', 'black' : 'noop'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 4
        jm = get_joint_move(state, {'white': 'noop', 'black' : '(mark 2 2)'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 5
        jm = get_joint_move(state, {'white': '(mark 1 3)', 'black' : 'noop'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertTrue(state.is_terminal())

        # Test the gaol scores
        gs = state.goals()
        print "Score: {0}".format(gs)
        self.assertEqual(gs[white], 100)
        self.assertEqual(gs[black], 0)


    #-----------------------------
    # Test tictactoe as produced by the Dresden gamecontroller
    #-----------------------------
    def test_tictactoe2(self):
        global g_ttt2, g_ttt3

        # Create game
        game = Game(gdl=g_ttt3)
        self.assertEqual(len(game.players()), 2)
        self.assertEqual(len(game.players()), game.num_players())
        white = next((r for r in game.players() if str(r) == "WHITE"), None)
        black = next((r for r in game.players() if str(r) == "BLACK"), None)
        self.assertTrue(white is not None)
        self.assertTrue(black is not None)

        # Track moves from the initial state to termination
        state = State(game)

        # Move 1
        jm = get_joint_move(state, {'WHITE': '(MARK 1 1)', 'BLACK' : 'NOOP'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 2
        jm = get_joint_move(state, {'WHITE': 'NOOP', 'BLACK' : '(MARK 2 1)'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 3
        jm = get_joint_move(state, {'WHITE': '(MARK 1 2)', 'BLACK' : 'NOOP'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 4
        jm = get_joint_move(state, {'WHITE': 'NOOP', 'BLACK' : '(MARK 2 2)'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertFalse(state.is_terminal())

        # Move 5
        jm = get_joint_move(state, {'WHITE': '(MARK 1 3)', 'BLACK' : 'NOOP'})
        self.assertTrue(jm is not None)
        print "Playing: {0}".format(jm)
        state.play(jm)
        self.assertTrue(state.is_terminal())

        # Test the gaol scores
        gs = state.goals()
        print "Score: {0}".format(gs)
        self.assertEqual(gs[white], 100)
        self.assertEqual(gs[black], 0)

#-----------------------------
# main
#-----------------------------

def main():
    unittest.main()

if __name__ == '__main__':
    main()
