#!/usr/bin/env python

import random
from collections import defaultdict
import argparse

from pyhsfc import *

def playout(state):
    while not state.is_terminal():
        joint_move = random.choice(state.joints())
        state.play(joint_move)
    return state.goals()

def crudeMCPolicy(playouts, role, state):
    rewards = defaultdict(int) # mapping moves to cumulated rewards
    for joint_move in state.joints():
        for _ in range(playouts):
            copied_state = State(state)
            copied_state.play(joint_move)
            goals = playout(copied_state)
            rewards[joint_move[role]] += goals[role]
    bestMove = None
    bestRewa = -1
    for move, reward in rewards.items():
        if reward > bestRewa:
            bestMove = move
    assert bestMove in state.legals()[role]
    return bestMove

def randomPolicy(role, state):
    legals = state.legals()
    return random.choice(legals[role])
    
def playGame(parameter, testedRole, game):
    state = State(game)
    while not state.is_terminal():
        chosenMove = crudeMCPolicy(parameter, testedRole, state)
        legals = state.legals()
        jointMove = { role : randomPolicy(role, state) for role in game.players() if role != testedRole }
        jointMove[testedRole] = chosenMove
        state.play(jointMove)
    return state.goals()

def playMatch(parameter, nbGames, game):
    assert nbGames > 0
    scores = defaultdict(float)
    for _ in range(nbGames):
        for role in game.players():
            goals = playGame(parameter, role, game)
            scores[role] += goals[role]
            print "Crude Monte Carlo played as {0} and the outcome was {1}".format(role, goals)
    for role in scores:
        scores[role] = scores[role]/100/nbGames
    return scores

#-----------------------------
# main
#-----------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("playouts", type=int)
    parser.add_argument("nbGames", type=int)
    parser.add_argument("inputFile")
    args = parser.parse_args()
    
    game = Game(file=args.inputFile,gadelac=True)
    goals = playMatch(args.playouts, args.nbGames, game)
    print "The normalized reward for Crude Monte Carlo is {0}".format(goals)
    
if __name__ == '__main__':
    main()
                                                                                        
