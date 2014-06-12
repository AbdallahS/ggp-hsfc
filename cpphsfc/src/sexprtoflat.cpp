/*****************************************************************************************
 *
 *****************************************************************************************/
#include <sstream>
#include <algorithm>
#include <vector>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <hsfc/hsfc.h>     /* Only needed for HSFCException */


#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "sexprtoflat.h"

namespace HSFC
{

/*****************************************************************
 * Because the output format is pretty simple I'll use manual
 * generation instead of Spirit Karma.
 *****************************************************************/

struct print_sexpr : public boost::static_visitor<>
{
	std::ostream& os_;
	print_sexpr(std::ostream& os) : os_(os){}

	void operator()(const std::string& name){ os_ << name; }
	void operator()(const SubTerms& ts){ this->print(ts); }
//	void operator()(const Term& t){ this->print(t); }

	std::ostream& print(const SubTerms& ts)
	{
		os_ << "(";
		unsigned int i = 1;
		BOOST_FOREACH(const Term& t, ts.children_)
		{
			boost::apply_visitor(*this, t);
			if (i++ < ts.children_.size()) os_ << " ";
		}
		return os_ << ")";
	}
};

struct print_flat : public boost::static_visitor<>
{
	std::ostream& os_;
	print_flat(std::ostream& os) : os_(os){}

	void operator()(const std::string& name){ os_ << name; }
	void operator()(const SubTerms& ts){ this->print(ts); }

	std::ostream& print(const SubTerms& ts)
	{
		unsigned int i = 0; 
		unsigned int arity = ts.children_.size()-1;
		BOOST_FOREACH(const Term& t, ts.children_)
		{
			boost::apply_visitor(*this, t);
			if (i == 0 && arity > 0) os_ << "|" << arity;
			if (i < arity)  os_ << " ";
			++i;
		}
	}
};

std::ostream& generate_sexpr(const Term& term, std::ostream& os)
{
	print_sexpr ps(os);
    boost::apply_visitor(ps, term);
	return os;
}

std::ostream& generate_flat(const Term& term, std::ostream& os)
{
	print_flat pf(os);
    boost::apply_visitor(pf, term);
	return os;
}

std::string generate_sexpr(const Term& term)
{
    std::ostringstream ss;
    generate_sexpr(term, ss);
    return ss.str();
}

std::string generate_flat(const Term& term)
{
    std::ostringstream ss;
    generate_flat(term, ss);
    return ss.str();
}


/**********************************************************************
 * Helper struct for parsing flat format (eg., "label|1")
 **********************************************************************/
struct LabelArity
{
	std::string label_;
	unsigned int arity_;
};

};
/**********************************************************************
 * The boost fusion adaptor allows the Term data structure to be
 * seemlessly used in the spirit parser. See the spirit documentation.
 * NOTE: it has to be defined in the global namespace.
 **********************************************************************
 */

BOOST_FUSION_ADAPT_STRUCT(
	HSFC::SubTerms,
	(std::vector<HSFC::Term>, children_)
	)

BOOST_FUSION_ADAPT_STRUCT(
	HSFC::LabelArity,
	(std::string, label_)
	(unsigned int, arity_)
	)

namespace HSFC
{
/*****************************************************************
 * Boost spirit namespace aliases
 *****************************************************************/

namespace _spirit = boost::spirit;
namespace _qi = boost::spirit::qi;
namespace _ascii = boost::spirit::ascii;
namespace _phoenix = boost::phoenix;

/*****************************************************************
 * Spirit grammars
 * SimpleTerm - a sequence of characters other than "(", ")", "|", or whitespace.
 *****************************************************************/

template<typename Iterator>
struct SimpleTermParser : public _qi::grammar<Iterator, std::string()>
{
	_qi::rule<Iterator, std::string()> simple_;

	SimpleTermParser() : SimpleTermParser::base_type(simple_, "Simple term")
	{		
		simple_ %= _qi::lexeme[+(_qi::char_ - (_qi::char_("|()") | _qi::space))];
	}
};

/*****************************************************************
 * S-expression parsing
 *****************************************************************/
template<typename Iterator>
struct SexprParser : public _qi::grammar<Iterator, Term(),  _ascii::space_type>
{
	_qi::rule<Iterator, Term(), _ascii::space_type> stmt_;
	_qi::rule<Iterator, SubTerms(), _ascii::space_type> cmplxstmt_;
	SimpleTermParser<Iterator> simpleterm_;

	SexprParser() : SexprParser::base_type(stmt_, "S-expression")
	{
        stmt_ %= simpleterm_ | cmplxstmt_;
        cmplxstmt_ %= '(' > *stmt_ > ')';
        
	}
};

 template<typename Iterator> 
void parse_sexpr(Iterator first, Iterator last, Term& result)
{
	Iterator begin = first;
	static SexprParser<Iterator> sexpparser;
    bool success;
    try 
    {
        success = _qi::phrase_parse(first, last, sexpparser, _ascii::space, result);
    } catch (std::exception& e)
    {
        throw std::invalid_argument(e.what());
    }
	if (success && first == last) return;
	std::ostringstream ss;
	ss << "Failed to parse S-expression: ";
	while (begin != last) ss<< *begin++;
	throw HSFCException() << ErrorMsgInfo(ss.str());
//	throw std::invalid_argument(ss.str());
}

void parse_sexpr(const std::string& sexpr, Term& term)
{
	parse_sexpr(sexpr.begin(), sexpr.end(), term);
}


/*****************************************************************
 * Parsing Flat format
 *****************************************************************/

struct FlatStack
{
    typedef boost::tuple<SubTerms&, int> element_t;
    std::vector<element_t> stack_;

    Term& t_;
    bool first_;
    FlatStack(Term& t) : t_(t), first_(true){}
    void operator()(const LabelArity& la);
};

void FlatStack::operator()(const LabelArity& la)
{
    if (stack_.empty() && !first_) 
        throw HSFCException() << ErrorMsgInfo("Parsing error");
    first_ = false;
    if (stack_.empty())
    {        
        if (la.arity_ == 0){ t_ = Term(la.label_); }
        else
        {
            t_ = Term(SubTerms());
            SubTerms& subterms = boost::get<SubTerms>(t_);
            subterms.children_.push_back(Term(la.label_));
            stack_.push_back(element_t(subterms, la.arity_+1));
        }
        return;
    }
    element_t& top = stack_.back();
    SubTerms& subterms = top.get<0>();
    int& st_size = top.get<1>();

    if (la.arity_ == 0) // Adding a 0 arity term
    {
        subterms.children_.push_back(Term(la.label_));

        // Unwind the stack as much as possible
        for(;;)
        {
            if (stack_.empty()) break;
            const element_t& tmp_top = stack_.back();
            const SubTerms& tmp_subterms = tmp_top.get<0>();
            int tmp_st_size = tmp_top.get<1>();            
            if (tmp_subterms.children_.size() < tmp_st_size) break;                
            stack_.pop_back();
        }
    }
    else       // Non 0-arity term so need to add a new subterm
    {
        subterms.children_.push_back(Term(SubTerms()));
        SubTerms& subsubterms = boost::get<SubTerms>(subterms.children_.back());
        subsubterms.children_.push_back(std::string(la.label_));
        stack_.push_back(element_t(subsubterms, la.arity_+1));
    }
}


template<typename Iterator>
struct FlatSubParser : public _qi::grammar<Iterator, LabelArity(),  _ascii::space_type>
{
	_qi::rule<Iterator, LabelArity(), _ascii::space_type> stmt_;
	SimpleTermParser<Iterator> simpleterm_;
	_qi::rule<Iterator, unsigned int()> arity_;

	FlatSubParser() : FlatSubParser::base_type(stmt_, "flat sub-expression")
	{
		stmt_ %= simpleterm_ >> arity_;
		arity_ %= ( '|' >> _qi::uint_ )  | _qi::attr(0);
	}
};

template<typename Iterator>
bool parse_flat(Iterator& first, Iterator last, Term& term)
{
    try
    {        
        std::vector<LabelArity> labels;
        FlatSubParser<Iterator> fsp;
        bool success = 
            _qi::phrase_parse(first, last, 
                              +(fsp[_phoenix::push_back(_phoenix::ref(labels), _qi::_1)])
                              , _ascii::space);
        if (!success) return false;
        FlatStack fs(term);
        std::for_each(labels.begin(), labels.end(), fs);
        return true;
    } catch (HSFCException)
    {
        return false; 
    }
}

void parse_flat(const std::string& flat, Term& term)
{
	std::stringstream ss;
	std::string::const_iterator first = flat.begin();
	std::string::const_iterator begin = flat.begin();
	std::string::const_iterator last = flat.end();
	bool success = parse_flat(first, last, term);
	if (!success || first != last)
	{
		ss << "Failed to parse S-expression: ";
		while (begin != last) ss<< *begin++;
		throw HSFCException() << ErrorMsgInfo(ss.str());
	}		
}


/*****************************************************************
 * Will throw HSFCException if the strings include disallow chars.
 *****************************************************************/

std::string sexpr_to_flat(const std::string& sexpr)
{
	std::stringstream ss;
	Term term;
	parse_sexpr(sexpr, term);
	generate_flat(term, ss);
	return ss.str();
}

std::string flat_to_sexpr(const std::string& flat)
{
	std::stringstream ss;
	Term term;
	parse_flat(flat, term);
	generate_sexpr(term, ss);
	return ss.str();
}

}
