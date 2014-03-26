/*****************************************************************************************
 *
 *****************************************************************************************/
#include <sstream>
#include <vector>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>
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
	void operator()(const Term& t){ this->print(t); }

	std::ostream& print(const Term& t)
	{
		os_ << "(";
		unsigned int i = 1;
		BOOST_FOREACH(const TermNode& tn, t.children_)
		{
			boost::apply_visitor(*this, tn);
			if (i++ < t.children_.size()) os_ << " ";
		}
		return os_ << ")";
	}
};

struct print_flat : public boost::static_visitor<>
{
	std::ostream& os_;
	print_flat(std::ostream& os) : os_(os){}

	void operator()(const std::string& name){ os_ << name; }
	void operator()(const Term& t){ this->print(t); }

	std::ostream& print(const Term& t)
	{
		unsigned int i = 0; 
		unsigned int arity = t.children_.size()-1;
		BOOST_FOREACH(const TermNode& tn, t.children_)
		{
			boost::apply_visitor(*this, tn);
			if (i == 0 && arity > 0) os_ << "|" << arity;
			if (i < arity)  os_ << " ";
			++i;
		}
	}
};

std::ostream& generate_sexpr(const Term& term, std::ostream& os)
{
	print_sexpr ps(os);
	return ps.print(term);
}

std::ostream& generate_flat(const Term& term, std::ostream& os)
{
	print_flat pf(os);
	return pf.print(term);
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
	HSFC::Term,
	(std::vector<HSFC::TermNode>, children_)
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
	_qi::rule<Iterator, TermNode(), _ascii::space_type> substmt_;
	SimpleTermParser<Iterator> simpleterm_;

	SexprParser() : SexprParser::base_type(stmt_, "S-expression")
	{
		stmt_ %= '(' > *substmt_ > ')';
		substmt_ %= simpleterm_ | stmt_;
	}
};

 template<typename Iterator> 
void parse_sexpr(Iterator first, Iterator last, Term& result)
{
	Iterator begin = first;
	static SexprParser<Iterator> sexprparser;
	bool success = _qi::phrase_parse(first, last, sexprparser, _ascii::space, result);
	if (success && first == last) return;
	std::ostringstream ss;
	ss << "Failed to parse S-expression: ";
	while (begin != last) ss<< *begin++;
	throw HSFCException() << ErrorMsgInfo(ss.str());
}

void parse_sexpr(const std::string& sexpr, Term& term)
{
	parse_sexpr(sexpr.begin(), sexpr.end(), term);
}


/*****************************************************************
 * Parsing Flat format
 *****************************************************************/

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
bool parse_subflat(Iterator& first, Iterator last, TermNode& tn)
{
	LabelArity result;
	FlatSubParser<std::string::const_iterator> fsp;
	bool success = _qi::phrase_parse(first, last, fsp, _ascii::space, result);
	if (!success) return false;
	if (result.arity_ == 0) 
	{
		tn = result.label_;
		return true;
	}
	tn = Term();
	Term& term = boost::get<Term>(tn);
	term.children_.push_back(result.label_);
	for (int i=0; i < result.arity_; ++i)
	{
		term.children_.push_back(TermNode());
//		TermNode& = term.children_.back();
		if (!parse_subflat(first,last,term.children_.back())) return false;
	}
	return true;
}

template<typename Iterator>
bool parse_flat(Iterator& first, Iterator last, Term& term)
{
	LabelArity result;
	FlatSubParser<std::string::const_iterator> fsp;
	bool success = _qi::phrase_parse(first, last, fsp, _ascii::space, result);
	if (!success) return false;
	term.children_.push_back(result.label_);
	for (int i=0; i < result.arity_; ++i)
	{
		term.children_.push_back(TermNode());
		if (!parse_subflat(first,last,term.children_.back())) return false;
	}
	return true;
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
