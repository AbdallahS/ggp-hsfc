#-----------------------------------------------------
# A collection of playermoves. This can be iterated over
# to generate all combinations of joint moves.
#
# Builds a set of arrays of the moves for each player.
# eg. result[xplayer] = [(move 1 2), (move 3 2)]
#     result[oplayer] = [noop]

#-----------------------------------------------------

import collections

def legals(playermoves):
    result = collections.defaultdict(lambda: [])
    for pm in playermoves:
        result[pm.player].append(pm.move)
    return result

class JointMoves:
    def __init__(self, playermoves):
        self.playlist = {}
        self.currposn = {}
        self.maxposn = {}
        for playermove in playermoves:
            player = playermove.player
            if player in self.playlist:
                self.playlist[player].append(playermove)
            else:
                self.playlist[player] = [playermove]            
        for player, plymvs in self.playlist.iteritems():
            self.currposn[player] = 0
            self.maxposn[player] = len(plymvs)

    def __iter__(self):
        self.done = False
        for player in self.currposn.keys():
            self.currposn[player] = 0            
        return self
    
    def next(self):
        if self.done: 
            raise StopIteration
        jm = []
        increment = True
        for player in sorted(self.currposn.keys()):
            plymvs = self.playlist[player]
            jm.append(plymvs[self.currposn[player]])
            if increment:
                self.currposn[player] = self.currposn[player] + 1
                if self.currposn[player] == self.maxposn[player]:
                    self.currposn[player] = 0
                    increment = True
                else:
                    increment = False
        if increment:
            self.done = True        
        return jm
            
