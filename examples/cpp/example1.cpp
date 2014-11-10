/***************************************************************************
 * Example of the HSFC in action. From each state run a single playout
 * for each joint move. The one that has the best score for the given
 * player is selected.  This process is repeated until the game
 * terminates.
 *
 * NOTE: This example is out of date. The JointMoveGenerator class
 * used below is now unnesseary as the HSFC interface includes a
 * helper member function "HSFC::State::joints()" that returns the set
 * of joint moves.
 ****************************************************************************/

#include <iostream>
#include <vector>
#include <exception>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <hsfc/hsfc.h>

using namespace std;
using namespace HSFC;

typedef boost::error_info<struct tag_my_info,string> errinfo;
struct someerror: virtual boost::exception, virtual std::exception { };


/***************************************************************************
 * Helper class to iterate over joint moves
 **************************************************************************/
class JointMoveGenerator
{
    typedef boost::unordered_map<Player, vector<PlayerMove> > movemap_t;
    movemap_t playlist_;

    struct PlayerMovesTracker
    {
        vector<PlayerMove>::const_iterator begin_;
        vector<PlayerMove>::const_iterator end_;
        vector<PlayerMove>::const_iterator curr_;
        PlayerMovesTracker(vector<PlayerMove>::const_iterator begin,
                           vector<PlayerMove>::const_iterator end,
                           vector<PlayerMove>::const_iterator curr) :
            begin_(begin), end_(end), curr_(curr){ };
    };

    typedef boost::unordered_map<Player,PlayerMovesTracker> ptmap_t;
    ptmap_t ptmap_;
    bool done;

public:
    JointMoveGenerator(const vector<PlayerMove>& pmvs)  : done(false)
        {
            BOOST_FOREACH(const PlayerMove& pmv, pmvs)
            {
                playlist_[pmv.first].push_back(pmv);
            }

            BOOST_FOREACH(const movemap_t::value_type& plymvs, playlist_)
            {
                ptmap_.insert(make_pair(plymvs.first,
                                        PlayerMovesTracker(plymvs.second.begin(),
                                                           plymvs.second.end(),
                                                           plymvs.second.begin())));
            }
        }

    bool get(vector<PlayerMove>& pmvs)
        {
            if (done) return false;
            pmvs.clear();
            bool increment = true;
            BOOST_FOREACH(ptmap_t::value_type& ptpair, ptmap_)
            {
                pmvs.push_back(*(ptpair.second.curr_));
                if (increment)
                {
                    if (++ptpair.second.curr_ == ptpair.second.end_)
                    {
                        ptpair.second.curr_ = ptpair.second.end_;
                        increment = true;
                    }
                    else
                    {
                        increment = false;
                    }
                }
            }
            if (increment) done = true;
            return true;
        }
};

ostream& operator<<(ostream& os, const PlayerMove& pm)
{
    return os << "(" << pm.first << ", " << pm.second << ")";
}

ostream& operator<<(ostream& os, const vector<PlayerMove>& pmvs)
{
    os << "[";
    int count=0;
    BOOST_FOREACH(const PlayerMove& pm, pmvs)
    {
        os << pm;
        if (++count != pmvs.size()) os << ", ";
    }
    return os << "]";
}

/***************************************************************************
 * make sure player in valid for the game
 **************************************************************************/
Player get_player(const Game& game, const string& playername)
{
    vector<Player> players = game.players();
    BOOST_FOREACH(const Player& p, players)
    {
        if (p.tostring() == playername) return p;
    }
    BOOST_THROW_EXCEPTION(someerror() << errinfo("invalid player"));
}

/***************************************************************************
 * get the score for the given player
 **************************************************************************/
unsigned int get_score(const vector<PlayerGoal>& scores, const Player& player)
{
    BOOST_FOREACH(const PlayerGoal& pg, scores)
    {
        if (pg.first == player)
        {
            return pg.second;
        }
    }

    BOOST_THROW_EXCEPTION(someerror() << errinfo("player has no score"));
}

/***************************************************************************
 * choose the joint move that has the best playout score for the
 * player.  For each joint move a random playout is performed. The
 * playout that returns the best score for the player is chosen. The
 * actual goal value is returned.
 **************************************************************************/

unsigned int choose_joint_move(State& state, const Player& player,vector<PlayerMove>& bestjmv)
{
    int bestscore = -1;
    vector<PlayerMove> lgls;
    vector<PlayerMove> tmpmvs;
    state.legals(std::back_inserter(lgls));
    JointMoveGenerator jmg(lgls);
    while (jmg.get(tmpmvs))
    {
        vector<PlayerGoal> scores;
        State tmpstate(state);
        tmpstate.play(tmpmvs);
        if (tmpstate.isTerminal())
            tmpstate.goals(std::back_inserter(scores));
        else
            tmpstate.playout(scores);
        int tmpscore = get_score(scores, player);
        if (tmpscore > bestscore)
        {
            bestjmv = tmpmvs;
            bestscore = tmpscore;
        }
    }
    if (bestscore == -1)
        BOOST_THROW_EXCEPTION(someerror() << errinfo("No score for player"));
    return (unsigned int) bestscore;
}

/***************************************************************************
 * Run the game to termination
 **************************************************************************/


void run(const string& gdlfilename, const string& playername)
{
    Game game(boost::filesystem::path(gdlfilename.c_str()));

    Player player = get_player(game, playername);

    State state(game);
    while (!state.isTerminal())
    {
        vector<PlayerMove> jmv;
        unsigned int score = choose_joint_move(state, player, jmv);
        cout << "Player " << player << " score " << score << " => " << jmv << endl;
        state.play(jmv);
    }
    vector<PlayerGoal> pgs;
    state.goals(std::back_inserter(pgs));
    cout << "Final score for " << playername << " is " << get_score(pgs, player) << endl;
}

/***************************************************************************
 * Main
 **************************************************************************/

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            cout << "usage: <gdlfilename> <player>" << endl;
            exit(0);
        }
        string gdlfilename(argv[1]);
        string playername(argv[2]);
        run(gdlfilename, playername);
    }
    catch(someerror& e)
    {
        if( string const * mi=boost::get_error_info<errinfo>(e) )
            std::cerr << "Error: " << *mi;
    }
}
