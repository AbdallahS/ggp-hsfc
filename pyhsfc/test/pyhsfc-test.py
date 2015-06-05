#!/usr/bin/env python

import tempfile
import unittest
from pyhsfc import *

#-------------------------------------------------------------
#
#-------------------------------------------------------------
g_ttt="""
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

(arg does/2 0 oplayer/0)
(arg does/2 0 xplayer/0)
(arg does/2 1 mark/2 0 1/0)
(arg does/2 1 mark/2 0 2/0)
(arg does/2 1 mark/2 0 3/0)
(arg does/2 1 mark/2 1 1/0)
(arg does/2 1 mark/2 1 2/0)
(arg does/2 1 mark/2 1 3/0)
(arg does/2 1 noop/0)
(arg goal/2 0 oplayer/0)
(arg goal/2 0 xplayer/0)
(arg goal/2 1 0/0)
(arg goal/2 1 100/0)
(arg goal/2 1 50/0)
(arg init/1 0 cell/3 0 1/0)
(arg init/1 0 cell/3 0 2/0)
(arg init/1 0 cell/3 0 3/0)
(arg init/1 0 cell/3 1 1/0)
(arg init/1 0 cell/3 1 2/0)
(arg init/1 0 cell/3 1 3/0)
(arg init/1 0 cell/3 2 b/0)
(arg init/1 0 control/1 0 xplayer/0)
(arg legal/2 0 oplayer/0)
(arg legal/2 0 xplayer/0)
(arg legal/2 1 mark/2 0 1/0)
(arg legal/2 1 mark/2 0 2/0)
(arg legal/2 1 mark/2 0 3/0)
(arg legal/2 1 mark/2 1 1/0)
(arg legal/2 1 mark/2 1 2/0)
(arg legal/2 1 mark/2 1 3/0)
(arg legal/2 1 noop/0)
(arg next/1 0 cell/3 0 1/0)
(arg next/1 0 cell/3 0 2/0)
(arg next/1 0 cell/3 0 3/0)
(arg next/1 0 cell/3 1 1/0)
(arg next/1 0 cell/3 1 2/0)
(arg next/1 0 cell/3 1 3/0)
(arg next/1 0 cell/3 2 b/0)
(arg next/1 0 cell/3 2 o/0)
(arg next/1 0 cell/3 2 x/0)
(arg next/1 0 control/1 0 oplayer/0)
(arg next/1 0 control/1 0 xplayer/0)
(arg role/1 0 oplayer/0)
(arg role/1 0 xplayer/0)
(arg terminal/0)
(arg true/1 0 cell/3 0 1/0)
(arg true/1 0 cell/3 0 2/0)
(arg true/1 0 cell/3 0 3/0)
(arg true/1 0 cell/3 1 1/0)
(arg true/1 0 cell/3 1 2/0)
(arg true/1 0 cell/3 1 3/0)
(arg true/1 0 cell/3 2 b/0)
(arg true/1 0 cell/3 2 o/0)
(arg true/1 0 cell/3 2 x/0)
(arg true/1 0 control/1 0 oplayer/0)
(arg true/1 0 control/1 0 xplayer/0)
(arg column/2 0 1/0)
(arg column/2 0 2/0)
(arg column/2 0 3/0)
(arg column/2 1 b/0)
(arg column/2 1 o/0)
(arg column/2 1 x/0)
(arg diagonal/1 0 b/0)
(arg diagonal/1 0 o/0)
(arg diagonal/1 0 x/0)
(arg line/1 0 b/0)
(arg line/1 0 o/0)
(arg line/1 0 x/0)
(arg open/0)
(arg row/2 0 1/0)
(arg row/2 0 2/0)
(arg row/2 0 3/0)
(arg row/2 1 b/0)
(arg row/2 1 o/0)
(arg row/2 1 x/0)
"""

#----------------------------------------------------------
#
#----------------------------------------------------------

class TictactoeTest(unittest.TestCase):

    #-----------------------------
    # Run a playout from any (non-terminal) tictactoe
    # game state, and ensure that someone is either a
    # winner or it is a 50-50 draw.
    #-----------------------------
    def playout_check(self, state, xplayer, oplayer):
        if state.is_terminal(): return
        tmpstate = State(state)
        results = tmpstate.playout()
        self.assertEqual(len(results),2)
        if results[xplayer] == 100:
            self.assertEqual(results[oplayer], 0)
        elif results[xplayer] == 0:
            self.assertEqual(results[oplayer], 100)
        else:
            self.assertEqual(results[xplayer], 50)
            self.assertEqual(results[oplayer], 50)


    #-----------------------------
    # check that the legal moves match the joint moves
    #-----------------------------
    def check_joints_match_legals(self, joints, legals):
        for jm in joints:
            for (p,m) in jm.iteritems():
                mvs = legals[p]
                self.assertTrue(m in mvs)

    #-----------------------------
    # Load a GDL description
    #-----------------------------

    def atest_load_gdldescription(self):
        global g_ttt
        game = Game(gdl=g_ttt)
        players = game.Players()
        self.assertEqual(len(players), 2)


    #-----------------------------
    # Exception checks
    #-----------------------------
    def atest_exceptions(self):
        self.assertRaises(ValueError, Game, file="./nofile.gdl")
        self.assertRaises(ValueError, Game, gdl="( dfdfl (dfd", gadelac=True)

    #-----------------------------
    # The main test: load up tictactoe and pick
    # moves until the game terminates while running
    # random playouts from each game state.
    #-----------------------------

    def atest_tictactoe(self):
        global g_ttt
        tf = tempfile.NamedTemporaryFile()
        tf.write(g_ttt)

        game = Game(gdl=g_ttt)
#        game = Game(file=tf.name)
        self.assertEqual(len(game.players()), 2)
        self.assertEqual(len(game.players()), game.num_players())

        xplayer = next(r for r in game.players() if str(r) == "xplayer")
        oplayer = next(r for r in game.players() if str(r) == "oplayer")

        # Test that the different ways of create states work
        state = State(game)
        self.assertFalse(state.is_terminal())
        state2 = State(state)
        self.assertFalse(state2.is_terminal())

        step=9
        turn=0
        while not state.is_terminal():
            self.playout_check(state, xplayer, oplayer)
            legals = state.legals()
            self.assertEqual(len(legals), 2)
            joints = state.joints()
            self.assertEqual(len(joints), step)
            self.check_joints_match_legals(joints, legals)

            if turn == 0:
                self.assertEqual(len(legals[xplayer]), step)
                self.assertEqual(len(legals[oplayer]), 1)
            else:
                self.assertEqual(len(legals[xplayer]), 1)
                self.assertEqual(len(legals[oplayer]), step)

            movep0 = legals[xplayer][0]
            movep1 = legals[oplayer][0]

            # The State.play() function can take two forms: a list of player-move pairs,
            # or a dictionary of players to moves. Alternate testing both these.
            if turn == 0:
                state.play([(xplayer, movep0), (oplayer, movep1)])
            else:
                state.play({xplayer : movep0, oplayer : movep1})
            turn = (turn + 1) % 2
            step = step - 1

        # game has terminated so check for a valid result
    # Tictactoe will terminate early only if there is a winner
    # so test for this and also that a draw is 50/50.
        self.assertTrue(step < 5)
        results = state.goals()
        self.assertEqual(len(results), 2)
        if step > 1:
            self.assertTrue(((results[xplayer] == 100) and (results[oplayer] == 0)) or
                            ((results[oplayer] == 100) and (results[xplayer] == 0)))
        else:
            self.assertTrue(((results[xplayer] == 100) and (results[oplayer] == 0)) or
                            ((results[oplayer] == 100) and (results[xplayer] == 0)) or
                            ((results[xplayer] == 50) and (results[oplayer] == 50)))



    #-----------------------------
    # Some PortableState tests on tictactoe
    #-----------------------------
    def find_jm(self, state, player, x, y):
        return next(jm for jm in state.joints() if str(jm[player]) == "(mark {} {})".format(x,y))

    def test_tictactoe(self):
        global g_ttt
        print "HERE1"
        game = Game(gdl=g_ttt)
        print "HERE2"
        self.assertEqual(len(game.players()), 2)
        self.assertEqual(len(game.players()), game.num_players())

        xplayer = next(r for r in game.players() if str(r) == "xplayer")
        oplayer = next(r for r in game.players() if str(r) == "oplayer")
        state1 = State(game)
        state2 = State(game)
        state3 = State(game)

        state1.play(self.find_jm(state1, xplayer, 1, 1))
        state1.play(self.find_jm(state1, oplayer, 3, 3))
        state1.play(self.find_jm(state1, xplayer, 3, 1))
        state1.play(self.find_jm(state1, oplayer, 1, 3))

        state2.play(self.find_jm(state2, xplayer, 3, 1))
        state2.play(self.find_jm(state2, oplayer, 1, 3))
        state2.play(self.find_jm(state2, xplayer, 1, 1))
        state2.play(self.find_jm(state2, oplayer, 3, 3))

        state3.play(self.find_jm(state3, xplayer, 3, 1))
        state3.play(self.find_jm(state3, oplayer, 1, 3))
        state3.play(self.find_jm(state3, xplayer, 1, 1))

        pstate1 = PortableState(state1)
        pstate2 = PortableState(state2)
        pstate3 = PortableState(state3)

        self.assertEqual(pstate1.__hash__(), pstate2.__hash__())
        self.assertTrue(pstate1 == pstate2)
        self.assertTrue(pstate1 != pstate3)
        self.assertFalse(pstate1 == pstate3)
        self.assertFalse(pstate1 != pstate2)


#-----------------------------
# main
#-----------------------------

def main():
    unittest.main()

if __name__ == '__main__':
    main()
